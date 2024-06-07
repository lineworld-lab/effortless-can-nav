#include "ec-server/v2/ec_system.h"


int read_sdo_u8(uint16 slave, uint16 index, uint8 subindex, uint8 *read_data)
{
   int read_data_size = sizeof(*read_data);
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_u16(uint16 slave, uint16 index, uint8 subindex, uint16 *read_data)
{
   int read_data_size = sizeof(*read_data);
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_u32(uint16 slave, uint16 index, uint8 subindex, uint32 *read_data)
{
   int read_data_size = sizeof(*read_data);
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_s8(uint16 slave, uint16 index, uint8 subindex, int8 *read_data)
{
   int read_data_size = sizeof(*read_data);
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_s16(uint16 slave, uint16 index, uint8 subindex, int16 *read_data)
{
   int read_data_size = sizeof(*read_data);
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_s32(uint16 slave, uint16 index, uint8 subindex, int32 *read_data)
{
   int read_data_size = sizeof(*read_data);
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_str18(uint16 slave, uint16 index, uint8 subindex, char *read_data)
{
   int read_data_size = 18;
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int read_sdo_str8(uint16 slave, uint16 index, uint8 subindex, char *read_data)
{
   int read_data_size = 8;
   int wkc = ec_SDOread(slave, index, subindex, FALSE, &read_data_size, read_data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to read SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int write_sdo_u8(uint16 slave, uint16 index, uint8 subindex, uint8 data)
{
   int wkc = ec_SDOwrite(slave, index, subindex, FALSE, sizeof(data), &data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to write SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int write_sdo_u16(uint16 slave, uint16 index, uint8 subindex, uint16 data)
{
   int wkc = ec_SDOwrite(slave, index, subindex, FALSE, sizeof(data), &data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to write SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int write_sdo_u32(uint16 slave, uint16 index, uint8 subindex, uint32 data)
{
   int wkc = ec_SDOwrite(slave, index, subindex, FALSE, sizeof(data), &data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to write SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int write_sdo_s8(uint16 slave, uint16 index, uint8 subindex, int8 data)
{
   int wkc = ec_SDOwrite(slave, index, subindex, FALSE, sizeof(data), &data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to write SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int write_sdo_s16(uint16 slave, uint16 index, uint8 subindex, int16 data)
{
   int wkc = ec_SDOwrite(slave, index, subindex, FALSE, sizeof(data), &data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to write SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}

int write_sdo_s32(uint16 slave, uint16 index, uint8 subindex, int32 data)
{
   int wkc = ec_SDOwrite(slave, index, subindex, FALSE, sizeof(data), &data, EC_TIMEOUTRXM);
   if (wkc == 0)
   {
      printf("Error: Failed to write SDO (slave:%d, index:0x%x, subindex:0x%x)\n", slave, index, subindex);
   }

   return wkc;
}



