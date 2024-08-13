#include "ec-server/wheel/core.h"
#include "ec-server/wheel/object_dictionary.h"
#include "ec-server/wheel/utils.h"

pthread_t CAN_SOCK_PTID;


FILE* TFP;

char CAN_DEV_NAME[MAX_CAN_DEV_NAME] = {0};



char CAN_NODE_ID_STR[MAX_CAN_NODE_NAME] = {0};

int CAN_SOCKFD;

char SET_LOCAL_SOCKET[MAX_SET_LOCAL_SOCK] = {0};
char LOCAL_SOCKET_NAME[MAX_SOCK_PATH] = {0};

int g_num_of_wheel_slaves = 0;
int* g_wheels_node_ids = NULL;
int* node_rotation_direction = NULL;

int wheel_acceleration = 0;
int wheel_deceleration = 0;



int InitWheelDaemon(char* can_dev_name, char* can_node_id){


    TFP = fopen("/tmp/EC_wheel_log", "a");

    fdebugLog("initiating...");

 
    get_overriding_socket();
    
    strcat(SET_LOCAL_SOCKET, "local-");
    strcat(SET_LOCAL_SOCKET, LOCAL_SOCKET_NAME);


    //int ret = pthread_create(&CAN_SOCK_PTID, NULL, &CO_daemon_start, NULL); 

    pid_t ppid = getpid();


    char* args[] = {"./canopend", CAN_DEV_NAME, "-i", CAN_NODE_ID_STR, "-c", SET_LOCAL_SOCKET, NULL};


    pid_t dpid = fork();

    if(dpid < 0){

        printf("failed to start daemon");

        exit(EXIT_FAILURE);
    }

    char pid_log[128] = {0};
    char pid_dbg[28] = {0};

    if(dpid == 0){

        setsid();

        signal(SIGINT, SIG_IGN);
 
        execvp(args[0], args);
    
    } else {
        
        sprintf(pid_dbg, "%d", dpid);

        strcat(pid_log, "daemon_pid: ");

        strcat(pid_log, pid_dbg);

        strcat(pid_log, "\n");

        FILE* pidfile = fopen("co.pid", "w");

        fputs(pid_dbg, pidfile);

        fclose(pidfile);

        fdebugLog(pid_log);

        //int status = 0;

        //waitpid(dpid, &status, 0);
    }

    fdebugLog("initiated");

    return 0;

}




int InitWheelCmdGateway(){

    //signal(SIGPIPE, SIG_IGN);

    int retry = 30;

    CAN_SOCKFD = socket(AF_UNIX, SOCK_STREAM, 0);
    if (CAN_SOCKFD == -1) {
        return -11;
    }

    struct sockaddr_un address;

    address.sun_family = AF_UNIX;


    strcpy(address.sun_path, LOCAL_SOCKET_NAME);


    int i = 0;
    int connection_result;

    while(i < retry){
        

        connection_result = connect(CAN_SOCKFD, (struct sockaddr *)&address, sizeof(address));

        if(connection_result == 0){

            break;
        }

        sleep(1);

        i++;
    }

    WheelCmdSetUp();

    return connection_result;

}


int WheelCmdSetUp(){


    char out[MAX_CAN_CMD_OUT] = {0};

    char incmd[MAX_CAN_CMD_IN] = {0};


    for(int i = 0; i < g_num_of_wheel_slaves; i ++){
        
        int nid = g_wheels_node_ids[i];

        int clockwise = node_rotation_direction[i];

        if(clockwise == 1){

            GetWheelCmd_SetRotationClockwise(incmd, nid);

        } else {

            GetWheelCmd_SetRotationCounterClockwise(incmd, nid);
        }

        WheelCmdGatewayASCII(out, incmd);

        GetWheelCmd_SetAcceleration(incmd, nid, wheel_acceleration);

        WheelCmdGatewayASCII(out, incmd);

        printf("wheel: set accel: %s\n", out);

        memset(out, 0, MAX_CAN_CMD_OUT);

        GetWheelCmd_SetDeceleration(incmd, nid, wheel_deceleration);

        WheelCmdGatewayASCII(out, incmd);

        printf("wheel: set decel: %s\n", out);

        memset(out, 0, MAX_CAN_CMD_OUT);

        GetWheelCmd_MotorEnable(incmd, nid);

        WheelCmdGatewayASCII(out, incmd);

        printf("wheel: motor enable: %s\n", out);

        memset(out, 0, MAX_CAN_CMD_OUT);


    }

    return 0;

}



void GetWheelCmd_TargetVelocity(char* incmd, int axis, char* speed){

    char node_id[16] = {0};

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s \n",
        SEQ_TARGET_VELOCITY,
        node_id, 
        OD_TARGET_VELOCITY, 
        DT_TARGET_VELOCITY, 
        speed
        );

}

void GetWheelCmd_MotorEnable(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "0x0F";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_CONTROL_WORD, 
        node_id, 
        OD_CONTROL_WORD, 
        DT_CONTROL_WORD, 
        this_value
        );

}

void GetWheelCmd_MotorDisable(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "0x06";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_CONTROL_WORD, 
        node_id, 
        OD_CONTROL_WORD, 
        DT_CONTROL_WORD, 
        this_value
        );

}

void GetWheelCmd_ClearEncoderValue(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "1";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_ACTUAL_POSITION_CLEAR, 
        node_id, 
        OD_ACTUAL_POSITION_CLEAR, 
        DT_ACTUAL_POSITION_CLEAR, 
        this_value
        );


}

void GetWheelCmd_SetRotationClockwise(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "1";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_ROTATION_DIRECTION, 
        node_id, 
        OD_ROTATION_DIRECTION, 
        DT_ROTATION_DIRECTION, 
        this_value
        );


}

void GetWheelCmd_SetRotationCounterClockwise(char* incmd, int axis){
    char node_id[16] = {0};

    char* this_value = "0";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_ROTATION_DIRECTION, 
        node_id, 
        OD_ROTATION_DIRECTION, 
        DT_ROTATION_DIRECTION, 
        this_value
        );

}


void GetWheelCmd_SetAcceleration(char* incmd, int axis, int accel){

    char node_id[16] = {0};

    char this_value[64] = {0};

    sprintf(node_id, "%d", axis);

    sprintf(this_value, "%d", accel);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_ACCELERATION, 
        node_id, 
        OD_ACCELERATION, 
        DT_ACCELERATION, 
        this_value
        );

}


void GetWheelCmd_SetDeceleration(char* incmd, int axis, int decel){

    char node_id[16] = {0};

    char this_value[64] = {0};

    sprintf(node_id, "%d", axis);

    sprintf(this_value, "%d", decel);

    sprintf(incmd, "[%s] %s w %s %s %s \n", 
        SEQ_DECELERATION, 
        node_id, 
        OD_DECELERATION, 
        DT_DECELERATION, 
        this_value
        );


}



int WheelCmdGatewayASCII(char* out, char* in){


    int ret;

    fdebugLog(in);

    char ret_out[MAX_CAN_CMD_OUT] = {0};

    if((ret = write(CAN_SOCKFD, in, MAX_CAN_CMD_IN)) < 0){

        return -2;

    }

    if ((ret = read(CAN_SOCKFD, ret_out, MAX_CAN_CMD_OUT)) < 0){
        return -1;
    }
    
    // int clen = strncpy_ignn(ret_out, out, MAX_CAN_CMD_OUT);

    ret = strncpy_process(ret_out, out);

    if(ret <=0){
        fdebugLog("failed output copying");
    }

    fdebugLog(out);

    memset(in, 0 , MAX_CAN_CMD_IN);

    return ret;

}



void WheelShutdown(){

    char out[MAX_CAN_CMD_OUT] = {0};

    char incmd[MAX_CAN_CMD_IN] = {0};

    for(int i = 0; i < g_num_of_wheel_slaves; i++){

        int nid = g_wheels_node_ids[i];

        GetWheelCmd_MotorDisable(incmd, nid);

        WheelCmdGatewayASCII(out, incmd);

        memset(out, 0, MAX_CAN_CMD_OUT);

    }


    FreeWheelRuntime();


    char pidstr[24] = {0};

    FILE* pidfile = fopen("co.pid", "r");

    fgets(pidstr, 24, pidfile);

    int pidnum;

    sscanf(pidstr, "%d", &pidnum);

    kill(pidnum, SIGKILL);

}


int strncpy_process(char* src, char* dst){

    int index = 0;

    int copied_len = 0;

    for(int i = 0; i < MAX_CAN_CMD_OUT; i ++){

        if(src[i] == ']'){
            index = i + 2;
            break;
        }


    }

    if(index == 0 ){
        return -2;
    }

    int i = 0;

    for(;index < MAX_CAN_CMD_OUT; index++){
        dst[i] = src[index];
        i++;
        copied_len += 1;
    }


    return copied_len;

}



int get_overriding_socket(){

    char fname[MAX_SOCK_PATH] = {0};

    strcat(fname, LOCAL_SOCKET_NAME);

    if (access(fname, F_OK) == 0) {

        unlink(fname);
    
    } 

    return 0;

}


void fdebugLog(char* msg){

    char log_message[MAX_LOG_MESSAGE] = {0};

    time_t t = time(NULL);
    struct tm loctm = *localtime(&t);

    sprintf(log_message, "[%d-%02d-%02d %02d:%02d:%02d]: ", loctm.tm_year + 1900, loctm.tm_mon + 1, loctm.tm_mday, loctm.tm_hour, loctm.tm_min, loctm.tm_sec);

    strcat(log_message, msg);

    fputs(log_message, TFP);
    fputs("\n", TFP);
    fflush(TFP);

}




