#include "ec-client/v1/cmd.h"



int RunInteractive(){


    int keep = 0 ;

    
    while(keep == 0){

        int ret_code;

        char line[MAX_BUFFLEN] = {0};

        char** runtime_args;

        int arg_len = 0;

        char* delim = " ";

        printf("type 'help' for .... help, obviously\n");

        printf("CMD $: ");

        fgets(line, MAX_BUFFLEN, stdin);

        int line_len = strlen(line);

        if (line_len < 2){
            printf("no argument specified \n");
            continue;
        }

        for (int i = 0; i < line_len; i ++){

            if(line[i] == '\n'){
                line[i] = '\0';
            }

        }

        char* tok = strtok(line, delim);

        while(tok != NULL){

            if(arg_len == 0){

                runtime_args = (char**)malloc(sizeof(char*) * 1);

            } else {

                runtime_args = (char**)realloc(runtime_args, sizeof(char*) * (arg_len + 1));

            }

            int tok_len = strlen(tok) + 1;

            runtime_args[arg_len] = (char*)malloc(sizeof(char) * tok_len);

            memset(runtime_args[arg_len], 0, sizeof(char) * tok_len);

            strcpy(runtime_args[arg_len], tok);

            arg_len += 1;

            tok = strtok(NULL, delim);

        }
        

        if(arg_len == 0){
            
            printf("no argument specified \n");
            continue;
        }
        
        if(strcmp(runtime_args[0],"connect") == 0){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = ConnectTo(res, arg_len, runtime_args);

            if(ret_code < 0){
 
                printf("connect failed\n");
            
            } else {
                printf("connect success\n");
            }

            printf("%s\n", res);

        }else if(strcmp(runtime_args[0],"send") == 0){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = SendTo(res, arg_len, runtime_args);

            if(ret_code < 0){
 
                printf("send failed\n");
            
            } else {
                printf("send success\n");
            }

            printf("%s\n", res);

        } else if(strcmp(runtime_args[0], "disconnect") == 0){

            char res[MAX_RESULT_STRLEN] = {0};

            ret_code = DisconnectFrom(res, arg_len, runtime_args);

            if(ret_code < 0){
 
                printf("disconnect failed\n");
            
            } else {
                printf("disconnnect success\n");
            }

            printf("%s\n", res);

            keep = 1;

        }else {

            printf("invalid argument: %s\n",runtime_args[0]);

            
        }

        for(int i = 0; i < arg_len; i++){

            free(runtime_args[i]);

        }

        free(runtime_args);

    }


    return 0;


}

int ConnectTo(char* res, int arg_len, char** runtime_args){


    int ret_code;


    if( arg_len < 3){
        printf("too few arguments for: %s\n",runtime_args[0]);
        printf("needs: addr port\n");

        return -1;
    }

    char addr[24] = {0};

    strcpy(addr, runtime_args[1]);

    int port = 0;

    sscanf(runtime_args[2], "%d", &port);

    ret_code = InitECCmdGatewayWithTimeout(res, addr, port, 5000);

    //ret_code = InitECCmdGateway(res, addr, port);


    return ret_code;


}

int SendTo(char* res, int arg_len, char** runtime_args){

    int ret_code;

    printf("\'abort\' to abort;\n");
    printf("\'send\' to send;\n");

    AxisReq var[MAX_ARG_LEN];

    int var_count = 0;

    memset(var, 0, sizeof(AxisReq) * MAX_ARG_LEN);

    for (;;){


        char new_buff[1024] = {0};

        printf("current vectors: \n");


        for(int i = 0; i < var_count; i ++){

            printf("============\n");

            printf(" - axis     : %d\n", var[i].axis);
            printf(" - action   : %s\n", var[i].action);
            printf(" - params   : %s\n", var[i].params);
            printf(" - status   : %d\n", var[i].status);
            printf(" - feedback : %d\n", var[i].feedback);

            printf("============\n");

        }

        AxisReq ar;

        memset(&ar, 0, sizeof(AxisReq));

        fgets(new_buff, 1024, stdin);

        printf("fgets: %s\n", new_buff);

        int in_buff_len = strlen(new_buff);

        for(int i = 0; i < in_buff_len; i++){

            if(new_buff[i] == '\n'){
                new_buff[i] = '\0';
            }


        }

        if(strcmp(new_buff, "abort") == 0){
            
            printf("ABORT.\n");
            
            ret_code = 1;

            break;

        } else if(strcmp(new_buff, "send") == 0){
            
            printf("SEND.\n");

            ret_code = ECCmdGatewayAR(res, var_count, var);


            break;


        } 


        int idx = 0;

        char* pch = strtok(new_buff, " ");

        while(pch != NULL){

            if(idx == 0){

                sscanf(pch, "%d", &(ar.axis));

            } else if (idx == 1){

                strcpy(ar.action, pch);

            } else {

                strcat(ar.params, pch);

                strcat(ar.params, " ");
            }

            pch = strtok(NULL, " ");

            idx += 1;

        }

        ar.status = 0;
        ar.feedback = 0;

        var[var_count] = ar;

        var_count += 1;


    }

    return ret_code;
}

int DisconnectFrom(char* res, int arg_len, char** runtime_args){



    int ret_code;
    
    AxisReq ar;

    ar.axis = 0;

    strcpy(ar.action,"DIEBRO");

    AxisReq var[1];

    var[0] = ar;

    ret_code = ECCmdGatewayAR(res, 1, var);

    return ret_code;


}