#include "ec-server/wheel/core.h"


pthread_t CAN_SOCK_PTID;


FILE* TFP;

char CAN_DEV_NAME[MAX_CAN_DEV_NAME] = {0};



char CAN_NODE_ID_STR[MAX_CAN_NODE_NAME] = {0};

int CAN_SOCKFD;

char SET_LOCAL_SOCKET[MAX_SET_LOCAL_SOCK] = {0};
char LOCAL_SOCKET_NAME[MAX_SOCK_PATH] = {0};




WheelCmdStruct ecwheel_control_word = { .seq = "101", .index = "0x6040", .subindex = "0", .datatype = "i16"};
WheelCmdStruct ecwheel_rotation_direction = { .seq = "501", .index = "0x6410" , .subindex = "0x13", .datatype = "u8"};
WheelCmdStruct ecwheel_actual_position = { .seq = "201", .index = "0x6063", .subindex = "0", .datatype = "i32"};
WheelCmdStruct ecwheel_actual_position_clear = { .seq = "202", .index = "0x607C", .subindex = "2", .datatype = "u8"};
WheelCmdStruct ecwheel_target_velocity = { .seq = "301", .index = "0x60FF", .subindex = "0", .datatype = "i32"};
WheelCmdStruct ecwheel_acceleration = { .seq = "401", .index = "0x6083", .subindex = "0", .datatype = "u32"};
WheelCmdStruct ecwheel_deceleration = { .seq = "402", .index = "0x6084", .subindex = "0", .datatype = "u32"};


int InitWheelDaemon(char* can_dev_name, int can_node_id){


    TFP = fopen("/tmp/CO_command_log", "a");

    fdebugLog("initiating...");


    strncpy(CAN_DEV_NAME, can_dev_name, MAX_CAN_DEV_NAME);

    sprintf(CAN_NODE_ID_STR, "%d", can_node_id);
 
    get_overriding_socket(LOCAL_SOCKET_NAME);
    
    strcat(SET_LOCAL_SOCKET, "local-");
    strcat(SET_LOCAL_SOCKET, LOCAL_SOCKET_NAME);


    int ret = pthread_create(&CAN_SOCK_PTID, NULL, &CO_daemon_start, NULL); 

    fdebugLog("initiated");

    return ret;

}

void* CO_daemon_start(void* varg){

    char* args[] = {"./canopend", CAN_DEV_NAME, "-i", CAN_NODE_ID_STR, "-c", SET_LOCAL_SOCKET, NULL};


    pid_t dpid = fork();

    if(dpid < 0){

        printf("failed to start daemon");

        exit(EXIT_FAILURE);
    }

    char pid_log[128] = {0};
    char pid_dbg[28] = {0};

    if(dpid == 0){

        execvp(args[0], args);

        exit(EXIT_FAILURE);

    } else {
        
        sprintf(pid_dbg, "%d", dpid);

        strcat(pid_log, "daemon_pid: ");

        strcat(pid_log, pid_dbg);

        strcat(pid_log, "\n");

        fdebugLog(pid_log);

        int status = 0;

        waitpid(dpid, &status, 0);

    }


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

    // TODO:
    //   use config

    char out[MAX_CAN_CMD_OUT] = {0};

    char incmd[MAX_CAN_CMD_IN] = {0};


    for(int i = 1; i < 5; i ++){
        // TODO:
        //   set rotation

        GetWheelCmd_SetAcceleration(incmd, i, 100);

        WheelCmdGatewayASCII(out, incmd);

        printf("wheel: set accel: %s\n", out);

        memset(out, 0, MAX_CAN_CMD_OUT);

        GetWheelCmd_SetDeceleration(incmd, i, 100);

        WheelCmdGatewayASCII(out, incmd);

        printf("wheel: set decel: %s\n", out);

        memset(out, 0, MAX_CAN_CMD_OUT);

        GetWheelCmd_MotorEnable(incmd, i);

        WheelCmdGatewayASCII(out, incmd);

        printf("wheel: motor enable: %s\n", out);

        memset(out, 0, MAX_CAN_CMD_OUT);


    }

    return 0;

}



void GetWheelCmd_TargetVelocity(char* incmd, int axis, char* speed){

    char node_id[16] = {0};

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[101] %s w %s %s %s %s \n", node_id, ecwheel_target_velocity.index, ecwheel_target_velocity.subindex, ecwheel_target_velocity.datatype, speed);

}

void GetWheelCmd_MotorEnable(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "0x0F";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_control_word.seq, 
        node_id, 
        ecwheel_control_word.index, 
        ecwheel_control_word.subindex, 
        ecwheel_control_word.datatype, 
        this_value
        );

}

void GetWheelCmd_MotorDisable(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "0x06";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_control_word.seq, 
        node_id, 
        ecwheel_control_word.index, 
        ecwheel_control_word.subindex, 
        ecwheel_control_word.datatype, 
        this_value
        );

}

void GetWheelCmd_ClearEncoderValue(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "1";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_control_word.seq, 
        node_id, 
        ecwheel_actual_position_clear.index, 
        ecwheel_actual_position_clear.subindex, 
        ecwheel_actual_position_clear.datatype, 
        this_value
        );


}

void GetWheelCmd_SetRotationClockwise(char* incmd, int axis){

    char node_id[16] = {0};

    char* this_value = "1";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_rotation_direction.seq, 
        node_id, 
        ecwheel_rotation_direction.index, 
        ecwheel_rotation_direction.subindex, 
        ecwheel_rotation_direction.datatype, 
        this_value
        );


}

void GetWheelCmd_SetRotationCounterClockwise(char* incmd, int axis){
    char node_id[16] = {0};

    char* this_value = "0";

    sprintf(node_id, "%d", axis);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_rotation_direction.seq, 
        node_id, 
        ecwheel_rotation_direction.index, 
        ecwheel_rotation_direction.subindex, 
        ecwheel_rotation_direction.datatype, 
        this_value
        );

}


void GetWheelCmd_SetAcceleration(char* incmd, int axis, int accel){

    char node_id[16] = {0};

    char this_value[64] = {0};

    sprintf(node_id, "%d", axis);

    sprintf(this_value, "%d", accel);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_acceleration.seq, 
        node_id, 
        ecwheel_acceleration.index, 
        ecwheel_acceleration.subindex, 
        ecwheel_acceleration.datatype, 
        this_value
        );

}


void GetWheelCmd_SetDeceleration(char* incmd, int axis, int decel){

    char node_id[16] = {0};

    char this_value[64] = {0};

    sprintf(node_id, "%d", axis);

    sprintf(this_value, "%d", decel);

    sprintf(incmd, "[%s] %s w %s %s %s %s \n", 
        ecwheel_deceleration.seq, 
        node_id, 
        ecwheel_deceleration.index, 
        ecwheel_deceleration.subindex, 
        ecwheel_deceleration.datatype, 
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



int get_overriding_socket(char* sock_target_name){

    char fname[MAX_SOCK_PATH] = {0};

    strcat(fname, "/tmp/CO_command_socket");

    if (access(fname, F_OK) == 0) {

        unlink(fname);
    
    } 

    strncpy(sock_target_name, fname, MAX_SOCK_PATH);

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




