#include "ec-server/v2/ec_system.h"
#include "ec-server/v2/utils.h"

int homing_to_move_toggle = 0;

void ECAT2_view(){

   for(int motor = 0; motor < ec_slavecount; motor++){

      printf_debug("\033[38;5;208m[DEBUG] Motor:%d, Control Word:0x%x, Target Position:%d, Profile Velocity:%d, Mode of Operation:0x%x\033[0m\n",

                     motor,

                     motor_rxpdos[motor]->control_word,

                     motor_rxpdos[motor]->target_position,

                     motor_rxpdos[motor]->profile_velocity,

                     motor_rxpdos[motor]->mode_of_operation);



   printf_debug("[DEBUG] Motor:%d, Status Word:0x%x, Actual Pos:%d, Actual Vel:%d, Home sensor:0x%x, Err Code:0x%x, Mode of OP Disp:0x%x\n",

                  motor,

                  motor_txpdos[motor]->status_word,

                  motor_txpdos[motor]->position_actual_value,

                  motor_txpdos[motor]->velocity_actual_value,

                  motor_txpdos[motor]->digital_inputs,

                  motor_txpdos[motor]->error_code,

                  motor_txpdos[motor]->mode_of_operation_display);


   }
}


ECAT2_PHASE ECAT2_exchange(int motor){


   osal_usleep(50000);

   ec_send_processdata();

   wkc = ec_receive_processdata(EC_TIMEOUTRET);


   if (wkc == -1)

   {

      printf("\033[1;31m[ERROR] Workcounter not met (actual WKC:%d, expected WKC:%d)\033[0m\n", wkc, expectedWKC);

      return ECAT2_ABORT;

   }

   else if (wkc < expectedWKC)

   {

      printf("\033[1;31m[ERROR] Workcounter not met (actual WKC:%d, expected WKC:%d)\033[0m\n", wkc, expectedWKC);

      working_counter_fail_count++;

      return ECAT2_CONTINUE;

   }


   if (working_counter_fail_count >= WORKING_COUNTER_FAIL_THRESHOLD)

   {

      printf("\033[1;31m[ERROR] Workcounter fail count exceeds threshold. Exiting...\033[0m\n");

      return ECAT2_ABORT;

   }


   working_counter_fail_count = 0;


   return ECAT2_FORWARD;
}


ECAT2_PHASE ECAT2_sync_status(int motor){



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

         return ECAT2_ABORT;

      }

   }


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

         return ECAT2_CONTINUE;

      }

      /* xxxx xxxx x1xx 0000 STATUS_WORD_SWITCH_ON_DISABLED */

      else if ((base | (sw & (1 << 6))) == STATUS_WORD_SWITCH_ON_DISABLED)

      {

         printf("[INFO] Motor:%d, Switch on disabled\n", motor);


         /* Automatic translation (2) */

         motor_rxpdos[motor]->control_word = 0x6;


         return ECAT2_CONTINUE;

      }

      /* xxxx xxxx x01x 0001 STATUS_WORD_READY_TO_SWITCH_ON */

      else if ((base | (sw & (3 << 5))) == STATUS_WORD_READY_TO_SWITCH_ON)

      {

         printf("[INFO] Motor:%d, Ready to switch on\n", motor);


         /* transition 3 */

         motor_rxpdos[motor]->control_word = 0xf;


         return ECAT2_CONTINUE;

      }

      /*xxxx xxxx x011 0011 STATUS_WORD_SWITCHED_ON*/

      else if ((base | (sw & (7 << 4))) == STATUS_WORD_SWITCHED_ON)

      {

         printf("[INFO] Motor:%d, Switched on\n", motor);


         /* Enable operation command for transition (4) */

         motor_rxpdos[motor]->control_word = 0xf;


         return ECAT2_CONTINUE;

      }

      /* xxxx xxxx x00x 0111 STATUS_WORD_QUICK_STOP_ACTIVE */

      else if ((base | (sw & (3 << 5))) == STATUS_WORD_QUICK_STOP_ACTIVE)

      {

         printf("[INFO] Motor:%d, Quick stop active\n", motor);

         return ECAT2_CONTINUE;

      }

      /* xxxx xxxx x0xx 1111 STATUS_WORD_FAULT_REACTION_ACTIVE */

      else if ((base | (sw & (1 << 6))) == STATUS_WORD_FAULT_REACTION_ACTIVE)

      {

         printf("[INFO] Motor:%d, Fault reaction active\n", motor);

         return ECAT2_CONTINUE;

      }

      /* xxxx xxxx x0xx 1000 STATUS_WORD_FAULT */

      else if ((base | (sw & (1 << 6))) == STATUS_WORD_FAULT)

      {

         printf("[WARNING] Motor:%d, Fault detected\n", motor);


         /* Returning to Switch On Disabled (transition 15) */

         motor_rxpdos[motor]->control_word = (1 << BIT_CONTROL_WORD_FAULTRESET);


         return ECAT2_CONTINUE;

      }

      else

      {

         printf("[ERROR] Motor:%d, Unknown status word:0x%x\n", motor, motor_txpdos[motor]->status_word);

         return ECAT2_ABORT;

      }

   }


   if (motor_txpdos[motor]->mode_of_operation_display == MODE_PP && TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_PP_SETPOINT_ACKNOWLEDGE))

   {

      // reset new_set_point to 0 in profile position mode

      motor_rxpdos[motor]->control_word = CONTROL_WORD_PP_NEW_SET;

      motor_rxpdos[motor]->mode_of_operation = MODE_PP;

      return ECAT2_CONTINUE;

   }


   return ECAT2_FORWARD;


}


ECAT2_PHASE ECAT2_homing(int motor){


   if (!has_moved_to_start_offset){

      int is_start_offset_complete = 1;

      for(int i = 0; i < g_num_of_slaves; i ++){

         if(!arr_has_moved_to_start_offset[i]){

            is_start_offset_complete = 0;

            break;

         }

      }

      if(is_start_offset_complete){

         has_moved_to_start_offset = 1;

      }

   } 

   if (_PHASE_ == ECAT2_HOMING)
   {

      int is_homing_complete = 1;

      for(int i = 0; i < g_num_of_slaves; i ++){

         if(!arr_is_homing_done[i]){

            is_homing_complete = 0;

            break;

         }

      }

      if(is_homing_complete){

         _PHASE_ = ECAT2_MOVING;

         phase_toggle = 1;

         printf("[INFO] All motors have completed homing\n");



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

         arr_has_moved_to_start_offset[motor] = 1;

      }

      else

      {

         motor_rxpdos[motor]->control_word = CONTROL_WORD_PP_CHANGE_SET;
         motor_rxpdos[motor]->mode_of_operation = MODE_PP;

         motor_rxpdos[motor]->profile_velocity = VELOCITY_TO_START_OFFSET;

      }

   } else if (arr_has_moved_to_start_offset[motor] && !arr_is_homing_done[motor]){

      /* Homming */

      int homing_error = TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_HM_HOMING_ERROR);

      int homing_attained = TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_HM_HOMING_ATTAINED);

      int target_reached = TEST_BIT(motor_txpdos[motor]->status_word, BIT_STATUS_WORD_HM_TARGET_REACHED);


      if (!target_reached)

      {

         printf_debug("[DEBUG] Motor:%d, Homing is ongoing\n", motor);

         return ECAT2_CONTINUE;

      }

      else if (homing_error)

      {

         printf("\033[1;31m[ERROR] Motor %d has failed to reach home position\033[0m\n", motor);

         return ECAT2_ABORT;

      }

      else if (!homing_attained)

      {

         printf("[INFO] Motor:%d, Start homing. status word:0x%x, homing error:%d, homing attained:%d\n", motor, motor_txpdos[motor]->status_word, homing_error, homing_attained);

         motor_rxpdos[motor]->mode_of_operation = MODE_HM;

         motor_rxpdos[motor]->control_word = CONTROL_WORD_HM_OP_START;

      }

      else if (homing_attained)

      {

         printf("[INFO] Motor %d has reached home position\n", motor);

         arr_is_homing_done[motor] = 1;

         motor_rxpdos[motor]->target_position = 0;

         motor_rxpdos[motor]->profile_velocity = 50000;

      }

   }

   return ECAT2_FORWARD;

}

ECAT2_PHASE ECAT2_moving(int motor){


   if(phase_toggle){

      if (write_sdo_s32(motor + 1, OD_SOFTWARE_LIMIT, 0x01, -200000) == 0)

      {

         printf("\033[1;31m[ERROR] Failed to set position min. limit for motor %d\033[0m\n", motor);

         return ECAT2_ABORT;

      }

      if (write_sdo_s32(motor + 1, OD_SOFTWARE_LIMIT, 0x02, 200000) == 0)

      {

         printf("\033[1;31m[ERROR] Failed to set position max. limit for motor %d\033[0m\n", motor);

         return ECAT2_ABORT;

      }

      phase_toggle = 0;
   }


   motor_rxpdos[motor]->control_word = CONTROL_WORD_PP_CHANGE_SET;
   motor_rxpdos[motor]->mode_of_operation = MODE_PP;

   motor_rxpdos[motor]->profile_velocity = 1500000;


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




   return ECAT2_FORWARD;

}