#ifndef FIREBASE_COMMON_H_
#define FIREBASE_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "esp_err.h"
#include <stdbool.h>
#include "esp_http_client.h"

esp_err_t firestore_http_event_handler(esp_http_client_event_t *pstEvent);


#ifdef __cplusplus
}   
#endif

#endif /* FIREBASE_COMMON_H_ */