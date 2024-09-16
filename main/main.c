#include <stdio.h>
#include <string.h>
#include "firestore_utils.h"
#include "firebase_auth.h"
#include "station_mode.h"


#define device_id "abcde"

void app_main(void)
{

    initWifiSta();

    char access_token[1024];
    firebase_get_access_token_from_refresh_token(CONFIG_FIREBASE_REFRESH_TOKEN, access_token);

    printf("Access token: %s\n", access_token); // This token is valid for 1 hour
    printf("Access token length: %d\n", strlen(access_token));  // usually 758

    // example document {"Sep14": 1000}
    char *example_doc = 
        "{"
            "\"fields\": {"
                "\"Sep14\": {"
                    "\"integerValue\": \"1000\""
                "},"
            "}"
        "}";

    // create new document, if document already exists, it will be throw an error
    firestore_createDocument("dev/develop/devices", "test_record_23", example_doc, access_token);

    /**
    record format:
    {
        "fields": {
            <timestamp>: {integer_value: <reading>}
        }
    }
    */
   char *record_format = 
        "{"
            "\"fields\": {"
                "\"%s\": {\"integerValue\": \"%s\"}"
            "}"
        "}";
    char *first_record = malloc(128 * sizeof(char));
    int string_len = snprintf(first_record, 128, record_format, "0917", "500");
    printf("First record: %s\n", first_record);
    firestore_patch("dev/develop/devices/test_dev/log/2409", first_record, access_token);

    char *second_record = malloc(128 * sizeof(char));
    string_len = snprintf(second_record, 128, record_format, "0918", "600");
    printf("Second record: %s\n", second_record);
    firestore_patch("dev/develop/devices/test_dev/log/2409", second_record, access_token);

    free(first_record);
    free(second_record);

}