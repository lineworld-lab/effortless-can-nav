#include "agvethcat/request.h"



int SOCK_FD;
char ADDR[MAX_BUFFLEN] = {0};
int PORT;
long TIMEOUT;
int VCONN = 0;



int InitECCmdGateway(char* res, char* addr, int port){

    fClientLog<std::string>("initiaing cmd gateway");

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


    fClientLog<std::string>("successfully initiated cmd gateway");

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



int ECCmdGatewayAR(char* res, std::vector<struct AxisReq>* var){

    std::string check_buff;

    std::string send_buff;

    std::string status;

    int status_flag;

    int valread;

    std::string rc_str;

    for(int i = 0 ; i < (*var).size(); i ++){


        send_buff = "";

        check_buff = "";

        char req_buff[MAX_BUFFLEN] = {0};

        char res_buff[MAX_BUFFLEN] = {0};

        std::string axis_str = std::to_string((*var)[i].axis);

        send_buff += axis_str + " ";

        send_buff += (*var)[i].action + " ";

        send_buff += (*var)[i].params;

        strcpy(req_buff, send_buff.c_str());

        send(SOCK_FD, req_buff, strlen(req_buff), 0);
        

        valread = read(SOCK_FD, res_buff, MAX_BUFFLEN);

        if((*var)[i].action == "DIEBRO"){

            strcpy(res, "DIEBRO sent");

            return 1;


        }

        status = res_buff;

        status_flag = stoi(status);

        if(status_flag < 0){

            rc_str = "send flag is: " + status + ", at: " + std::to_string(i) + "\n";

            rc_str += ">> send command was: " + send_buff;

            strcpy(res, rc_str.c_str());

            return -1;
        }

        (*var)[i].status = status;

    }


    strcpy(res, "successfully asserted on vector ar");


    return 0;
}



int ECCmdGatewayARFeedback(char* res, std::vector<struct AxisReq>* var){

    std::string check_buff;

    std::string send_buff;

    std::string status;

    int status_flag;

    int valread;

    std::string rc_str;

    for(int i = 0 ; i < (*var).size(); i ++){


        send_buff = "";

        check_buff = "";

        char req_buff[MAX_BUFFLEN] = {0};

        char res_buff[MAX_BUFFLEN] = {0};

        std::string axis_str = std::to_string((*var)[i].axis);

        send_buff += axis_str + " ";

        send_buff += (*var)[i].action + " ";

        send_buff += (*var)[i].params;

        strcpy(req_buff, send_buff.c_str());

        send(SOCK_FD, req_buff, strlen(req_buff), 0);
        

        valread = read(SOCK_FD, res_buff, MAX_BUFFLEN);

        if((*var)[i].action == "DIEBRO"){

            strcpy(res, "DIEBRO sent");

            return 1;


        }

        status = res_buff;

        status_flag = stoi(status);

        (*var)[i].status = status;

    }


    strcpy(res, "successfully asserted on vector ar");


    return 0;
}



int ECCmdGatewayLoadAR(char* res, std::vector<struct AxisReq>* var, int iteration, int distance, int interval){


    std::cout << "load testing" << std::endl;


    int negative = 0;

    int accum = 0;

    for(int i = 0 ; i < iteration; i++){

        std::vector<struct AxisReq> new_var;

        int status = 0;

        for(int j = 0; j < (*var).size(); j ++){

            struct AxisReq ar;

            ar.axis = (*var)[j].axis;

            ar.action = (*var)[j].action;


            std::string pos = (*var)[j].params;

            int pos_int = stoi(pos);

            accum = pos_int + (i * distance);

            pos = std::to_string(accum);

            ar.params += pos;

            ar.params += " ";

            new_var.push_back(ar);

        }

        status = ECCmdGatewayAR(res, &new_var);


        if(status < 0){

            fClientLog<std::string>("failed load testing phase1 at: " + std::to_string(i));
        
            return -1;
        }

         std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }


    for(int i = iteration ; i > 0; i--){

        std::vector<struct AxisReq> new_var;

        int status = 0;

        for(int j = 0; j < (*var).size(); j ++){

            struct AxisReq ar;

            ar.axis = (*var)[j].axis;

            ar.action = (*var)[j].action;

            std::string pos = (*var)[j].params;

            int pos_int = stoi(pos);

            accum = pos_int + (i * distance);

            pos = std::to_string(accum);

            ar.params += pos;

            ar.params += " ";

            new_var.push_back(ar);

        }

        status = ECCmdGatewayAR(res, &new_var);


        if(status < 0){

            fClientLog<std::string>("failed load testing phase2 at: " + std::to_string(i));
        
            return -2;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

    }

    fClientLog<std::string>("load test successful");

    return 0;

}

EC_CODE ECCmdGatewayARFeedback2(char* res, std::vector<struct AxisReq2>* var){


    int cmd_stat = 0;

    int init_stat = 0;

    int ret_stat = 0;



    cmd_stat = TryECCmdGatewayARFeedback(res, var);


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

    EC_CODE ret_code = (EC_CODE)ret_stat;

    return ret_code;
}

int TryECCmdGatewayARFeedback(char* res, std::vector<struct AxisReq2>* var){

    if(VCONN != 1){
        return -100;
    }

    std::string check_buff;

    std::string send_buff;

    std::string status_str;

    std::string feedback_str;

    int feedback;

    int status_flag;

    int valsent;

    int valread;

    std::string rc_str;

    char* delim = " ";

    for(int i = 0 ; i < (*var).size(); i ++){


        send_buff = "";

        check_buff = "";

        char req_buff[MAX_BUFFLEN] = {0};

        char res_buff[MAX_BUFFLEN] = {0};

        std::string axis_str = std::to_string((*var)[i].axis);

        send_buff += axis_str + " ";

        send_buff += (*var)[i].action + " ";

        send_buff += (*var)[i].params;

        strcpy(req_buff, send_buff.c_str());

        // valsent = send(SOCK_FD, req_buff, strlen(req_buff), 0);


        valsent = write(SOCK_FD, req_buff, MAX_BUFFLEN);



        if(valsent < 1){

            (*var)[i].status = -1;

            (*var)[i].feedback = valsent;

            strcpy(res, "error send");

            return -1;
        }


        valread = read(SOCK_FD, res_buff, MAX_BUFFLEN);

        if((*var)[i].action == "DIEBRO"){

            strcpy(res, "DIEBRO sent");

            return 1;


        }

        if(valread < 1){

            (*var)[i].status = -2;

            (*var)[i].feedback = valread;

            strcpy(res, "error read");

            return -2;
        }

        char* ptk;

        int idx = 0;



        ptk = strtok(res_buff, delim);

        while(ptk != NULL){


            if(idx == 0){


                status_str = ptk;


            } else if (idx == 1){

                feedback_str = ptk;

            }

            idx += 1;

            ptk = strtok(NULL, delim);
        }



        status_flag = stoi(status_str);

        feedback = stoi(feedback_str);

        (*var)[i].status = status_flag;

        (*var)[i].feedback = feedback;

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




int ECCmdGatewayLoadAR2(char* res, std::vector<struct AxisReq2>* var, int iteration, int distance, int interval){


    std::cout << "load testing" << std::endl;


    int negative = 0;

    int accum = 0;

    int i = 0;


    while(i < iteration){

        std::vector<struct AxisReq2> new_var;

        EC_CODE status;

        for(int j = 0; j < (*var).size(); j ++){

            struct AxisReq2 ar;

            ar.axis = (*var)[j].axis;

            ar.action = (*var)[j].action;


            std::string pos = (*var)[j].params;

            int pos_int = stoi(pos);

            accum = pos_int + (i * distance);

            pos = std::to_string(accum);

            ar.params += pos;

            ar.params += " ";

            new_var.push_back(ar);

        }

        status = ECCmdGatewayARFeedback2(res, &new_var);


        if(status != EC_CODE::OKAY){

            fClientLog<std::string>("failed load testing phase1 at: " + std::to_string(i));

            fClientLog<std::string>(EC_CODEToString(status));

            continue;
        }

        i += 1;


        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }


    while(i > 0){

        std::vector<struct AxisReq2> new_var;

        EC_CODE status;

        for(int j = 0; j < (*var).size(); j ++){

            struct AxisReq2 ar;

            ar.axis = (*var)[j].axis;

            ar.action = (*var)[j].action;

            std::string pos = (*var)[j].params;

            int pos_int = stoi(pos);

            accum = pos_int + (i * distance);

            pos = std::to_string(accum);

            ar.params += pos;

            ar.params += " ";

            new_var.push_back(ar);

        }

        status = ECCmdGatewayARFeedback2(res, &new_var);


        if(status != EC_CODE::OKAY){

            fClientLog<std::string>("failed load testing phase2 at: " + std::to_string(i));

            fClientLog<std::string>(EC_CODEToString(status));

            continue;
        }


        i -= 1;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

    }

    fClientLog<std::string>("load test successful");

    return 0;

}

std::string EC_CODEToString(EC_CODE ecc){


    std::string ret_str;


    switch(ecc){

        case EC_CODE::OKAY:

            ret_str = "EC_SUCCESS: okay";

            break;

        case EC_CODE::EREAD:

            ret_str = "EC_FAIL: read";


            break;

        case EC_CODE::EWRITE:

            ret_str = "EC_FAIL: write";

            break;

        case EC_CODE::ECONNREAD:

            ret_str = "EC_FAIL: conn+read";

            break;

        case EC_CODE::ECONNWRITE:

            ret_str = "EC_FAIL: conn+write";

            break;


        default:

            ret_str = "EC_FAIL: " + std::to_string((int)ecc);

            break;


    }


    return ret_str;

}