#include "ec-server/v1/ec_system.h"
#include "ec-server/v1/utils.h"
#include "ec-server/v1/sock.h"


sig_atomic_t sig = 1;
ec_master_t        * g_master ;  /// EtherCAT master
ec_master_state_t    g_master_state ; /// EtherCAT master state

ec_domain_t       * g_master_domain ; /// Ethercat data passing master domain
ec_domain_state_t   g_master_domain_state ;   /// EtherCAT master domain state

struct timespec      g_sync_timer ;                       /// timer for DC sync .
const struct timespec       g_cycle_time = {0, PERIOD_NS} ;      /// cycletime settings in ns. 
uint32_t             g_sync_ref_counter;                  /// To sync every cycle.

uint32_t  g_drivers_num;   

int32_t g_homing_offset_switch[MAX_DRIVERS_NUM];


int we_are_testing = 0;

/*
    int32_t HomingOffsetSwitch[g_drivers_num] = {
        400000 - 335000,
        300000,
        392520,
        400000 - 145000
    };
*/

EC_LIFE ECAT_LIFECYCLE_NODE;

void signalHandler();
void signalKillHandler();

int main(int argc, char **argv){

    signal(SIGINT,signalHandler);

    signal(SIGUSR1, signalKillHandler);

    int init_result = InitRuntimeFromConfigJSON();

    if (init_result < 0 ){

        return init_result;

    }

    if(we_are_testing == 1){

        printf("TESTED.\n");

        return 0;
    }

    SetRealTimeSettings();

    memset(&ECAT_LIFECYCLE_NODE, 0, sizeof(EC_LIFE));

    EC_LIFE_init(&ECAT_LIFECYCLE_NODE);

#if HOMING_AT_START

    //  ecat_lifecycle_node->zeroed_all_ = 0;
    ECAT_LIFECYCLE_NODE->homed_all_ = 0;


#else

    //  ecat_lifecycle_node->zeroed_all_ = 1;
    (&ECAT_LIFECYCLE_NODE)->homed_all_ = 1;


#endif

    if(EC_LIFE_on_configure(&ECAT_LIFECYCLE_NODE)){
        return -1;
    }

    if(EC_LIFE_on_activate(&ECAT_LIFECYCLE_NODE)){
        return -1;
    }

    int server_stat = 0 ;

    server_stat = ListenAndServe(SOCK_PORT);


    if(server_stat < 0 ){
        printf("server failure");

        EC_LIFE_on_shutdown(&ECAT_LIFECYCLE_NODE);

        exit(EXIT_FAILURE);
    }


    return 0;
}



void signalHandler(int /*signum*/){
    sig = 0;
    nanosleep((const struct timespec[]){0,g_kNsPerSec},NULL);

    EC_LIFE_on_shutdown(&ECAT_LIFECYCLE_NODE);

    exit(0);
}


void signalKillHandler(int /*signum*/){
    sig = 0;
    nanosleep((const struct timespec[]){0,g_kNsPerSec},NULL);

    EC_LIFE_on_shutdown(&ECAT_LIFECYCLE_NODE);

    exit(0);
}


