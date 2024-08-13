#include "ec-client/v1/request.h"



int SOCK_FD;
char ADDR[MAX_BUFFLEN] = {0};
int PORT;
long TIMEOUT;
int VCONN = 0;


int InitECCmdGateway(char* res, char* addr, int port){

    printf("initiaing cmd gateway\n");

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        
        strcpy(res, "socket creation error");

        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
 

    if (inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0) {

        strcpy(res, "invalid address, address not supported");
        return -2;
    }
 
    if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
    
        strcpy(res, "connection failure");

        return -3;
    }


    SOCK_FD = client_fd;

    strcpy(ADDR, addr);

    PORT = port;

    strcpy(res, "successfully connected");


    printf("successfully initiated cmd gateway\n");

    return 0;

}




int InitECCmdGatewayWithTimeout(char* res, char* addr, int port, long timeout){

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        
        printf("socket creation error\n");

        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
 

    if (inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0) {

        printf("invalid address, address not supported\n");
        return -2;
    }


    SOCK_FD = client_fd;

    struct timeval u_timeout;      
    u_timeout.tv_sec = timeout / 1000;
    u_timeout.tv_usec = 0;
    

    if (setsockopt (SOCK_FD, SOL_SOCKET, SO_RCVTIMEO, &u_timeout, sizeof(u_timeout)) < 0){

        printf("set recv timeout\n");

        return -3;
    }
    

    if (setsockopt (SOCK_FD, SOL_SOCKET, SO_SNDTIMEO, &u_timeout, sizeof(u_timeout)) < 0) {

        printf("set send timeout\n");
    
        return -4;
    }
    

    int rc = 0;
    
    int sockfd_flags_before;

    sockfd_flags_before = fcntl(SOCK_FD,F_GETFL,0);
    
    if(sockfd_flags_before < 0) {
    
        printf("failed to getfl\n");

        return -5;
    
    }
    
    

    if(fcntl(SOCK_FD, F_SETFL, sockfd_flags_before | O_NONBLOCK)<0){


        printf("failed to setfl\n");

        return -6;
    
    } 



 

    if (connect(SOCK_FD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {

            rc = -11;

        } else {
            
            struct timespec now;

            clock_gettime(CLOCK_MONOTONIC_RAW, &now);

            struct timespec deadline;


            do{

                clock_gettime(CLOCK_MONOTONIC_RAW, &deadline);

                int ms_until_deadline = ((deadline.tv_sec - now.tv_sec) * 1000 + (deadline.tv_nsec - now.tv_nsec) / 1000000);
 
                if(ms_until_deadline > timeout) { 
                    rc = -10; 
                    break; 
                }

                struct timeval tv;
                
                tv.tv_sec = 0;

                tv.tv_usec = 10000; // 10ms

                fd_set sel_fd; 

                FD_ZERO(&sel_fd); 
                FD_SET(SOCK_FD, &sel_fd); 

                rc = select(SOCK_FD + 1, NULL, &sel_fd, NULL, &tv); 
                

                if(rc > 0) {
                    int error = 0; 
                    socklen_t len = sizeof(error);
                    int retval = getsockopt(SOCK_FD, SOL_SOCKET, SO_ERROR, &error, &len);
                    if(retval == 0) {
                        errno = error;
                    }
                    if(error != 0) {
                        rc = -11;
                    }
                }

            } while(rc == -11 && errno == EINTR);


            if(rc == -10) {
                
                errno = ETIMEDOUT;
                
            }

        }

    } 

    
    

    if(fcntl(SOCK_FD,F_SETFL,sockfd_flags_before ) < 0) {

        return -20;
    }

    strcpy(ADDR, addr);

    PORT = port;

    TIMEOUT = timeout;

    VCONN = 1;

    strcpy(res, "successfully connected before timed out");

    return rc;
}




int ECCmdGatewayAR(char* res, int ar_len, AxisReq* var){



    int cmd_stat = 0;

    int init_stat = 0;

    int ret_stat = 0;



    cmd_stat = TryECCmdGatewayARFeedback(res, ar_len, var);


    if(cmd_stat < 0){

 
        init_stat = TryInitECCmdGateway(res);

        
    }

    if(init_stat < 0){
        VCONN = 0;
        init_stat = -10;
    } else {
        VCONN = 1;
    }


    ret_stat = cmd_stat + init_stat;

    return ret_stat;



}

int ECCmdGatewayARTest(char* res){


    AxisReq var[MAX_ARG_LEN];

    int var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);

    AxisReq ar;

    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "twv");

        strcpy(ar.params, "0x2BB0"); // 10rpm

        var[var_count] = ar;

        var_count += 1;
    }

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 0;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "50000"); // 10rpm

    var[var_count] = ar;

    var_count += 1;

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 1;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "50000"); // 10rpm

    var[var_count] = ar;
    
    var_count += 1;


    int ret_code = ECCmdGatewayAR(res, var_count, var);

    sleep(3);

    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);


    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "twv");

        strcpy(ar.params, "0x2BB0"); // 10rpm

        var[var_count] = ar;

        var_count += 1;
    }

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 0;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "-50000"); // 10rpm

    var[var_count] = ar;

    var_count += 1;

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 1;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "-50000"); // 10rpm

    var[var_count] = ar;
    
    var_count += 1;

    ret_code = ECCmdGatewayAR(res, var_count, var);

    sleep(3);

    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);

    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "twv");

        strcpy(ar.params, "0x2BB0"); // 10rpm

        var[var_count] = ar;

        var_count += 1;
    }

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 0;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "0"); // 10rpm

    var[var_count] = ar;

    var_count += 1;

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 1;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "0"); // 10rpm

    var[var_count] = ar;
    
    var_count += 1;

    ret_code = ECCmdGatewayAR(res, var_count, var);

    sleep(3);

    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);


    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "twv");

        strcpy(ar.params, "0"); // 10rpm

        var[var_count] = ar;

        var_count += 1;
    }

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 0;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "0"); // 10rpm

    var[var_count] = ar;

    var_count += 1;

    memset(&ar, 0, sizeof(AxisReq));

    ar.axis = 1;

    strcpy(ar.action, "tmo");

    strcpy(ar.params, "0"); // 10rpm

    var[var_count] = ar;
    
    var_count += 1;

    ret_code = ECCmdGatewayAR(res, var_count, var);


    sleep(1);

    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);

    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "wro");

        strcpy(ar.params, "0"); // backward

        var[var_count] = ar;

        var_count += 1;
    }

    ret_code = ECCmdGatewayAR(res, var_count, var);

    sleep(1);

    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);


    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "twv");

        strcpy(ar.params, "0x2BB0"); // 10rpm

        var[var_count] = ar;

        var_count += 1;
    }


    ret_code = ECCmdGatewayAR(res, var_count, var);

    sleep(5);



    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);


    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "twv");

        strcpy(ar.params, "0"); // 10rpm

        var[var_count] = ar;

        var_count += 1;
    }


    ret_code = ECCmdGatewayAR(res, var_count, var);

    sleep(1);

    var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);

    for(int i = 0 ; i < 4; i ++){

        memset(&ar, 0, sizeof(AxisReq));

        ar.axis = i;

        strcpy(ar.action, "wro");

        strcpy(ar.params, "1"); // frontward

        var[var_count] = ar;

        var_count += 1;
    }

    ret_code = ECCmdGatewayAR(res, var_count, var);


    return ret_code;

}

int TryECCmdGatewayARFeedback(char* res, int ar_len, AxisReq* var){



    if(VCONN != 1){
        return -100;
    }

    char check_buff[MAX_BUFFLEN] = {0};

    char send_buff[MAX_BUFFLEN] = {0};

    char status_str[MAX_BUFFLEN] = {0};

    char feedback_str[MAX_BUFFLEN] = {0};

    int feedback;

    int status_flag;

    int valsent;

    int valread;


    char* delim = " ";

    for(int i = 0 ; i < ar_len; i ++){


        memset(send_buff, 0, MAX_BUFFLEN);
        memset(check_buff, 0, MAX_BUFFLEN);

        char recv_buff[MAX_BUFFLEN] = {0};

        char res_buff[MAX_BUFFLEN] = {0};

        char axis_str[20] = {0};

        sprintf(axis_str, "%d", var[i].axis);

        strcat(send_buff, axis_str);
        strcat(send_buff, " ");


        strcat(send_buff, var[i].action);
        strcat(send_buff, " ");


        strcat(send_buff, var[i].params);
        strcat(send_buff, " ");


        // valsent = send(SOCK_FD, req_buff, strlen(req_buff), 0);


        valsent = write(SOCK_FD, send_buff, MAX_BUFFLEN);

        if(valsent < 1){

            var[i].status = -1;

            var[i].feedback = valsent;

            strcpy(res, "error send");

            return -1;
        }

        if(strcmp(var[i].action,"DIEBRO") == 0){

            strcpy(res, "DIEBRO sent");

            return 1;


        }

        while(valread < MAX_BUFFLEN){


            int tmpread = read(SOCK_FD, recv_buff, MAX_BUFFLEN);

            if(tmpread < 1){

                var[i].status = -2;

                var[i].feedback = valread;

                strcpy(res, "error read");

                return -2;
            }

            for(int i = 0; i < tmpread; i ++){

                int idx = valread + i;

                res_buff[idx] = recv_buff[i];
            }

            memset(recv_buff, 0, MAX_BUFFLEN);

            valread += tmpread;

        }

        char* ptk;

        int idx = 0;



        ptk = strtok(res_buff, delim);

        while(ptk != NULL){


            if(idx == 0){


                strcpy(status_str, ptk);


            } else if (idx == 1){

                strcpy(feedback_str,ptk);

            }

            idx += 1;

            ptk = strtok(NULL, delim);
        }



        sscanf(status_str, "%d", &status_flag);

        sscanf(feedback_str, "%d", &feedback);

        var[i].status = status_flag;

        var[i].feedback = feedback;

    }


    strcpy(res, "successfully tried vector ar");


    return 0;





}


int TryInitECCmdGateway(char* res){



    struct timespec ts;


    long msec = 1000;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    nanosleep(&ts, &ts);

    int status, valread, client_fd;
    struct sockaddr_in serv_addr;

    char addr[MAX_BUFFLEN] = {0};
    int port;
    long timeout;

    strcpy(addr, ADDR);

    port = PORT;

    timeout = TIMEOUT;

    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        
        printf("socket creation error\n");

        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
 

    if (inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0) {

        printf("invalid address, address not supported\n");
        return -2;
    }


    SOCK_FD = client_fd;

    struct timeval u_timeout;      
    u_timeout.tv_sec = timeout / 1000;
    u_timeout.tv_usec = 0;
    

    if (setsockopt (SOCK_FD, SOL_SOCKET, SO_RCVTIMEO, &u_timeout, sizeof(u_timeout)) < 0){

        printf("set recv timeout\n");

        return -3;
    }
    

    if (setsockopt (SOCK_FD, SOL_SOCKET, SO_SNDTIMEO, &u_timeout, sizeof(u_timeout)) < 0) {

        printf("set send timeout\n");
    
        return -4;
    }
    

    int rc = 0;
    
    int sockfd_flags_before;

    sockfd_flags_before = fcntl(SOCK_FD,F_GETFL,0);
    
    if(sockfd_flags_before < 0) {
    
        printf("failed to getfl\n");

        return -5;
    
    }
    
    

    if(fcntl(SOCK_FD, F_SETFL, sockfd_flags_before | O_NONBLOCK)<0){


        printf("failed to setfl\n");

        return -6;
    
    } 



 

    if (connect(SOCK_FD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {

            rc = -11;

        } else {
            
            struct timespec now;

            clock_gettime(CLOCK_MONOTONIC_RAW, &now);

            struct timespec deadline;


            do{

                clock_gettime(CLOCK_MONOTONIC_RAW, &deadline);

                int ms_until_deadline = (int)((deadline.tv_sec - now.tv_sec) * 1000 + (deadline.tv_nsec - now.tv_nsec) / 1000000);

                if(ms_until_deadline > timeout) { 
                    rc = -10; 
                    break; 
                }

                struct timeval tv;
                
                tv.tv_sec = 0;

                tv.tv_usec = 10000; // 10ms

                fd_set sel_fd; 

                FD_ZERO(&sel_fd); 
                FD_SET(SOCK_FD, &sel_fd); 

                rc = select(SOCK_FD + 1, NULL, &sel_fd, NULL, &tv); 
                

                if(rc > 0) {
                    int error = 0; 
                    socklen_t len = sizeof(error);
                    int retval = getsockopt(SOCK_FD, SOL_SOCKET, SO_ERROR, &error, &len);
                    if(retval == 0) {
                        errno = error;
                    }
                    if(error != 0) {
                        rc = -11;
                    }
                }

            } while(rc == -11 && errno == EINTR);


            if(rc == -10) {
                
                errno = ETIMEDOUT;
                
            }

        }

    } 

    
    

    if(fcntl(SOCK_FD,F_SETFL,sockfd_flags_before ) < 0) {

        return -20;
    }

    VCONN = 1;

    strcpy(res, "successfully re-connected before timed out");

    return rc;





}

int ECCmdGatewayStr(char* res, const char* cmd) {
    if(VCONN != 1){
        return -100;
    }

    char send_buff[MAX_BUFFLEN] = {0};
    char recv_buff[MAX_BUFFLEN] = {0};
    int valsent;
    int valread = 0;

    strncpy(send_buff, cmd, MAX_BUFFLEN - 1);

    valsent = write(SOCK_FD, send_buff, MAX_BUFFLEN);

    if(valsent < 1){
        strcpy(res, "error send");
        return -1;
    }

    while(valread < MAX_BUFFLEN){
        int tmpread = read(SOCK_FD, recv_buff + valread, MAX_BUFFLEN - valread);
        if(tmpread < 1){
            strcpy(res, "error read");
            return -2;
        }
        valread += tmpread;
    }

    char* response = malloc(MAX_RESULT_STRLEN);
    if (!response) {
        strcpy(res, "Memory allocation failed");
        return -3;
    }
    
    strncpy(response, recv_buff, MAX_RESULT_STRLEN - 1);
    response[MAX_RESULT_STRLEN - 1] = '\0';

    strncpy(res, recv_buff, MAX_RESULT_STRLEN - 1);
    res[MAX_RESULT_STRLEN - 1] = '\0';

    free(response);
    return 0;
}