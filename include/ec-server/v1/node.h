#ifndef _ECAT_NODE_H_
#define _ECAT_NODE_H_

#include "ec-server/v1/core.h"
#include "ec-server/v1/slave.h"




typedef struct EthercatNode {

    EthercatSlave slaves_[NUM_OF_SLAVES];
    int fd;


} EthercatNode;



int ECnode_init(EthercatNode* ecn);

int ECnode_destroy(EthercatNode* ecn);

int ECnode_ConfigureMaster(EthercatNode* ecn);

int ECnode_ConfigureSlaves(EthercatNode* ecn);

int ECnode_SetProfilePositionParameters(EthercatNode* ecn, ProfilePosParam* P , int position);

int ECnode_SetProfilePositionParametersAll(EthercatNode* ecn, ProfilePosParam* P);

int ECnode_MapDefaultPdos(EthercatNode* ecn);

void ECnode_ConfigDcSyncDefault(EthercatNode* ecn);

void ECnode_CheckSlaveConfigurationState(EthercatNode* ecn);

int  ECnode_CheckMasterState(EthercatNode* ecn);

void ECnode_CheckMasterDomainState(EthercatNode* ecn);

int  ECnode_ActivateMaster(EthercatNode* ecn);

int  ECnode_RegisterDomain(EthercatNode* ecn);

int  ECnode_WaitForOperationalMode(EthercatNode* ecn);

int ECnode_OpenEthercatMaster(EthercatNode* ecn);


int ECnode_GetNumberOfConnectedSlaves(EthercatNode* ecn);


void ECnode_GetAllSlaveInformation(EthercatNode* ecn);

void ECnode_DeactivateCommunication(EthercatNode* ecn);

void ECnode_ReleaseMaster(EthercatNode* ecn);

int ECnode_ShutDownEthercatMaster(EthercatNode* ecn);

uint8_t ECnode_SdoRead(EthercatNode* ecn, SDO_data *pack);

uint8_t ECnode_SdoWrite(EthercatNode* ecn, SDO_data *pack);


#endif