#include "ec-server/v2/ec_system.h"
#include "ec-server/v2/sock.h"
#include "ec-server/v2/utils.h"


void ECAT2_shutdown()

{

   printf("\nRequest init state for all slaves\n");

   ec_slave[0].state = EC_STATE_INIT;

   /* request INIT state for all slaves */

   ec_writestate(0);


   printf("End servo controller, close socket\n");

   /* stop SOEM, close socket */

   ec_close();


   FreeRuntime();

}


void sigint_handler(int signum)

{

   printf("Received signal %d: %s. Exiting...\n", signum, strsignal(signum));


   ECAT2_shutdown();

   exit(signum);

}


int ECAT2_init_slave(uint16 slave)

{

   // print ec_slave[slave].CoEdetails

   printf("Slave:%d, CoEdetails: 0x%x\n", slave, ec_slave[slave].CoEdetails);


   // PDO assign

   uint16 map_1c12[2] = {0x0001, OD_RXPDO_MAP_1};

   if (ec_SDOwrite(slave, 0x1c12, 0x00, TRUE, sizeof(map_1c12), &map_1c12, EC_TIMEOUTRXM) == 0)

      return -1;

   uint16 map_1c13[2] = {0x0001, OD_TXPDO_MAP_1};

   if (ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_1c13), &map_1c13, EC_TIMEOUTRXM) == 0)

      return -1;


   // PDO mapping

   if (write_sdo_u8(slave, OD_RXPDO_MAP_1, 0x00, 5) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_RXPDO_MAP_1, 0x01, OD_CONTROL_WORD_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_RXPDO_MAP_1, 0x02, OD_TARGET_POSITION_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_RXPDO_MAP_1, 0x03, OD_PROFILE_VELOCITY_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_RXPDO_MAP_1, 0x04, OD_PHYSICAL_OUTPUTS_1_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_RXPDO_MAP_1, 0x05, OD_MODE_OF_OPERATION_MAPPING_CONTENT) == 0)

      return -1;


   if (write_sdo_u8(slave, OD_TXPDO_MAP_1, 0x00, 6) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_TXPDO_MAP_1, 0x01, OD_STATUS_WORD_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_TXPDO_MAP_1, 0x02, OD_POSITION_ACTUAL_VALUE_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_TXPDO_MAP_1, 0x03, OD_VELOCITY_ACTUAL_VALUE_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_TXPDO_MAP_1, 0x04, OD_DIGITAL_INPUTS_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_TXPDO_MAP_1, 0x05, OD_ERROR_CODE_MAPPING_CONTENT) == 0)

      return -1;

   if (write_sdo_u32(slave, OD_TXPDO_MAP_1, 0x06, OD_MODE_OF_OPERATION_DISPLAY_MAPPING_CONTENT) == 0)

      return -1;


   // read PDO mapping information

   uint8 rxpdo_assign_count = 0;

   uint8 txpdo_assign_count = 0;

   uint16 rxpdo_assign_1 = 0;

   uint16 txpdo_assign_1 = 0;


   read_sdo_u8(slave, OD_RXPDO_ASSIGN, 0x00, &rxpdo_assign_count);

   read_sdo_u8(slave, OD_TXPDO_ASSIGN, 0x00, &txpdo_assign_count);

   read_sdo_u16(slave, OD_RXPDO_ASSIGN, 0x01, &rxpdo_assign_1);

   read_sdo_u16(slave, OD_TXPDO_ASSIGN, 0x01, &txpdo_assign_1);


   printf("Slave:%d, Number of RxPDO assign : %d\n", slave, rxpdo_assign_count);

   printf("Slave:%d, Number of TxPDO assign : %d\n", slave, txpdo_assign_count);

   printf("Slave:%d, RxPDO assign 1 : 0x%x\n", slave, rxpdo_assign_1);

   printf("Slave:%d, TxPDO assign 1 : 0x%x\n", slave, txpdo_assign_1);


   uint8 rxpdo_map_number = 0;

   wkc = read_sdo_u8(slave, OD_RXPDO_MAP_1, 0x00, &rxpdo_map_number);

   printf("Slave:%d, Number of RxPDO mapping : %d\n", slave, rxpdo_map_number);


   for (int i = 1; i < rxpdo_map_number; i++)

   {

      uint32 rxpdo_map = 0;

      read_sdo_u32(slave, OD_RXPDO_MAP_1, i, &rxpdo_map);

      printf("Slave:%d, RxPDO mapping %d : 0x%x\n", slave, i, rxpdo_map);

   }


   uint8 txpod_map_number = 0;

   wkc = read_sdo_u8(slave, OD_TXPDO_MAP_1, 0x00, &txpod_map_number);

   printf("Slave:%d, Number of TxPDO mapping : %d\n", slave, txpod_map_number);


   for (int i = 1; i < txpod_map_number; i++)

   {

      uint32 txpdo_map = 0;

      read_sdo_u32(slave, OD_TXPDO_MAP_1, i, &txpdo_map);

      printf("Slave:%d, TxPDO mapping %d : 0x%x\n", slave, i, txpdo_map);

   }


   /* set parameters */

   // set Home offset for each motor

   switch (slave)

   {

   case 1:

      if (write_sdo_s32(slave, OD_HOME_OFFSET, 0x00, arr_home_sensor_position[0]) == 0)

         return -1;

      break;

   case 2:

      if (write_sdo_s32(slave, OD_HOME_OFFSET, 0x00, arr_home_sensor_position[1]) == 0)

         return -1;

      break;

   case 3:

      if (write_sdo_s32(slave, OD_HOME_OFFSET, 0x00, arr_home_sensor_position[2]) == 0)

         return -1;

      break;

   case 4:

      if (write_sdo_s32(slave, OD_HOME_OFFSET, 0x00, arr_home_sensor_position[3]) == 0)

         return -1;

      break;

   }


   // set homing method 28 - Homing on Origin Switch (Negative Direction, Positive Edge) -> Something is wrong in FASTECH device. Direction is reversed

   if (write_sdo_u8(slave, OD_HOMING_METHOD, 0x00, 24) == 0)

      return -1;


   // set homing speed during search for switch -> Something is wrong in FASTECH device. This speed is applied when the motor is going to home position(zero shift)

   if (write_sdo_u32(slave, OD_HOMING_SPEEDS, 0x01, 200000) == 0)

      return -1;


   // set homing speed during search for zero (search for home sensor) -> Something is wrong in FASTECH device. This speed is applied when the motor is going to find the home sensor

   if (write_sdo_u32(slave, OD_HOMING_SPEEDS, 0x02, 25000) == 0)

      return -1;


   // set homing acceleration

   if (write_sdo_u32(slave, OD_HOMING_ACCELERATION, 0x00, 100000) == 0)

      return -1;


   // set rotation direction

   if (slave == 1 || slave == 4)

   {

      // reverse rotation direction

      if (write_sdo_u8(slave, OD_POLARITY, 0x00, 0x80) == 0)

         return -1;

   }

   else

   {

      if (write_sdo_u8(slave, OD_POLARITY, 0x00, 0x00) == 0)

         return -1;

   }


   // set software limit to max

   if (write_sdo_s32(slave, OD_SOFTWARE_LIMIT, 0x01, -2147483648) == 0)

      return -1;

   if (write_sdo_s32(slave, OD_SOFTWARE_LIMIT, 0x02, 2147483647) == 0)

      return -1;


   // read parameters

   uint8 polarity = 0;

   uint32 max_profile_velocity = 0;

   uint32 profile_acceleration = 0;

   uint32 profile_deceleration = 0;

   int home_offset = 0;

   int8 homing_method = 0;

   uint32 homing_speed_during_search_for_switch = 0;

   uint32 homing_speed_during_search_for_zero = 0;

   uint32 homing_acceleration = 0;

   int software_limit_min = 0;

   int software_limit_max = 0;

   read_sdo_u8(slave, OD_POLARITY, 0x00, &polarity);

   read_sdo_u32(slave, OD_MAX_PROFILE_VELOCITY, 0x00, &max_profile_velocity);

   read_sdo_u32(slave, OD_PROFILE_ACCERATION, 0x00, &profile_acceleration);

   read_sdo_u32(slave, OD_PROFILE_DECELERATION, 0x00, &profile_deceleration);

   read_sdo_s32(slave, OD_HOME_OFFSET, 0x00, &home_offset);

   read_sdo_s8(slave, OD_HOMING_METHOD, 0x00, &homing_method);

   read_sdo_u32(slave, OD_HOMING_SPEEDS, 0x01, &homing_speed_during_search_for_switch);

   read_sdo_u32(slave, OD_HOMING_SPEEDS, 0x02, &homing_speed_during_search_for_zero);

   read_sdo_u32(slave, OD_HOMING_ACCELERATION, 0x00, &homing_acceleration);

   read_sdo_s32(slave, OD_SOFTWARE_LIMIT, 0x01, &software_limit_min);

   read_sdo_s32(slave, OD_SOFTWARE_LIMIT, 0x02, &software_limit_max);

   printf("Slave:%d, Polarity: 0x%x\n", slave, polarity);

   printf("Slave:%d, Max Profile Velocity: %d\n", slave, max_profile_velocity);

   printf("Slave:%d, Profile Acceleration: %d\n", slave, profile_acceleration);

   printf("Slave:%d, Profile Deceleration: %d\n", slave, profile_deceleration);

   printf("Slave:%d, Home Offset: %d\n", slave, home_offset);

   printf("Slave:%d, Homing Method: %d\n", slave, homing_method);

   printf("Slave:%d, Homing Speed During Search for Switch: %d\n", slave, homing_speed_during_search_for_switch);

   printf("Slave:%d, Homing Speed During Search for Zero: %d\n", slave, homing_speed_during_search_for_zero);

   printf("Slave:%d, Homing Acceleration: %d\n", slave, homing_acceleration);

   printf("Slave:%d, Software Limit Min: %d\n", slave, software_limit_min);

   printf("Slave:%d, Software Limit Max: %d\n", slave, software_limit_max);


   // read hardware_version and software_version

   char hardware_version[8];

   char software_version[8];

   read_sdo_str8(slave, OD_HARDWARE_VERSION, 0x00, hardware_version);

   read_sdo_str8(slave, OD_SOFTWARE_VERSION, 0x00, software_version);

   printf("Slave:%d, Hardware Version: %s, Software Version: %s\n", slave, hardware_version, software_version);


   printf("\033[1;32mSlave %d setup complete.\033[0m\n", slave);

   return 1;

}


void ECAT2_init_pdo(ec_slavet *ec_slave){

   for (int i = 0 ; i < g_num_of_slaves; i++){

      int slave_idx = i + 1;

      motor_rxpdos[i] = (servo_rxpdo_t*)malloc(sizeof(servo_rxpdo_t));

      memset(motor_rxpdos[i], 0 , sizeof(servo_rxpdo_t));

      motor_rxpdos[i] = (servo_rxpdo_t*)ec_slave[slave_idx].outputs;

   }

   for (int i = 0 ; i < g_num_of_slaves; i++){

      int slave_idx = i + 1;

      motor_txpdos[i] = (servo_txpdo_t*)malloc(sizeof(servo_txpdo_t));

      memset(motor_txpdos[i], 0 , sizeof(servo_txpdo_t));

      motor_txpdos[i] = (servo_txpdo_t*)ec_slave[slave_idx].inputs;

   }


}



void ECAT2_lifecycle(char *ifname)

{

   int chk;

   inOP = FALSE;


   for (int i = 0; i < g_num_of_slaves; i++)

   {

      double home_sensor_degrees = POSITION_TO_DEGREE(arr_home_sensor_position[i]);

      printf("[INFO] Motor %d Home sensor degrees: %f\n", i, home_sensor_degrees);


      // if the home sensor is at the first quadrant

      if (270 <= home_sensor_degrees && home_sensor_degrees <= 360)

      {

         // minimum distance to move to make home sensor is positioned at the clockwise side from steering center

         arr_start_offset[i] -= DEGREE_TO_POSITION((home_sensor_degrees - 270));

         // distance to move to make homing quicker

         arr_start_offset[i] -= DEGREE_TO_POSITION((180 - HOMING_MARGIN_DEGREES));

      }

      // if the motor is at the second quadrant

      else if (0 <= home_sensor_degrees && home_sensor_degrees <= 90)

      {

         home_sensor_degrees -= 360;


         // minimum distance to move to make home sensor is positioned at the clockwise side from steering center

         arr_start_offset[i] -= DEGREE_TO_POSITION((home_sensor_degrees - 270));

         // distance to move to make homing quicker

         arr_start_offset[i] -= DEGREE_TO_POSITION((180 - HOMING_MARGIN_DEGREES));

      }

      // if the motor is at the third or fourth quadrant

      else if (90 < home_sensor_degrees && home_sensor_degrees < 270)

      {

         arr_start_offset[i] -= DEGREE_TO_POSITION((home_sensor_degrees - 90 - HOMING_MARGIN_DEGREES));

      }

   }


   // print arr_start_offset

   printf("[INFO] Start offset: %d %d %d %d\n", arr_start_offset[0], arr_start_offset[1], arr_start_offset[2], arr_start_offset[3]);



   printf("Starting servo controller\n");


   /* initialise SOEM, bind socket to ifname */

   if (ec_init(ifname) <= 0)

   {

      printf("No socket connection on %s\nExecute as root\n", ifname);

      return;

   }

   printf("ec_init on %s succeeded.\n", ifname);


   /* find and auto-config slaves */

   if (ec_config_init(FALSE) <= 0)

   {

      printf("No slaves found!\n");

      ec_close();

      return;

   }

   printf("%d slaves found and configured.\n", ec_slavecount);


   for (int i = 1; i <= ec_slavecount; i++)

   {

      // set hook before transition from pre_op to safe_op

      ec_slave[i].PO2SOconfig = ECAT2_init_slave;

   }


   ec_config_map(&IOmap);


   ec_configdc();


   printf("Slaves mapped, state to SAFE_OP.\n");

   /* wait for all slaves to reach SAFE_OP state */

   ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);


   printf("Request operational state for all slaves\n");

   expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;

   printf("Calculated(expected) workcounter %d\n", expectedWKC);

   /* send one valid process data to make outputs in slaves happy*/

   ec_send_processdata();

   ec_receive_processdata(EC_TIMEOUTRET);


   /* request OP state for all slaves */

   ec_slave[0].state = EC_STATE_OPERATIONAL;

   ec_writestate(0);

   chk = 200;

   /* wait for all slaves to reach OP state */

   do

   {

      ec_send_processdata();

      ec_receive_processdata(EC_TIMEOUTRET);

      ec_statecheck(0, EC_STATE_OPERATIONAL, 25000);

   } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));


   if (ec_slave[0].state != EC_STATE_OPERATIONAL)

   {

      printf("Not all slaves reached operational state.\n");

      ec_readstate();

      for (int slave = 1; slave <= ec_slavecount; slave++)

      {

         if (ec_slave[slave].state != EC_STATE_OPERATIONAL)

         {

            printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",

                   slave, ec_slave[slave].state, ec_slave[slave].ALstatuscode, ec_ALstatuscode2string(ec_slave[slave].ALstatuscode));

         }

      }

      // ECAT2_shutdown();

      return;

   }


   for (int i = 1; i <= ec_slavecount; i++)

   {

      // print slaves input/output bytes and bits

      printf("Slave:%d, Name:%s, Obytes:%d(%d bits), Ibytes:%d(%d bits)\n",

             i, ec_slave[i].name, ec_slave[i].Obytes, ec_slave[i].Obits, ec_slave[i].Ibytes, ec_slave[i].Ibits);

   }


   printf("Operational state reached for all slaves.\n");

   inOP = TRUE;


   ECAT2_alloc_pdo(ec_slave);


   pthread_create(&ECAT2_tid, NULL, ECAT2_listen_and_serve, NULL);


   // read initial actual position

   int arr_initial_positions[] = {0, 0, 0, 0};

   for (int motor = 0; motor < ec_slavecount; motor++)

   {

      read_sdo_s32(motor + 1, OD_POSITION_ACTUAL_VALUE, 0x00, &arr_initial_positions[motor]);

      printf("[INFO] Motor:%d, Initial Actual Position: %d\n", motor, arr_initial_positions[motor]);

   }


   bool arr_has_moved_to_start_offset[] = {false, false, false, false};

   bool has_moved_to_start_offset = false;

   bool arr_is_homing_done[] = {false, false, false, false};

   bool is_homing_complete = false;


   // To check if the motor is not following the target position

   int arr_move_fail_count[] = {0, 0, 0, 0};


   /* cyclic loop */

   while (true)

   {

      for (int motor = 0; motor < ec_slavecount; motor++)

      {

         printf_debug("\033[38;5;208m[DEBUG] Motor:%d, Control Word:0x%x, Target Position:%d, Profile Velocity:%d, Mode of Operation:0x%x\033[0m\n",

                      motor,

                      motor_rxpdos[motor]->control_word,

                      motor_rxpdos[motor]->target_position,

                      motor_rxpdos[motor]->profile_velocity,

                      motor_rxpdos[motor]->mode_of_operation);

      }


      osal_usleep(50000);

      ec_send_processdata();

      wkc = ec_receive_processdata(EC_TIMEOUTRET);


      if (wkc == -1)

      {

         printf("\033[1;31m[ERROR] Workcounter not met (actual WKC:%d, expected WKC:%d)\033[0m\n", wkc, expectedWKC);

         ECAT2_shutdown();

         return;

      }

      else if (wkc < expectedWKC)

      {

         printf("\033[1;31m[ERROR] Workcounter not met (actual WKC:%d, expected WKC:%d)\033[0m\n", wkc, expectedWKC);

         working_counter_fail_count++;

         continue;

      }


      if (working_counter_fail_count >= WORKING_COUNTER_FAIL_THRESHOLD)

      {

         printf("\033[1;31m[ERROR] Workcounter fail count exceeds threshold. Exiting...\033[0m\n");

         ECAT2_shutdown();

         return;

      }


      working_counter_fail_count = 0;


      // motor 0: front_right, motor 1: front_left, motor 2: rear_right, motor 3: rear_left

      // slave number == motor + 1

      for (int motor = 0; motor < ec_slavecount; motor++)

      {

         if (motor_txpdos[motor]->error_code != 0)

         {

            printf("\033[1;31m[ERROR] Motor:%d, Error Code:0x%x\033[0m\n", motor, motor_txpdos[motor]->error_code);


            if (motor_txpdos[motor]->error_code != 0x7500)

            {

               error_count++;

            }


            if (error_count >= ERROR_COUNT_THRESHOLD)

            {

               printf("\033[1;31m[ERROR] Error count exceeds threshold. Exiting...\033[0m\n");

               ECAT2_shutdown();

               return;

            }

         }


         printf_debug("[DEBUG] Motor:%d, Status Word:0x%x, Actual Pos:%d, Actual Vel:%d, Home sensor:0x%x, Err Code:0x%x, Mode of OP Disp:0x%x\n",

                      motor,

                      motor_txpdos[motor]->status_word,

                      motor_txpdos[motor]->position_actual_value,

                      motor_txpdos[motor]->velocity_actual_value,

                      motor_txpdos[motor]->digital_inputs,

                      motor_txpdos[motor]->error_code,

                      motor_txpdos[motor]->mode_of_operation_display);


         // trim status word to 7 bits

         int sw = motor_txpdos[motor]->status_word;

         int base = motor_txpdos[motor]->status_word & 0x000F;


         /* xxxx xxxx x011 0111b OPERATION_ENABLED */

         if ((base | (sw & (7 << 4))) != STATUS_WORD_OPERATION_ENABLED)

         {

            /* xxxx xxxx x0xx 0000 STATUS_WORD_NOT_READY_TO_SWITCH_ON */

            if (base == STATUS_WORD_NOT_READY_TO_SWITCH_ON)

            {

               /* Now the FSM should authomatically go to the SWITCH_ON_DISABLED state */

               printf("[INFO] Motor:%d, Not ready to switch on\n", motor);

               continue;

            }

            /* xxxx xxxx x1xx 0000 STATUS_WORD_SWITCH_ON_DISABLED */

            else if ((base | (sw & (1 << 6))) == STATUS_WORD_SWITCH_ON_DISABLED)

            {

               printf("[INFO] Motor:%d, Switch on disabled\n", motor);


               /* Automatic translation (2) */

               motor_rxpdos[motor]->control_word = 0x6;


               continue;

            }

            /* xxxx xxxx x01x 0001 STATUS_WORD_READY_TO_SWITCH_ON */

            else if ((base | (sw & (3 << 5))) == STATUS_WORD_READY_TO_SWITCH_ON)

            {

               printf("[INFO] Motor:%d, Ready to switch on\n", motor);


               /* transition 3 */

               motor_rxpdos[motor]->control_word = 0xf;


               continue;

            }

            /*xxxx xxxx x011 0011 STATUS_WORD_SWITCHED_ON*/

            else if ((base | (sw & (7 << 4))) == STATUS_WORD_SWITCHED_ON)

            {

               printf("[INFO] Motor:%d, Switched on\n", motor);


               /* Enable operation command for transition (4) */

               motor_rxpdos[motor]->control_word = 0xf;


               continue;

            }

            /* xxxx xxxx x00x 0111 STATUS_WORD_QUICK_STOP_ACTIVE */

            else if ((base | (sw & (3 << 5))) == STATUS_WORD_QUICK_STOP_ACTIVE)

            {

               printf("[INFO] Motor:%d, Quick stop active\n", motor);

               continue;

            }

            /* xxxx xxxx x0xx 1111 STATUS_WORD_FAULT_REACTION_ACTIVE */

            else if ((base | (sw & (1 << 6))) == STATUS_WORD_FAULT_REACTION_ACTIVE)

            {

               printf("[INFO] Motor:%d, Fault reaction active\n", motor);

               continue;

            }

            /* xxxx xxxx x0xx 1000 STATUS_WORD_FAULT */

            else if ((base | (sw & (1 << 6))) == STATUS_WORD_FAULT)

            {

               printf("[WARNING] Motor:%d, Fault detected\n", motor);


               /* Returning to Switch On Disabled (transition 15) */

               motor_rxpdos[motor]->control_word = (1 << BIT_CONTROL_WORD_FAULTRESET);


               continue;

            }

            else

            {

               printf("[ERROR] Motor:%d, Unknown status word:0x%x\n", motor, motor_txpdos[motor]->status_word);

               ECAT2_shutdown();

               return;

            }

         }


         if (motor_txpdos[motor]->mode_of_operation_display == 0x01 && TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_PP_SETPOINT_ACKNOWLEDGE))

         {

            // reset new_set_point to 0 in profile position mode

            motor_rxpdos[motor]->control_word = 0x002F;

            motor_rxpdos[motor]->mode_of_operation = 0x01;

            continue;

         }


         if (!has_moved_to_start_offset && arr_has_moved_to_start_offset[0] && arr_has_moved_to_start_offset[1] && arr_has_moved_to_start_offset[2] && arr_has_moved_to_start_offset[3])

         {

            has_moved_to_start_offset = true;

         }


         if (!is_homing_complete && arr_is_homing_done[0] && arr_is_homing_done[1] && arr_is_homing_done[2] && arr_is_homing_done[3])

         {

            is_homing_complete = true;

            printf("[INFO] All motors have completed homing\n");


            // set software limit to -200000 and 200000 by sdo

            if (write_sdo_s32(motor + 1, OD_SOFTWARE_LIMIT, 0x01, -200000) == 0)

            {

               printf("\033[1;31m[ERROR] Failed to set position min. limit for motor %d\033[0m\n", motor);

               ECAT2_shutdown();

               return;

            }

            if (write_sdo_s32(motor + 1, OD_SOFTWARE_LIMIT, 0x02, 200000) == 0)

            {

               printf("\033[1;31m[ERROR] Failed to set position max. limit for motor %d\033[0m\n", motor);

               ECAT2_shutdown();

               return;

            }

         }


         if (!arr_has_moved_to_start_offset[motor])

         {

            // The motors need to move in the negative direction for the calculated

            // distance to ensure that the home sensor is located on the negative side

            // relative to the front of the wheel.

            motor_rxpdos[motor]->target_position = arr_initial_positions[motor] + arr_start_offset[motor];


            if (motor_txpdos[motor]->position_actual_value == motor_rxpdos[motor]->target_position)

            {

               printf("[INFO] Motor %d has reached start offset\n", motor);

               arr_has_moved_to_start_offset[motor] = true;

            }

            else

            {

               motor_rxpdos[motor]->control_word = 0x003F;

               motor_rxpdos[motor]->mode_of_operation = 0x01;

               motor_rxpdos[motor]->profile_velocity = 100000;

            }

         }

         else if (arr_has_moved_to_start_offset[motor] && !arr_is_homing_done[motor])

         {

            /* Homming */

            int homing_error = TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_HM_HOMING_ERROR);

            int homing_attained = TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_HM_HOMING_ATTAINED);

            int target_reached = TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_HM_TARGET_REACHED);


            if (!target_reached)

            {

               printf_debug("[DEBUG] Motor:%d, Homing is ongoing\n", motor);

               continue;

            }

            else if (homing_error)

            {

               printf("\033[1;31m[ERROR] Motor %d has failed to reach home position\033[0m\n", motor);

               ECAT2_shutdown();

               return;

            }

            else if (!homing_attained)

            {

               printf("[INFO] Motor:%d, Start homing. status word:0x%x, homing error:%d, homing attained:%d\n", motor, motor_txpdos[motor]->status_word, homing_error, homing_attained);

               motor_rxpdos[motor]->mode_of_operation = 0x06;

               motor_rxpdos[motor]->control_word = 0x001F;

            }

            else if (homing_attained)

            {

               printf("[INFO] Motor %d has reached home position\n", motor);

               arr_is_homing_done[motor] = true;

               motor_rxpdos[motor]->target_position = 0;

               motor_rxpdos[motor]->profile_velocity = 50000;

            }

         }


         /* Normal operation */

         if (is_homing_complete)

         {

            motor_rxpdos[motor]->control_word = 0x003F;

            motor_rxpdos[motor]->mode_of_operation = 0x01;

            motor_rxpdos[motor]->profile_velocity = 1500000;

         }


         // check if the motor is not following the target position

         if (motor_txpdos[motor]->position_actual_value != motor_rxpdos[motor]->target_position && motor_txpdos[motor]->velocity_actual_value == 0)

         {

            arr_move_fail_count[motor]++;

            if (arr_move_fail_count[motor] >= MOVE_FAIL_THRESHOLD)

            {

               printf("\033[1;31m[WARN] Motor %d seems not to follow the target position.\033[0m\n", motor);

               printf("[WARN] Motor:%d, Status Word:0x%x, Actual Pos:%d, Actual Vel:%d, Home sensor:0x%x, Err Code:0x%x, Mode of OP Disp:0x%x\n",

                      motor,

                      motor_txpdos[motor]->status_word,

                      motor_txpdos[motor]->position_actual_value,

                      motor_txpdos[motor]->velocity_actual_value,

                      motor_txpdos[motor]->digital_inputs,

                      motor_txpdos[motor]->error_code,

                      motor_txpdos[motor]->mode_of_operation_display);

               printf("\033[38;5;208m[WARN] Motor:%d, Control Word:0x%x, Target Position:%d, Profile Velocity:%d, Mode of Operation:0x%x\033[0m\n",

                            motor,

                            motor_rxpdos[motor]->control_word,

                            motor_rxpdos[motor]->target_position,

                            motor_rxpdos[motor]->profile_velocity,

                            motor_rxpdos[motor]->mode_of_operation);

               // print OD_VELOCITY_DEMAND_VALUE

               int32 velocity_demand_value = 0;

               read_sdo_s32(motor + 1, OD_VELOCITY_DEMAND_VALUE, 0x00, &velocity_demand_value);

               printf("[WARN] Motor:%d, Velocity Demand Value: %d\n", motor, velocity_demand_value);

            }

         }

         else

         {

            arr_move_fail_count[motor] = 0;

         }

      }

   }


   inOP = FALSE;

   ECAT2_shutdown();

}


OSAL_THREAD_FUNC ecatcheck(void *ptr)

{

   int slave;

   (void)ptr; /* Not used */


   while (1)

   {

      if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))

      {

         if (needlf)

         {

            needlf = FALSE;

            printf("\n");

         }

         /* one ore more slaves are not responding */

         ec_group[currentgroup].docheckstate = FALSE;

         ec_readstate();

         for (slave = 1; slave <= ec_slavecount; slave++)

         {

            if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))

            {

               ec_group[currentgroup].docheckstate = TRUE;

               if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))

               {

                  printf("[ERROR] : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);

                  ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);

                  ec_writestate(slave);

               }

               else if (ec_slave[slave].state == EC_STATE_SAFE_OP)

               {

                  printf("[WARN] : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);

                  ec_slave[slave].state = EC_STATE_OPERATIONAL;

                  ec_writestate(slave);

               }

               else if (ec_slave[slave].state > EC_STATE_NONE)

               {

                  if (ec_reconfig_slave(slave, EC_TIMEOUTMON))

                  {

                     ec_slave[slave].islost = FALSE;

                     printf("[INFO] : slave %d reconfigured\n", slave);

                  }

               }

               else if (!ec_slave[slave].islost)

               {

                  /* re-check state */

                  ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);

                  if (ec_slave[slave].state == EC_STATE_NONE)

                  {

                     ec_slave[slave].islost = TRUE;

                     printf("[ERROR] : slave %d lost\n", slave);

                  }

               }

            }

            if (ec_slave[slave].islost)

            {

               if (ec_slave[slave].state == EC_STATE_NONE)

               {

                  if (ec_recover_slave(slave, EC_TIMEOUTMON))

                  {

                     ec_slave[slave].islost = FALSE;

                     printf("[INFO] : slave %d recovered\n", slave);

                  }

               }

               else

               {

                  ec_slave[slave].islost = FALSE;

                  printf("[INFO] : slave %d found\n", slave);

               }

            }

         }

         if (!ec_group[currentgroup].docheckstate)

            printf("OK : all slaves resumed OPERATIONAL.\n");

      }

      osal_usleep(50000);

   }

}

