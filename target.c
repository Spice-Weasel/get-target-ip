#include<stdio.h>
#include<termios.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>

#include"app.h"
#include"target.h"

const char device[] = "/dev/ttytarget";

const char *shell_states[] = {
    "login:",
    "Password:",
    "user@target",
    "EOF"
};

char buf[BUFFERMAX];

// Open and jet the device and check that it is a tty file
int target_open() {
    int fd;

    fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(fd == -1) {
        printf("failed to open %s\n", device);
        return fd;
    }

    if(!isatty(fd)){
        printf("device %s is not a tty\n", device);
    }
    return fd;
}

// uses termios to configure the tty for the target
int target_configure(int *fd){

    struct termios config;

    if(tcgetattr(*fd, &config) < 0) {
        printf("Could not get tty config\n");
    }

    config.c_lflag = 0;
    config.c_iflag &= ~(IXON | IXOFF | IXANY);
    config.c_cflag |= (CLOCAL | CREAD);

    config.c_oflag = 0;
    
    config.c_cflag &= ~CSIZE;
    config.c_cflag |= CS8; // 8 bit character size
    config.c_cflag &= ~(PARENB | CSTOPB);  // disable parity checking, one stop-bit

    config.c_cc[VMIN]= 1;
    config.c_cc[VTIME] = 0;

    cfsetispeed(&config, B115200);
    cfsetospeed(&config, B115200);

    return tcsetattr(*fd, TCSAFLUSH, &config);
}

// send a linefeed to the target
int target_ping(int *fd) {
    return write(*fd, "\n", 1);
}

int target_write(int *fd, char *command)
{
    return write(*fd, command, strlen(command));
}

int target_clear_txrx_buffers(int *fd) {
    int error;
    error = tcflush(*fd, TCIOFLUSH);
    printf("flushing buffers code : %d\n", error);
    return error;
}

char* target_save_ip(int *fd) {

    if(target_write(fd, GET_IP) < 0)
    {
        printf("target write %s error\n", GET_IP);
        return 0;
    }

    memset(buf, '\0', sizeof(buf));

    usleep(100000);

    if(read(*fd, buf, sizeof(buf)) < 0)
    {
        printf("target read IP Failed\n");
        return 0;
    }

    printf("Acquired Data\n%s", buf);

    return buf;
}

//TODO Why doesn't this work first time?
int target_add_key(int *fd, char *pkey) {
    // pointer to key
    int command_length = 0;
    char *pcommand;

    const char *prefix = "echo \"";
    const char *suffix = "\" >> .ssh/authorized_keys";

    printf("Pub Key is : %s\n", pkey);
    command_length = strlen(prefix) + strlen(pkey) + strlen(suffix);

    // Allocating memory for string
    pcommand = (char *)malloc(command_length);

    strcpy(pcommand, prefix);
    strcpy(pcommand + strlen(prefix), pkey);
    strcpy(pcommand + strlen(prefix) + (strlen(pkey) - 1), suffix);

    printf("Command Buffer is %s\n The size is %ld\n", pcommand, sizeof(pcommand));

    if(target_write(fd, pcommand) < 1)
    {
        DEBUG_ERROR("No Bytes written\n");
    }

    free(pcommand);

    return 0;
}

// read until linefeed
int target_readline(int *fd, char *buf) {
    int i = 0;
    char c;

    usleep(100000);

    if(read(*fd, &c, 1) > 0){
        memset(buf, '\0', sizeof(buf));
        buf[i++] = c;
    } else {
        return -1;
    }

    while(read(*fd, &c, 1) >= 0) {
        usleep(1000);
        printf("%c", c);
        if(c == '\n') {
            //puts("Line parsed.");
            break;
        } else {
            buf[i++] = c;
            if(i == BUFFERMAX){
                puts("\nInput buffer is full\n");
                break;
            }
        }

    }
    buf[i]='\0';

    return 0;
}

// Call target_readline() until no more data is returned
void target_lastline(int *fd, char *buf) {
    
    int ret;

    while(1){
        ret = target_readline(fd, buf);
        printf("%s", buf);      
        if(ret < 0){
            printf("Last line \n");
            break;
        }
    }
}

// Determine from a list of options, what state the shell
// is in by matching the lastline received to predicted responses
int target_shell_state(int *fd) {

    int i = 0;

    target_lastline(fd, buf);
    while(1){
        if(strstr(buf, shell_states[i]) || shell_states[i] == "EOF") {
            break;
        }
        printf("not state %s\n", shell_states[i]);
        i++;
    }

    return i;
}

int target_close(int *fd) {
    return close(*fd);
}

