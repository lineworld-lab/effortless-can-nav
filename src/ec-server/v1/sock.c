#include "ec-server/v1/ec_system.h"
#include "ec-server/v1/sock.h"


int ListenAndServe(int port){

    printf("creating socket for listening...");

    int server_fd, client_socket;
    ssize_t valread;
    ssize_t valwrite;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    int running_stat = 0;
 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }
 
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        return EXIT_FAILURE;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return EXIT_FAILURE;
    }

    if (listen(server_fd, MAX_CONN) < 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    printf("listening on: %d\n",port);


    for(;;){

        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        printf("connection accepted\n");

        int status = 0;

        int failed_read = 0;

        for(;;){

            char req_buffer[MAX_REQ_STRLEN] = {0};
            char recv_buffer[MAX_REQ_STRLEN] = {0};
            char ret_buffer[MAX_RESULT_STRLEN] = {0};

            valread = 0;
            valwrite = 0;

            while(valread < MAX_REQ_STRLEN){

                int tmpread = read(client_socket, recv_buffer, MAX_REQ_STRLEN); 

                if(tmpread < 1){

                    printf("failed to read: %d \n", tmpread);

                    failed_read = 1;

                    break;


                }

                for(int i = 0 ; i < tmpread; i++){

                    int idx = valread + i;

                    req_buffer[idx] = recv_buffer[i]; 

                }

                memset(recv_buffer, 0, MAX_REQ_STRLEN);

                valread += tmpread;


            }

            if(failed_read == 1){

                status = 10;
                break;

            }


            printf("recv: %s\n",req_buffer);

            status = ProcessBuffer(ret_buffer, req_buffer);

            printf("write: %s\n",ret_buffer);

            valwrite = write(client_socket, ret_buffer, MAX_RESULT_STRLEN);

            if(status < 0){

                printf("process error: %d\n",status);

                running_stat = status;

                break;

            }

            if(status == 99){

                printf("legal exit\n");

                running_stat = status;

                break;
            }

        }

        if(status < 0){

            printf("process error: %d\n",status);

            running_stat = status;

            break;

        }


        if(status == 99){

            break;
        }


    }



    ExitServer(client_socket, server_fd);

    return running_stat;

}



int ProcessBuffer(char* res, char* req){

    char* pch;

    int idx = 0;

    char axis_str[MAX_TOKEN_STRLEN] = {0};

    int axis = 0;

    char action[MAX_TOKEN_STRLEN] = {0};

    char** params;

    int params_count = 0;

    pch = strtok(req, " ");

    while (pch != NULL){
        
        if(idx == 0){

            strcpy(axis_str,pch);

        } else if (idx == 1){

            strcpy(action, pch);

        } else {

            if(params_count == 0){

                params = (char**)malloc(sizeof(char*) * 1);

            } else {

                params = (char**)realloc(params, sizeof(char*) * (params_count + 1));

            }

            int pch_len = strlen(pch) + 1;

            params[params_count] = (char*)malloc(sizeof(char) * pch_len);

            memset(params[params_count], 0, sizeof(char) * pch_len);

            strcpy(params[params_count], pch);

            params_count += 1;

        }

        pch = strtok(NULL, " ");

        idx += 1;
    }


    sscanf(axis_str, "%d", &axis);

    int status = 0;

    if(strcmp(action,"hc") == 0){ // home check

        status = GetHomingStatusByAxis(res, axis);
    }else if (strcmp(action,"tmo") == 0){ // try move override

        int pos = 0;

        sscanf(params[0], "%d", &pos);
  
//        status = PostPositionWithFeedbackByAxis(res, axis, pos);

        status = PostPositionWithStatusFeedbackByAxis(res,  axis, pos);
    
    
    } else if (strcmp(action,"DIEBRO") == 0){

        printf("DIEBRO received\n");

        strcpy(res, "DIEBRO");

        status = 99;


    }  else {

        status = -1;

        char invalid[MAX_MESSAGE] = {0};

        strcat(invalid, "invalid action: ");
        strcat(invalid, action);

        strcpy(res, invalid);

    }

    if(params_count != 0){

        for(int i = 0 ; i < params_count; i ++){


            free(params[i]);

        }

        free(params);

    }

    return status;
}



void ExitServer(int client_sock, int server_fd){


    printf("closing server...");

    close(client_sock);

    close(server_fd);

    printf("successfully closed server");


}





int GetHomingStatusByAxis(char* res, int axis){


    int op_mode = (int)ECAT_LIFECYCLE_NODE.op_mode_[axis];

    int feedback = (int)ECAT_LIFECYCLE_NODE.feedback_position_[axis];

    char status_str[10] = {0};
    /*

    if(op_mode != 1 && feedback != 0){

        status_str = std::to_string(0);

    } else if (op_mode != 1 && feedback == 0){

        status_str = std::to_string(-1);

    } else if (op_mode == 1 && feedback != 0){

        status_str = std::to_string(-2);

    } else if (op_mode == 1 && feedback == 0){

        status_str = std::to_string(1);

    } 

    */


   if(op_mode == 1 && feedback == 0){
  
        strcpy(status_str, "1");

   } else {

        strcpy(status_str, "0");
   }


    strcpy(res, status_str);


    return 0;
}

int PostHomeShiftByAxis(char* res, int axis, int shift){




    return 0;
}


int PostPositionByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    ECAT_LIFECYCLE_NODE.posted_position_[axis] = pos_32;

    strcpy(res, "0");

    return 0;
}


int PostPositionWithFeedbackByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    ECAT_LIFECYCLE_NODE.posted_position_[axis] = pos_32;

    int feedback = (int)ECAT_LIFECYCLE_NODE.feedback_position_[axis];

    char feedback_str[MAX_POSITION_STRLEN] = {0};

    sprintf(feedback_str, "%d", feedback);

    strcpy(res, feedback_str);

    return 0;
}




int PostPositionWithStatusFeedbackByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    ECAT_LIFECYCLE_NODE.posted_position_[axis] = pos_32;

    int status_int = (int)ECAT_LIFECYCLE_NODE.op_mode_[axis];

    int feedback_int = (int)ECAT_LIFECYCLE_NODE.feedback_position_[axis];

    char status_str[MAX_STATUS_STRLEN] = {0};
    char feedback_str[MAX_POSITION_STRLEN] = {0};

    sprintf(status_str, "%d", status_int);

    sprintf(feedback_str, "%d", feedback_int);

    strcat(res, status_str);

    strcat(res, " ");

    strcat(res, feedback_str);

    return 0;
}