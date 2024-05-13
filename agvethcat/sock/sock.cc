
#include "ecat_lifecycle.h"
#include "agvethcat/sock.h"


int ListenAndServe(int port){

    fLog<std::string>("creating socket for listening...");

    int server_fd, client_socket;
    ssize_t valread;
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

    fLog<std::string>("listening on: " + std::to_string(port));


    for(;;){

        if ((client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        fLog<std::string>("connection accepted");

        int status = 0;

        for(;;){

            char req_buffer[MAX_REQ_STRLEN] = {0};

            char ret_buffer[MAX_RESULT_STRLEN] = {0};


            valread = read(client_socket, req_buffer, MAX_REQ_STRLEN); 

            for(int i = 0; i < MAX_REQ_STRLEN; i ++){

                if(req_buffer[i] == '\n'){
                    req_buffer[i] = ' ';
                }


            }

            std::string log_buff = req_buffer;

            fLog<std::string>("recv: " + log_buff);

            if(log_buff == "" || log_buff == " " || log_buff == "\n"){

                fLog<std::string>("empty recv");

                status = 10;

                break;

            }

            status = ProcessBuffer(ret_buffer, req_buffer);

            log_buff = ret_buffer;

            fLog<std::string>("write: " + log_buff);

            send(client_socket, ret_buffer, strlen(ret_buffer), 0);

            if(status < 0){

                fLog<std::string>("process error: " + std::to_string(status));

                running_stat = status;

                break;

            }

            if(status == 99){

                fLog<std::string>("legal exit");

                running_stat = status;

                break;
            }

        }

        if(status < 0){

            fLog<std::string>("process error: " + std::to_string(status));

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

    std::string axis_str = "";

    int axis = 0;

    std::string action = "";

    std::vector<std::string> params;

    pch = strtok(req, " ");

    while (pch != NULL){
        
        if(idx == 0){

            axis_str = pch;

        } else if (idx == 1){

            action = pch;

        } else {

            std::string p;

            p = pch;

            params.push_back(p);

        }

        pch = strtok(NULL, " ");

        idx += 1;
    }


    axis = stoi(axis_str);

    int status = 0;

    if(action == "hc"){ // home check

        status = GetHomingStatusByAxis(res, axis);

    } else if (action == "mo"){ // move override

        int pos = stoi(params[0]);

        status = PostPositionWithFeedbackByAxis(res, axis, pos);

    } else if (action == "tmo"){ // try move override

        int pos = stoi(params[0]);
  
//        status = PostPositionWithFeedbackByAxis(res, axis, pos);

        status = PostPositionWithStatusFeedbackByAxis(res,  axis, pos);
    } else if (action == "DIEBRO"){

        fLog<std::string>("DIEBRO received");

        strcpy(res, "DIEBRO");

        status = 99;


    }  else {

        status = -1;

        std::string invalid;

        invalid = "invalid action: " + action; 

        strcpy(res, invalid.c_str());

    }


    return status;
}



void ExitServer(int client_sock, int server_fd){


    fLog<std::string>("closing server...");

    close(client_sock);

    close(server_fd);


    fLog<std::string>("successfully closed server");


}
