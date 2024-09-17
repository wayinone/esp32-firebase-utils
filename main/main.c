#include <stdio.h>
#include <string.h>
#include "firestore_utils.h"
#include "firebase_auth.h"
#include "station_mode.h"
#include "esp_heap_caps.h"
#include "cJSON.h"

// #include "freertos/FreeRTOS.h"

void app_main(void)
{

     /**
     * Initialize the wifi station
     * See readme about how to configure the wifi (ssid and password)
     */
    initWifiSta();

    /**
     * Example of getting an access token from a refresh token
    //  */
    firebase_auth_init();
    char *access_token = (char *)heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);
    firebase_get_access_token_from_refresh_token(access_token);
    firebase_auth_cleanup();
    printf("Access token: %s\n", access_token); // This token is valid for 1 hour

    /**
     * Example of creating a document
     *
     * The document at the path "dev/develop/devices" will be created with the name "test_record_XX" and the content of the document will be {"Sep14": 1000}
     * If the document already exists, it will throw error.
     * Note that if your firestore doesn't require token, you can pass NULL as the last argument. (This is not recommended, but it is good to test if the firestore is working)
     */
    char example_doc[] = "{\"fields\": { \"Sep30\": {\"integerValue\": \"1000\"}}}";
    firestore_createDocument("dev/develop/devices", "test_record_27", example_doc, access_token);

    /**
     * Example of patching a document
     *
     * The document at the path "dev/develop/devices/test_dev/log/2410" will be updated with the key:value "OctXX: 500" and "OctXX: 500"
     * If the fields do not exist, they will be created.
     * * Note that if your firestore doesn't require token, you can pass NULL as the last argument. (This is not recommended, but it is good to test if the firestore is working)
     */
    printf("Patching a document with upsert method... \n");
    char example_path_record[] = "{\"fields\": { \"Aug05\": {\"integerValue\": \"700\"}, \"Aug06\": {\"integerValue\": \"700\"}}}";
    firestore_patch("dev/develop/devices/test_dev/log/2408", example_path_record, access_token, FIRESTORE_DOC_UPSERT);

    printf("Patching a document with overwrite method... \n");
    char overwrite_example_doc[] = "{\"fields\": { \"Nov01\": {\"integerValue\": \"20\"}}}"; // This will overwrite the entire document
    firestore_patch("dev/develop/devices/test_record_27", overwrite_example_doc, access_token, FIRESTORE_DOC_OVERWRITE);


    /** 
     * Example of getting a field value from a document
     */
    char field_value[10];
    char field_to_get[] = "Nov01";
    firestore_get_a_field_value("dev/develop/devices/test_record_27", field_to_get, access_token, field_value);

    printf("Get field \"%s\"'s value: %s\n", field_to_get, field_value);



    // heap_caps_free(access_token);
}