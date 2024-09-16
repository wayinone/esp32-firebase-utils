/**
 * @file firestore_utils.cc
 * @brief Firestore utility functions based on
 * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/
 */

#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG // set the log level for this file

// #include "firebase_common.h" // import firebase_http_event_handler
#include "firebase_auth.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"

static const char *TAG = "FIREBASE_AUTH";
static const char *TAG_EVENT_HANDLER = "FIREBASE_AUTH_HTTP_EVENT";

#define FIREBASE_TOKEN_REQUEST_HOSTNAME "securetoken.googleapis.com"

/**
 * The following are defined by embedded data in CMakeLists.txt
 * and are used to verify the server's certificate.
 * One can download the certificate from:
 *  Firefox -> put website `FIRESTORE_HOSTNAME` -> click on the lock icon -> More Information -> View Certificate -> Details -> Export
 */
extern const char get_token_api_pem_start[] asm("_binary_secure_token_googleapis_chain_pem_start");
extern const char get_token_api_pem_end[] asm("_binary_secure_token_googleapis_chain_pem_end");

static const int SEND_BUF_SIZE = 1024; // this is also called transmit (tx) buffer size
static const int RECEIVE_BUF_SIZE = 4096;

static char RECEIVE_BODY[RECEIVE_BUF_SIZE] = {0};
static int receive_body_len = 0;

esp_err_t firebase_http_event_handler(esp_http_client_event_t *client_event);

/**
 * @brief Get the value from json object
 *
 * @param[in] json e.g. '\{"key1": "value1", "key2": "value2"\}'
 * @param[in] key e.g. "key1"
 * @param[out] value The output buffer to store the value, e.g. "value1"
 */
void get_value_from_json(const char *json, const char *key, char *value)
{
  cJSON *root = cJSON_Parse(json);
  cJSON *field = cJSON_GetObjectItem(root, key);
  if (field != NULL)
  {
    strcpy(value, field->valuestring);
  }
  cJSON_Delete(root);
}

esp_err_t firebase_get_access_token_from_refresh_token(char *access_token)
{

  char full_path[] = "/v1/token?key=" FIREBASE_API_KEY;
  char http_body[] = "grant_type=refresh_token&refresh_token=" FIREBASE_REFRESH_TOKEN;

  esp_http_client_config_t http_config = {
      .host = FIREBASE_TOKEN_REQUEST_HOSTNAME,
      .path = full_path,
      .cert_pem = get_token_api_pem_start,
      .method = HTTP_METHOD_POST,
      .event_handler = firebase_http_event_handler,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .buffer_size = RECEIVE_BUF_SIZE,
      .buffer_size_tx = SEND_BUF_SIZE,
      .user_data = RECEIVE_BODY // The actual receive buffer stored operation is happen in firebase_http_event_handler
  };

  esp_http_client_handle_t firebase_client_handle = esp_http_client_init(&http_config);
  ESP_LOGI(TAG, "http config initialized");

  esp_http_client_set_header(firebase_client_handle, "Content-Type", "application/x-www-form-urlencoded");
  esp_http_client_set_post_field(firebase_client_handle, http_body, strlen(http_body));

  ESP_LOGI(TAG, "http headers set up! Making request...");
  if (esp_http_client_perform(firebase_client_handle) != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to perform HTTP request");
    esp_http_client_cleanup(firebase_client_handle);
    receive_body_len = 0;
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "HTTP request performed");
  int response_code = esp_http_client_get_status_code(firebase_client_handle);
  ESP_LOGD(TAG,
           "HTTP Response code: %d, content_length: %d",
           response_code,
           (int)esp_http_client_get_content_length(firebase_client_handle));

  if (response_code != 200)
  {
    ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", response_code);
    {
      ESP_LOGE(TAG, "Error message: %s", RECEIVE_BODY);
    }
    esp_http_client_cleanup(firebase_client_handle);
    receive_body_len = 0;
    return ESP_FAIL;
  }
  ESP_LOGD(TAG, "total received body length: %d", strlen(RECEIVE_BODY)); // the auth request should return a json object of size about 1870

  if (strlen(RECEIVE_BODY) > 0)
  {
    get_value_from_json(RECEIVE_BODY, "id_token", access_token);
  }

  esp_http_client_cleanup(firebase_client_handle);
  receive_body_len = 0;
  return ESP_OK;
}

esp_err_t firebase_http_event_handler(esp_http_client_event_t *client_event)
{

  switch (client_event->event_id)
  {
  case HTTP_EVENT_ERROR:
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP error");
    receive_body_len = 0; // reset the receive body length
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP connected to server");
    receive_body_len = 0; // reset the receive body length
    break;
  case HTTP_EVENT_HEADERS_SENT:
    ESP_LOGI(TAG_EVENT_HANDLER, "All HTTP headers are sent to server");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP header received");
    break;
  case HTTP_EVENT_ON_DATA: // note that this might be called multiple times because the data might be chunked
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP data received, with length: %d", client_event->data_len);

    ESP_LOGD(TAG_EVENT_HANDLER, "received data: %s", (char *)client_event->data);
    if (client_event->user_data)
    {
      strncpy((char *)client_event->user_data + receive_body_len, // destination buffer
              (char *)client_event->data,                         // source buffer
              client_event->data_len);
      receive_body_len += client_event->data_len;
      ESP_LOGD(TAG_EVENT_HANDLER, "received data length: %d", receive_body_len);
      ESP_LOGD(TAG_EVENT_HANDLER, "received data: %s", RECEIVE_BODY);
    }

    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP session is finished");
    *((char *)client_event->user_data + receive_body_len) = '\0'; // write the null terminator to the buffer
    receive_body_len = 0;                                         // reset the receive body length
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP connection is closed");
    receive_body_len = 0; // reset the receive body length
    break;
  case HTTP_EVENT_REDIRECT:
    ESP_LOGI(TAG_EVENT_HANDLER, "HTTP redirect");
    receive_body_len = 0; // reset the receive body length
    break;
  }
  return ESP_OK;
}
