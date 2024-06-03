#include "ec-server/v1/ec_system.h"
#include "ec-server/v1/utils.h"

int EC_SLAVE_check_slave_conf_state(EC_SLAVE* ecs){


    ecrt_slave_config_state(ecs->slave_config_, &ecs->slave_config_state_);
    if (ecs->slave_config_state_.al_state != 0x08) {
        printf(" Slave is not operational AL state is : %x\n", ecs->slave_config_state_.al_state); 
    }
    return 0;
}




int EC_NODE_conf_master(EC_NODE* ecn){


    g_master = ecrt_request_master(0);    
    if (!g_master) {
        
        printf( "Requesting master instance failed ! ");
        return -1 ;
    }

    g_master_domain = ecrt_master_create_domain(g_master);
    if(!g_master_domain) {
        printf( "Failed to create master domain ! ");
        return -1 ;
    }
    return 0 ;



}


void EC_NODE_get_all_slaves_info(EC_NODE* ecn){

    for(int i=0;i < g_drivers_num ; i++){
        ecrt_master_get_slave(g_master, i , &(ecn->slaves_[i].slave_info_));
    }

}


int EC_NODE_conf_slaves(EC_NODE* ecn){


    for(int i = 0 ; i < g_drivers_num ; i++ ){
        ecn->slaves_[i].slave_config_ = ecrt_master_slave_config(g_master,ecn->slaves_[i].slave_info_.alias,
                                                                     ecn->slaves_[i].slave_info_.position,
                                                                     ecn->slaves_[i].slave_info_.vendor_id,
                                                                     ecn->slaves_[i].slave_info_.product_code); 
        if(!(ecn->slaves_[i].slave_config_)) {
        printf( "Failed to  configure slave ! ");
            return -1;
        }
    }

    return 0 ;

}

int EC_NODE_map_default_pdo(EC_NODE* ecn){


#if POSITION_MODE
        // for position mode
    ec_pdo_entry_info_t target_pdo_entries[12] = {
        {OD_CONTROL_WORD, 16},
        {OD_TARGET_POSITION, 32},
        {OD_PROFILE_VELOCITY, 32},
        {OD_DIGITAL_OUTPUTS, 32},
        {OD_OPERATION_MODE, 8},

        {OD_HOMING_METHOD, 8},
        
        {OD_STATUS_WORD, 16},
        {OD_POSITION_ACTUAL_VALUE, 32},
        {OD_VELOCITY_ACTUAL_VALUE, 32},
        {OD_DIGITAL_INPUTS, 32},
        {OD_ERROR_CODE, 16},
        {OD_OPERATION_MODE_DISPLAY, 8}

    };
#endif

#if POSITION_MODE
    // for position mode
    ec_pdo_info_t target_pdos[2] = {
        {0x1601, 6, target_pdo_entries + 0},    // - RxPDO index of --
        {0x1a01, 6, target_pdo_entries + 6}     // - TxPDO index of --
    };
#endif

    ec_sync_info_t target_syncs[5] = {
        {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
        {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
        {2, EC_DIR_OUTPUT, 1, target_pdos + 0, EC_WD_ENABLE},
        {3, EC_DIR_INPUT, 1, target_pdos + 1, EC_WD_DISABLE},
        {0xff}
    };

    // Connect sync_manager to corresponding slaves.
    for(int i = 0 ; i < g_drivers_num ; i++){
        if(ecrt_slave_config_pdos(ecn->slaves_[i].slave_config_,EC_END,target_syncs)){
            printf( "Slave PDO configuration failed... ");
            return -1;
        }
    }
#if POSITION_MODE
    for(int i = 0; i < g_drivers_num ; i++){

        ecn->slaves_[i].offset_.control_word     = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                  OD_CONTROL_WORD,g_master_domain,NULL);
        ecn->slaves_[i].offset_.target_pos       = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_TARGET_POSITION,g_master_domain,NULL);
        ecn->slaves_[i].offset_.profile_vel      = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_PROFILE_VELOCITY,g_master_domain,NULL);
        ecn->slaves_[i].offset_.digital_out     = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_DIGITAL_OUTPUTS,g_master_domain,NULL);
        ecn->slaves_[i].offset_.op_mode          = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_OPERATION_MODE,g_master_domain,NULL);

        ecn->slaves_[i].offset_.homing_method    = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_HOMING_METHOD,g_master_domain,NULL);

        ecn->slaves_[i].offset_.status_word      = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_STATUS_WORD,g_master_domain,NULL);
        ecn->slaves_[i].offset_.actual_pos       = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_POSITION_ACTUAL_VALUE,g_master_domain,NULL);
        ecn->slaves_[i].offset_.actual_vel       = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_VELOCITY_ACTUAL_VALUE,g_master_domain,NULL); 
        ecn->slaves_[i].offset_.digital_in       = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_DIGITAL_INPUTS,g_master_domain,NULL);
        ecn->slaves_[i].offset_.error_code       = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_ERROR_CODE,g_master_domain,NULL);
        ecn->slaves_[i].offset_.op_mode_display  = ecrt_slave_config_reg_pdo_entry(ecn->slaves_[i].slave_config_,
                                                                                    OD_OPERATION_MODE_DISPLAY,g_master_domain,NULL);
        if (ecn->slaves_[i].offset_.control_word < 0
        || ecn->slaves_[i].offset_.target_pos < 0
        || ecn->slaves_[i].offset_.profile_vel < 0
        || ecn->slaves_[i].offset_.digital_out < 0
        || ecn->slaves_[i].offset_.op_mode < 0
        || ecn->slaves_[i].offset_.homing_method < 0
        || ecn->slaves_[i].offset_.status_word < 0
        || ecn->slaves_[i].offset_.actual_pos < 0
        || ecn->slaves_[i].offset_.actual_vel < 0
        || ecn->slaves_[i].offset_.digital_in < 0
        || ecn->slaves_[i].offset_.error_code < 0
        || ecn->slaves_[i].offset_.op_mode_display < 0)
        {
            printf( "Failed to configure  PDOs for motors.!");
            return -1;
        }
#endif
    }

    return 0;
}

void EC_NODE_conf_dcsync_default(EC_NODE* ecn){

    for(int i=0; i < g_drivers_num ; i++){
        ecrt_slave_config_dc(ecn->slaves_[i].slave_config_, 0X0300, PERIOD_NS, ecn->slaves_[i].kSync0_shift_, 0, 0);
    }


}


int EC_NODE_activate_master(EC_NODE* ecn){


    if ( ecrt_master_activate(g_master) ) {
        printf( "Master activation error ! ");
        return -1 ;
    }
    return 0 ; 

}

int EC_NODE_register_domain(EC_NODE* ecn){


    for(int i = 0 ; i < g_drivers_num ; i++){
        ecn->slaves_[i].slave_pdo_domain_ = ecrt_domain_data(g_master_domain);
        if(!(ecn->slaves_[i].slave_pdo_domain_) )
        {
            printf( "Domain PDO registration error");
            return -1;
        }
    }
    return 0;

}

int EC_NODE_set_parameters_pp(EC_NODE* ecn, PROFILE_POS_PARAM* P, int position){



  // Operation mode to ProfilePositionMode for slave on that position.
    if( ecrt_slave_config_sdo8(ecn->slaves_[position].slave_config_,OD_OPERATION_MODE, K_PROFILE_POSITION) ){
        printf( "Set operation mode config error ! ");
        return  -1 ;
    }
    //profile velocity
    if(ecrt_slave_config_sdo32(ecn->slaves_[position].slave_config_,OD_PROFILE_VELOCITY, P->profile_vel) < 0) {
        printf( "Set profile velocity config error ! ");
        return -1;
    }
    //max profile velocity
    if(ecrt_slave_config_sdo32(ecn->slaves_[position].slave_config_,OD_MAX_PROFILE_VELOCITY,P->max_profile_vel) < 0) {
        printf( "Set max profile velocity config error !");
        return -1;
    }
    //profile acceleration
    if(ecrt_slave_config_sdo32(ecn->slaves_[position].slave_config_,OD_PROFILE_ACCELERATION, P->profile_acc) < 0) {
        printf( "Set profile acceleration failed ! ");
        return -1;
    }
    //profile deceleration
    if(ecrt_slave_config_sdo32(ecn->slaves_[position].slave_config_,OD_PROFILE_DECELERATION,P->profile_dec) < 0) {
        printf( "Set profile deceleration failed ! ");
        return -1;
    }
    // quick stop deceleration 
    if(ecrt_slave_config_sdo32(ecn->slaves_[position].slave_config_,OD_QUICK_STOP_DECELERATION,P->quick_stop_dec) < 0) {
        printf( "Set quick stop deceleration failed !");
        return -1;
    }
    // max following error 
    if(ecrt_slave_config_sdo32(ecn->slaves_[position].slave_config_,OD_MAX_FOLLOWING_ERROR,P->max_fol_err) < 0) {
        printf( "Set max following error failed ! ");
        return -1;
    }   
    return 0;


}


int EC_NODE_set_all_parameters_pp(EC_NODE* ecn, PROFILE_POS_PARAM* P){

    for(int i = 0 ; i < g_drivers_num ; i++)
    {
        // Set operation mode to ProfilePositionMode for all motors.
        if( ecrt_slave_config_sdo8(ecn->slaves_[i].slave_config_,OD_OPERATION_MODE, K_PROFILE_POSITION) ){
            printf( "Set operation mode config error ! ");
            return  -1 ;
        }
        //profile velocity
        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_PROFILE_VELOCITY, P->profile_vel) < 0) {
            printf( "Set profile velocity failed ! ");
            return -1;
        }
        //max profile velocity
        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_MAX_PROFILE_VELOCITY,P->max_profile_vel) < 0) {
            printf( "Set max profile velocity failed ! ");
            return -1;
        }
        //profile acceleration
        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_PROFILE_ACCELERATION, P->profile_acc) < 0) {
            printf( "Set profile acceleration failed ! ");
            return -1;
        }
        //profile deceleration
        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_PROFILE_DECELERATION,P->profile_dec) < 0) {
            printf( "Set profile deceleration failed ! ");
            return -1;
        }

        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_HOMING_SPEED_SWITCH,P->homing_speed_switch) < 0) {
            printf( "Set homing speed switch failed ! ");
            return -1;
        }

        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_HOMING_SPEED_ZERO,P->homing_speed_zero) < 0) {
            printf( "Set homing speed zero failed ! ");
            return -1;
        }

        if(ecrt_slave_config_sdo32(ecn->slaves_[i].slave_config_,OD_HOMING_OFFSET_SWITCH,P->homing_offset_switch[i]) < 0) {
            printf( "Set homing offset switch failed ! ");
            return -1;
        }

    }
    // print success message
    printf("Profile position mode parameters set successfully for all motors ! ");
    return 0; 

}

int EC_NODE_wait_for_op_mode(EC_NODE* ecn){

    int try_counter=0;
    int check_state_count=0;
    int time_out = 20e3;
    while (g_master_state.al_states != EC_AL_STATE_OP){
        if(try_counter < time_out){
            clock_gettime(CLOCK_MONOTONIC, &g_sync_timer);
            ecrt_master_application_time(g_master, TIMESPEC2NS(g_sync_timer));

            ecrt_master_receive(g_master);
            ecrt_domain_process(g_master_domain);
            
            // /100 = FREQ: 10  X
            // /80  = FREQ: 12  X
            // /50  = FREQ: 20  X
            // /40  = FREQ: 25 
            // /20  = FREQ: 50
            // /10  = FREQ: 100

            usleep(PERIOD_US / 20);
            if(!check_state_count){
                EC_NODE_check_master_state(ecn);
                EC_NODE_check_master_domain_state(ecn);
                EC_NODE_check_slave_conf_state(ecn);
                check_state_count = PERIOD_US / 20 ;
            }

            ecrt_domain_queue(g_master_domain);                
            ecrt_master_sync_slave_clocks(g_master);
            ecrt_master_sync_reference_clock_to(g_master, TIMESPEC2NS(g_sync_timer));
            ecrt_master_send(g_master);
            try_counter++;
            check_state_count--;
        }else {
            printf( "Error : Time out occurred while waiting for OP mode.!  ");
            //ecrt_master_deactivate_slaves(g_master);
            ecrt_master_deactivate(g_master);
            ecrt_release_master(g_master);
            return -1;
        }
    }
    return 0;


}

void EC_NODE_ConfigDcSync(EC_NODE* ecn, uint16_t assign_activate, int position){


    return ecrt_slave_config_dc(ecn->slaves_[position].slave_config_, assign_activate, PERIOD_NS, ecn->slaves_[position].kSync0_shift_, 0, 0);


}


void EC_NODE_check_slave_conf_state(EC_NODE* ecn){

    for(int i = 0 ; i < g_drivers_num ;i++)
    {

        EC_SLAVE_check_slave_conf_state(&(ecn->slaves_[i]));

    }

}

int EC_NODE_check_master_state(EC_NODE* ecn){

    ec_master_state_t ms;
    ecrt_master_state(g_master, &ms);
    if (ms.slaves_responding != g_master_state.slaves_responding){
        printf("%u slave(s).\n", ms.slaves_responding);
        if (ms.slaves_responding < 1) {
            printf( "Connection error,no response from slaves.");
            return -1;
        }
    }
    if (ms.al_states != g_master_state.al_states){
        printf("AL states: 0x%02X.\n", ms.al_states);
    }
    if (ms.link_up != g_master_state.link_up){
        printf("Link is %s.\n", ms.link_up ? "up" : "down");
        if(!ms.link_up){ 
            printf( "Master state link down");
            return -1;
        }
    }
    g_master_state = ms;
    return 0;

}

void EC_NODE_check_master_domain_state(EC_NODE* ecn){
    ec_domain_state_t ds;                     //Domain instance
    ecrt_domain_state(g_master_domain, &ds);
    if (ds.working_counter != g_master_domain_state.working_counter)
        printf("masterDomain: WC %u.\n", ds.working_counter);
    if (ds.wc_state != g_master_domain_state.wc_state)
        printf("masterDomain: State %u.\n", ds.wc_state);
    if(g_master_domain_state.wc_state == EC_WC_COMPLETE){
        g_master_domain_state = ds;
    }
    g_master_domain_state = ds;


}


int EC_NODE_get_num_of_connected_slaves(EC_NODE* ecn){

    unsigned int number_of_slaves;
    usleep(1e6);
    ecrt_master_state(g_master,&g_master_state);
    number_of_slaves = g_master_state.slaves_responding ;
    if(g_drivers_num != number_of_slaves){
        printf("Please enter correct number of slaves... \n");
        printf("Entered number of slave : %d\n", g_drivers_num); 
        printf("Connected slaves        : %d\n",number_of_slaves);
        return -1; 
    }
    return 0 ;

}

void EC_NODE_decativate_comm(EC_NODE* ecn){


    ecrt_master_deactivate(g_master);
}

void EC_NODE_release_master(EC_NODE* ecn){

    ecrt_master_deactivate(g_master);
    ecrt_release_master(g_master);

}


int EC_NODE_open_ethercat_master(EC_NODE* ecn){

    ecn->fd = system("ls /dev | grep EtherCAT* > /dev/null");
    if(ecn->fd){
        printf( "Opening EtherCAT master...");
        system("cd ~; sudo ethercatctl start");
        usleep(2e6);
        ecn->fd = system("ls /dev | grep EtherCAT* > /dev/null");
        if(ecn->fd){
            printf( "Error : EtherCAT device not found.");
            return -1;
            }else {
                return 0 ;
            }
    }
    return 0 ; 
}


int EC_NODE_shutdown_ethercat_master(EC_NODE* ecn){

    ecn->fd = system("ls /dev | grep EtherCAT* > /dev/null\n");
    if(!ecn->fd){
        printf( "Shutting down EtherCAT master...");
        system("cd ~; sudo ethercatctl stop\n");
        usleep(1e6);
        ecn->fd = system("ls /dev | grep EtherCAT* > /dev/null\n");
        if(ecn->fd){
            printf("EtherCAT shut down succesfull.");
            return 0;
        }else {
            printf( "Error : EtherCAT shutdown error.");
            return -1 ;
        }
    }


    return 0;
}


uint8_t EC_NODE_read_sdo(EC_NODE* ecn, SDO_DATA *pack){


    if (ecrt_master_sdo_upload(g_master, pack->slave_position,pack->index,pack->sub_index,
                    (uint8_t*)(&pack->data), pack->data_sz,&pack->result_sz,&pack->err_code)){
        printf("SDO read error, code: %d \n", &pack->err_code);
        return -1;
    }
    return 0;
}

uint8_t EC_NODE_write_sdo(EC_NODE* ecn, SDO_DATA *pack){

    if (ecrt_master_sdo_download(g_master,pack->slave_position,pack->index,pack->sub_index,(uint8_t*)(&pack->data),pack->data_sz,&pack->err_code)){
        printf("SDO write error, code : %d \n ", &pack->err_code);
        return -1;
    }
    return 0;

}


int EC_LIFE_init(EC_LIFE* eclc){

    EC_NODE* new_ecn = (EC_NODE*)malloc(sizeof(EC_NODE));

    memset(new_ecn, 0, sizeof(EC_NODE));
    
    eclc->ecat_node_ = new_ecn;

#if CYCLIC_POSITION_MODE
    // for syncronous cyclic position mode
    
    sent_data_.control_word.resize(g_drivers_num);
    sent_data_.target_pos.resize(g_drivers_num);
    sent_data_.digital_out.resize(g_drivers_num);
    sent_data_.touch_probe_func.resize(g_drivers_num);

    received_data_.status_word.resize(g_drivers_num);
    received_data_.actual_pos.resize(g_drivers_num);
    received_data_.digital_in.resize(g_drivers_num);
    received_data_.error_code.resize(g_drivers_num);
    received_data_.touch_probe_stat.resize(g_drivers_num);
    received_data_.touch_probe_1_pval.resize(g_drivers_num);
    received_data_.touch_probe_2_pval.resize(g_drivers_num);
#endif

#if POSITION_MODE
    // for position mode

    memset(eclc->sent_data_.control_word, 0, sizeof(uint16_t) * g_drivers_num);
    memset(eclc->sent_data_.target_pos, 0, sizeof(int32_t) * g_drivers_num);
    memset(eclc->sent_data_.profile_vel, 0, sizeof(uint32_t) * g_drivers_num);
    memset(eclc->sent_data_.digital_out, 0, sizeof(uint32_t) * g_drivers_num);
    memset(eclc->sent_data_.op_mode, 0, sizeof(int8_t) * g_drivers_num);
    memset(eclc->sent_data_.homing_method, 0, sizeof(int8_t) * g_drivers_num);

    memset(eclc->received_data_.status_word, 0, sizeof(uint16_t) * g_drivers_num);
    memset(eclc->received_data_.actual_pos, 0, sizeof(int32_t) * g_drivers_num);
    memset(eclc->received_data_.actual_vel, 0, sizeof(int32_t) * g_drivers_num);
    memset(eclc->received_data_.digital_in, 0, sizeof(uint32_t) * g_drivers_num);
    memset(eclc->received_data_.error_code, 0, sizeof(uint16_t) * g_drivers_num);
    memset(eclc->received_data_.op_mode_display, 0, sizeof(int8_t) * g_drivers_num);


    eclc->emergency_status_ = 1;
    eclc->al_state_ = 0;
    eclc->command_ = 0x004F;

#endif

    return 0;

}

int EC_LIFE_destroy(EC_LIFE* eclc){


    free(eclc->ecat_node_);

    return 0;

}


uint8_t EC_LIFE_on_configure(EC_LIFE* eclc){

    printf("Configuring EtherCAT device...\n");
    if (EC_LIFE_init_ethercat_comm(eclc))
    {
        printf("Configuration phase failed\n");
        return FAILURE;
    }
    else
    {
        return SUCCESS;
    }

}

uint8_t EC_LIFE_on_activate(EC_LIFE* eclc){

    if (EC_LIFE_start_ethercat_comm(eclc))
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


uint8_t EC_LIFE_on_deactivate(EC_LIFE* eclc){


    printf("Deactivating.");

    EC_NODE_decativate_comm(eclc->ecat_node_);

    return SUCCESS;
}


uint8_t EC_LIFE_on_cleanup(EC_LIFE* eclc){



    printf("Cleaning up.");

    free(eclc->ecat_node_);

    return SUCCESS;

}



uint8_t EC_LIFE_on_shutdown(EC_LIFE* eclc){

    printf("On_Shutdown... Waiting for control thread.");
    sig = 0;
    usleep(1e3);

    pthread_cancel(eclc->ethercat_thread_);

    EC_NODE_release_master(eclc->ecat_node_);

    EC_NODE_shutdown_ethercat_master(eclc->ecat_node_);

    return SUCCESS;
}


uint8_t EC_LIFE_on_kill(EC_LIFE* eclc){

    printf("On_Kill...");
    sig = 0;
    EC_NODE_release_master(eclc->ecat_node_);
    EC_NODE_shutdown_ethercat_master(eclc->ecat_node_);
    printf("master killed.");
    return SUCCESS;

}


uint8_t EC_LIFE_on_error(EC_LIFE* eclc){

    printf("On Error.");

    free(eclc->ecat_node_);

    return SUCCESS;
}

int EC_LIFE_set_comm_thread_priorities(EC_LIFE* eclc){


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
    CPU_SET(1, &mask);

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


int EC_LIFE_init_ethercat_comm(EC_LIFE* eclc){


    printf("Opening EtherCAT device...\n");

    if(EC_NODE_open_ethercat_master(eclc->ecat_node_)){


        return -1;
    }

    printf("Configuring EtherCAT master...\n");

    if(EC_NODE_conf_master(eclc->ecat_node_)){

        return -1;
    }

    printf("Getting connected slave informations...\n");

    if(EC_NODE_get_num_of_connected_slaves(eclc->ecat_node_)){

        return -1;
    }

    EC_NODE_get_all_slaves_info(eclc->ecat_node_);

    for (int i = 0; i < g_drivers_num; i++)
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

    if(EC_NODE_conf_slaves(eclc->ecat_node_)){

        return -1;
    }

    if(EC_LIFE_set_conf_parameters(eclc)){

        printf("Configuration parameters set failed\n");

        return -1;

    }

    printf("Mapping default PDOs...\n");

    if(EC_NODE_map_default_pdo(eclc->ecat_node_)){

        return -1;
    }

    if (DISTRIBUTED_CLOCK)
    {
        printf("Configuring DC synchronization...\n");
        EC_NODE_conf_dcsync_default(eclc->ecat_node_);
    }

    printf("Activating master...\n");

    if(EC_NODE_activate_master(eclc->ecat_node_)){

        return -1;
    }

    printf("Registering master domain...\n");

    if(EC_NODE_register_domain(eclc->ecat_node_)){

        return -1;
    }

    if(EC_NODE_wait_for_op_mode(eclc->ecat_node_)){


        return -1;
    }

#if PREEMPT_RT_MODE
    if (EC_LIFE_set_comm_thread_priorities(eclc))
    {
        return -1;
    }
#endif
    printf("Initialization succesfull...\n");
    return 0;

}

int EC_LIFE_set_conf_parameters(EC_LIFE* eclc){


#if POSITION_MODE
    PROFILE_POS_PARAM P;
    uint32_t max_fol_err;
    P.profile_vel = 10000; //3000;
    P.profile_acc = 1000000; //1e6;
    P.profile_dec = 1000000; //1e6;
    P.max_profile_vel = 2500000; //1e5;
    P.homing_speed_switch = 1000000;
    P.homing_speed_zero = 20000;

    for(int i = 0 ; i < g_drivers_num; i++){

        P.homing_offset_switch[i] = g_homing_offset_switch[i];

    }

    P.quick_stop_dec = 3e4;
    P.motion_profile_type = 0;
    
    return EC_NODE_set_all_parameters_pp(eclc->ecat_node_, &P);
#endif



    return 0;
}


int EC_LIFE_start_ethercat_comm(EC_LIFE* eclc){

    eclc->err_ = pthread_create(&(eclc->ethercat_thread_), &(eclc->ethercat_thread_attr_), EC_LIFE_pass_cyclic_exchange, eclc);

    if(eclc->err_){

        printf("Error : Couldn't start communication thread.!\n");
        return -1;
    }
    printf("Communication thread called.\n");
    return 0;
}


void* EC_LIFE_pass_cyclic_exchange(void *arg){

    EC_LIFE* eclc = (EC_LIFE*)arg;

    EC_LIFE_start_pdo_exchange(eclc);
}


void EC_LIFE_start_pdo_exchange(EC_LIFE* eclc){


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

        EC_LIFE_read_from_slaves(eclc);

        for (int i = 0; i < g_drivers_num; i++)
        {
            eclc->sent_data_.target_pos[i] = eclc->received_data_.actual_pos[i];
            //sent_data_.target_vel[i] = 0;
        }

        if(EC_LIFE_enable_drivers(eclc) == g_drivers_num){
            printf("All drives enabled\n");
            break;
        }

        if (status_check_counter){
            status_check_counter--;
        }
        else
        {
            // Checking master/domain/slaves state every 1sec.
            if (EC_NODE_check_master_state(eclc->ecat_node_) < 0)
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

                for (int i = 0; i < g_drivers_num; i++)
                {
                    printf("State of Drive %d : %d\n", i, eclc->motor_state_[i]);
                    printf("Trying to enable motors\n");
                }
            }
        }

#if POSITION_MODE
        EC_LIFE_write_to_slaves_pp(eclc);
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
            if (EC_NODE_check_master_state(eclc->ecat_node_) < 0)
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

        EC_LIFE_read_from_slaves(eclc);
        for (int i = 0; i < g_drivers_num; i++)
        {
            if (eclc->received_data_.error_code[i] != 0)
            {
                printf("Drive in error state\n");
                //GetErrorMessage(eclc->received_data_.error_code[i]);
            }
        }

#if POSITION_MODE
        EC_LIFE_update_parameters_pp(eclc);
        EC_LIFE_write_to_slaves_pp(eclc);
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

    EC_LIFE_read_from_slaves(eclc);
    for (int i = 0; i < g_drivers_num; i++)
    {
        eclc->sent_data_.control_word[i] = SM_GO_SWITCH_ON_DISABLE;
    }
    EC_LIFE_write_to_slaves_vc(eclc);

    ecrt_domain_queue(g_master_domain);
    ecrt_master_send(g_master);
    usleep(10000);
    // ------------------------------------------------------- //

    printf("Leaving control thread.\n");
    EC_NODE_decativate_comm(eclc->ecat_node_);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_cancel(eclc->ethercat_thread_);
    pthread_exit(NULL);
    return;

}

void EC_LIFE_read_from_slaves(EC_LIFE* eclc){

    for (int i = 0; i < g_drivers_num; i++)
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

int EC_LIFE_get_comm_state(EC_LIFE* eclc){

    return eclc->al_state_;

}

#if POSITION_MODE

void EC_LIFE_update_parameters_pp(EC_LIFE* eclc){

    printf("Updating position mode parameters\n");

    EC_LIFE_update_motor_state_pp(eclc);

    /// WRITE YOUR CUSTOM CONTROL ALGORITHM, VARIABLES DECLARATAION HERE, LIKE IN EXAMPLE BELOW.
    /// KEEP IN MIND THAT YOU WILL HAVE TO WAIT FOR THE MOTION TO FINISH IN POSITION MODE, THEREFORE
    /// YOU HAVE TO CHECK 10th BIT OF STATUS WORD TO CHECK WHETHER TARGET IS REACHED OR NOT.

    if(eclc->homed_all_ != 1){


        EC_LIFE_update_home_state_pp(eclc);


    } else{

        EC_LIFE_update_move_state_pp(eclc);

    }

}


#endif

void EC_LIFE_update_motor_state_pp(EC_LIFE* eclc){

    for (int i = 0; i < g_drivers_num; i++)
    {
        if ((eclc->received_data_.status_word[i] & eclc->command_) == 0X08)
        {
            // if status is fault, reset fault state.
            eclc->command_ = 0X04F;
            eclc->sent_data_.control_word[i] = SM_FULL_RESET;
            eclc->motor_state_[i] = K_FAULT;

        }
        if (eclc->motor_state_[i] != K_OPERATION_ENABLED)
        {
            eclc->sent_data_.control_word[i] = SM_GO_READY_TO_SWITCH_ON;
            if ((eclc->received_data_.status_word[i] & eclc->command_) == 0x0040)
            {
                // If status is "Switch on disabled", \
                change state to "Ready to switch on"
                eclc->sent_data_.control_word[i] = SM_GO_READY_TO_SWITCH_ON;
                eclc->command_ = 0x006f;
                eclc->motor_state_[i] = K_SWITCH_ON_DISABLED;
     
            }
            else if ((eclc->received_data_.status_word[i] & eclc->command_) == 0x0021)
            {
                // If status is "Ready to switch on", \
                        change state to "Switched on"
                eclc->sent_data_.control_word[i] = SM_GO_SWITCH_ON;
                eclc->command_ = 0x006f;
                eclc->motor_state_[i] = K_READY_TO_SWITCH_ON;
  
            }
            else if ((eclc->received_data_.status_word[i] & eclc->command_) == 0x0023)
            {
                // If status is "Switched on", change state to "Operation enabled"
                eclc->sent_data_.control_word[i] = SM_GO_ENABLE;
                eclc->command_ = 0x006f;
                eclc->motor_state_[i] = K_SWITCHED_ON;
      
            }
            else if ((eclc->received_data_.status_word[i] & eclc->command_) == 0X08)
            {
                // if status is fault, reset fault state.
                eclc->command_ = 0X04f;

                eclc->sent_data_.control_word[i] = SM_FULL_RESET;
                eclc->motor_state_[i] = K_FAULT;

            }
        }
        else
        {


            if(eclc->homed_all_ != 1){
                int hit = 0;

                for (int i = 0; i < g_drivers_num; i++){
                    if(eclc->homed_[i] == 1){
                        hit += 1;
                    }
                }

                if(hit == g_drivers_num){
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

void EC_LIFE_update_home_state_pp(EC_LIFE* eclc){




    for (int i = 0; i < g_drivers_num; i++)
    {
        if (eclc->motor_state_[i] == K_OPERATION_ENABLED ||
            eclc->motor_state_[i] == K_TARGET_REACHED || eclc->motor_state_[i] == K_SWITCHED_ON)
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


void EC_LIFE_update_move_state_pp(EC_LIFE* eclc){



    for (int i = 0; i < g_drivers_num; i++)
    {
        if (eclc->motor_state_[i] == K_OPERATION_ENABLED ||
            eclc->motor_state_[i] == K_TARGET_REACHED || eclc->motor_state_[i] == K_SWITCHED_ON)
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


int EC_LIFE_get_driver_state(const int statusWord){




    int state = 0;

    // bit 6 is 1
    if (TEST_BIT(statusWord, 6))
    {
        state = K_SWITCH_ON_DISABLED;
        return state;
    }

    // bit 6 is 0 and bit 5 is 1
    if (TEST_BIT(statusWord, 5))
    {
        if (TEST_BIT(statusWord, 2))
        {
            state = K_OPERATION_ENABLED;
            return state;
        }
        if (TEST_BIT(statusWord, 1))
        {
            state = K_SWITCHED_ON;
            return state;
        }
        if (TEST_BIT(statusWord, 0))
        {
            state = K_READY_TO_SWITCH_ON;
            return state;
        }
    }

    // bit 6 is 0 and bit 5 is 0
    if (TEST_BIT(statusWord, 3))
    {

        state = K_FAULT;
        return state;
    }
    else
    {

        state = K_QUICK_STOP;
        return state;
    }
    return state;

}

int EC_LIFE_enable_motors(EC_LIFE* eclc){

    int cnt = 0;
    for (int i = 0; i < g_drivers_num; i++)
    {

        eclc->sent_data_.control_word[i] = 0xF;

        cnt += 1;
    }
    return cnt;


}


int EC_LIFE_enable_drivers(EC_LIFE* eclc){


    int cnt = 0;
    for (int i = 0; i < g_drivers_num; i++)
    {
        eclc->motor_state_[i] = EC_LIFE_get_driver_state(eclc->received_data_.status_word[i]);

        // if status is fault, reset fault state.
        if (eclc->motor_state_[i] == K_FAULT)
        {
            // printf( "Driver %d in Fault",i);
            eclc->sent_data_.control_word[i] = SM_FULL_RESET;
        }

        // If status is "Switch on disabled", change state to "Ready to switch on"
        if (eclc->motor_state_[i] == K_SWITCH_ON_DISABLED)
        {
            eclc->sent_data_.control_word[i] = SM_GO_READY_TO_SWITCH_ON;
        }

        // If status is "Ready to switch on", change state to "Switched on"
        if (eclc->motor_state_[i] == K_READY_TO_SWITCH_ON)
        {
            eclc->sent_data_.control_word[i] = SM_GO_SWITCH_ON;
        }

        // If status is "Switched on", change state to "Operation enabled"
        if (eclc->motor_state_[i] == K_SWITCHED_ON)
        {
            eclc->sent_data_.control_word[i] = SM_GO_ENABLE;
        }

        // If status is "Switched on", change state to "Operation enabled"
        if (eclc->motor_state_[i] == K_OPERATION_ENABLED)
        {
            cnt++;
        }


    }
    return cnt;


}


void EC_LIFE_write_to_slaves_pp(EC_LIFE* eclc){

    for (int i = 0; i < g_drivers_num; i++)
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

void EC_LIFE_write_to_slaves_vc(EC_LIFE* eclc){


    eclc->emergency_status_ = 1;
    if (!eclc->emergency_status_)
    {
        for (int i = 0; i < g_drivers_num; i++)
        {
            EC_WRITE_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.control_word, eclc->sent_data_.control_word[i]);
            EC_WRITE_S32(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.target_vel, 0);
        }
    }
    else
    {
        for (int i = 0; i < g_drivers_num; i++)
        {
            EC_WRITE_U16(eclc->ecat_node_->slaves_[i].slave_pdo_domain_ + eclc->ecat_node_->slaves_[i].offset_.control_word, eclc->sent_data_.control_word[i]);
            //    EC_WRITE_S32(ecat_node_->slaves_[i].slave_pdo_domain_ + ecat_node_->slaves_[i].offset_.target_vel,sent_data_.target_vel[i]);
        }
    }

}



