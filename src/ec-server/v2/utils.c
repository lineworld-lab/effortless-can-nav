#include "ec-server/v2/utils.h"



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


    cJSON *drivers_num = cJSON_GetObjectItemCaseSensitive(conf_json, "drivers_num");

    free(source);

    printf("successfully initiated runtimer from config json\n");


    return 0;
}
