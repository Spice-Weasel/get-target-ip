
#define BUFFERMAX 1024
#define LOGIN_ATTEMPTS 10

// TARGET COMMANDS
#define PING "\n"
#define USER "user\n"
#define PW "password\n"
#define GET_IP "ip addr show usb0\n"

int target_open();
int target_configure(int *fd);
int target_ping(int *fd);
int target_clear_txrx_buffers(int *fd);
int target_readline(int *fd, char *line);
void target_lastline(int *fd, char *line);
int target_close(int *fd);
int target_shell_state(int *fd);
int target_write(int *fd, char *message);
char* target_save_ip(int *fd);
int target_add_key(int *fd, char *buf);