

#include "ec-server/v2/ec_system.h"
#include "ec-server/v2/sock.h"
#include "ec-server/v2/utils.h"



char IOmap[4096];

OSAL_THREAD_HANDLE thread1;

int expectedWKC;

boolean needlf;

volatile int wkc;

boolean inOP;

uint8 currentgroup = 0;

boolean forceByteAlignment = FALSE;

int debug_mode = 0;




// Home sensor position from front, anti-clockwise. 400000 is one rotation

int arr_home_sensor_position[] = {332000, 300000, 393000, 139000};


// Homing start offset

int arr_start_offset[] = {0, 0, 0, 0};


// margin degrees to make homing reliable

int HOMING_MARGIN_DEGREES = 5;


int WORKING_COUNTER_FAIL_THRESHOLD = 20;

int working_counter_fail_count = 0;


int ERROR_COUNT_THRESHOLD = 10;

int error_count = 0;


int MOVE_FAIL_THRESHOLD = 50;


int g_num_of_slaves = 4;

servo_rxpdo_t **motor_rxpdos = NULL;

servo_txpdo_t **motor_txpdos = NULL;

pthread_t ECAT2_tid;

extern char* main_if_name = "enp1s0";

int main(int argc, char *argv[])

{

   printf("SOEM (Simple Open EtherCAT Master)\nServo controller\n");


   signal(SIGINT, sigint_handler);


   for (int i = 1; i < argc; i++)

   {

      if (strcmp(argv[i], "--log-level") == 0)

      {

         printf("Log level: %s\n", argv[i + 1]);


         if (strcmp(argv[i + 1], "debug") == 0)

         {

            debug_mode = 1;

            printf("Debug mode enabled\n");

         }

         else

         {

            debug_mode = 0;

            printf("Debug mode disabled\n");

         }

      }

   }


   if (argc > 1)

   {
      ec_adaptert *adapter = NULL;

      printf("Usage: controller ifname1\nifname = eth0 for example\n");


      printf("\nAvailable adapters:\n");

      adapter = ec_find_adapters();

      while (adapter != NULL)

      {

         printf("    - %s  (%s)\n", adapter->name, adapter->desc);

         adapter = adapter->next;

      }

      ec_free_adapters(adapter);


   }

   else

   {

      osal_thread_create(&thread1, 128000, &ecatcheck, NULL);

      /* start cyclic part */

      ECAT2_lifecycle(main_if_name);


   }


   printf("End program\n");

   return (0);

}


