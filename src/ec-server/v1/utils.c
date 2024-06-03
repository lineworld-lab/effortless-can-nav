#include "ec-server/v1/utils.h"



int InitRuntimeFromConfigJSON(){


    char *source = NULL;

    FILE *fp = fopen("config.json", "r");

    if (fp != NULL) {

        if (fseek(fp, 0L, SEEK_END) == 0) {
    
            long bufsize = ftell(fp);

            if (bufsize == -1) {

                printf("failed to read config.json\n");

                return -1;
            }

        
            source = malloc(sizeof(char) * (bufsize + 1));

            if (fseek(fp, 0L, SEEK_SET) != 0) { 

                printf("failed to read config.json, unexpected EOF\n");

                return -2;

            }

        
            size_t new_len = fread(source, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                
                printf("error reading config.json\n");

                fclose(fp);

                free(source);

                return -3;

            } else {

                source[new_len++] = '\0'; 
            
            }
        }
        fclose(fp);
    }




    cJSON *conf_json = cJSON_Parse(source);

    if (conf_json == NULL){


        printf("error parsing config.json\n");

        free(source);

        return -4;

    }

    cJSON *test = cJSON_GetObjectItemCaseSensitive(conf_json, "test");

    we_are_testing = test->valueint;


    cJSON *drivers_num = cJSON_GetObjectItemCaseSensitive(conf_json, "drivers_num");

    int tmp_drivers_num = drivers_num->valueint;

    if (tmp_drivers_num > MAX_DRIVERS_NUM){

        printf("invalid driver num: too many: %d\n", tmp_drivers_num);


        return -5;

    }


    if (tmp_drivers_num < 1){

        printf("invalid driver num: too few: %d\n", tmp_drivers_num);

        return -6;

    }

    g_drivers_num = (uint32_t)tmp_drivers_num;

    cJSON *drivers = cJSON_GetObjectItemCaseSensitive(conf_json, "drivers");

    for (int i = 0; i < g_drivers_num; i++){

        int homing_offset_value = 0;

        cJSON *driver = cJSON_GetArrayItem(drivers, i);

        cJSON *driver_id = cJSON_GetObjectItemCaseSensitive(driver, "id");

        if(driver_id->valueint != i){

            printf("invalid driver id sequence: at %d: got: %d\n", i, driver_id->valueint);

            return -7;

        }

        cJSON *homing_offset = cJSON_GetObjectItemCaseSensitive(driver, "homing_offset");

        homing_offset_value = homing_offset->valueint;

        g_homing_offset_switch[i] = (int32_t)homing_offset_value;


    }

    if(we_are_testing){

        printf("your config: \n%s\n",cJSON_Print(conf_json));

        for(int i = 0; i < g_drivers_num; i ++){

            printf("homing offset: %d -> %d\n", i, g_homing_offset_switch[i]);

        }

    }



    free(source);

    printf("successfully initiated runtimer from config json\n");


    return 0;
}


void SetRealTimeSettings(){

      // Configure stdout sream buffer. _IONBF means no buffering. Each I/O operation is written as soon as possible. 
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    
    // Associate 'signalHandler' function with interrupt signal (Ctrl+C key)

#if PREEMPT_RT_MODE

    // Prepare memory for real time performance 
    // https://design.ros2.org/articles/realtime_background.html
    
    // Lock this processe's memory. Necessary for real time performance....
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        printf( "Mlockall failed, check if you have sudo authority.");
        return ;
    }
    /* Turn off malloc trimming.*/
    mallopt(M_TRIM_THRESHOLD, -1);

    /* Turn off mmap usage. */
    mallopt(M_MMAP_MAX, 0);
    // -----------------------------------------------------------------------------

#endif
}

struct timespec timespec_add(struct timespec time1, struct timespec time2){
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= g_kNsPerSec)
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - g_kNsPerSec;
    }
    else
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}
