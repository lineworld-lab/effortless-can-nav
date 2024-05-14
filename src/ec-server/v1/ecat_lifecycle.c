#include "ec-server/v1/ecat_lifecycle.h"


int EClife_init(EthercatLifeCycle* eclc){

    eclc->ecat_node_ = (EthercatNode*)malloc(sizeof(EthercatNode));

    memset(eclc->ecat_node_, 0, sizeof(EthercatNode));

#if CYCLIC_POSITION_MODE
    // for syncronous cyclic position mode
    
    sent_data_.control_word.resize(g_kNumberOfServoDrivers);
    sent_data_.target_pos.resize(g_kNumberOfServoDrivers);
    sent_data_.digital_out.resize(g_kNumberOfServoDrivers);
    sent_data_.touch_probe_func.resize(g_kNumberOfServoDrivers);

    received_data_.status_word.resize(g_kNumberOfServoDrivers);
    received_data_.actual_pos.resize(g_kNumberOfServoDrivers);
    received_data_.digital_in.resize(g_kNumberOfServoDrivers);
    received_data_.error_code.resize(g_kNumberOfServoDrivers);
    received_data_.touch_probe_stat.resize(g_kNumberOfServoDrivers);
    received_data_.touch_probe_1_pval.resize(g_kNumberOfServoDrivers);
    received_data_.touch_probe_2_pval.resize(g_kNumberOfServoDrivers);
#endif

#if POSITION_MODE
    // for position mode

    memset(eclc->sent_data_.control_word, 0, sizeof(uint16_t) * g_kNumberOfServoDrivers);
    memset(eclc->sent_data_.target_pos, 0, sizeof(int32_t) * g_kNumberOfServoDrivers);
    memset(eclc->sent_data_.profile_vel, 0, sizeof(uint32_t) * g_kNumberOfServoDrivers);
    memset(eclc->sent_data_.digital_out, 0, sizeof(uint32_t) * g_kNumberOfServoDrivers);
    memset(eclc->sent_data_.op_mode, 0, sizeof(int8_t) * g_kNumberOfServoDrivers);
    memset(eclc->sent_data_.homing_method, 0, sizeof(int8_t) * g_kNumberOfServoDrivers);

    memset(eclc->received_data_.status_word, 0, sizeof(uint16_t) * g_kNumberOfServoDrivers);
    memset(eclc->received_data_.actual_pos, 0, sizeof(int32_t) * g_kNumberOfServoDrivers);
    memset(eclc->received_data_.actual_vel, 0, sizeof(int32_t) * g_kNumberOfServoDrivers);
    memset(eclc->received_data_.digital_in, 0, sizeof(uint32_t) * g_kNumberOfServoDrivers);
    memset(eclc->received_data_.error_code, 0, sizeof(uint16_t) * g_kNumberOfServoDrivers);
    memset(eclc->received_data_.op_mode_display, 0, sizeof(int8_t) * g_kNumberOfServoDrivers);


    eclc->emergency_status_ = 1;
    eclc->al_state_ = 0;
    eclc->command_ = 0x004F;

#endif

    return 0;

}

int EClife_destroy(EthercatLifeCycle* eclc){


    free(eclc->ecat_node_);

    return 0;

}


uint8_t EClife_on_configure(EthercatLifeCycle* eclc){

    printf("Configuring EtherCAT device...\n");
    if (EClife_InitEthercatCommunication(eclc))
    {
        printf("Configuration phase failed\n");
        return FAILURE;
    }
    else
    {
        return SUCCESS;
    }

}

uint8_t EClife_on_activate(EthercatLifeCycle* eclc){

    if (EClife_StartEthercatCommunication(eclc))
    {

        printf("Activation phase failed");
        return FAILURE;
    }
    else
    {
        printf("Activation complete, real-time communication started.");
        return SUCCESS;
    }


}


uint8_t EClife_on_deactivate(EthercatLifeCycle* eclc){


    printf("Deactivating.");

    ECnode_DeactivateCommunication(eclc->ecat_node_);

    return SUCCESS;
}


uint8_t EClife_on_cleanup(EthercatLifeCycle* eclc){



    printf("Cleaning up.");

    free(eclc->ecat_node_);

    return SUCCESS;

}



uint8_t EClife_on_shutdown(EthercatLifeCycle* eclc){

    printf("On_Shutdown... Waiting for control thread.");
    sig = 0;
    usleep(1e3);

    pthread_cancel(eclc->ethercat_thread_);

    ECnode_ReleaseMaster(eclc->ecat_node_);

    ECnode_ShutDownEthercatMaster(eclc->ecat_node_);

    return SUCCESS;
}


uint8_t EClife_on_kill(EthercatLifeCycle* eclc){

    printf("On_Kill...");
    sig = 0;
    ECnode_ReleaseMaster(eclc->ecat_node_);
    ECnode_ShutDownEthercatMaster(eclc->ecat_node_);
    printf("master killed.");
    return SUCCESS;

}


uint8_t EClife_on_error(EthercatLifeCycle* eclc){

    printf("On Error.");

    free(eclc->ecat_node_);

    return SUCCESS;
}

int EClife_SetComThreadPriorities(EthercatLifeCycle* eclc){


    eclc->ethercat_sched_param_.sched_priority = 98;

    printf("Using priority %i\n.", eclc->ethercat_sched_param_.sched_priority);


    if (sched_setscheduler(0, SCHED_FIFO, &(eclc->ethercat_sched_param_)) == -1)
    {
        printf("Set scheduler failed. ! ");
        return -1;
    }
    eclc->err_ = pthread_attr_init(&(eclc->ethercat_thread_attr_));
    if (eclc->err_)
    {
        printf("Error initializing thread attribute  ! ");
        return -1;
    }

    /**********************************************************************************************/
    // This part is for CPU isolation to dedicate one or two cores for EtherCAT communication.
    // for this feature to be active fist you have to modify GRUB_CMDLINE_LINUX_DEFAULT in /etc/default/grub
    // add isolcpus=4,5 so after editing it will be ; GRUB_CMDLINE_LINUX_DEFAULT = "quiet splash isolcpus=4,5"
    // save and exit, and type sudo update-grub and reboot.

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(4, &mask);
    CPU_SET(5, &mask);

    int result = sched_setaffinity(0, sizeof(mask), &mask);
    /**********************************************************************************************/

    eclc->err_ = pthread_attr_setstacksize(&(eclc->ethercat_thread_attr_), 4096 * 64);

    if(eclc->err_){
        printf("Error setting thread stack size  ! ");
        return -1;

    }

    eclc->err_ = pthread_attr_setschedpolicy(&(eclc->ethercat_thread_attr_), SCHED_FIFO);
    if (eclc->err_)
    {
        printf("Pthread setschedpolicy failed ! ");
        return -1;
    }


    eclc->err_ = pthread_attr_setschedparam(&(eclc->ethercat_thread_attr_), &(eclc->ethercat_sched_param_));
    if (eclc->err_)
    {
        printf("Pthread setschedparam failed ! ");
        return -1;
    }

    eclc->err_ = pthread_attr_setinheritsched(&(eclc->ethercat_thread_attr_), PTHREAD_EXPLICIT_SCHED);
    if (eclc->err_)
    {
        printf("Pthread setinheritsched failed ! ");
        return -1;
    }


}


int EClife_InitEthercatCommunication(EthercatLifeCycle* eclc){


    printf("Opening EtherCAT device...\n");

    if(ECnode_OpenEthercatMaster(eclc->ecat_node_)){


        return -1;
    }

    printf("Configuring EtherCAT master...\n");

    if(ECnode_ConfigureMaster(eclc->ecat_node_)){

        return -1;
    }

    printf("Getting connected slave informations...\n");

    if(ECnode_GetNumberOfConnectedSlaves(eclc->ecat_node_)){

        return -1;
    }

    ECnode_GetAllSlaveInformation(eclc->ecat_node_);

    for (int i = 0; i < NUM_OF_SLAVES; i++)
    {
        printf("--------------------Slave Info -------------------------\n"
               "Slave alias         = %d\n "
               "Slave position      = %d\n "
               "Slave vendor_id     = 0x%08x\n "
               "Slave product_code  = 0x%08x\n "
               "Slave name          = %s\n "
               "--------------------EOF %d'th Slave Info ----------------\n ",
               eclc->ecat_node_->slaves_[i].slave_info_.alias,
               eclc->ecat_node_->slaves_[i].slave_info_.position,
               eclc->ecat_node_->slaves_[i].slave_info_.vendor_id,
               eclc->ecat_node_->slaves_[i].slave_info_.product_code,
               eclc->ecat_node_->slaves_[i].slave_info_.name, i);
    }

    printf("Configuring  slaves...\n");

    if(ECnode_ConfigureSlaves(eclc->ecat_node_)){

        return -1;
    }

    if(EClife_SetConfigurationParameters(eclc)){

        printf("Configuration parameters set failed\n");

        return -1;

    }

    printf("Mapping default PDOs...\n");

    if(ECnode_MapDefaultPdos(eclc->ecat_node_)){

        return -1;
    }

    if (DISTRIBUTED_CLOCK)
    {
        printf("Configuring DC synchronization...\n");
        ECnode_ConfigDcSyncDefault(eclc->ecat_node_);
    }

    printf("Activating master...\n");

    if(ECnode_ActivateMaster(eclc->ecat_node_)){

        return -1;
    }

    printf("Registering master domain...\n");

    if(ECnode_RegisterDomain(eclc->ecat_node_)){

        return -1;
    }

    if(ECnode_WaitForOperationalMode(eclc->ecat_node_)){


        return -1;
    }

#if PREEMPT_RT_MODE
    if (EClife_SetComThreadPriorities(eclc))
    {
        return -1;
    }
#endif
    printf("Initialization succesfull...\n");
    return 0;

}

int EClife_SetConfigurationParameters(EthercatLifeCycle* eclc){


#if POSITION_MODE
    ProfilePosParam P;
    uint32_t max_fol_err;
    P.profile_vel = 10000; //3000;
    P.profile_acc = 1000000; //1e6;
    P.profile_dec = 1000000; //1e6;
    P.max_profile_vel = 2500000; //1e5;
    P.homing_speed_switch = 1000000;
    P.homing_speed_zero = 20000;

    int32_t HomingOffsetSwitch[NUM_OF_SLAVES] = {
        400000 - 335000,
        300000,
        392520,
        400000 - 145000
    };
    
    for(int i = 0 ; i < g_kNumberOfServoDrivers; i++){

        P.homing_offset_switch[i] = HomingOffsetSwitch[i];

    }

    P.quick_stop_dec = 3e4;
    P.motion_profile_type = 0;
    
    return ECnode_SetProfilePositionParametersAll(eclc->ecat_node_, &P);
#endif



    return 0;
}


int EClife_StartEthercatCommunication(EthercatLifeCycle* eclc){

    eclc->err_ = pthread_create(&(eclc->ethercat_thread_), &(eclc->ethercat_thread_attr_), EClife_PassCycylicExchange, eclc);

    if(eclc->err_){

        printf("Error : Couldn't start communication thread.!\n");
        return -1;
    }
    printf("Communication thread called.\n");
    return 0;
}


void* EClife_PassCycylicExchange(void *arg){

    EthercatLifeCycle* eclc = (EthercatLifeCycle*)arg;

    EClife_StartPdoExchange(eclc);
}


void EClife_StartPdoExchange(EthercatLifeCycle* eclc){


    printf("Starting PDO exchange....\n");
    uint32_t print_val = 1e4;
    int error_check = 0;
    uint8_t sync_ref_counter = 1;
    struct timespec wake_up_time, time, publish_time_start = {}, publish_time_end = {};
    int begin = 1e4;
    int status_check_counter = 1000;

    printf("Enabling motors...\n");
    // get current time
    clock_gettime(CLOCK_TO_USE, &wake_up_time);
    while (sig){

        // Sleep for 1 ms
        wake_up_time = timespec_add(wake_up_time, g_cycle_time);
        
        clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wake_up_time, NULL);
        
        ecrt_master_application_time(g_master, TIMESPEC2NS(wake_up_time));
        
        // Receive process data
        ecrt_master_receive(g_master);
        ecrt_domain_process(g_master_domain);

        EClife_ReadFromSlaves(eclc);

        for (int i = 0; i < g_kNumberOfServoDrivers; i++)
        {
            eclc->sent_data_.target_pos[i] = eclc->received_data_.actual_pos[i];
            //sent_data_.target_vel[i] = 0;
        }

        if(EClife_EnableDrivers(eclc) == g_kNumberOfServoDrivers){
            printf("All drives enabled\n");
            break;
        }

        if (status_check_counter){
            status_check_counter--;
        }
        else
        {
            // Checking master/domain/slaves state every 1sec.
            if (ECnode_CheckMasterState(eclc->ecat_node_) < 0)
            {
                printf("Connection error, check your physical connection.\n");
                eclc->al_state_ = g_master_state.al_states;
                //      received_data_.emergency_switch_val=0;
                eclc->emergency_status_ = 0;
                error_check++;
                if (error_check == 5){

                    return;

                }



            }
            else
            {
                // ecat_node_->CheckMasterDomainState();
                // ecat_node_->CheckSlaveConfigurationState();
                error_check = 0;
                eclc->al_state_ = g_master_state.al_states;
                status_check_counter = 1000;

                for (int i = 0; i < g_kNumberOfServoDrivers; i++)
                {
                    printf("State of Drive %d : %d\n", i, eclc->motor_state_[i]);
                    printf("Trying to enable motors\n");
                }
            }
        }

#if POSITION_MODE
        EClife_WriteToSlavesInPositionMode(eclc);
#endif

        // Sync Timer
        if (sync_ref_counter)
        {
            sync_ref_counter--;
        }
        else
        {
            sync_ref_counter = 1; // sync every cycle

            clock_gettime(CLOCK_TO_USE, &time);
            ecrt_master_sync_reference_clock_to(g_master, TIMESPEC2NS(time));
        }
        ecrt_master_sync_slave_clocks(g_master);

        ecrt_domain_queue(g_master_domain);
        // Send process data
        ecrt_master_send(g_master);
    } // while(sig)
    printf("All motors enabled, entering control loop\n");

    while (sig)
    {
        wake_up_time = timespec_add(wake_up_time, g_cycle_time);
        clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wake_up_time, NULL);
        ecrt_master_application_time(g_master, TIMESPEC2NS(wake_up_time));

        // receive process data
        ecrt_master_receive(g_master);
        ecrt_domain_process(g_master_domain);


        if (status_check_counter)
        {
            status_check_counter--;
        }
        else
        {
            // Checking master/domain/slaves state every 1sec.
            if (ECnode_CheckMasterState(eclc->ecat_node_) < 0)
            {
                printf("Connection error, check your physical connection.\n");
                eclc->al_state_ = g_master_state.al_states;
                //      received_data_.emergency_switch_val=0;
                eclc->emergency_status_ = 0;
                // PublishAllData();
                error_check++;
                if (error_check == 5){

                    raise(SIGUSR1);
                    return;


                }

            }
            else
            {
                // ecat_node_->CheckMasterDomainState();
                // ecat_node_->CheckSlaveConfigurationState();
                error_check = 0;
                eclc->al_state_ = g_master_state.al_states;
                status_check_counter = 1000;
            }
        }

        EClife_ReadFromSlaves(eclc);
        for (int i = 0; i < g_kNumberOfServoDrivers; i++)
        {
            if (eclc->received_data_.error_code[i] != 0)
            {
                printf("Drive in error state\n");
                //GetErrorMessage(eclc->received_data_.error_code[i]);
            }
        }

#if POSITION_MODE
        EClife_UpdatePositionModeParameters(eclc);
        EClife_WriteToSlavesInPositionMode(eclc);
#endif

        ecrt_domain_queue(g_master_domain);

        if (sync_ref_counter)
        {
            sync_ref_counter--;
        }
        else
        {
            sync_ref_counter = 1; // sync every cycle

            clock_gettime(CLOCK_TO_USE, &time);
            ecrt_master_sync_reference_clock_to(g_master, TIMESPEC2NS(time));
        }
        ecrt_master_sync_slave_clocks(g_master);
        // send process data
        ecrt_master_send(g_master);
        if (begin){
            begin--;
        }

    }

    wake_up_time = timespec_add(wake_up_time, g_cycle_time);
    clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &wake_up_time, NULL);
    ecrt_master_application_time(g_master, TIMESPEC2NS(wake_up_time));

    ecrt_master_receive(g_master);
    ecrt_domain_process(g_master_domain);

    EClife_ReadFromSlaves(eclc);
    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {
        eclc->sent_data_.control_word[i] = SM_GO_SWITCH_ON_DISABLE;
    }
    EClife_WriteToSlavesVelocityMode(eclc);

    ecrt_domain_queue(g_master_domain);
    ecrt_master_send(g_master);
    usleep(10000);
    // ------------------------------------------------------- //

    printf("Leaving control thread.\n");
    ECnode_DeactivateCommunication(eclc->ecat_node_);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cancel(eclc->ethercat_thread_);
    pthread_exit(NULL);
    return;

}

void EClife_ReadFromSlaves(EthercatLifeCycle* eclc){

    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {

#if POSITION_MODE
        // for position mode

        eclc->received_data_.status_word[i] = EC_READ_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.status_word);
        eclc->received_data_.actual_pos[i] = EC_READ_S32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.actual_pos);
        eclc->received_data_.actual_vel[i] = EC_READ_S32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.actual_vel);
        eclc->received_data_.digital_in[i] = EC_READ_U32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.digital_in);
        eclc->received_data_.error_code[i] = EC_READ_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.error_code);
        eclc->received_data_.op_mode_display[i] = EC_READ_U8(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.op_mode_display);


#endif
    }
    eclc->received_data_.com_status = eclc->al_state_;


}

int EClife_GetComState(EthercatLifeCycle* eclc){

    return eclc->al_state_;

}

#if POSITION_MODE

void EClife_UpdatePositionModeParameters(EthercatLifeCycle* eclc){

    printf("Updating position mode parameters\n");

    EClife_UpdateMotorStatePositionMode(eclc);

    /// WRITE YOUR CUSTOM CONTROL ALGORITHM, VARIABLES DECLARATAION HERE, LIKE IN EXAMPLE BELOW.
    /// KEEP IN MIND THAT YOU WILL HAVE TO WAIT FOR THE MOTION TO FINISH IN POSITION MODE, THEREFORE
    /// YOU HAVE TO CHECK 10th BIT OF STATUS WORD TO CHECK WHETHER TARGET IS REACHED OR NOT.

    if(eclc->homed_all_ != 1){


        EClife_UpdateHomeStatePositionMode(eclc);


    } else{

        EClife_UpdateMoveStatePositionMode(eclc);

    }

}


#endif

void EClife_UpdateMotorStatePositionMode(EthercatLifeCycle* eclc){

    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {
        if ((eclc->received_data_.status_word[i] & eclc->command_) == 0X08)
        {
            // if status is fault, reset fault state.
            eclc->command_ = 0X04F;
            eclc->sent_data_.control_word[i] = SM_FULL_RESET;
            eclc->motor_state_[i] = kFault;

        }
        if (eclc->motor_state_[i] != kOperationEnabled)
        {
            eclc->sent_data_.control_word[i] = SM_GO_READY_TO_SWITCH_ON;
            if ((eclc->received_data_.status_word[i] & eclc->command_) == 0x0040)
            {
                // If status is "Switch on disabled", \
                change state to "Ready to switch on"
                eclc->sent_data_.control_word[i] = SM_GO_READY_TO_SWITCH_ON;
                eclc->command_ = 0x006f;
                eclc->motor_state_[i] = kSwitchOnDisabled;
     
            }
            else if ((eclc->received_data_.status_word[i] & eclc->command_) == 0x0021)
            {
                // If status is "Ready to switch on", \
                        change state to "Switched on"
                eclc->sent_data_.control_word[i] = SM_GO_SWITCH_ON;
                eclc->command_ = 0x006f;
                eclc->motor_state_[i] = kReadyToSwitchOn;
  
            }
            else if ((eclc->received_data_.status_word[i] & eclc->command_) == 0x0023)
            {
                // If status is "Switched on", change state to "Operation enabled"
                eclc->sent_data_.control_word[i] = SM_GO_ENABLE;
                eclc->command_ = 0x006f;
                eclc->motor_state_[i] = kSwitchedOn;
      
            }
            else if ((eclc->received_data_.status_word[i] & eclc->command_) == 0X08)
            {
                // if status is fault, reset fault state.
                eclc->command_ = 0X04f;

                eclc->sent_data_.control_word[i] = SM_FULL_RESET;
                eclc->motor_state_[i] = kFault;

            }
        }
        else
        {


            if(eclc->homed_all_ != 1){
                int hit = 0;

                for (int i = 0; i < g_kNumberOfServoDrivers; i++){
                    if(eclc->homed_[i] == 1){
                        hit += 1;
                    }
                }

                if(hit == g_kNumberOfServoDrivers){
                    eclc->homed_all_ =1;
                }        
            }



            if (eclc->homed_all_ != 1){

                eclc->sent_data_.control_word[i] = 0x1F;
                
                int bit_10 = TEST_BIT(eclc->received_data_.status_word[i], 10);
                
                int bit_12 = TEST_BIT(eclc->received_data_.status_word[i], 12);

                int bit_13 = TEST_BIT(eclc->received_data_.status_word[i], 13);


                if(
                    bit_10 == 1 
                    && bit_12 == 1
                    && bit_13 == 0
                ){
                    eclc->homed_[i] = 1;
                } else {
                    eclc->homed_[i] = 0;
                }

            } else {

                eclc->sent_data_.control_word[i] = SM_EXPEDITE;

                eclc->new_set_pos_[i] = TEST_BIT(eclc->received_data_.status_word[i],12);

                if(eclc->new_set_pos_[i] == 1){
                    eclc->sent_data_.control_word[i] = 0x2F;
                }


            }

        }
    }


    return;

}

void EClife_UpdateHomeStatePositionMode(EthercatLifeCycle* eclc){




    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {
        if (eclc->motor_state_[i] == kOperationEnabled ||
            eclc->motor_state_[i] == kTargetReached || eclc->motor_state_[i] == kSwitchedOn)
        {

            printf("axis: %d\n", i);
            printf("home: ap: %d, op mode: %d, homed: %d\n", eclc->received_data_.actual_pos[i], eclc->received_data_.op_mode_display[i], eclc->homed_[i]);
            //GetErrorMessage(eclc->received_data_.error_code[2]);

            eclc->feedback_position_[i] = eclc->received_data_.actual_pos[i];
            eclc->op_mode_[i] = eclc->received_data_.op_mode_display[i];

            eclc->sent_data_.homing_method[i] = 24;
            eclc->sent_data_.op_mode[i] = 6;


        } else {
            printf("axis: %d skipped due to an error\n", i);
            printf("home: ap: %d, op mode: %d, homed: %d\n", eclc->received_data_.actual_pos[i], eclc->received_data_.op_mode_display[i], eclc->homed_[i]);
            //GetErrorMessage(eclc->received_data_.error_code[2]);

            raise(SIGUSR1);
        }
    }
    return;

}


void EClife_UpdateMoveStatePositionMode(EthercatLifeCycle* eclc){



    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {
        if (eclc->motor_state_[i] == kOperationEnabled ||
            eclc->motor_state_[i] == kTargetReached || eclc->motor_state_[i] == kSwitchedOn)
        {

            printf("axis: %d\n", i);
            printf("move: ap: %d, op mode: %d, homed: %d\n", eclc->received_data_.actual_pos[i], eclc->received_data_.op_mode_display[i], eclc->homed_[i]);
            //GetErrorMessage(eclc->received_data_.error_code[2]);

            eclc->feedback_position_[i] = eclc->received_data_.actual_pos[i];
            eclc->op_mode_[i] = eclc->received_data_.op_mode_display[i];

            eclc->sent_data_.target_pos[i] = eclc->posted_position_[i];
            eclc->sent_data_.profile_vel[i] = 1000000;
            eclc->sent_data_.op_mode[i] = 1;


        } else {
            printf("axis: %d skipped due to an error\n", i);
            printf("move: ap: %d, op mode: %d, homed: %d\n", eclc->received_data_.actual_pos[i], eclc->received_data_.op_mode_display[i], eclc->homed_[i]);
            //GetErrorMessage(eclc->received_data_.error_code[2]);
            
            raise(SIGUSR1);

        }
    }


    return;



}


int EClife_GetDriveState(const int statusWord){




    int state = 0;

    // bit 6 is 1
    if (TEST_BIT(statusWord, 6))
    {
        state = kSwitchOnDisabled;
        return state;
    }

    // bit 6 is 0 and bit 5 is 1
    if (TEST_BIT(statusWord, 5))
    {
        if (TEST_BIT(statusWord, 2))
        {
            state = kOperationEnabled;
            return state;
        }
        if (TEST_BIT(statusWord, 1))
        {
            state = kSwitchedOn;
            return state;
        }
        if (TEST_BIT(statusWord, 0))
        {
            state = kReadyToSwitchOn;
            return state;
        }
    }

    // bit 6 is 0 and bit 5 is 0
    if (TEST_BIT(statusWord, 3))
    {
        // For EPOS4, Fault or Fault Reaction Active,
        // See P2-14 of the Firmware Manual
        state = kFault;
        return state;
    }
    else
    {
        // For EPOS4, Quick Stop Active or Not Switched on
        // See P2-14 of the Firmware Manual
        state = kQuickStop;
        return state;
    }
    return state;

}

int EClife_EnableMotors(EthercatLifeCycle* eclc){

    int cnt = 0;
    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {

        eclc->sent_data_.control_word[i] = 0xF;

        cnt += 1;
    }
    return cnt;


}


int EClife_EnableDrivers(EthercatLifeCycle* eclc){


    int cnt = 0;
    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {
        eclc->motor_state_[i] = EClife_GetDriveState(eclc->received_data_.status_word[i]);

        // if status is fault, reset fault state.
        if (eclc->motor_state_[i] == kFault)
        {
            // printf( "Driver %d in Fault",i);
            eclc->sent_data_.control_word[i] = SM_FULL_RESET;
        }

        // If status is "Switch on disabled", change state to "Ready to switch on"
        if (eclc->motor_state_[i] == kSwitchOnDisabled)
        {
            eclc->sent_data_.control_word[i] = SM_GO_READY_TO_SWITCH_ON;
        }

        // If status is "Ready to switch on", change state to "Switched on"
        if (eclc->motor_state_[i] == kReadyToSwitchOn)
        {
            eclc->sent_data_.control_word[i] = SM_GO_SWITCH_ON;
        }

        // If status is "Switched on", change state to "Operation enabled"
        if (eclc->motor_state_[i] == kSwitchedOn)
        {
            eclc->sent_data_.control_word[i] = SM_GO_ENABLE;
        }

        // If status is "Switched on", change state to "Operation enabled"
        if (eclc->motor_state_[i] == kOperationEnabled)
        {
            cnt++;
        }


    }
    return cnt;


}


void EClife_WriteToSlavesInPositionMode(EthercatLifeCycle* eclc){

    for (int i = 0; i < g_kNumberOfServoDrivers; i++)
    {

#if POSITION_MODE     
        
        EC_WRITE_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.control_word, eclc->sent_data_.control_word[i]);
        EC_WRITE_S32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.target_pos, eclc->sent_data_.target_pos[i]);
        EC_WRITE_U32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.profile_vel, eclc->sent_data_.profile_vel[i]);
        EC_WRITE_S8(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.op_mode, eclc->sent_data_.op_mode[i]);                   

        EC_WRITE_S8(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.homing_method, eclc->sent_data_.homing_method[i]); 

#endif
    }

}

void EClife_WriteToSlavesVelocityMode(EthercatLifeCycle* eclc){


    eclc->emergency_status_ = 1;
    if (!eclc->emergency_status_)
    {
        for (int i = 0; i < g_kNumberOfServoDrivers; i++)
        {
            EC_WRITE_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.control_word, eclc->sent_data_.control_word[i]);
            EC_WRITE_S32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.target_vel, 0);
        }
    }
    else
    {
        for (int i = 0; i < g_kNumberOfServoDrivers; i++)
        {
            EC_WRITE_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.control_word, eclc->sent_data_.control_word[i]);
            //    EC_WRITE_S32(ecat_node_->slaves_[i].slave_pdo_domain_ + ecat_node_->slaves_[i].offset_.target_vel,sent_data_.target_vel[i]);
        }
    }

}





int GetHomingStatusByAxis(char* res, int axis){


    int op_mode = (int)ECAT_LIFECYCLE_NODE->op_mode_[axis];

    int feedback = (int)ECAT_LIFECYCLE_NODE->feedback_position_[axis];

    char status_str[10] = {0};
    /*

    if(op_mode != 1 && feedback != 0){

        status_str = std::to_string(0);

    } else if (op_mode != 1 && feedback == 0){

        status_str = std::to_string(-1);

    } else if (op_mode == 1 && feedback != 0){

        status_str = std::to_string(-2);

    } else if (op_mode == 1 && feedback == 0){

        status_str = std::to_string(1);

    } 

    */


   if(op_mode == 1 && feedback == 0){
  
        strcpy(status_str, "1");

   } else {

        strcpy(status_str, "0");
   }


    strcpy(res, status_str);


    return 0;
}

int PostHomeShiftByAxis(char* res, int axis, int shift){




    return 0;
}


int PostPositionByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    ECAT_LIFECYCLE_NODE->posted_position_[axis] = pos_32;

    strcpy(res, "0");

    return 0;
}


int PostPositionWithFeedbackByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    ECAT_LIFECYCLE_NODE->posted_position_[axis] = pos_32;

    int feedback = (int)ECAT_LIFECYCLE_NODE->feedback_position_[axis];

    char feedback_str[MAX_POSITION_STRLEN] = {0};

    sprintf(feedback_str, "%d", feedback);

    strcpy(res, feedback_str);

    return 0;
}




int PostPositionWithStatusFeedbackByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    ECAT_LIFECYCLE_NODE->posted_position_[axis] = pos_32;

    int status_int = (int)ECAT_LIFECYCLE_NODE->op_mode_[axis];

    int feedback_int = (int)ECAT_LIFECYCLE_NODE->feedback_position_[axis];

    char status_str[MAX_STATUS_STRLEN] = {0};
    char feedback_str[MAX_POSITION_STRLEN] = {0};

    sprintf(status_str, "%d", status_int);

    sprintf(feedback_str, "%d", feedback_int);

    strcat(res, status_str);

    strcat(res, " ");

    strcat(res, feedback_str);

    return 0;
}