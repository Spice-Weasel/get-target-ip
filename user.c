#include<stdio.h>
#include<stdlib.h>

#include "app.h"

FILE *fp_pub_key;

const char ssh_pub_key_path[] = "/home/user/.ssh/id_rsa.pub";

int user_get_pub_key(char *key_buf)
{
    int n_bytes;
    // Read the public key for the node into file_buffer
    fp_pub_key = fopen(ssh_pub_key_path, "r");

    n_bytes = fread(key_buf, sizeof(char), BUFFERMAX, fp_pub_key);

    printf("Public Key read : %s\n", key_buf);

    fclose(fp_pub_key);

    return n_bytes;
}