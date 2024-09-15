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

    char access_token[1024];
    firebase_get_access_token_from_refresh_token(CONFIG_FIREBASE_REFRESH_TOKEN, access_token);

    printf("Access token: %s\n", access_token);


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

    // int64_t current_time_int = esp_timer_get_time();
    // char current_time[20];
    // snprintf(current_time, sizeof(current_time), "%lld", current_time_int);
    // char *reading = "1000";
    // char *reading_document = malloc(128 * sizeof(char));
    // int string_len = snprintf(reading_document, 128, reading_document_format, current_time, reading);
    // printf("Reading document: %s\n", reading_document);

    // char *access_token = "eyJhbGciOiJSUzI1NiIsImtpZCI6IjAyMTAwNzE2ZmRkOTA0ZTViNGQ0OTExNmZmNWRiZGZjOTg5OTk0MDEiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL3NlY3VyZXRva2VuLmdvb2dsZS5jb20vbWV0ZXItbW9uaXRvci1hMmQyOSIsImF1ZCI6Im1ldGVyLW1vbml0b3ItYTJkMjkiLCJhdXRoX3RpbWUiOjE3MjYzMjU5NjIsInVzZXJfaWQiOiJxbWh2eiIsInN1YiI6InFtaHZ6IiwiaWF0IjoxNzI2MzM0MjQyLCJleHAiOjE3MjYzMzc4NDIsImZpcmViYXNlIjp7ImlkZW50aXRpZXMiOnt9LCJzaWduX2luX3Byb3ZpZGVyIjoiY3VzdG9tIn19.CKp7jZpfDbT_PnNYJ0tPpsqlUmgBrSv26by5s0wgh-JzyDRark6NESS9pq5d2NQb7O7J_iXTdo3Zk8SovZ-tVjiXYMQmJMMahfFSP3HlWwoDSV8bPpApIYYpk6dzirTus_aAlhIxeMGeOmdnswbWXHnzcLhdiLa9KlmSZQ2GlQwb9Hyj3dsQUNaGncb67v1_vRSp7jEU-dkX66kYTlH9UcTy4ynEkbAETiRE2NmSj8iHrmvgA41eJRIom05ADWIcpwTdZl7m1foyxS5ctaxLHyhEOLbWFu35_auXY-zWU7-s8N6KBiyX6kTCFvYjRaKe5c-1-v-a8Np4mAFsLXnyHA";

    // firestore_createDocument("dev/develop/devices", "test_record_8", reading_document, access_token);





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