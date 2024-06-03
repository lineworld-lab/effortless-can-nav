#ifndef _ECAT_GLOBALS_H_
#define _ECAT_GLOBALS_H_

#define _GNU_SOURCE

#include <limits.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sched.h>


/****************************************************************************/
// IgH EtherCAT library header file the user-space real-time interface library.
// IgH, EtherCAT related functions and data types.
#include "ecrt.h"  
     
// Object dictionary paramaters PDO index and default values in here.
#include "ec-server/v1/object_dictionary.h"  

/****************************************************************************/
                /*** USER SHOULD DEFINE THIS AREAS ***/
/// Total number of connected slave to the bus.
#define MAX_DRIVERS_NUM     16 

/// Number of connected servo drives. 
/// @note If you are using custom slaves (not servo drives) this value must be different than MAX_DRIVERS_NUM.
extern uint32_t  g_drivers_num;   

extern int32_t g_homing_offset_switch[MAX_DRIVERS_NUM];

//const uint32_t  g_drivers_numTarget = 1;   

#define PREEMPT_RT_MODE 1

#define HOMING_AT_START 0

/// If you have EtherCAT slave different than CiA402 supported motor drive, set this macro to 1
/// @note  That you'll have to manually specify PDO mapping for your custom slave.
#define CUSTOM_SLAVE     0

/// Ethercat PDO exchange loop frequency in Hz    

#if PREEMPT_RT_MODE



#define FREQUENCY       50        

#else 

#define FREQUENCY       50      

#endif

/// If you want to measure timings leave it as one, otherwise make it 0.
#define MEASURE_TIMING         0    
 /// set this to 1 if you want to use it in velocity mode (and set other modes 0)
#define VELOCITY_MODE          0
/// set this to 1 if you want to use it in position mode (and set other modes 0)  
#define POSITION_MODE          1    
/// set this to 1 if you want to use it in cyclic synchronous position mode (and set other modes 0)
#define CYCLIC_POSITION_MODE   0    
/// set this to 1 if you want to use it in cyclic synchronous velocity mode (and set other modes 0)
#define CYCLIC_VELOCITY_MODE   0    
/// set this to 1 if you want to use it in cyclic synchronous torque mode (and set other modes 0)
#define CYCLIC_TORQUE_MODE     0  
/// set this to 1 if you want to activate distributed clock, by default leave it 1.  
#define DISTRIBUTED_CLOCK      1    
/*****************************************************************************/
/// If you are using geared motor define ratio
/// @note If you have different types of motors in your system you may need to create different macros for them.
#define GEAR_RATIO          40
/// Motor encoder resolution
#define ENCODER_RESOLUTION  10000

#define INC_PER_ROTATION      GEAR_RATIO*ENCODER_RESOLUTION*4
#define FIVE_DEGREE_CCW      int(INC_PER_ROTATION/72)
#define THIRTY_DEGREE_CCW    int(INC_PER_ROTATION/12)

/// Nanoseconds per second.
#define g_kNsPerSec 1000000000     
#define PERIOD_NS       (g_kNsPerSec/FREQUENCY)  /// EtherCAT communication period in nanoseconds.
#define PERIOD_US       (PERIOD_NS / 1000)      /// EtherCAT communication period in microseconds.
#define PERIOD_MS       (PERIOD_US / 1000)      /// EtherCAT communication period in milliseconds.
#if CUSTOM_SLAVE
    #define FINAL_SLAVE     (MAX_DRIVERS_NUM-1)
#endif
/****************************************************************************/
//// Global variable declarations, definitions are in @file ethercat_node.cpp
extern sig_atomic_t sig;
extern ec_master_t        * g_master ;  /// EtherCAT master
extern ec_master_state_t    g_master_state ; /// EtherCAT master state

extern ec_domain_t       * g_master_domain ; /// Ethercat data passing master domain
extern ec_domain_state_t   g_master_domain_state ;   /// EtherCAT master domain state

extern struct timespec      g_sync_timer ;                       /// timer for DC sync .
extern const struct timespec       g_cycle_time;      /// cycletime settings in ns. 
extern uint32_t             g_sync_ref_counter;                  /// To sync every cycle.


/****************************************************************************/
#define TEST_BIT(NUM,N)    ((NUM &  (1 << N))>>N)  /// Check specific bit in the data. 0 or 1.
#define SET_BIT(NUM,N)      (NUM |  (1 << N))  /// Set(1) specific bit in the data.
#define RESET_BIT(NUM,N)    (NUM & ~(1 << N))  /// Reset(0) specific bit in the data
/// Convert timespec struct to nanoseconds
#define TIMESPEC2NS(T)      ((uint64_t) (T).tv_sec * g_kNsPerSec + (T).tv_nsec) 
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * g_kNsPerSec + (B).tv_nsec - (A).tv_nsec)
/// Using Monotonic system-wide clock.
#define CLOCK_TO_USE        CLOCK_MONOTONIC  

/**
 * @brief Add two timespec struct.
 * 
 * @param time1 Timespec struct 1
 * @param time2 Timespec struct 2
 * @return Addition result
 */
struct timespec timespec_add(struct timespec time1, struct timespec time2);

 /// Class states.
enum LIFE_CYCLE_STATE
{
    FAILURE = -1,
    SUCCESS,
    TRANSITIONING
};

/// SDO_data Structure holding all data needed to send an SDO object
typedef struct SDO_DATA {
    uint16_t slave_position;    // Position based addressing.
    uint16_t index;		        // Index in Object dictionary
    uint8_t  sub_index;	        // Subindex in Object dictionary
    uint32_t  data ; 
    size_t   data_sz;	        // Size
    size_t   result_sz;         // Resulted data size
    uint32_t err_code;	        // Error code
} SDO_DATA;

/// Motor operation modes based on CiA402

typedef enum OP_MODE
{
    K_PROFILE_POSITION = 1,
    K_PROFILE_VELOCITY = 3,
    K_PROFILE_TORQUE   = 4,
    K_HOMING = 6,
    K_INTERPOLATED_POSITION = 7,
    K_CS_POSITION = 8,
    K_CS_VELOCITY = 9,
    K_CS_TORQUE = 10,
};

/// Structure for data to be received from slaves.
typedef struct DATA_RECEIVED
{
    uint16_t  com_status;
#if CYCLIC_POSITION_MODE 
    // for Syncronous Cyclic Position mode
        
    std::vector<int32_t>  actual_pos ;
    std::vector<uint16_t> status_word ;
    std::vector<uint16_t> error_code ;
    std::vector<uint32_t>  digital_in;
    std::vector<uint16_t> touch_probe_stat;
    std::vector<int32_t>  touch_probe_1_pval;
    std::vector<int32_t>  touch_probe_2_pval;
    
#endif

#if POSITION_MODE
    // for position mode
    uint16_t status_word[MAX_DRIVERS_NUM];
    int32_t  actual_pos[MAX_DRIVERS_NUM];
    int32_t  actual_vel[MAX_DRIVERS_NUM];
    uint32_t  digital_in[MAX_DRIVERS_NUM];
    uint16_t error_code[MAX_DRIVERS_NUM];
    int8_t   op_mode_display[MAX_DRIVERS_NUM];
#endif

} DATA_RECEIVED;

/// Structure for data to be sent to slaves.
typedef struct DATA_SENT
{
    // for Syncronous Cyclic Position mode
#if CYCLIC_POSITION_MODE 
    std::vector<int32_t>   target_pos ;
    std::vector<uint16_t>  control_word ;
    std::vector<uint16_t>   touch_probe_func;
    std::vector<uint32_t>  digital_out;
#endif

#if POSITION_MODE
    // for position mode
    uint16_t  control_word[MAX_DRIVERS_NUM] ;
    int32_t   target_pos[MAX_DRIVERS_NUM] ;
    uint32_t  profile_vel[MAX_DRIVERS_NUM];
    uint32_t  digital_out[MAX_DRIVERS_NUM];
    int8_t    op_mode[MAX_DRIVERS_NUM];

    int8_t homing_method[MAX_DRIVERS_NUM];
#endif

//    std::vector<int32_t>   target_vel ;
//    std::vector<int16_t>   target_tor ;
//    std::vector<int16_t>   max_tor ;
//    OpMode    op_mode ;
//    std::vector<int32_t>   vel_offset ;
//    std::vector<int16_t>   tor_offset ;
    
}DATA_SENT;


/// CIA 402 state machine motor states

enum MOTOR_STATES{
	K_READY_TO_SWITCH_ON = 1,
	K_SWITCHED_ON,
	K_OPERATION_ENABLED,
	K_FAULT,
	K_VOLTAGE_ENABLED,
	K_QUICK_STOP,
	K_SWITCH_ON_DISABLED,
	K_WARNING,
	K_REMOTE,
	K_TARGET_REACHED,
	K_INTERNAL_LIMIT_ACTIVATE
};

/// CiA402 Error register bits for error index.


enum ERROR_REGISTER_BITS{
    K_GENERIC_ERROR = 0,
    K_CURRENT_ERROR,
    K_VOLTAGE_ERROR,
    K_TEMPERATURE_ERROR,
    K_COMMUNICATION_ERROR,
    K_DEVICE_PROFILE_SPECIFIC_ERROR,
    K_RESERVED,
    K_MOTION_ERROR
};


/// offset for PDO entries to register PDOs.
typedef struct PDO_OFFSET
{
    uint32_t target_pos ;
    uint32_t target_vel ;
    uint32_t target_tor ;
    uint32_t torque_offset;
    uint32_t max_tor  ;
    uint32_t control_word ;
    uint32_t op_mode ;
    uint32_t profile_acc ;
    uint32_t profile_dec ;
    uint32_t quick_stop_dec ;
    uint32_t profile_vel ;
    uint32_t digital_out;
    uint32_t touch_probe_func;

    uint32_t homing_method;


    uint32_t actual_pos ;
    uint32_t pos_fol_err ;
    uint32_t actual_vel ;
    uint32_t actual_cur ;
    uint32_t actual_tor ;
    uint32_t status_word ;
    uint32_t op_mode_display ;
    uint32_t error_code ;
    uint32_t extra_status_reg ;
    uint32_t digital_in;
    uint32_t touch_probe_stat;
    uint32_t touch_probe_1_pval;
    uint32_t touch_probe_2_pval;

    uint32_t emergency_switch;
    uint32_t pressure_sensor; 
} PDO_OFFSET ;


/// EtherCAT SDO request structure for configuration phase.
typedef struct SDO_REQUEST
{
    ec_sdo_request_t * profile_acc ;    
    ec_sdo_request_t * profile_dec ;      
    ec_sdo_request_t * profile_vel ;  
    ec_sdo_request_t * quick_stop_dec ;
    ec_sdo_request_t * motion_profile_type ;
    ec_sdo_request_t * max_profile_vel ;
    ec_sdo_request_t * max_fol_err ;
    ec_sdo_request_t * speed_for_switch_search;
    ec_sdo_request_t * speed_for_zero_search;
    ec_sdo_request_t * homing_acc;
    ec_sdo_request_t * curr_threshold_homing;
    ec_sdo_request_t * home_offset;
    ec_sdo_request_t * homing_method;		
} SDO_REQUEST ;


/// Parameters that should be specified in position mode.
typedef struct PROFILE_POS_PARAM
{
    uint32_t profile_vel ;
    uint32_t profile_acc ;
    uint32_t profile_dec ;
    uint32_t max_fol_err ;
    uint32_t max_profile_vel ; 
    uint32_t quick_stop_dec ;
    uint16_t motion_profile_type ; 

    uint32_t homing_speed_zero;
    uint32_t homing_speed_switch;

    int32_t homing_offset_switch[MAX_DRIVERS_NUM];

    uint32_t p_gain;
    uint32_t i_gain;
    uint32_t d_gain;
} PROFILE_POS_PARAM ;




/**
 * @brief Struct containing 'velocity control parameter set' 0x30A2
 * Has 4 sub index. 
 */
 typedef struct VELOCITY_CONTROL_PARAM
 {
    uint32_t p_gain;     // micro amp sec per radian
    uint32_t i_gain;    // micro amp per radian
    uint32_t ff_vel_gain;
    uint32_t ff_acc_gain;
 } VELOCITY_CONTROL_PARAM;




/// Homing mode configuration parameters.
typedef struct HOMING_PARAM
{
	uint32_t	max_fol_err;
	uint32_t	max_profile_vel;
	uint32_t	quick_stop_dec;
	uint32_t	speed_for_switch_search;
	uint32_t	speed_for_zero_search;
	uint32_t	homing_acc;
    /// Used when homing by touching mechanical limit and sensing current
	uint16_t	curr_threshold_homing;
    /// Amount to move away from the sensed limit	
	int32_t		home_offset;
	int8_t		homing_method;
} HOMING_PARAM;

/// Profile velocity mode configuration parameters.
typedef struct PROFILE_VELOCITY_PARAM
{
    uint32_t	max_profile_vel;
    uint32_t	quick_stop_dec;
    uint32_t	profile_acc;
    uint32_t	profile_dec;
    uint16_t    motion_profile_type;
} PROFILE_VELOCITY_PARAM ;

enum ERROR_TYPE
{   
    NO_ERROR = 0,
    GENERIC_ERROR = 0x1000,
    GENERIC_INIT_ERROR = 0x1080,
    GENERIC_INIT_ERROR_1 = 0x1081,
    GENERIC_INIT_ERROR_2 = 0x1082,
    GENERIC_INIT_ERROR_3 = 0x1083,
    GENERIC_INIT_ERROR_4 = 0x1084,
    GENERIC_INIT_ERROR_5 = 0x1085,
    GENERIC_INIT_ERROR_6 = 0x1086,
    GENERIC_INIT_ERROR_7 = 0x1087,
    GENERIC_INIT_ERROR_8 = 0x1088,
    FIRMWARE_INCOMPATIBLITY_ERROR = 0x1090,
    OVER_CURRENT_ERROR = 0x2310,
    POWER_STAGE_PROTECTION_ERROR = 0x2320,
    OVER_VOLTAGE_ERROR = 0x3210,
    UNDER_VOLTAGE_ERROR = 0x3220,
    THERMAL_OVERLOAD_ERROR = 0x4210,
    THERMAL_MOTOR_OVERLOAD_ERRROR = 0x4380,
    LOGIC_SUPPLY_TOO_LOW_ERROR = 0x5113,
    HARDWARE_DEFECT_ERROR = 0x5280,
    HARDWARE_INCOMPATIBLITY_ERROR = 0x5281,
    HARDWARE_ERROR = 0x5480,
    HARDWARE_ERROR_1 = 0x5481,
    HARDWARE_ERROR_2 = 0x5482,
    HARDWARE_ERROR_3 = 0x5483,
    SIGN_OF_LIFE_ERROR = 0x6080,
    EXTENSION_1_WATCHDOG_ERROR = 0x6081,
    INTERNAL_SOFTWARE_ERROR = 0x6180,
    SOFTWARE_PARAMETER_ERROR = 0x6320,
    PERSISTENT_PARAMETER_CORRUPT_ERROR = 0x6380,
    POSITION_SENSOR_ERROR = 0x7320,
    POSITION_SENSOR_BREACH_ERROR = 0x7380,
    POSITION_SENSOR_RESOLUTION_ERROR  = 0x7381,
    POSITION_SENSOR_INDEX_ERROR = 0x7382,
    HALL_SENSOR_ERROR = 0x7388,
    HALL_SENSOR_NOT_FOUND_ERROR =  0x7389,
    HALL_ANGLE_DETECTION_ERROR = 0x738A,
    SSI_SENSOR_ERROR = 0x738C,
    SSI_SENSOR_FRAME_ERROR = 0x738D,
    MISSING_MAIN_SENSOR_ERROR = 0x7390,
    MISSING_COMMUTATION_SENSOR_ERROR = 0x7391, 
    MAIN_SENSOR_DIRECTION_ERROR = 0x7392,
    ETHERCAT_COMMUNCATION_ERROR = 0x8180,
    ETHERCAT_INITIALIZATION_ERROR = 0x8181,
    ETHERCAT_RX_QUEUE_OVERFLOW_ERROR = 0x8182,
    ETHERCAT_COMMUNICATION_ERROR_INTERNAL  = 0x8183,
    ETHERCAT_COMMUNICATION_CYCLE_TIME_ERROR = 0x8184,
    ETHERCAT_PDO_COMMUNICATION_ERROR = 0x8280,
    ETHERCAT_SDO_COMMUNICATION_ERROR = 0x8281,
    FOLLOWING_ERROR = 0x8611,
    NEGATIVE_LIMIT_SWITCH_ERROR = 0x8A80,
    POSITIVE_LIMIT_SWITCH_ERROR = 0x8A81,
    SOFTWARE_POSITION_LIMIT_ERROR = 0x8A82,
    STO_ERROR = 0x8A88,
    SYSTEM_OVERLOADED_ERROR = 0xFF01,
    WATCHDOG_ERROR = 0xFF02,
    SYSTEM_PEAK_OVERLOADED_ERROR = 0XFF0B,
    CONTROLLER_GAIN_ERROR = 0xFF10,
    AUTO_TUNING_INDENTIFICATION_ERROR = 0xFF11,
    AUTO_TUNING_CURRENT_LIMIT_ERROR = 0xFF12,
    AUTO_TUNING_IDENTIFICATION_CURRENT_ERROR = 0xFF13,
    AUTO_TUNING_DATA_SAMPLING_ERROR  = 0xFF14,
    AUTO_TUNING_SAMPLE_MISMATCH_ERROR = 0xFF15,
    AUTO_TUNING_PARAMETER_ERROR = 0xFF16,
    AUTO_TUNING_AMPLITUDE_MISMATCH_ERROR = 0xFF17,
    AUTO_TUNING_TIMEOUT_ERROR = 0xFF19,
    AUTO_TUNING_STAND_STILL_ERROR = 0xFF20,
    AUTO_TUNING_TORQUE_INVALID_ERROR = 0xFF21,
    AUTO_TUNING_MAX_SYSTEM_SPEED_ERROR = 0XFF22,
    AUTO_TUNING_MOTOR_CONNECTION_ERROR = 0xFF23,
    AUTO_TUNING_SENSOR_SIGNAL_ERROR = 0XFF24
};

/*
static std::string GetErrorMessage(const int& err_code)
{
    switch (err_code)
    {
        case NO_ERROR:
            return "No error";
        case GENERIC_ERROR:
            return "Generic error";
        case GENERIC_INIT_ERROR:
            return "Generic initialization error";
        case GENERIC_INIT_ERROR_1:
            return "Generic initialization error 1";
        case GENERIC_INIT_ERROR_2:
            return "Generic initialization error 2";
        case GENERIC_INIT_ERROR_3:
            return "Generic initialization error 3";
        case GENERIC_INIT_ERROR_4:
            return "Generic initialization error 4";
        case GENERIC_INIT_ERROR_5:
            return "Generic initialization error 5";
        case GENERIC_INIT_ERROR_6:
            return "Generic initialization error 6";
        case GENERIC_INIT_ERROR_7:
            return "Generic initialization error 7";
        case GENERIC_INIT_ERROR_8:
            return "Generic initialization error 8";
        case FIRMWARE_INCOMPATIBLITY_ERROR:
            return "Firmware incompatibility error";
        case OVER_CURRENT_ERROR:
            return "Over current error";
        case POWER_STAGE_PROTECTION_ERROR:
            return "Power stage protection error";
        case OVER_VOLTAGE_ERROR:
            return "Over voltage error";
        case UNDER_VOLTAGE_ERROR:
            return "Under voltage error";
        case THERMAL_OVERLOAD_ERROR:
            return "Thermal overload error";
        case THERMAL_MOTOR_OVERLOAD_ERRROR:
            return "Thermal motor overload error";
        case LOGIC_SUPPLY_TOO_LOW_ERROR:     
            return "Logic supply too low error";
        case HARDWARE_DEFECT_ERROR:
            return "Hardware defect error"; 
        case HARDWARE_INCOMPATIBLITY_ERROR:
            return "Hardware incompatibility error";
        case HARDWARE_ERROR:
            return "Hardware error";
        case HARDWARE_ERROR_1:
            return "Hardware error 1";
        case HARDWARE_ERROR_2:
            return "Hardware error 2";
        case HARDWARE_ERROR_3:
            return "Hardware error 3";
        case SIGN_OF_LIFE_ERROR:
            return "Sign of life error";
        case EXTENSION_1_WATCHDOG_ERROR:
            return "Extension 1 watchdog error";
        case INTERNAL_SOFTWARE_ERROR:  
            return "Internal software error";
        case SOFTWARE_PARAMETER_ERROR:
            return "Software parameter error";
        case PERSISTENT_PARAMETER_CORRUPT_ERROR:
            return "Persistent parameter corrupt error";
        case POSITION_SENSOR_ERROR:
            return "Position sensor error";
        case POSITION_SENSOR_BREACH_ERROR:
            return "Position sensor breach error";
        case POSITION_SENSOR_RESOLUTION_ERROR:  
            return "Position sensor resolution error";
        case POSITION_SENSOR_INDEX_ERROR:
            return "Position sensor index error";
        case HALL_SENSOR_ERROR:
            return "Hall sensor error";
        case HALL_SENSOR_NOT_FOUND_ERROR:
            return "Hall sensor not found error";
        case HALL_ANGLE_DETECTION_ERROR:
            return "Hall angle detection error";
        case SSI_SENSOR_ERROR:
            return "SSI sensor error";
        case SSI_SENSOR_FRAME_ERROR:
            return "SSI sensor frame error";
        case MISSING_MAIN_SENSOR_ERROR:
            return "Missing main sensor error";
        case MISSING_COMMUTATION_SENSOR_ERROR:
            return "Missing commutation sensor error";
        case MAIN_SENSOR_DIRECTION_ERROR:
            return "Main sensor direction error";
        case ETHERCAT_COMMUNCATION_ERROR:
            return "Ethercat communication error";
        case ETHERCAT_INITIALIZATION_ERROR:
            return "Ethercat initialization error";
        case ETHERCAT_RX_QUEUE_OVERFLOW_ERROR:
            return "Ethercat RX queue overflow error";
        case ETHERCAT_COMMUNICATION_ERROR_INTERNAL:
            return "Ethercat communication error internal";
        case ETHERCAT_COMMUNICATION_CYCLE_TIME_ERROR:
            return "Ethercat communication cycle time error";
        case ETHERCAT_PDO_COMMUNICATION_ERROR:
            return "Ethercat PDO communication error";
        case ETHERCAT_SDO_COMMUNICATION_ERROR:
            return "Ethercat SDO communication error";
        case FOLLOWING_ERROR:
            return "Following error";
        case NEGATIVE_LIMIT_SWITCH_ERROR :
            return "Negative limit switch error";
        case POSITIVE_LIMIT_SWITCH_ERROR :
            return "Positive limit switch error";
        case SOFTWARE_POSITION_LIMIT_ERROR:
            return "Software position limit error";
        case STO_ERROR : 
            return "STO error";
        case SYSTEM_OVERLOADED_ERROR:
            return "System overloaded error";
        case WATCHDOG_ERROR:
            return "Watchdog error";
        case SYSTEM_PEAK_OVERLOADED_ERROR: 
            return "System peak overloaded error";
        case CONTROLLER_GAIN_ERROR:
            return "Controller gain error";
        case AUTO_TUNING_INDENTIFICATION_ERROR: 
            return "Auto tuning identification error";
        case AUTO_TUNING_CURRENT_LIMIT_ERROR:
            return "Auto tuning current limit error";
        case AUTO_TUNING_IDENTIFICATION_CURRENT_ERROR:
            return "Auto tuning identification current error";
        case AUTO_TUNING_DATA_SAMPLING_ERROR:
            return "Auto tuning data sampling error";
        case AUTO_TUNING_SAMPLE_MISMATCH_ERROR:
            return "Auto tuning sample mismatch error";
        case AUTO_TUNING_PARAMETER_ERROR:
            return "Auto tuning parameter error";
        case AUTO_TUNING_AMPLITUDE_MISMATCH_ERROR:
            return "Auto tuning amplitude mismatch error";
        case AUTO_TUNING_TIMEOUT_ERROR:
            return "Auto tuning timeout error";
        case AUTO_TUNING_STAND_STILL_ERROR:
            return "Auto tuning stand still error";
        case AUTO_TUNING_TORQUE_INVALID_ERROR:
            return "Auto tuning torque invalid error";
        case AUTO_TUNING_MAX_SYSTEM_SPEED_ERROR:
            return "Auto tuning max system speed error";
        case AUTO_TUNING_MOTOR_CONNECTION_ERROR:
            return "Auto tuning motor connection error";
        case AUTO_TUNING_SENSOR_SIGNAL_ERROR:
            return "Auto tuning sensor signal error";        
        default:
            return "Unknown error";
    }
}

*/

#define MAX_STATUS_STRLEN 8
#define MAX_POSITION_STRLEN 32
#define MAX_BUFF 1024 * 10


extern int we_are_testing;

#endif