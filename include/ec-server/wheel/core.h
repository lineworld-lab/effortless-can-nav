#ifndef _EC_SERVER_WHEEL_H_
#define _EC_SERVER_WHEEL_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <sys/epoll.h>
#include <net/if.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/wait.h>


#define MAX_CAN_DEV_NAME 10

#define MAX_CAN_NODE_NAME 10

#define MAX_CAN_CMD_IN 1000

#define MAX_CAN_CMD_OUT 1000

#define MAX_LOG_MESSAGE 2048

#define MAX_SET_LOCAL_SOCK 128

#define MAX_SOCK_PATH 108


extern pthread_t SOCK_PTID;

extern pid_t DAEMON_PID;

extern FILE* TFP;

extern char CAN_DEV_NAME[MAX_CAN_DEV_NAME];

extern int CAN_NODE_ID;

extern int SOCKFD;

extern char LOCAL_SOCKET_NAME[MAX_SOCK_PATH];



int InitWheelDaemon(char* can_dev_name, int can_node_id);

void* CO_daemon_start(void* varg);


int InitWheelCmdGateway();


int WheelCmdGatewayASCII(char* in, char* out);

int strncpy_process(char* src, char* dst);

int get_overriding_socket(char* sock_target_name);

void fdebugLog(char* msg);


#endif