#include "ec-server/wheel/utils.h"



int InitWheelRuntimeFromObject(cJSON *conf_json_root){

    cJSON *conf_json = cJSON_GetObjectItemCaseSensitive(conf_json_root, "wheel");


    cJSON *wheels_num = cJSON_GetObjectItemCaseSensitive(conf_json, "wheels_num");


    int tmp_wheels_num = wheels_num->valueint;

    if(tmp_wheels_num > MAX_WHEELS_NUM){

        printf("invalid wheel num: too many: %d\n", tmp_wheels_num);

        return -1;
    }

    if (tmp_wheels_num < 1){

        printf("invalid wheel num: too few: %d\n", tmp_wheels_num);

        return -2;
    }


    g_num_of_wheel_slaves = tmp_wheels_num;

    int alloc_result = AllocWheelRuntime();
    

    if(alloc_result < 0){

        printf("failed to allocate wheel runtime mem\n");

        return -3;

    }

    cJSON *wheels = cJSON_GetObjectItemCaseSensitive(conf_json, "wheels");

    for(int i = 0; i < g_num_of_wheel_slaves; i++){


        cJSON *wheel = cJSON_GetArrayItem(wheels, i);

        cJSON *wheel_id = cJSON_GetObjectItemCaseSensitive(wheel, "id");

        g_wheels_node_ids[i] = wheel_id->valueint;

        cJSON *wheel_rotation_clockwise = cJSON_GetObjectItemCaseSensitive(wheel, "rotation_clockwise");

        node_rotation_direction[i] = wheel_rotation_clockwise->valueint;



    }

    cJSON *if_name = cJSON_GetObjectItemCaseSensitive(conf_json, "if_name");

    if(strlen(if_name->valuestring) > MAX_CAN_DEV_NAME){

        printf("wheel if_name too long\n");

        return -4;

    }

    strcpy(CAN_DEV_NAME, if_name->valuestring);

    cJSON *master_node_id = cJSON_GetObjectItemCaseSensitive(conf_json, "master_node_id");

    
    if(strlen(master_node_id->valuestring) > MAX_CAN_NODE_NAME){

        printf("wheel master_node_id too long\n");

        return -5;

    }

    strcpy(CAN_NODE_ID_STR, master_node_id->valuestring);


    cJSON *local_socket_name = cJSON_GetObjectItemCaseSensitive(conf_json, "local_socket_name");

        
    if(strlen(local_socket_name->valuestring) > MAX_SOCK_PATH){

        printf("wheel local_socket_name too long\n");

        return -6;

    }

    strcpy(LOCAL_SOCKET_NAME, local_socket_name->valuestring);


    cJSON *acceleration = cJSON_GetObjectItemCaseSensitive(conf_json, "acceleration");

    wheel_acceleration = acceleration->valueint;

    cJSON *deceleration = cJSON_GetObjectItemCaseSensitive(conf_json, "deceleration");

    wheel_deceleration = deceleration->valueint;

    printf("successfully initiated wheel runtime from config json\n");

    return 0;
}


int AllocWheelRuntime(){


    g_wheels_node_ids = (int*)malloc(g_num_of_wheel_slaves * sizeof(int));

    memset(g_wheels_node_ids, 0, g_num_of_wheel_slaves * sizeof(int));

    node_rotation_direction = (int*)malloc(g_num_of_wheel_slaves * sizeof(int));

    memset(node_rotation_direction, 0, g_num_of_wheel_slaves * sizeof(int));

}




void FreeWheelRuntime(){


    do{

        if(g_wheels_node_ids == NULL){
            break;
        }

        free(g_wheels_node_ids);

    }while(0);


    do {

        if(node_rotation_direction == NULL){

            break;
        }

        free(node_rotation_direction);

    } while(0);

}