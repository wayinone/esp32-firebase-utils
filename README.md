# ESP32-FIREBASE-UTILS
This repo provides some useful APIs for esp32 developer to interact with Firebase Firestore. 


# APIs
Currently the APIs here includes:
* Acquire access token with refresh token
  * Note that refresh token will never expired until you delete the corresponding private key from (GCP -> service account -> key)
  * User of this should store the refresh key in `menuconfig` -> FIREBASE_REFRESH_TOKEN
* Firestore data IO
  * Create a document
  * Upsert or overwrite a document
  * get a value from specified field from a document

## Menu Configuration for this Component
  `idf.py menuconfig` -> Firebase Utils Configuration

## Coding Philosophy
* While developing this, I realize that even 100 bytes variable shouldn't be put into stack, so I make most of the long string with SPIRAM.
* I will assume user that use this code wouldn't want to flash the code every time because of the change google API website certificate. So I remove the certification part from the http request.

## Using the code
* If you expect a long response from the official rest APIs used here, then you will hit stack overflow.
* When patching a field with `firestore_path` , I put a limit (5) to how many fields can be patched, this is to ensure the request query isn't too long to cause trouble.
* You should keep the json content small so that request string will not too long. 


## Examples
In `main/main.c` there is example of using the API.

# Kconfigproject.build

* [v] enable SPIRAM
* mbedTLS -> Memory allocation strategy -> External SPIRAM
* Component config->ESP LTS-> (enable these options) "Allow potentially insecure options" and then "Skip server verification by default": This skip the https request certificate process

i.e.
```
CONFIG_ESP_TLS_INSECURE=y
CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY=y

CONFIG_SPIRAM=y
CONFIG_MBEDTLS_EXTERNAL_MEM_ALLOC=y
```

