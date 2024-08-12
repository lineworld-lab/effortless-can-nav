#include "ec-server/wheel/core.h"


pthread_t SOCK_PTID;


FILE* TFP;

char CAN_DEV_NAME[MAX_CAN_DEV_NAME] = {0};

int CAN_NODE_ID; 

char* CAN_NODE_ID_STR[MAX_CAN_NODE_NAME] = {0};

int SOCKFD;

char SET_LOCAL_SOCKET[MAX_SET_LOCAL_SOCK] = {0};
char LOCAL_SOCKET_NAME[MAX_SOCK_PATH] = {0};



int InitWheelDaemon(char* can_dev_name, int can_node_id){


    TFP = fopen("/tmp/CO_command_log", "a");

    fdebugLog("initiating...");


    strncpy(CAN_DEV_NAME, can_dev_name, MAX_CAN_DEV_NAME);

    sprintf(CAN_NODE_ID_STR, "%d", can_node_id);
 
    get_overriding_socket(LOCAL_SOCKET_NAME);
    
    strcat(SET_LOCAL_SOCKET, "\"");
    strcat(SET_LOCAL_SOCKET, "local-");
    strcat(SET_LOCAL_SOCKET, LOCAL_SOCKET_NAME);
    strcat(SET_LOCAL_SOCKET, "\"");

    int ret = pthread_create(&SOCK_PTID, NULL, &CO_daemon_start, NULL); 

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

    SOCKFD = socket(AF_UNIX, SOCK_STREAM, 0);
    if (SOCKFD == -1) {
        return -11;
    }

    struct sockaddr_un address;

    address.sun_family = AF_UNIX;


    strcpy(address.sun_path, LOCAL_SOCKET_NAME);


    int i = 0;
    int connection_result;

    while(i < retry){
        

        connection_result = connect(SOCKFD, (struct sockaddr *)&address, sizeof(address));

        if(connection_result == 0){

            break;
        }

        sleep(1);

        i++;
    }

    return connection_result;

}



int WheelCmdGatewayASCII(char* in, char* out){


    int ret;

    fdebugLog(in);

    char ret_out[MAX_CAN_CMD_OUT] = {0};

    if((ret = write(SOCKFD, in, MAX_CAN_CMD_IN)) < 0){

        return -2;

    }

    if ((ret = read(SOCKFD, ret_out, MAX_CAN_CMD_OUT)) < 0){
        return -1;
    }
    
    // int clen = strncpy_ignn(ret_out, out, MAX_CAN_CMD_OUT);

    ret = strncpy_process(ret_out, out);

    if(ret <=0){
        fdebugLog("failed output copying");
    }

    fdebugLog(out);


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




