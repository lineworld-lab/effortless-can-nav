#ifndef _ECAT_V2_UTILS_H_
#define _ECAT_V2_UTILS_H_

#include "ec-server/v2/core.h"

#include "cJSON.h"

int InitRuntimeFrom(char* filename);


int AllocRuntime();

void FreeRuntime();

#endif