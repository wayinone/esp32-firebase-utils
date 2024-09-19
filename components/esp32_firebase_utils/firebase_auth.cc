/**
 * @file firestore_utils.cc
 * @brief Firestore auth functions based on google's secure token API
 * https://cloud.google.com/identity-platform/docs/use-rest-api#section-refresh-token
 * Note that I deliberately remove the requirement for website certificate verification
 * So that I can make a request to the Firestore API without having to use the root certificate
 * Remember to go to `idf.py menuconfig` and set
 * Component config->ESP LTS-> (enable these options) "Allow potentially insecure options" and 
 * then "Skip server verification by default"
 * 
 */

#include "firebase_auth.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"

#define FIREBASE_TOKEN_REQUEST_HOSTNAME "securetoken.googleapis.com"
#define FIREBASE_AUTH_PATH "/v1/token?key=" FIREBASE_API_KEY
#define FIREBASE_AUTH_BODY "grant_type=refresh_token&refresh_token=" FIREBASE_REFRESH_TOKEN

static const char *TAG = "FB_AUTH";
static const char *TAG_EVENT_HANDLER = "FB_EVENT";

static const int SEND_BUF_SIZE = 1024; // this is also called transmit (tx) buffer size
static const int RECEIVE_BUF_SIZE = 4096;

static char *RECEIVE_BODY = NULL;
static int receive_body_len = 0;

void firebase_auth_init()
{
  // initialize the receive body buffer over SPIRAM
  RECEIVE_BODY = (char *)heap_caps_malloc(RECEIVE_BUF_SIZE, MALLOC_CAP_SPIRAM);
}

void firebase_auth_cleanup()
{
  heap_caps_free(RECEIVE_BODY);
  RECEIVE_BODY = NULL;
}


static esp_err_t firebase_http_event_handler(esp_http_client_event_t *client_event);

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

  esp_http_client_config_t http_config = {
      .host = FIREBASE_TOKEN_REQUEST_HOSTNAME,
      .path = FIREBASE_AUTH_PATH,
      .method = HTTP_METHOD_POST,
      .event_handler = firebase_http_event_handler,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .buffer_size = RECEIVE_BUF_SIZE,
      .buffer_size_tx = SEND_BUF_SIZE,
      .user_data = RECEIVE_BODY // note that this is the response body buffer
  };

  esp_http_client_handle_t firebase_client_handle = esp_http_client_init(&http_config);
  ESP_LOGI(TAG, "http config initialized");

  esp_http_client_set_header(firebase_client_handle, "Content-Type", "application/x-www-form-urlencoded");
  esp_http_client_set_post_field(firebase_client_handle, FIREBASE_AUTH_BODY, strlen(FIREBASE_AUTH_BODY));

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
  ESP_LOGI(TAG,
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
  // the auth request should return a json object of size about 1870 bytes
  ESP_LOGD(TAG, "total received body length: %d", strlen(RECEIVE_BODY));

  if (strlen(RECEIVE_BODY) > 0)
  {
    get_value_from_json(RECEIVE_BODY, "id_token", access_token);
  }

  esp_http_client_cleanup(firebase_client_handle);
  receive_body_len = 0;
  ESP_LOGI(TAG, "HTTP request cleanup");
  return ESP_OK;
}


/**
 * @brief HTTP event handler for FirebaseAPI
 * 
 * TODO: this is exactly copy-pasted from firebase_auth.cc, I can't find a way to make this a shared function
 * (because there is a static variable `receive_body_len` that is used in the function)
 */
static esp_err_t firebase_http_event_handler(esp_http_client_event_t *client_event)
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
    if (client_event->user_data)
    {
      strncpy((char *)client_event->user_data + receive_body_len, // destination buffer
              (char *)client_event->data,                         // source buffer
              client_event->data_len);
      receive_body_len += client_event->data_len;
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
