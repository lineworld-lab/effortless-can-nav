#include "agvethcat/request.h"

#include "agvethcat/cmd.h"




int ReadinessProbe(char* address, int port, int axis_c, int* axis){



    char res[MAX_RESULT_STRLEN] = {0};

    int ret_code;

    std::string result;

    ret_code = InitECCmdGateway(res, address, port);

    if(ret_code < 0){

        result = res;

        WriteProbeFailure(result);

        return -1;
    }



    std::vector<struct AxisReq> var;

    for(int i = 0; i < axis_c; i ++){

        struct AxisReq ar;

        ar.axis = axis[i];
        ar.action = "hc";

        var.push_back(ar);
    }


    ret_code = ECCmdGatewayAR(res, &var);


    int hit = 0;

    for(int i = 0; i < axis_c; i ++){

        if(var[i].status == "1"){
            hit += 1;
        }

    }

    if(hit == axis_c){

        WriteProbeSuccess();

    } else {

        WriteProbeFailure("0");

    }

    return 0;

}


void WriteProbeFailure(std::string res){

    printf("%s", res.c_str());

    fflush(NULL);    

    return;

}

void WriteProbeSuccess(){

    printf("%s","pass");

    fflush(NULL);

    return;

}

int RunInteractive(){

    int keep = 0 ;

    
    while(keep == 0){

        int ret_code;

        std::string line;

        std::string x;

        std::vector<std::string> runtime_args;
        
        std::cout << "type 'help' for .... help, obviously" << std::endl;

        std::cout << "CMD $: ";

        std::getline(std::cin, line);

        std::istringstream iss(line);

        while ( iss >> x) {    
            runtime_args.push_back(x);
        }


        if(runtime_args.size() == 0){
            
            std::cout << "no argument specified " << std::endl;
            continue;
        }
        
        if(runtime_args[0] == "connect"){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = ConnectTo(res, runtime_args);

            if(ret_code < 0){
 
                std::cout << "connect failed" << std::endl;
            
            } else {
                std::cout << "connect success" << std::endl;
            }

            std::cout << res << std::endl;

        }else if(runtime_args[0] == "send"){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = SendTo(res, runtime_args);

            if(ret_code < 0){
 
                std::cout << "send failed" << std::endl;
            
            } else {
                std::cout << "send success" << std::endl;
            }

            std::cout << res << std::endl;

        }else if(runtime_args[0] == "send2"){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = SendTo2(res, runtime_args);

            if(ret_code < 0){
 
                std::cout << "send2 failed" << std::endl;
            
            } else {
                std::cout << "send2 success" << std::endl;
            }

            std::cout << res << std::endl;

        }else if(runtime_args[0] == "disconnect"){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = DisconnectFrom(res, runtime_args);

            if(ret_code < 0){
 
                std::cout << "disconnect failed" << std::endl;
            
            } else {
                std::cout << "disconnnect success" << std::endl;
            }

            std::cout << res << std::endl;

            keep = 1;

        }else {

            std::cout << "invalid argument: " << runtime_args[0] << std::endl;
        }


    }


    return 0;
}



int ConnectTo(char* res, std::vector<std::string> runtime_args){

    int ret_code;


    if( runtime_args.size() < 3){
        std::cout << "too few arguments for: " << runtime_args[0] << std:: endl;
        std::cout << "needs: addr port" << std::endl;

        return -1;
    }

    char addr[24] = {0};

    strcpy(addr, runtime_args[1].c_str());

    int port = stoi(runtime_args[2]);

    ret_code = InitECCmdGatewayWithTimeout(res, addr, port, 5000);

    //ret_code = InitECCmdGateway(res, addr, port);


    return ret_code;

}

int SendTo(char* res, std::vector<std::string> runtime_args){

    int ret_code;

    std::vector<struct AxisReq> var;

    std::cout << "'abort' to abort;" << std::endl;

    std::cout << "'send' to send;" << std::endl;

    std::cout << "$axis $action [$params]" << std::endl;

    for(;;){

        std::string new_line;

        char new_buff[1024] = {0};

        std::cout << std::endl;
        std::cout << "current vectors:" << std::endl;

        for(int i = 0; i < var.size(); i++){

            std::cout << "==========" << std::endl;

            std::cout << " - axis   : "  << std::to_string(var[i].axis) << std::endl;
            std::cout << " - action : "  << var[i].action << std::endl;
            std::cout << " - params : "  << var[i].params << std::endl;
            std::cout << " - status : "  << var[i].status << std::endl;

            std::cout << "==========" << std::endl;
        }

        std::cout << std::endl;


        struct AxisReq ar;

        fgets(new_buff, 1024, stdin);

        int in_buff_len = strlen(new_buff);

        for(int i = 0; i < in_buff_len; i++){

            if(new_buff[i] == '\n'){
                new_buff[i] = '\0';
            }


        }

        new_line = new_buff;

        if(new_line == "abort" || new_line == "abort\n"){

            std::cout << "ABORT." << std::endl;

            ret_code = 1;

            break;

        } else if (new_line == "send" || new_line == "send\n"){

            std::cout << "SEND." << std::endl;

            ret_code = ECCmdGatewayARFeedback(res, &var);

            break;

        }  else if (new_line == "load" || new_line == "load\n"){

            std::cout << "LOAD." << std::endl;

            int iter = 0;

            int dist = 0;

            int intv = 0;

            printf("iteration? : ");

            scanf("%d", &iter);

            printf("distance?: ");

            scanf("%d", &dist);

            printf("interval?: ");

            scanf("%d", &intv);

            ret_code = ECCmdGatewayLoadAR(res, &var, iter, dist, intv);

            break;

        }



        int idx = 0;

        char* pch = strtok(new_buff, " ");

        std::string p;

        while(pch != NULL){

            if(idx == 0){

                std::string axis_str = pch;

                ar.axis = stoi(axis_str);

            } else if (idx == 1){

                ar.action = pch;

            } else {

                std::string p = pch;

                ar.params += p + " ";
            }

            pch = strtok(NULL, " ");

            idx += 1;

        }

        var.push_back(ar);

        std::cout << "added new request" << std::endl;

    }


    return ret_code;
}



int SendTo2(char* res, std::vector<std::string> runtime_args){

    int ret_code;

    std::vector<struct AxisReq2> var;

    std::cout << "'abort' to abort;" << std::endl;

    std::cout << "'send2' to send;" << std::endl;

    std::cout << "$axis $action [$params]" << std::endl;

    for(;;){

        std::string new_line;

        char new_buff[1024] = {0};

        std::cout << std::endl;
        std::cout << "current vectors:" << std::endl;

        for(int i = 0; i < var.size(); i++){

            std::cout << "==========" << std::endl;

            std::cout << " - axis     : "  << std::to_string(var[i].axis) << std::endl;
            std::cout << " - action   : "  << var[i].action << std::endl;
            std::cout << " - params   : "  << var[i].params << std::endl;
            std::cout << " - status   : "  << std::to_string(var[i].status) << std::endl;
            std::cout << " - feedback : "  << std::to_string(var[i].feedback) << std::endl;

            std::cout << "==========" << std::endl;
        }

        std::cout << std::endl;


        struct AxisReq2 ar;

        fgets(new_buff, 1024, stdin);

        int in_buff_len = strlen(new_buff);

        for(int i = 0; i < in_buff_len; i++){

            if(new_buff[i] == '\n'){
                new_buff[i] = '\0';
            }


        }

        new_line = new_buff;

        if(new_line == "abort" || new_line == "abort\n"){

            std::cout << "ABORT." << std::endl;

            ret_code = 1;

            break;

        } else if (new_line == "send2" || new_line == "send2\n"){

            std::cout << "SEND2." << std::endl;

            ret_code = ECCmdGatewayARFeedback2(res, &var);

            break;

        }  else if (new_line == "load2" || new_line == "load2\n"){

            std::cout << "LOAD2." << std::endl;

            int iter = 0;

            int dist = 0;

            int intv = 0;

            printf("iteration? : ");

            scanf("%d", &iter);

            printf("distance?: ");

            scanf("%d", &dist);

            printf("interval?: ");

            scanf("%d", &intv);

            ret_code = ECCmdGatewayLoadAR2(res, &var, iter, dist, intv);

            break;

        }



        int idx = 0;

        char* pch = strtok(new_buff, " ");

        std::string p;

        while(pch != NULL){

            if(idx == 0){

                std::string axis_str = pch;

                ar.axis = stoi(axis_str);

            } else if (idx == 1){

                ar.action = pch;

            } else {

                std::string p = pch;

                ar.params += p + " ";
            }

            pch = strtok(NULL, " ");

            idx += 1;

        }

        ar.status = 0;
        ar.feedback = 0;
        

        var.push_back(ar);

        std::cout << "added new request" << std::endl;

    }


    return ret_code;
}

int DisconnectFrom(char* res, std::vector<std::string> runtime_args){


    int ret_code;
    
    struct AxisReq ar;

    ar.axis = 0;

    ar.action = "DIEBRO";

    std::vector<struct AxisReq> var;

    var.push_back(ar);

    ret_code = ECCmdGatewayAR(res, &var);

    return ret_code;
}



int main(int argc, char* argv[]){


    std::string flag = argv[1];

    int status = 0;

    if(flag == "i"){

        status = RunInteractive();

    } else if (flag == "p"){

        std::string address_str = argv[2];

        std::string port_str = argv[3];

        char addr_char[128] = {0};

        int axis_count = argc - 4;

        int* axis = (int*)malloc(axis_count * sizeof(int));

        for(int i = 0; i < axis_count; i ++){

            int check = i + 4;

            std::string axis_str = argv[check];

            int axis_num = stoi(axis_str);

            axis[i] = axis_num;

        }

        strcpy(addr_char, address_str.c_str());

        int port_int = stoi(port_str);
/*
        printf("%s\n%d\n", addr_char, port_int);

        for(int i = 0; i < axis_count; i++){

            printf("%d ", axis[i]);

        }
        printf("\n");
*/
        status = ReadinessProbe(addr_char, port_int, axis_count, axis);    

        free(axis);

    } else {

        std::cout<< "wrong argument: " << flag << std::endl;

        status = -99;
    }


    return status;
}