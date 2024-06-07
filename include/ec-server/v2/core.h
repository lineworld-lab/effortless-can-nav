#ifndef _ECAT_V2_H_
#define _ECAT_V2_H_


#define _GNU_SOURCE

#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sched.h>

#include <stdio.h>

#include <string.h>

#include <syslog.h>

#include <inttypes.h>

#include <stdbool.h>

#include <stdlib.h>

#include <signal.h>

#include <sys/ipc.h>

#include <sys/shm.h>

#include "soem/ethercat.h"

#include "ec-server/v2/object_dictionary.h"



#define IOMAP_SIZE 4096

#define MAX_STATUS_STRLEN 8
#define MAX_POSITION_STRLEN 32
#define MAX_BUFF 1024 * 10

#define printf_debug    printf


#define TEST_BIT(NUM, N) ((NUM & (1 << N)) >> N) /// Check specific bit in the data. 0 or 1.

#define SET_BIT(NUM, N) (NUM | (1 << N))         /// Set(1) specific bit in the data.

#define RESET_BIT(NUM, N) (NUM & ~(1 << N))      /// Reset(0) specific bit in the data

#define TOGGLE_BIT(NUM, N) (NUM ^ (1 << N))      /// Toggle specific bit in the data.


#define EC_TIMEOUTMON 500



#define DEGREE_TO_POSITION(DEGREE) (DEGREE / 360.0 * 400000.0)

#define POSITION_TO_DEGREE(POSITION) (POSITION / 400000.0 * 360.0)


typedef struct servo_rxpdo_t

{

   uint16 control_word;

   int32 target_position;

   int32 profile_velocity;

   uint32 physical_outputs_1;

   uint8 mode_of_operation;

} servo_rxpdo_t;

typedef struct servo_txpdo_t

{

   uint16 status_word;

   int32 position_actual_value;

   int32 velocity_actual_value;

   uint32 digital_inputs;

   uint16 error_code;

   uint8 mode_of_operation_display;

} servo_txpdo_t;



extern char IOmap[IOMAP_SIZE];

extern OSAL_THREAD_HANDLE thread1;

extern int expectedWKC;

extern boolean needlf;

extern volatile int wkc;

extern boolean inOP;

extern uint8 currentgroup;

extern boolean forceByteAlignment;

extern int debug_mode;




// Home sensor position from front, anti-clockwise. 400000 is one rotation

extern int arr_home_sensor_position[];


// Homing start offset

extern int arr_start_offset[];


// margin degrees to make homing reliable

extern int HOMING_MARGIN_DEGREES;


extern int WORKING_COUNTER_FAIL_THRESHOLD ;

extern int working_counter_fail_count;


extern int ERROR_COUNT_THRESHOLD ;

extern int error_count;


extern int MOVE_FAIL_THRESHOLD;


extern int we_are_testing;

extern int g_num_of_slaves;

extern servo_rxpdo_t **motor_rxpdos;
extern servo_txpdo_t **motor_txpdos;


extern pthread_t ECAT2_tid;

extern char* main_if_name;


#endif