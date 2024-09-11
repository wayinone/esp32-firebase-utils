/*
This uses the firebase api: https://firebase.google.com/docs/reference/rest/auth/

*/
#include <sys/param.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "firebase_auth.h"
#include "esp_tls.h"
#include "cJSON.h"

static const char *TAG = "FIREBASE_AUTH";

#define MAX_HTTP_RECV_BUFFER 1024
#define MAX_HTTP_OUTPUT_BUFFER 1024

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  static char *output_buffer; // Buffer to store response of http request from event handler
  static int output_len;      // Stores number of bytes read
  switch (evt->event_id)
  {
  case HTTP_EVENT_REDIRECT:
    ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
    esp_http_client_set_header(evt->client, "From", "user@example.com");
    esp_http_client_set_header(evt->client, "Accept", "text/html");
    esp_http_client_set_redirection(evt->client);
    break;
  case HTTP_EVENT_ERROR:
    ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
    // Clean the buffer in case of a new request
    if (output_len == 0 && evt->user_data)
    {
      // we are just starting to copy the output data into the use
      memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
    }
    /*
     *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
     *  However, event handler can also be used in case chunked encoding is used.
     */
    if (!esp_http_client_is_chunked_response(evt->client))
    {
      // If user_data buffer is configured, copy the response into the buffer
      int copy_len = 0;
      if (evt->user_data)
      {
        // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
        copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
        if (copy_len)
        {
          memcpy(evt->user_data + output_len, evt->data, copy_len);
        }
      }
      else
      {
        int content_len = esp_http_client_get_content_length(evt->client);
        if (output_buffer == NULL)
        {
          // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
          output_buffer = (char *)calloc(content_len + 1, sizeof(char));
          output_len = 0;
          if (output_buffer == NULL)
          {
            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
            return ESP_FAIL;
          }
        }
        copy_len = MIN(evt->data_len, (content_len - output_len));
        if (copy_len)
        {
          memcpy(output_buffer + output_len, evt->data, copy_len);
        }
      }
      output_len += copy_len;
    }

    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
    if (output_buffer != NULL)
    {
      // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
      // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
      free(output_buffer);
      output_buffer = NULL;
    }
    output_len = 0;
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
    int mbedtls_err = 0;
    esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
    if (err != 0)
    {
      ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
      ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
    }
    if (output_buffer != NULL)
    {
      free(output_buffer);
      output_buffer = NULL;
    }
    output_len = 0;
    break;
  }
  return ESP_OK;
}

esp_err_t firebase_auth_login(char *email, char *password, firebase_auth_response_t *auth_response)
{

  const char *auth_request_body_payload_format = "{\"email\":\"%s\",\"password\":\"%s\",\"returnSecureToken\":\"true\"}";

  char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0}; // Buffer to store response of http request

  char *request_body_payload = (char *)malloc(strlen(auth_request_body_payload_format) + strlen(email) + strlen(password) + 1);
  sprintf(request_body_payload, auth_request_body_payload_format, email, password);

  char *url = NULL;
  asprintf(&url, "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=%s", FIREBASE_API_KEY);

  esp_http_client_config_t config = {
      .url = url,
      .cert_pem = FIREBASE_CA_CERT_PEM,
      .method = HTTP_METHOD_POST,
      .event_handler = _http_event_handler,
      .buffer_size = MAX_HTTP_RECV_BUFFER,
      .buffer_size_tx = 1024,
      .user_data = local_response_buffer // Pass address of local buffer to get response
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_post_field(client, request_body_payload, strlen(request_body_payload));

  // obtain the response
  esp_err_t err = esp_http_client_perform(client);
  if (err == ESP_OK)
  {
    ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    // Response is now stored in local_response_buffer, use json parser to extract the required fields

    cJSON *root = cJSON_Parse(local_response_buffer);
    if (root == NULL)
    {
      ESP_LOGE(TAG, "firebase_auth_login: Error parsing response");
    }
    else
    {
      cJSON *idToken = cJSON_GetObjectItem(root, "idToken");
      cJSON *email = cJSON_GetObjectItem(root, "email");
      cJSON *refreshToken = cJSON_GetObjectItem(root, "refreshToken");
      cJSON *expiresIn = cJSON_GetObjectItem(root, "expiresIn");
      cJSON *localId = cJSON_GetObjectItem(root, "localId");
      cJSON *registered = cJSON_GetObjectItem(root, "registered");

      if (idToken && email && refreshToken && expiresIn && localId && registered)
      {
        strcpy(auth_response->idToken, idToken->valuestring);
        strcpy(auth_response->email, email->valuestring);
        strcpy(auth_response->refreshToken, refreshToken->valuestring);
        strcpy(auth_response->expiresIn, expiresIn->valuestring);
        strcpy(auth_response->localId, localId->valuestring);
        auth_response->registered = registered->valueint;
      }
      cJSON_Delete(root);
    }
    return ESP_OK;
  }
  else
  {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    return ESP_FAIL;
  }
}