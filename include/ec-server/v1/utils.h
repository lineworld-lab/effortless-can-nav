#ifndef _ECAT_UTILS_H_
#define _ECAT_UTILS_H_

#include "ec-server/v1/core.h"

#include "cJSON.h"

int InitRuntimeFromConfigJSON();


void SetRealTimeSettings();

struct timespec timespec_add(struct timespec time1, struct timespec time2);


#endif