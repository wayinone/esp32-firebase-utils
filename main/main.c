#include <stdio.h>
#include <string.h>
#include "firestore_utils.h"
#include "firebase_auth.h"
#include "station_mode.h"


void app_main(void)
{

    initWifiSta();

    /**
     * Example of getting an access token from a refresh token
     */
    char access_token[1024];
    firebase_get_access_token_from_refresh_token(CONFIG_FIREBASE_REFRESH_TOKEN, access_token);

    printf("Access token: %s\n", access_token); // This token is valid for 1 hour
    printf("Access token length: %d\n", strlen(access_token));  // usually 758

    /**
     * Example of creating a document
     * 
     * The document at the path "dev/develop/devices" will be created with the name "test_record_23" and the content of the document will be {"Sep14": 1000}
     * If the document already exists, it will be overwritten.
     * Note that if your firestore doesn't require token, you can pass NULL as the last argument. (This is not recommended, but it is good to test if the firestore is working)
     */
    char *example_doc = 
        "{"
            "\"fields\": {"
                "\"Sep14\": {"
                    "\"integerValue\": \"1000\""
                "},"
            "}"
        "}";
    firestore_createDocument("dev/develop/devices", "test_record_23", example_doc, access_token);

    /**
     * Example of patching a document
     * 
     * The document at the path "dev/develop/devices/test_dev/log/2409" will be updated with the key:value "Oct23: 500" and "Oct22: 500"
     * If the fields do not exist, they will be created.
     * * Note that if your firestore doesn't require token, you can pass NULL as the last argument. (This is not recommended, but it is good to test if the firestore is working)
    */
   char example_path_record[] = "{\"fields\": { \"Oct23\": {\"integerValue\": \"500\"}, \"Oct22\": {\"integerValue\": \"500\"}}}";
   firestore_patch("dev/develop/devices/test_dev/log/2409", example_path_record, access_token);


}