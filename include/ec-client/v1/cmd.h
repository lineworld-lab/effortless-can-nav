#ifndef _ECAT_CLIENT_CMD_H_
#define _ECAT_CLIENT_CMD_H_

#include "ec-client/v1/request.h"

int ReadinessProbe(char* address, int port, int axis_c, int* axis);

int RunInteractive();

int ConnectTo(char* res, int arg_len, char** runtime_args);

int SendTo(char* res, int arg_len, char** runtime_args);

int DisconnectFrom(char* res, int arg_len, char** runtime_args);


#endif
