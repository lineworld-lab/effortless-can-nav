#include "ec-server/v1/node.h"
#include "ec-server/v1/lifecycle.h"
#include "ec-server/v1/sock.h"


sig_atomic_t sig = 1;
ec_master_t        * g_master ;  /// EtherCAT master
ec_master_state_t    g_master_state ; /// EtherCAT master state

ec_domain_t       * g_master_domain ; /// Ethercat data passing master domain
ec_domain_state_t   g_master_domain_state ;   /// EtherCAT master domain state

struct timespec      g_sync_timer ;                       /// timer for DC sync .
const struct timespec       g_cycle_time = {0, PERIOD_NS} ;      /// cycletime settings in ns. 
uint32_t             g_sync_ref_counter;                  /// To sync every cycle.

uint32_t  g_kNumberOfServoDrivers = NUM_OF_SLAVES;   

EthercatLifeCycle ECAT_LIFECYCLE_NODE;

void signalHandler();
void SetRealTimeSettings();


int main(int argc, char **argv){

    SetRealTimeSettings();

    memset(&ECAT_LIFECYCLE_NODE, 0, sizeof(EthercatLifeCycle));

    EClife_init(&ECAT_LIFECYCLE_NODE);

#if HOMING_AT_START

    //  ecat_lifecycle_node->zeroed_all_ = 0;
    ECAT_LIFECYCLE_NODE->homed_all_ = 0;


#else

    //  ecat_lifecycle_node->zeroed_all_ = 1;
    (&ECAT_LIFECYCLE_NODE)->homed_all_ = 1;


#endif

    if(EClife_on_configure(&ECAT_LIFECYCLE_NODE)){
        return -1;
    }

    if(EClife_on_activate(&ECAT_LIFECYCLE_NODE)){
        return -1;
    }

    int server_stat = 0 ;

    server_stat = ListenAndServe(SOCK_PORT);


    if(server_stat < 0 ){
        printf("server failure");

        EClife_on_shutdown(&ECAT_LIFECYCLE_NODE);

        exit(EXIT_FAILURE);
    }


    return 0;
}



void signalHandler(int /*signum*/){
    sig = 0;
    nanosleep((const struct timespec[]){0,g_kNsPerSec},NULL);

    EClife_on_shutdown(&ECAT_LIFECYCLE_NODE);

    exit(0);
}


void signalKillHandler(int /*signum*/){
    sig = 0;
    nanosleep((const struct timespec[]){0,g_kNsPerSec},NULL);

    EClife_on_shutdown(&ECAT_LIFECYCLE_NODE);

    exit(0);
}


void SetRealTimeSettings()
{
      // Configure stdout sream buffer. _IONBF means no buffering. Each I/O operation is written as soon as possible. 
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    
    // Associate 'signalHandler' function with interrupt signal (Ctrl+C key)
    signal(SIGINT,signalHandler);


    signal(SIGUSR1, signalKillHandler);

#if PREEMPT_RT_MODE

    // Prepare memory for real time performance 
    // https://design.ros2.org/articles/realtime_background.html
    
    // Lock this processe's memory. Necessary for real time performance....
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        printf( "Mlockall failed, check if you have sudo authority.");
        return ;
    }
    /* Turn off malloc trimming.*/
    mallopt(M_TRIM_THRESHOLD, -1);

    /* Turn off mmap usage. */
    mallopt(M_MMAP_MAX, 0);
    // -----------------------------------------------------------------------------

#endif
}

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= g_kNsPerSec)
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - g_kNsPerSec;
    }
    else
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}
