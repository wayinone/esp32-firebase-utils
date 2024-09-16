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

static const int RECEIVE_BUF_SIZE = 4096; 

// make a global variable to store the received data
// todo: somehow I can't initialize this here, so I initialize it in firebase_auth.cc
extern char RECEIVE_BODY[RECEIVE_BUF_SIZE]; // note that this is initialized in firebase_auth.cc

esp_err_t firestore_http_event_handler(esp_http_client_event_t *pstEvent);

void return_received_data_buffer(void);

#ifdef __cplusplus
}   
#endif

#endif 