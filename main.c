/*
Description: This uses /dev/ttytarget to get the IP address of an
serial target if that device hosts a linux shell.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<unistd.h>

#include "app.h"
#include "target.h"
#include "user.h"

char file_buffer[BUFFERMAX];
char *p_buf;
int state = -1;
FILE *fptr;

uint8_t main(void) {

    int fd = target_open();
    if(fd == -1)
    {
        DEBUG_INFO("Quitting\n");
        return -1;
    }

    target_configure(&fd);

    target_clear_txrx_buffers(&fd);

    target_write(&fd, PING);

    for(int i = 0; i < LOGIN_ATTEMPTS; i++)
    {
        state = target_shell_state(&fd);

        switch(state)
        {
            case 0:
                // The login prompt contains some useless data following
                // reboot, so this either needs to be deleted or it needs
                // to be accepted that initial login will fail.
                DEBUG_INFO("login prompt found\n");
                target_write(&fd, USER);
                break;
            case 1:
                DEBUG_INFO("password prompt found\n");
                target_write(&fd, PW);
                // Need to wait for a while after entering password.
                sleep(5);
                break;
            case 2:
                DEBUG_INFO("Shell logged in\n");
                p_buf = target_save_ip(&fd);
                DEBUG_INFO("Buffer contents: %s \n", p_buf);
                
                fptr = fopen("target_usb0_ip","w");
                fwrite(p_buf, sizeof(char), strlen(p_buf), fptr);
                fclose(fptr);

                // Read the public key for the node into file_buffer
                user_get_pub_key(file_buffer);

                target_add_key(&fd, file_buffer);
                
                i = LOGIN_ATTEMPTS;
                break;
            default:
                DEBUG_INFO("no matches found\n");
        }
    }
    target_close(&fd);
}