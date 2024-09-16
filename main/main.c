#include <stdio.h>
#include <string.h>
#include "firestore_utils.h"
#include "firebase_auth.h"
#include "station_mode.h"


void app_main(void)
{

    /**
     * Initialize the wifi station
     * See readme about how to configure the wifi (ssid and password)
     */
    initWifiSta();

    /**
     * Example of getting an access token from a refresh token
     */
    char access_token[1024];
    firebase_get_access_token_from_refresh_token(access_token);

    printf("Access token: %s\n", access_token); // This token is valid for 1 hour

    /**
    //  * Example of creating a document
    //  * 
    //  * The document at the path "dev/develop/devices" will be created with the name "test_record_23" and the content of the document will be {"Sep14": 1000}
    //  * If the document already exists, it will be overwritten.
    //  * Note that if your firestore doesn't require token, you can pass NULL as the last argument. (This is not recommended, but it is good to test if the firestore is working)
    //  */
    // char example_doc[] = 
    //     "{"
    //         "\"fields\": {"
    //             "\"Sep14\": {"
    //                 "\"integerValue\": \"1000\""
    //             "},"
    //         "}"
    //     "}";
    // firestore_createDocument("dev/develop/devices", "test_record_23", example_doc, NULL);
    // free(example_doc);

//     /**
//      * Example of patching a document
//      * 
//      * The document at the path "dev/develop/devices/test_dev/log/2410" will be updated with the key:value "Oct27: 500" and "Oct28: 500"
//      * If the fields do not exist, they will be created.
//      * * Note that if your firestore doesn't require token, you can pass NULL as the last argument. (This is not recommended, but it is good to test if the firestore is working)
//     */
//    char example_path_record[] = "{\"fields\": { \"Oct27\": {\"integerValue\": \"500\"}, \"Oct28\": {\"integerValue\": \"500\"}}}";
//    firestore_patch("dev/develop/devices/test_dev/log/2410", example_path_record, NULL);
//    free(example_path_record);

   /**
    // * Example of getting a document
    // * 
    // */
    // char document[2048];
    // firestore_get("dev/develop/devices/test_dev/log/2410", NULL, document);
    // printf("Document: %s\n", document);
    

}