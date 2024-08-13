#ifndef _EC_SERVER_WHEEL_UTILS_H_
#define _EC_SERVER_WHEEL_UTILS_H_



#include "ec-server/wheel/core.h"

#include "cJSON.h"



int InitWheelRuntimeFromObject(cJSON *conf_json_root);

int AllocWheelRuntime();

void FreeWheelRuntime();

#endif