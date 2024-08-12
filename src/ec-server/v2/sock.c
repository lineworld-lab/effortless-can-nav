#include "ec-server/v2/ec_system.h"
#include "ec-server/v2/sock.h"


void* ECAT2_listen_and_serve(void* varg){

    ListenAndServe(SOCK_PORT);

}


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
                    if(recv_buffer[i] == '\n'){
                        
                        req_buffer[idx + 1] = '\0';
                        status = ProcessBuffer(ret_buffer, req_buffer);
                        printf("Received command: %s", req_buffer);
                        printf("Processed result: %s\n", ret_buffer);
                        valwrite = write(client_socket, ret_buffer, strlen(ret_buffer));
                        
                        memset(req_buffer, 0, MAX_REQ_STRLEN);
                        valread = 0;
                        break;
                    }
                }

                if(valread == 0) continue;  

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

    req[strcspn(req, "\n")] = 0;

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


    } else if (strcmp(action, "discovery") == 0){

      status = GetAvailableCommands(res);


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


    int op_mode = (int)motor_txpdos[axis]->mode_of_operation_display;

    int feedback = (int)motor_txpdos[axis]->position_actual_value;

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



int PostPositionWithStatusFeedbackByAxis(char* res, int axis, int pos){

    int32_t pos_32 = (int32_t)pos;

    motor_rxpdos[axis]->target_position = pos_32;

    int status_int = (int)motor_txpdos[axis]->mode_of_operation_display;

    int feedback_int = (int)motor_txpdos[axis]->position_actual_value;

    char status_str[MAX_STATUS_STRLEN] = {0};
    char feedback_str[MAX_POSITION_STRLEN] = {0};

    sprintf(status_str, "%d", status_int);

    sprintf(feedback_str, "%d", feedback_int);

    strcat(res, status_str);

    strcat(res, " ");

    strcat(res, feedback_str);

    return 0;
}

int GetAvailableCommands(char* res) {
    char* current = res;
    int remaining = MAX_RESULT_STRLEN;
    int written = snprintf(current, remaining, "List of available command and arguments:\n");

    if (written < 0 || written >= remaining) {
        strcpy(res, "Buffer overflow in GetAvailableCommands");
        return -1;
    }

    current += written;
    remaining -= written;

    for (int i = 0; i < available_cmd_count && remaining > 0; i++) {

        written = snprintf(current, remaining, "%s : %s %s %s : ex) %s\n", 

                           available_cmd[i].cmd, 

                           available_cmd[i].args1,
                           available_cmd[i].cmd,
                           available_cmd[i].args2,

                           available_cmd[i].comment
                           );
        if (written < 0 || written >= remaining) {
            strcat(res, "\nBuffer overflow, command list truncated");
            return -2;
        }
        
        current += written;
        remaining -= written;
    }

    if (remaining <= 0) {
        strcat(res, "\nBuffer overflow, command list may be incomplete");
        return -3;
    }

    return 0;
}