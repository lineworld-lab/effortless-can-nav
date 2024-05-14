#ifndef _ECAT_SLAVE_H_
#define _ECAT_SLAVE_H_

#include "ec-server/v1/ecat_globals.h"



typedef struct EthercatSlave {
 
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
    OffsetPDO                 offset_ ;
    /// Received data from servo drivers.
    DataReceived              data_ ;
    /// Slave velocity parameters.
    ProfileVelocityParam    velocity_param_ ;
    /// Slave position parameters.
    ProfilePosParam         position_param_ ;
    // Slave homing parameters. 
    HomingParam             homing_param_ ;


} EthercatSlave;




int ECslave_init(EthercatSlave* ecs);


int ECslave_destroy(EthercatSlave* ecs);


int ECslave_CheckSlaveConfigState(EthercatSlave* ecs);



#endif