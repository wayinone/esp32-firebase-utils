# ESP32-FIREBASE-UTILS
This repo provides some useful APIs for esp32 developer to interact with Firebase Firestore. 

# Installation
To include this into your esp-idf project, add the following **git dependencies** in your `idf_component.yaml` file (note that you should use the latest version here.) For more information about git dependency, you can view official document [here](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#component-dependencies)

```yaml
dependencies:
  wayinone__esp32-firebase-utils:
    git: "https://github.com/wayinone/esp32-firebase_utils.git"
    path: "components/esp32_firebase_utils"
    version: "0.0.1"
```
Note that, if for some reason, you would like to change the version, you will have to remove the `dependencies.lock` file from your project's root folder, and rebuild again.


## APIs
Currently the APIs here includes:
* **Acquire access token with refresh token**
  * Note that refresh token will never expired until you delete the corresponding private key from (GCP -> service account -> key)
  * User of this should store the refresh key in `menuconfig` -> FIREBASE_REFRESH_TOKEN
  ```cpp
  #include "firebase_auth.h"

  firebase_auth_init();
  char *access_token = (char *)heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);
  firebase_get_access_token_from_refresh_token(access_token);
  firebase_auth_cleanup();
  printf("Access token: %s\n", access_token); // This token is valid for 1 hour
  ```
* **Firestore data IO**
  * `firestore_createDocument`: Create a document (It will throw error if document is existed. Note that all the paths to the collection specified don't need to be existed.)
    ```cpp

    #include "firestore_utils.h"

    char example_doc[] = "{\"fields\": { \"Sep30\": {\"integerValue\": \"1000\"}}}";
    firestore_createDocument("dev/develop/devices", "test_record_27", example_doc, access_token);

    ```
  * `firestore_path`: Patch a document
      firestore_patch("dev/develop/devices/test_dev/log/2408", example_path_record, access_token, FIRESTORE_DOC_UPSERT);
    * mode `FIRESTORE_DOC_UPSERT`:  Upsert (insert new fields or update existed fields. This will create document if not existed)
      ```cpp
      #include "firestore_utils.h"

      char example_path_record[] = "{\"fields\": { \"Aug05\": {\"integerValue\": \"700\"}, \"Aug06\": {\"integerValue\": \"700\"}}}";
      firestore_patch("dev/develop/devices/test_dev/log/2408", example_path_record, access_token, FIRESTORE_DOC_UPSERT);
      ```
    * mode `FIRESTORE_DOC_OVERWRITE`: Overwrite (This will overwrite any existed fields. Also document will be created if not existed)
      ```cpp
      printf("Patching a document with overwrite method... \n");
      char overwrite_example_doc[] = "{\"fields\": { \"Nov01\": {\"integerValue\": \"20\"}}}"; // This will overwrite the entire document
      firestore_patch("dev/develop/devices/test_record_27", overwrite_example_doc, access_token, FIRESTORE_DOC_OVERWRITE);
      ```
  * `firestore_get_a_field_value`: get a value from specified field from a document
    ```cpp
    #include "firestore_utils.h"

    char field_value[10];
    char field_to_get[] = "Nov01";
    firestore_get_a_field_value("dev/develop/devices/test_record_27", field_to_get, access_token, field_value);

    printf("Get field \"%s\"'s value: %s\n", field_to_get, field_value);
    ```

## Configuration for this Component

### Firebase Configuration
 * For WIFI setup: `WIFI STA Configuration`
 * For Firebase setup: `Firebase Utils Configuration`

### HTTP Client (MbedTLS) Configuration

* enable SPIRAM
* mbedTLS -> Memory allocation strategy -> enable External SPIRAM
* Component config->ESP LTS-> (enable these options) 
  * "Allow potentially insecure options" and then, 
  * "Skip server verification by default": This skip the https request certificate process

i.e.
```
CONFIG_ESP_TLS_INSECURE=y
CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY=y
CONFIG_SPIRAM=y
CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC=y
```


## Using the code
* If you expect a long response from the official rest APIs used here, then you will hit stack overflow.
* When patching a field with `firestore_path` , I put a limit (5) to how many fields can be patched, this is to ensure the request query isn't too long to cause trouble.
* You should keep the json content small so that request string will not too long. 


## How to Run Examples
First, you need to fill the configuration, see the Menu Configuration for this Component Section

Then you can edit the example in `main/main.c`, and run through usually `idf.py build flash` process.


## Coding Philosophy
* While developing this, I realize that even 100 bytes variable shouldn't be put into stack, so I make sure that most of the long string with SPIRAM.
* I will assume user that use this code wouldn't want to flash the code every time because of the change google API website certificate. So I remove the certification part from the http request.

## License
*esp32-firebase-utils* is MIT licensed. As such, it can be included in any project, commercial or not, as long as you retain original copyright. Please make sure to read the license file.