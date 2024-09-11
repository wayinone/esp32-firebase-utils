#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"

#include "firestore.h"
#include "config.h"

#define TAG "FIRESTORE"

#define FIRESTORE_DEBUG

esp_err_t _firestore_http_event_handler(esp_http_client_event_t *pstEvent);

extern const char firestore_api_pem_start[] asm("_binary_googleapis_com_chain_pem_start");
extern const char firestore_api_pem_end[] asm("_binary_googleapis_com_chain_pem_end");

typedef struct
{
  char tcPath[FIRESTORE_HTTP_PATH_SIZE];  // tc for "text char"
  uint32_t u32PathLen;
  char tcHttpBody[FIRESTORE_HTTP_RX_BUF_SIZE];
  uint32_t u32HttpBodyLen;
  esp_http_client_handle_t pstHttpClient; // pst for "pointer to struct"
  esp_http_client_config_t stHttpconfig; // st for "struct"
} firestore_ctx_t;

static firestore_ctx_t stCtx; // st for "static"

firestore_err_t firestore_init(void)
{
#ifndef FIRESTORE_DEBUG
  esp_log_level_set("FIRESTORE", ESP_LOG_NONE);
#endif /* FIRESTORE_DEBUG */
  memset(&stCtx, 0x00, sizeof(stCtx));
  stCtx.stHttpconfig.host = FIRESTORE_HOSTNAME;
  stCtx.stHttpconfig.buffer_size = FIRESTORE_HTTP_INTERNAL_RX_BUF_SIZE;
  stCtx.stHttpconfig.buffer_size_tx = FIRESTORE_HTTP_INTERNAL_TX_BUF_SIZE;
  stCtx.stHttpconfig.cert_pem = firestore_api_pem_start;
  stCtx.stHttpconfig.event_handler = _firestore_http_event_handler;
  stCtx.stHttpconfig.transport_type = HTTP_TRANSPORT_OVER_SSL;
  stCtx.stHttpconfig.user_data = stCtx.tcHttpBody;  // tc for "text char"
  return FIRESTORE_OK;
}

firestore_err_t firestore_get_collection(char *pcCollectionId,
                                         char **ppcDocs,
                                         uint32_t *pu32DocsLen)
{
  int16_t s16HttpCode;
  int32_t s32Length;
  firestore_err_t eRetVal;

  eRetVal = FIRESTORE_OK;
  if (pcCollectionId)
  {
    s32Length = snprintf(stCtx.tcPath,
                         FIRESTORE_HTTP_PATH_SIZE,
                         "/v1/projects/%s/databases/(default)/documents/%s?key=%s",
                         FIRESTORE_FIREBASE_PROJECT_ID,
                         pcCollectionId,
                         FIREBASE_API_KEY);
    if (s32Length > 0)
    {
      stCtx.stHttpconfig.path = stCtx.tcPath;
      ESP_LOGI(TAG, "HTTP path: %s", stCtx.stHttpconfig.path);
      stCtx.pstHttpClient = esp_http_client_init(&stCtx.stHttpconfig);
      if (ESP_OK == esp_http_client_perform(stCtx.pstHttpClient))
      {
        s16HttpCode = esp_http_client_get_status_code(stCtx.pstHttpClient);
        ESP_LOGI(TAG,
                 "HTTP code: %d, content_length: %d",
                 (int)s16HttpCode,
                 (int)esp_http_client_get_content_length(stCtx.pstHttpClient));
        if (200 != s16HttpCode)
        {
          ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", s16HttpCode);
          eRetVal = FIRESTORE_ERR_HTTP;
        }
        else
        {
          *ppcDocs = stCtx.tcHttpBody;
          *pu32DocsLen = stCtx.u32HttpBodyLen;
        }
      }
      else
      {
        eRetVal = FIRESTORE_ERR_HTTP;
      }
    }
    else
    {
      eRetVal = FIRESTORE_ERR;
    }
  }
  else
  {
    eRetVal = FIRESTORE_ERR_ARG;
  }
  stCtx.u32HttpBodyLen = 0;
  esp_http_client_cleanup(stCtx.pstHttpClient);
  return eRetVal;
}

firestore_err_t firestore_get_document(char *pcCollectionId,
                                       char *pcDocumentId,
                                       char **ppcDocument,
                                       uint32_t *pu32DocumentLen)
{
  int16_t s16HttpCode;
  int32_t s32Length;
  firestore_err_t eRetVal;

  eRetVal = FIRESTORE_OK;
  if (pcCollectionId && pcDocumentId)
  {
    s32Length = snprintf(stCtx.tcPath,
                         FIRESTORE_HTTP_PATH_SIZE,
                         "/v1/projects/%s/databases/(default)/documents/%s/%s?key=%s",
                         FIRESTORE_FIREBASE_PROJECT_ID,
                         pcCollectionId,
                         pcDocumentId,
                         FIREBASE_API_KEY);
    if (s32Length > 0)
    {
      stCtx.stHttpconfig.path = stCtx.tcPath;
      ESP_LOGI(TAG, "HTTP path: %s", stCtx.stHttpconfig.path);
      stCtx.pstHttpClient = esp_http_client_init(&stCtx.stHttpconfig);
      if (ESP_OK == esp_http_client_perform(stCtx.pstHttpClient))
      {
        s16HttpCode = esp_http_client_get_status_code(stCtx.pstHttpClient);
        ESP_LOGI(TAG,
                 "HTTP code: %d, content_length: %d",
                 (int)s16HttpCode,
                 (int)esp_http_client_get_content_length(stCtx.pstHttpClient));
        if (200 != s16HttpCode)
        {
          ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", s16HttpCode);
          eRetVal = FIRESTORE_ERR_HTTP;
        }
        else
        {
          *ppcDocument = stCtx.tcHttpBody;
          *pu32DocumentLen = stCtx.u32HttpBodyLen;
        }
      }
      else
      {
        eRetVal = FIRESTORE_ERR_HTTP;
      }
    }
    else
    {
      eRetVal = FIRESTORE_ERR;
    }
  }
  else
  {
    eRetVal = FIRESTORE_ERR_ARG;
  }
  stCtx.u32HttpBodyLen = 0;
  esp_http_client_cleanup(stCtx.pstHttpClient);
  return eRetVal;
}

firestore_err_t firestore_add_document(char *pcCollectionId,
                                       char *pcDocumentId,
                                       char *pcDocument)
{
  int16_t s16HttpCode;
  int32_t s32Length;
  firestore_err_t eRetVal;

  eRetVal = FIRESTORE_OK;
  if (pcCollectionId && pcDocumentId)
  {
    s32Length = snprintf(stCtx.tcPath,
                         FIRESTORE_HTTP_PATH_SIZE,
                         "/v1/projects/%s/databases/(default)/documents/%s?documentId=%s&key=%s",
                         FIRESTORE_FIREBASE_PROJECT_ID,
                         pcCollectionId,
                         pcDocumentId,
                         FIREBASE_API_KEY);
    if (s32Length > 0)
    {
      stCtx.stHttpconfig.path = stCtx.tcPath;
      ESP_LOGI(TAG, "HTTP path: %s", stCtx.stHttpconfig.path);
      stCtx.pstHttpClient = esp_http_client_init(&stCtx.stHttpconfig);
      esp_http_client_set_method(stCtx.pstHttpClient, HTTP_METHOD_POST);
      esp_http_client_set_header(stCtx.pstHttpClient, "Content-Type", "application/json");
      esp_http_client_set_post_field(stCtx.pstHttpClient, pcDocument, strlen(pcDocument));
      if (ESP_OK == esp_http_client_perform(stCtx.pstHttpClient))
      {
        s16HttpCode = esp_http_client_get_status_code(stCtx.pstHttpClient);
        ESP_LOGI(TAG,
                 "HTTP PATCH Status = %d, content_length = %d",
                 (int)esp_http_client_get_status_code(stCtx.pstHttpClient),
                 (int)esp_http_client_get_content_length(stCtx.pstHttpClient));
        if (200 != s16HttpCode)
        {
          ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", s16HttpCode);
          eRetVal = FIRESTORE_ERR_HTTP;

          // Print the error message
          if (stCtx.u32HttpBodyLen > 0)
          {
            ESP_LOGE(TAG, "Error message: %s", stCtx.tcHttpBody);
          }
        }
        else
        {
          ESP_LOGI(TAG, "Document added successfully");
        }
      }
      else
      {
        eRetVal = FIRESTORE_ERR_HTTP;
      }
    }
    else
    {
      eRetVal = FIRESTORE_ERR;
    }
  }
  else
  {
    eRetVal = FIRESTORE_ERR_ARG;
  }
  stCtx.u32HttpBodyLen = 0;
  esp_http_client_cleanup(stCtx.pstHttpClient);
  return eRetVal;
}

firestore_err_t firestore_update_document(char *pcCollectionId,
                                          char *pcDocumentId,
                                          char *pcDocument,
                                          uint32_t *pu32DocumentLen)
{
  int16_t s16HttpCode;
  int32_t s32Length;
  firestore_err_t eRetVal;

  eRetVal = FIRESTORE_OK;
  if (pcCollectionId && pcDocumentId)
  {
    s32Length = snprintf(stCtx.tcPath,
                         FIRESTORE_HTTP_PATH_SIZE,
                         "/v1/projects/%s/databases/(default)/documents/%s/%s?key=%s",
                         FIRESTORE_FIREBASE_PROJECT_ID,
                         pcCollectionId,
                         pcDocumentId,
                         FIREBASE_API_KEY);
    if (s32Length > 0)
    {
      stCtx.stHttpconfig.path = stCtx.tcPath;
      ESP_LOGI(TAG, "HTTP path: %s", stCtx.stHttpconfig.path);
      stCtx.pstHttpClient = esp_http_client_init(&stCtx.stHttpconfig);
      esp_http_client_set_method(stCtx.pstHttpClient, HTTP_METHOD_PATCH);
      esp_http_client_set_header(stCtx.pstHttpClient, "Content-Type", "application/json");
      esp_http_client_set_post_field(stCtx.pstHttpClient, pcDocument, strlen(pcDocument));
      if (ESP_OK == esp_http_client_perform(stCtx.pstHttpClient))
      {
        s16HttpCode = esp_http_client_get_status_code(stCtx.pstHttpClient);
        ESP_LOGI(TAG,
                 "HTTP PATCH Status = %d, content_length = %d",
                 (int)esp_http_client_get_status_code(stCtx.pstHttpClient),
                 (int)esp_http_client_get_content_length(stCtx.pstHttpClient));
        if (200 != s16HttpCode)
        {
          ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", s16HttpCode);
          eRetVal = FIRESTORE_ERR_HTTP;
        }
        else
        {
          pcDocument = stCtx.tcHttpBody;
          *pu32DocumentLen = stCtx.u32HttpBodyLen;
        }
      }
      else
      {
        eRetVal = FIRESTORE_ERR_HTTP;
      }
    }
    else
    {
      eRetVal = FIRESTORE_ERR;
    }
  }
  else
  {
    eRetVal = FIRESTORE_ERR_ARG;
  }
  stCtx.u32HttpBodyLen = 0;
  esp_http_client_cleanup(stCtx.pstHttpClient);
  return eRetVal;
}

firestore_err_t firestore_delete_document(char *pcCollectionId, char *pcDocumentId)
{
  int16_t s16HttpCode;
  int32_t s32Length;
  firestore_err_t eRetVal;

  eRetVal = FIRESTORE_OK;
  if (pcCollectionId && pcDocumentId)
  {
    s32Length = snprintf(stCtx.tcPath,
                         FIRESTORE_HTTP_PATH_SIZE,
                         "/v1/projects/%s/databases/(default)/documents/%s/%s?key=%s",
                         FIRESTORE_FIREBASE_PROJECT_ID,
                         pcCollectionId,
                         pcDocumentId,
                         FIREBASE_API_KEY);
    if (s32Length > 0)
    {
      stCtx.stHttpconfig.path = stCtx.tcPath;
      ESP_LOGI(TAG, "HTTP path: %s", stCtx.stHttpconfig.path);
      stCtx.pstHttpClient = esp_http_client_init(&stCtx.stHttpconfig);
      esp_http_client_set_method(stCtx.pstHttpClient, HTTP_METHOD_DELETE);
      if (ESP_OK == esp_http_client_perform(stCtx.pstHttpClient))
      {
        s16HttpCode = esp_http_client_get_status_code(stCtx.pstHttpClient);
        ESP_LOGI(TAG,
                 "HTTP code: %d, content_length: %d",
                 (int)s16HttpCode,
                 (int)esp_http_client_get_content_length(stCtx.pstHttpClient));
        if (200 != s16HttpCode)
        {
          ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", s16HttpCode);
          eRetVal = FIRESTORE_ERR_HTTP;
        }
        else
        {
          ESP_LOGI(TAG, "Document deleted successfully");
        }
      }
      else
      {
        eRetVal = FIRESTORE_ERR_HTTP;
      }
    }
    else
    {
      eRetVal = FIRESTORE_ERR;
    }
  }
  else
  {
    eRetVal = FIRESTORE_ERR_ARG;
  }
  stCtx.u32HttpBodyLen = 0;
  esp_http_client_cleanup(stCtx.pstHttpClient);
  return eRetVal;
}

esp_err_t _firestore_http_event_handler(esp_http_client_event_t *pstEvent)
{
  esp_err_t s32RetVal;

  s32RetVal = ESP_OK;
  switch (pstEvent->event_id)
  {
  case HTTP_EVENT_ERROR:
    ESP_LOGI(TAG, "HTTP error");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGI(TAG, "HTTP connected to server");
    break;
  case HTTP_EVENT_HEADERS_SENT:
    ESP_LOGI(TAG, "All HTTP headers are sent to server");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGI(TAG, "HTTP header received");
    break;
  case HTTP_EVENT_ON_DATA:
    /* If user_data buffer is configured, copy the response into it */
    if (pstEvent->user_data)
    {
      strncpy((char*)pstEvent->user_data + stCtx.u32HttpBodyLen,
              (char *)pstEvent->data,
              pstEvent->data_len);
      stCtx.u32HttpBodyLen += pstEvent->data_len;
    }
    /* Else you can copy the response into a global HTTP buffer */
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGI(TAG, "HTTP session is finished");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "HTTP connection is closed");
    break;
  case HTTP_EVENT_REDIRECT:
    ESP_LOGI(TAG, "HTTP redirect");
    break;
  }
  return s32RetVal;
}


