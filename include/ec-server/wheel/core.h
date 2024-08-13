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
#include <linux/prctl.h>
#include <sys/prctl.h>


#define MAX_CAN_DEV_NAME 10

#define MAX_CAN_NODE_NAME 10

#define MAX_CAN_CMD_IN 1000

#define MAX_CAN_CMD_OUT 1000

#define MAX_LOG_MESSAGE 2048

#define MAX_SET_LOCAL_SOCK 128

#define MAX_SOCK_PATH 108

#define MAX_WHEELS_NUM 16



extern pthread_t CAN_SOCK_PTID;


extern FILE* TFP;

extern char CAN_DEV_NAME[MAX_CAN_DEV_NAME];

extern char CAN_NODE_ID_STR[MAX_CAN_NODE_NAME];

extern int CAN_SOCKFD;
extern char SET_LOCAL_SOCKET[MAX_SET_LOCAL_SOCK];
extern char LOCAL_SOCKET_NAME[MAX_SOCK_PATH];

extern int g_num_of_wheel_slaves;
extern int* g_wheels_node_ids;
extern int* node_rotation_direction; 

extern int wheel_acceleration;
extern int wheel_deceleration;

int InitWheelDaemon(char* can_dev_name, char* can_node_id);


int InitWheelCmdGateway();


int WheelCmdSetUp();


void GetWheelCmd_TargetVelocity(char* incmd, int axis, char* speed);

void GetWheelCmd_MotorEnable(char* incmd, int axis);

void GetWheelCmd_MotorDisable(char* incmd, int axis);

void GetWheelCmd_ClearEncoderValue(char* incmd, int axis);

void GetWheelCmd_SetRotationClockwise(char* incmd, int axis);

void GetWheelCmd_SetRotationCounterClockwise(char* incmd, int axis);


void GetWheelCmd_SetAcceleration(char* incmd, int axis, int accel);


void GetWheelCmd_SetDeceleration(char* incmd, int axis, int decel);

int WheelCmdGatewayASCII(char* out, char* in);

void WheelShutdown();

int strncpy_process(char* src, char* dst);

int get_overriding_socket();

void fdebugLog(char* msg);


#endif