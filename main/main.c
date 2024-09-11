#include <stdio.h>
#include "firestore_utils.h"
#include "station_mode.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "cJSON.h"


#define FIREBASE_USER_EMAIL "xitydyxo@pelagius.net"
#define FIREBASE_USER_PASSWORD "abc1234"

#define device_id "abcde"

void app_main(void)
{

    initWifiSta();

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

    firestore_createDocument("users/nnTE6Tb3k8Y7EaC6atzfaPoiHs03/devices/logs/log1", "record1", reading_document);

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