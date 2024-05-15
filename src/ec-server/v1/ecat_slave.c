#include "ec-server/v1/slave.h"

int ECslave_CheckSlaveConfigState(EthercatSlave* ecs){


    ecrt_slave_config_state(ecs->slave_config_, &ecs->slave_config_state_);
    if (ecs->slave_config_state_.al_state != 0x08) {
        printf(" Slave is not operational AL state is : %x\n", ecs->slave_config_state_.al_state); 
    }
    return 0;
}