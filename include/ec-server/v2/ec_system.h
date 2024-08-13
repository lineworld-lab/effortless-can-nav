#ifndef _ECAT_SYSTEM_V2_H_
#define _ECAT_SYSTEM_V2_H_


#include "ec-server/v2/core.h"


typedef struct {
    const char* cmd;
    const char* args1;
    const char* args2;
    const char* comment;
} AvailableCmd;

typedef enum AvailableCmdEnum {

    ECCMD_HOME_CHECK = 0,
    ECCMD_TRY_MOVE_OVERRIDE,
    ECCMD_TRY_WHEEL_VELOCITY,
    ECCMD_WHEEL_ROTATION,
    ECCMD_DISCONNECT,
    ECCMD_DISCOVERY,
    ECCMD_LEN

} AvailableCmdEnum;

extern const AvailableCmd available_cmd[];

extern const int available_cmd_count;



// system

void ECAT2_sigint_handler(int signum);

void ECAT2_shutdown();

int ECAT2_init_slave(uint16 slave);

void ECAT2_init_pdo(ec_slavet *ec_slave);

void ECAT2_lifecycle(char *ifname);

OSAL_THREAD_FUNC ECAT2_check(void *ptr);


// modules

void ECAT2_view();

ECAT2_PHASE ECAT2_exchange(int motor);

ECAT2_PHASE ECAT2_sync_status(int motor);

ECAT2_PHASE ECAT2_homing(int motor);

ECAT2_PHASE ECAT2_moving(int motor);

// utils

int read_sdo_u8(uint16 slave, uint16 index, uint8 subindex, uint8 *read_data);

int read_sdo_u16(uint16 slave, uint16 index, uint8 subindex, uint16 *read_data);

int read_sdo_u32(uint16 slave, uint16 index, uint8 subindex, uint32 *read_data);

int read_sdo_s8(uint16 slave, uint16 index, uint8 subindex, int8 *read_data);

int read_sdo_s16(uint16 slave, uint16 index, uint8 subindex, int16 *read_data);

int read_sdo_s32(uint16 slave, uint16 index, uint8 subindex, int32 *read_data);

int read_sdo_str18(uint16 slave, uint16 index, uint8 subindex, char *read_data);

int read_sdo_str8(uint16 slave, uint16 index, uint8 subindex, char *read_data);

int write_sdo_u8(uint16 slave, uint16 index, uint8 subindex, uint8 data);

int write_sdo_u16(uint16 slave, uint16 index, uint8 subindex, uint16 data);

int write_sdo_u32(uint16 slave, uint16 index, uint8 subindex, uint32 data);

int write_sdo_s8(uint16 slave, uint16 index, uint8 subindex, int8 data);

int write_sdo_s16(uint16 slave, uint16 index, uint8 subindex, int16 data);

int write_sdo_s32(uint16 slave, uint16 index, uint8 subindex, int32 data);



#endif