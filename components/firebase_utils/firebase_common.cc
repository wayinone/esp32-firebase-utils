#include "firebase_common.h"
#include "esp_log.h"

static const char *TAG = "FIRESTORE_COMMON";


esp_err_t firestore_http_event_handler(esp_http_client_event_t *pstEvent)
{
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
        ESP_LOGI(TAG, "HTTP data received");
        /* If user_data buffer is configured, copy the response into it */
        if (pstEvent->user_data)
        {
            strncpy((char *)pstEvent->user_data,
                    (char *)pstEvent->data,
                    pstEvent->data_len);
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
    return ESP_OK;
}
