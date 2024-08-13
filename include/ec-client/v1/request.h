#ifndef _ECAT_CLIENT_REQUEST_H_
#define _ECAT_CLIENT_REQUEST_H_

#include "ec-client/v1/core.h"



#include <arpa/inet.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <poll.h>
#include <time.h>



extern int SOCK_FD;

extern char ADDR[MAX_BUFFLEN];

extern int PORT;

extern long TIMEOUT;

extern int VCONN;


typedef struct AxisReq{

    int axis;
    char action[MAX_BUFFLEN];
    char params[MAX_BUFFLEN];
    int status;
    int feedback;

} AxisReq;

enum EC_CODE {

    OKAY = 0,
    EREAD = -1,
    EWRITE = -2,
    ECONNREAD = -11,
    ECONNWRITE = -12,


};

int InitECCmdGateway(char* res, char* addr, int port);

int InitECCmdGatewayWithTimeout(char* res, char* addr, int port, long timeout);

int ECCmdGatewayAR(char* res, int ar_len, AxisReq* var);

int ECCmdGatewayARTest(char* res);

int ECCmdGatewayLoadAR(char* res, int ar_len, AxisReq* var, int iteration, int distance, int interval);

int TryECCmdGatewayARFeedback(char* res, int ar_len, AxisReq* var);

int TryInitECCmdGateway(char* res);





#endif