#ifndef _AGVETHCAT_H_SOCK_
#define _AGVETHCAT_H_SOCK_

#include "agvethcat.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define SOCK_PORT 9999
#define MAX_CONN 1024
#define MAX_REQ_STRLEN 1024




int ListenAndServe(int port);

int ProcessBuffer(char* res, char* req);


int ListenAndServe_Test(int port);

int ProcessBuffer_Test(char* res, char* req);

void ExitServer(int client_sock, int server_fd);


#include "agvethcat/utils.h"

#endif