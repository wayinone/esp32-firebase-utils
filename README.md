# ESP32-FIREBASE-UTILS

This repo provides some useful APIs for esp32 developer to interact with Firebase Firestore.
This includes basic authentication and firestore read / write functions.

## Installation

To include this into your esp-idf project, add the following **git dependencies** in your `idf_component.yaml` file (note that you should use the latest version here.) For more information about git dependency, you can view official document [here](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#component-dependencies)

```yaml
dependencies:
  wayinone_esp32-firebase-utils:
    git: "https://github.com/wayinone/esp32-firebase-utils.git"
    path: "components/esp32_firebase_utils"
    version: "v0.2.0"
```

Note that, if for some reason, you would like to change the version, you will have to remove the `dependencies.lock` file from your project's root folder, and rebuild again (either by running command `idf.py reconfigure` or simply `idf.py build`)

## APIs

Note that all the APIs here are wrappers of Firebase's REST API, hence, WIFI is necessary. 
Also please make sure you have configure the project with the setting described in section [Configuration for this Component](#configuration-for-this-component)

* If you don't have WIFI setup in your project, you can simply include my `components/wifi` (by copy paste or use `idf_component.yaml` like above, with `path: "components/wifi`. Then set your WIFI's SSID and password in `idf.py menuconfig`-> WIFI STA Configuration)
  
    ```cpp
    #include "station_mode.h"
    initWifiSta();
    ```

Currently the APIs here includes:

* **Acquire access token with refresh token**
  * Note that refresh token will never expired until you delete the corresponding private key from (GCP -> service account -> key)
  * In this example, the refresh token (the first argument) is provided as NULL because the refresh token has been set in `CONFIG_FIREBASE_REFRESH_TOKEN`. Otherwise, user needs to provide the refresh token as the first argument.
  
    At global scope:

    ```cpp
    #include "firebase_auth.h"

    char *access_token[1024]
    ```

    In your function block:

    ```cpp
    firebase_get_access_token_from_refresh_token(NULL, access_token);
    printf("Access token: %s\n", access_token); // This token is valid for 1 hour
    ```

* **Firestore data IO**
  
  ```cpp
  #include "firestore_utils.h"
  ```

  Note that for the following examples, if your database doesn't require token, you can replace the `access_token` with `NULL`.
  * **`firestore_createDocument`**: Create a document (It will throw error if document is existed. Note that all the paths to the collection specified don't need to be existed.)
  
    ```cpp
    char example_doc[] = "{\"fields\": { \"Sep30\": {\"integerValue\": \"1000\"}}}";

    firestore_createDocument("dev/develop/devices", "test_record_27", example_doc, access_token);
    ```

  * **`firestore_patch`**: Patch a document (insert, update, or overwrite)

    There are two modes available:
    * mode `FIRESTORE_DOC_UPSERT`:  Upsert (insert new fields or update existed fields. This will create document if not existed)
  
      ```cpp
      char example_path_record[] = "{\"fields\": { \"Aug05\": {\"integerValue\": \"700\"}, \"Aug06\": {\"integerValue\": \"700\"}}}";

      firestore_patch
        "dev/develop/devices/test_dev/log/2408", 
        example_path_record, 
        access_token, 
        FIRESTORE_DOC_UPSERT);
      ```

    * mode `FIRESTORE_DOC_OVERWRITE`: Overwrite (This will overwrite any existed fields. Also document will be created if not existed)
  
      ```cpp
      char overwrite_example_doc[] = "{\"fields\": { \"Nov01\": {\"integerValue\": \"20\"}}}"; 

      firestore_patch(
        "dev/develop/devices/test_record_27", 
        overwrite_example_doc,
        access_token,
        FIRESTORE_DOC_OVERWRITE);
      ```

  * **`firestore_get_a_field_value`**: Get a value from a specified field from a document
  
    ```cpp
    char field_value[10];
    char field_to_get[] = "Nov01";
    
    firestore_get_a_field_value(
      "dev/develop/devices/test_record_27", 
      field_to_get, 
      access_token, 
      field_value);

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
* When patching a field with `firestore_patch` function, I put a limit (5) to how many fields can be patched, this is to ensure the request query isn't too long to cause trouble.
* You should keep the json content small so that request string will not too long. 

## How to Run Examples

First, you need to fill the configuration, see the section [Configuration for this Component Section](#configuration-for-this-component)

Then you can edit the example in `main/main.c`, and run through usually `idf.py build flash` process.

## Coding Philosophy

* I will assume user that use this code wouldn't want to flash the code every time because of the change google API website certificate. So I remove the certification part from the http request.

## Credits

* The firestore IO part of code was simplified from [kaizoku-oh/firestore](https://github.com/kaizoku-oh).

## About GCP's Refresh Token

* A refresh token can be generated with a service account. The usage of it is that it allows devices write to Google's service like Firestore without user to log in (the log in token only valid for 1 hour).
* A single service account (with sufficient roles attached) can generate a private key, and generate multiple refreshed accounts, each with a special JWT subject (`uid`), that can be used in Authentication.
  * E.g. in Firestore Rules, you can use `request.auth.uid` keyword to ensure the JWT subject is allowed in the service.
* In this repo we also provides a command make from Python to help you get refresh token from a private key. See [here](gcp_auth/README.md).

## License

*esp32-firebase-utils* is MIT licensed. As such, it can be included in any project, commercial or not, as long as you retain original copyright. Please make sure to read the license file.