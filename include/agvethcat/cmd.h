#ifndef _AGVETHCAT_H_CMD_
#define _AGVETHCAT_H_CMD_

#include "agvethcat.h"

int ReadinessProbe(char* address, int port, int axis_c, int* axis);

void WriteProbeSuccess();

void WriteProbeFailure(std::string res);

int RunInteractive();

int ConnectTo(char* res, std::vector<std::string> runtime_args);

int SendTo(char* res, std::vector<std::string> runtime_args);

int SendTo2(char* res, std::vector<std::string> runtime_args);

int DisconnectFrom(char* res, std::vector<std::string> runtime_args);


#endif
