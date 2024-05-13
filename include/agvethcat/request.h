#ifndef _AGVETHCAT_H_REQUEST_
#define _AGVETHCAT_H_REQUEST_


#include "agvethcat.h"



#include <arpa/inet.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <poll.h>
#include <time.h>

#define MAX_BUFFLEN 1024


extern int SOCK_FD;

extern char ADDR[MAX_BUFFLEN];

extern int PORT;

extern long TIMEOUT;

extern int VCONN;

struct AxisReq{

    int axis;
    std::string action;
    std::string params;
    std::string status;

};


struct AxisReq2{

    int axis;
    std::string action;
    std::string params;
    int status;
    int feedback;

};

enum EC_CODE {

    OKAY = 0,
    EREAD = -1,
    EWRITE = -2,
    ECONNREAD = -11,
    ECONNWRITE = -12,


};

int InitECCmdGateway(char* res, char* addr, int port);

int InitECCmdGatewayWithTimeout(char* res, char* addr, int port, long timeout);


int ECCmdGatewayAR(char* res, std::vector<struct AxisReq>* var);

int ECCmdGatewayARFeedback(char* res, std::vector<struct AxisReq>* var);

int ECCmdGatewayLoadAR(char* res, std::vector<struct AxisReq>* var, int iteration, int distance, int interval);


EC_CODE ECCmdGatewayARFeedback2(char* res, std::vector<struct AxisReq2>* var);

int TryECCmdGatewayARFeedback(char* res, std::vector<struct AxisReq2>* var);

int TryInitECCmdGateway(char* res);

int ECCmdGatewayLoadAR2(char* res, std::vector<struct AxisReq2>* var, int iteration, int distance, int interval);

std::string EC_CODEToString(EC_CODE ecc);



#include "agvethcat/utils.h"

#endif