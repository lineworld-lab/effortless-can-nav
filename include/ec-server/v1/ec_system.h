#ifndef _ECAT_SYSTEM_H_
#define _ECAT_SYSTEM_H_


#include "ec-server/v1/core.h"
#include "ec-server/v1/object_dictionary.h"







typedef struct EC_SLAVE {
 
    /// DC sync shift setting, zero will give best synchronization.
    uint32_t   kSync0_shift_;

    /// Slave configuration parameters, assinged to each slave.
    ec_slave_config_t       * slave_config_ ;

    /// Slave state handle to check if slave is online and slaves state machine status(INIT/PREOP/SAFEOP/0P)
    ec_slave_config_state_t  slave_config_state_ ;
    
    /// For custom PDO configuration, check \see MapDefaultPdos() function @file ethercat_node.cpp as an example 
    ec_pdo_info_t          * slave_pdo_info_ ;
    /// For custom PDO configuration, check \see MapDefaultPdos() function @file ethercat_node.cpp as an example 
    ec_pdo_entry_info_t    * slave_pdo_entry_info_;
    /// For custom PDO configuration, check \see MapDefaultPdos() function @file ethercat_node.cpp as an example 
    ec_sync_info_t         * slave_sync_info_;
    /// For custom PDO configuration, check \see MapDefaultPdos() function @file ethercat_node.cpp as an example 
    ec_pdo_entry_reg_t     * slave_pdo_entry_reg_ ;
    /// PDO domain for data exchange
    uint8_t                * slave_pdo_domain_ ;

    /// Variable for checking motor state 
    int32_t                  motor_state_ ;
    /**
     * @brief Slave information data structure.
     *      This structure contains all information related to slave.
     *      It will be used to get slave's information from master.
     */
    ec_slave_info_t         slave_info_;
    /// Offset for PDO entries to assign pdo registers.
    PDO_OFFSET                 offset_ ;
    /// Received data from servo drivers.
    DATA_RECEIVED              data_ ;
    /// Slave velocity parameters.
    PROFILE_VELOCITY_PARAM    velocity_param_ ;
    /// Slave position parameters.
    PROFILE_POS_PARAM         position_param_ ;
    // Slave homing parameters. 
    HOMING_PARAM             homing_param_ ;


} EC_SLAVE;




int EC_SLAVE_init(EC_SLAVE* ecs);


int EC_SLAVE_destroy(EC_SLAVE* ecs);


int EC_SLAVE_check_slave_conf_state(EC_SLAVE* ecs);



typedef struct EC_NODE {

    EC_SLAVE slaves_[MAX_DRIVERS_NUM];
    int fd;


} EC_NODE;



int EC_NODE_init(EC_NODE* ecn);

int EC_NODE_destroy(EC_NODE* ecn);

int EC_NODE_conf_master(EC_NODE* ecn);

int EC_NODE_conf_slaves(EC_NODE* ecn);

int EC_NODE_set_parameters_pp(EC_NODE* ecn, PROFILE_POS_PARAM* P , int position);

int EC_NODE_set_all_parameters_pp(EC_NODE* ecn, PROFILE_POS_PARAM* P);

int EC_NODE_map_default_pdo(EC_NODE* ecn);

void EC_NODE_conf_dcsync_default(EC_NODE* ecn);

void EC_NODE_check_slave_conf_state(EC_NODE* ecn);

int  EC_NODE_check_master_state(EC_NODE* ecn);

void EC_NODE_check_master_domain_state(EC_NODE* ecn);

int  EC_NODE_activate_master(EC_NODE* ecn);

int  EC_NODE_register_domain(EC_NODE* ecn);

int  EC_NODE_wait_for_op_mode(EC_NODE* ecn);

int EC_NODE_open_ethercat_master(EC_NODE* ecn);


int EC_NODE_get_num_of_connected_slaves(EC_NODE* ecn);


void EC_NODE_get_all_slaves_info(EC_NODE* ecn);

void EC_NODE_decativate_comm(EC_NODE* ecn);

void EC_NODE_release_master(EC_NODE* ecn);

int EC_NODE_shutdown_ethercat_master(EC_NODE* ecn);

uint8_t EC_NODE_read_sdo(EC_NODE* ecn, SDO_DATA *pack);

uint8_t EC_NODE_write_sdo(EC_NODE* ecn, SDO_DATA *pack);




typedef struct EC_LIFE {

    DATA_RECEIVED     received_data_;

    DATA_SENT         sent_data_;

    EC_NODE* ecat_node_;

    pthread_t ethercat_thread_;
    /// Scheduler parameter
    struct sched_param ethercat_sched_param_;
    /// Thread attribute parameter
    pthread_attr_t ethercat_thread_attr_;
    /// Error flag
    int32_t err_;
    /// Application layer of slaves seen by master.(INIT/PREOP/SAFEOP/OP)
    uint8_t al_state_; 
    
    uint32_t motor_state_[MAX_DRIVERS_NUM];


    //uint32_t zeroed_all_ = 0;
    uint32_t homed_all_;


    //uint32_t zeroed_[g_drivers_num];
    //uint32_t shifted_[g_drivers_num];
    uint32_t homed_[MAX_DRIVERS_NUM];

    uint32_t op_mode_[MAX_DRIVERS_NUM];


    //int32_t shift_param_[g_drivers_num];
    //uint32_t shift_set_pos_[g_drivers_num];


    //uint32_t target_reached_[g_drivers_num];
    uint32_t new_set_pos_[MAX_DRIVERS_NUM];
  
    //int32_t posted_home_shift_[g_drivers_num];
    
    int32_t posted_position_[MAX_DRIVERS_NUM];
    int32_t feedback_position_[MAX_DRIVERS_NUM];
    



    uint32_t command_;
    /// Structure for Xbox Controller values
    uint8_t emergency_status_;
    // Will be used as a parameter for taking timing measurements.
    int32_t measurement_time; 

} EC_LIFE;


int EC_LIFE_init(EC_LIFE* eclc);

int EC_LIFE_destroy(EC_LIFE* eclc);

uint8_t EC_LIFE_on_configure(EC_LIFE* eclc);

uint8_t EC_LIFE_on_activate(EC_LIFE* eclc);

uint8_t EC_LIFE_on_deactivate(EC_LIFE* eclc);

uint8_t EC_LIFE_on_cleanup(EC_LIFE* eclc);

uint8_t EC_LIFE_on_shutdown(EC_LIFE* eclc);

uint8_t EC_LIFE_on_kill(EC_LIFE* eclc);

uint8_t EC_LIFE_on_error(EC_LIFE* eclc);

int EC_LIFE_set_comm_thread_priorities(EC_LIFE* eclc);


int EC_LIFE_init_ethercat_comm(EC_LIFE* eclc) ; 


void *EC_LIFE_pass_cyclic_exchange(void *arg);

int  EC_LIFE_start_ethercat_comm(EC_LIFE* eclc); 

void EC_LIFE_start_pdo_exchange(EC_LIFE* eclc); 

int EC_LIFE_get_comm_state(EC_LIFE* eclc);

void EC_LIFE_read_from_slaves(EC_LIFE* eclc);

void EC_LIFE_write_to_slaves_pp(EC_LIFE* eclc);

void EC_LIFE_update_parameters_pp(EC_LIFE* eclc);

void EC_LIFE_update_motor_state_pp(EC_LIFE* eclc);

void EC_LIFE_update_home_state_pp(EC_LIFE* eclc);

void EC_LIFE_update_move_state_pp(EC_LIFE* eclc);

void EC_LIFE_write_to_slaves_vc(EC_LIFE* eclc);


int EC_LIFE_get_driver_state(const int statusWord);

int EC_LIFE_enable_motors(EC_LIFE* eclc);

int EC_LIFE_enable_drivers(EC_LIFE* eclc);

int EC_LIFE_set_conf_parameters(EC_LIFE* eclc);


extern EC_LIFE ECAT_LIFECYCLE_NODE;

#endif
