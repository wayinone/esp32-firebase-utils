#include <stdio.h>
#include <string.h>
#include "firestore_utils.h"
#include "firebase_auth.h"
#include "station_mode.h"
#include "esp_timer.h"


#define device_id "abcde"

void app_main(void)
{

    initWifiSta();

    char access_token[1024];
    firebase_get_access_token_from_refresh_token(CONFIG_FIREBASE_REFRESH_TOKEN, access_token);

    printf("Access token: %s\n", access_token);
    printf("Access token length: %d\n", strlen(access_token));  // usually 758



    char *reading_document_format = 
        "{"
            "\"fields\": {"
                "\"val\": {"
                    "\"integerValue\": \"%s\""
                "},"
                "\"dt\": {"
                    "\"integerValue\": \"%s\""
                "}"
            "}"
        "}";

    int64_t current_time_int = esp_timer_get_time();
    char current_time[20];
    snprintf(current_time, sizeof(current_time), "%lld", current_time_int);
    char *reading = "1000";
    char *reading_document = malloc(128 * sizeof(char));
    int string_len = snprintf(reading_document, 128, reading_document_format, current_time, reading);
    printf("Reading document: %s\n", reading_document);

    firestore_createDocument("dev/develop/devices", "test_record_21", reading_document, access_token);


    /**
    record format:
    {
        "fields": {
            "record": {
                <timestamp>: {integer_value: <reading>}
            }
        }
    }
    */
    



    // char *firestore_path = "devices/abcde";

    // char *content = malloc(512 * sizeof(char));

    // esp_err_t result = firestore_get(firestore_path, content);

    // if (result == ESP_OK)
    // {
    //     printf("Content: %s\n", content);
    // }
    // else
    // {
    //     printf("Error: %d\n", result);
    // }
    

    // firestore_init();


    // char *collection_id = "devices";
    // char *document_id = device_id;
    // // get current utc time
    // int64_t current_time_int = esp_timer_get_time();
    // char current_time[20];
    // snprintf(current_time, sizeof(current_time), "%lld", current_time_int);
    // char *reading = "188032000";

    // /**
    //  * Format the reading document
    //  * 
    //     {
    //       "fields": {
    //         "val": {
    //         "integerValue": "%s"
    //         },
    //         "dt": {
    //         "integerValue": "%s"
    //         }
    //       }
        
    //     }
    //  */
    // char *reading_document_format = 
    //     "{"
    //         "\"fields\": {"
    //             "\"val\": {"
    //                 "\"integerValue\": \"%s\""
    //             "},"
    //             "\"dt\": {"
    //                 "\"integerValue\": \"%s\""
    //             "}"
    //         "}"
    //     "}";

    // char *reading_document = malloc(128 * sizeof(char));
    // int string_len = snprintf(reading_document, 128, reading_document_format, current_time, reading);
    // printf("Reading document: %s\n", reading_document);

    // firestore_add_document(
    //     collection_id,
    //     document_id,
    //     reading_document
    // );


    // free(reading_document);
}