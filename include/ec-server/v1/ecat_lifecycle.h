#ifndef _ECAT_LIFECYCLE_H_
#define _ECAT_LIFECYCLE_H_

#include "ec-server/v1/ecat_node.h"
//#include "timing.h"
/******************************************************************************/



typedef struct EthercatLifeCycle {

    DataReceived     received_data_;

    DataSent         sent_data_;

    EthercatNode* ecat_node_;

    pthread_t ethercat_thread_;
    /// Scheduler parameter
    struct sched_param ethercat_sched_param_;
    /// Thread attribute parameter
    pthread_attr_t ethercat_thread_attr_;
    /// Error flag
    int32_t err_;
    /// Application layer of slaves seen by master.(INIT/PREOP/SAFEOP/OP)
    uint8_t al_state_; 
    
    uint32_t motor_state_[NUM_OF_SLAVES];


    //uint32_t zeroed_all_ = 0;
    uint32_t homed_all_;


    //uint32_t zeroed_[g_kNumberOfServoDrivers];
    //uint32_t shifted_[g_kNumberOfServoDrivers];
    uint32_t homed_[NUM_OF_SLAVES];

    uint32_t op_mode_[NUM_OF_SLAVES];


    //int32_t shift_param_[g_kNumberOfServoDrivers];
    //uint32_t shift_set_pos_[g_kNumberOfServoDrivers];


    //uint32_t target_reached_[g_kNumberOfServoDrivers];
    uint32_t new_set_pos_[NUM_OF_SLAVES];
  
    //int32_t posted_home_shift_[g_kNumberOfServoDrivers];
    
    int32_t posted_position_[NUM_OF_SLAVES];
    int32_t feedback_position_[NUM_OF_SLAVES];
    



    uint32_t command_;
    /// Structure for Xbox Controller values
    uint8_t emergency_status_;
    // Will be used as a parameter for taking timing measurements.
    int32_t measurement_time; 

} EthercatLifeCycle;


int EClife_init(EthercatLifeCycle* eclc);

int EClife_destroy(EthercatLifeCycle* eclc);

uint8_t EClife_on_configure(EthercatLifeCycle* eclc);

uint8_t EClife_on_activate(EthercatLifeCycle* eclc);

uint8_t EClife_on_deactivate(EthercatLifeCycle* eclc);

uint8_t EClife_on_cleanup(EthercatLifeCycle* eclc);

uint8_t EClife_on_shutdown(EthercatLifeCycle* eclc);

uint8_t EClife_on_kill(EthercatLifeCycle* eclc);

uint8_t EClife_on_error(EthercatLifeCycle* eclc);

int EClife_SetComThreadPriorities(EthercatLifeCycle* eclc);


int EClife_InitEthercatCommunication(EthercatLifeCycle* eclc) ; 


void *EClife_PassCycylicExchange(void *arg);

int  EClife_StartEthercatCommunication(EthercatLifeCycle* eclc); 

void EClife_StartPdoExchange(EthercatLifeCycle* eclc); 

int EClife_GetComState(EthercatLifeCycle* eclc);

void EClife_ReadFromSlaves(EthercatLifeCycle* eclc);

void EClife_WriteToSlavesInPositionMode(EthercatLifeCycle* eclc);

void EClife_UpdatePositionModeParameters(EthercatLifeCycle* eclc);

void EClife_UpdateMotorStatePositionMode(EthercatLifeCycle* eclc);

void EClife_UpdateHomeStatePositionMode(EthercatLifeCycle* eclc);

void EClife_UpdateMoveStatePositionMode(EthercatLifeCycle* eclc);

void EClife_WriteToSlavesVelocityMode(EthercatLifeCycle* eclc);


int EClife_GetDriveState(const int statusWord);

int EClife_EnableMotors(EthercatLifeCycle* eclc);

int EClife_EnableDrivers(EthercatLifeCycle* eclc);

int EClife_SetConfigurationParameters(EthercatLifeCycle* eclc);



extern EthercatLifeCycle* ECAT_LIFECYCLE_NODE;


int GetHomingStatusByAxis(char* res, int axis);

int GetPositionByAxis(char* res, int axis);

int PostHomeShiftByAxis(char* res, int axis, int shift);

int PostPositionByAxis(char* res, int axis, int pos);

int PostPositionWithFeedbackByAxis(char* res, int axis, int pos);

int PostPositionWithStatusFeedbackByAxis(char* res, int axis, int pos);


#endif