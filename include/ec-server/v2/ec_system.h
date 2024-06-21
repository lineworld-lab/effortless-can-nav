#ifndef _ECAT_SYSTEM_V2_H_
#define _ECAT_SYSTEM_V2_H_


#include "ec-server/v2/core.h"



void ECAT2_sigint_handler(int signum);

void ECAT2_shutdown();

int ECAT2_init_slave(uint16 slave);

void ECAT2_init_pdo(ec_slavet *ec_slave);

void ECAT2_lifecycle(char *ifname);

void ECAT2_homing();

void ECAT2_moving();



OSAL_THREAD_FUNC ECAT2_check(void *ptr);


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