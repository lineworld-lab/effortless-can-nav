#include "ec-server/v1/node.h"


int ECnode_ConfigureMaster(EthercatNode* ecn){


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


void ECnode_GetAllSlaveInformation(EthercatNode* ecn){

    for(int i=0;i < NUM_OF_SLAVES ; i++){
        ecrt_master_get_slave(g_master, i , &(ecn->slaves_[i].slave_info_));
    }

}


int ECnode_ConfigureSlaves(EthercatNode* ecn){


    for(int i = 0 ; i < NUM_OF_SLAVES ; i++ ){
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

int ECnode_MapDefaultPdos(EthercatNode* ecn){


#if POSITION_MODE
        // for position mode
    ec_pdo_entry_info_t maxon_epos_pdo_entries[12] = {
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
    ec_pdo_info_t maxon_pdos[2] = {
        {0x1601, 6, maxon_epos_pdo_entries + 0},    // - RxPDO index of the EPOS4
        {0x1a01, 6, maxon_epos_pdo_entries + 6}     // - TxPDO index of the EPOS4
    };
#endif

    ec_sync_info_t maxon_syncs[5] = {
        {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
        {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
        {2, EC_DIR_OUTPUT, 1, maxon_pdos + 0, EC_WD_ENABLE},
        {3, EC_DIR_INPUT, 1, maxon_pdos + 1, EC_WD_DISABLE},
        {0xff}
    };

    // Connect sync_manager to corresponding slaves.
    for(int i = 0 ; i < g_kNumberOfServoDrivers ; i++){
        if(ecrt_slave_config_pdos(ecn->slaves_[i].slave_config_,EC_END,maxon_syncs)){
            printf( "Slave PDO configuration failed... ");
            return -1;
        }
    }
#if POSITION_MODE
    for(int i = 0; i < g_kNumberOfServoDrivers ; i++){

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

void ECnode_ConfigDcSyncDefault(EthercatNode* ecn){

    for(int i=0; i < g_kNumberOfServoDrivers ; i++){
        ecrt_slave_config_dc(ecn->slaves_[i].slave_config_, 0X0300, PERIOD_NS, ecn->slaves_[i].kSync0_shift_, 0, 0);
    }


}


int ECnode_ActivateMaster(EthercatNode* ecn){


    if ( ecrt_master_activate(g_master) ) {
        printf( "Master activation error ! ");
        return -1 ;
    }
    return 0 ; 

}

int ECnode_RegisterDomain(EthercatNode* ecn){


    for(int i = 0 ; i < NUM_OF_SLAVES ; i++){
        ecn->slaves_[i].slave_pdo_domain_ = ecrt_domain_data(g_master_domain);
        if(!(ecn->slaves_[i].slave_pdo_domain_) )
        {
            printf( "Domain PDO registration error");
            return -1;
        }
    }
    return 0;

}

int ECnode_SetProfilePositionParameters(EthercatNode* ecn, ProfilePosParam* P, int position){



  // Operation mode to ProfilePositionMode for slave on that position.
    if( ecrt_slave_config_sdo8(ecn->slaves_[position].slave_config_,OD_OPERATION_MODE, kProfilePosition) ){
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


int ECnode_SetProfilePositionParametersAll(EthercatNode* ecn, ProfilePosParam* P){

    for(int i = 0 ; i < g_kNumberOfServoDrivers ; i++)
    {
        // Set operation mode to ProfilePositionMode for all motors.
        if( ecrt_slave_config_sdo8(ecn->slaves_[i].slave_config_,OD_OPERATION_MODE, kProfilePosition) ){
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

int ECnode_WaitForOperationalMode(EthercatNode* ecn){

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
                ECnode_CheckMasterState(ecn);
                ECnode_CheckMasterDomainState(ecn);
                ECnode_CheckSlaveConfigurationState(ecn);
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

void ECnode_ConfigDcSync(EthercatNode* ecn, uint16_t assign_activate, int position){


    return ecrt_slave_config_dc(ecn->slaves_[position].slave_config_, assign_activate, PERIOD_NS, ecn->slaves_[position].kSync0_shift_, 0, 0);


}


void ECnode_CheckSlaveConfigurationState(EthercatNode* ecn){

    for(int i = 0 ; i < NUM_OF_SLAVES ;i++)
    {

        ECslave_CheckSlaveConfigState(&(ecn->slaves_[i]));

    }

}

int ECnode_CheckMasterState(EthercatNode* ecn){

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

void ECnode_CheckMasterDomainState(EthercatNode* ecn){
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


int ECnode_GetNumberOfConnectedSlaves(EthercatNode* ecn){

    unsigned int number_of_slaves;
    usleep(1e6);
    ecrt_master_state(g_master,&g_master_state);
    number_of_slaves = g_master_state.slaves_responding ;
    if(NUM_OF_SLAVES != number_of_slaves){
        printf("Please enter correct number of slaves... \n");
        printf("Entered number of slave : %d\n", NUM_OF_SLAVES); 
        printf("Connected slaves        : %d\n",number_of_slaves);
        return -1; 
    }
    return 0 ;

}

void ECnode_DeactivateCommunication(EthercatNode* ecn){


    ecrt_master_deactivate(g_master);
}

void ECnode_ReleaseMaster(EthercatNode* ecn){

    ecrt_master_deactivate(g_master);
    ecrt_release_master(g_master);

}


int ECnode_OpenEthercatMaster(EthercatNode* ecn){

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


int ECnode_ShutDownEthercatMaster(EthercatNode* ecn){

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


uint8_t ECnode_SdoRead(EthercatNode* ecn, SDO_data *pack){


    if (ecrt_master_sdo_upload(g_master, pack->slave_position,pack->index,pack->sub_index,
                    (uint8_t*)(&pack->data), pack->data_sz,&pack->result_sz,&pack->err_code)){
        printf("SDO read error, code: %d \n", &pack->err_code);
        return -1;
    }
    return 0;
}

uint8_t ECnode_SdoWrite(EthercatNode* ecn, SDO_data *pack){

    if (ecrt_master_sdo_download(g_master,pack->slave_position,pack->index,pack->sub_index,(uint8_t*)(&pack->data),pack->data_sz,&pack->err_code)){
        printf("SDO write error, code : %d \n ", &pack->err_code);
        return -1;
    }
    return 0;

}