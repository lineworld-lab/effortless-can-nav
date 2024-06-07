#ifndef _ECAT_V2_SOCK_H_
#define _ECAT_V2_SOCK_H_


#include "ec-server/v2/core.h"


#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define SOCK_PORT 9999
#define MAX_CONN 1024
#define MAX_REQ_STRLEN 1024
#define MAX_RESULT_STRLEN 1024

#define MAX_TOKEN_STRLEN 24
#define MAX_MESSAGE 512

void* ECAT2_listen_and_serve(void *varg);

int ListenAndServe(int port);

int ProcessBuffer(char* res, char* req);

void ExitServer(int client_sock, int server_fd);

int GetHomingStatusByAxis(char* res, int axis);

int GetPositionByAxis(char* res, int axis);

int PostHomeShiftByAxis(char* res, int axis, int shift);

int PostPositionByAxis(char* res, int axis, int pos);

int PostPositionWithFeedbackByAxis(char* res, int axis, int pos);

int PostPositionWithStatusFeedbackByAxis(char* res, int axis, int pos);



#endif