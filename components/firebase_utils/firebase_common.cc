#include "firebase_common.h"
#include "esp_log.h"

static const char *TAG = "FIREBASE_COMMON";

// Note that the RECEIVE_BODY is defined in firebase_common.h and is a global variable
// The RECEIVE_BODY is used to store the received data from the firestore server
// The following defines the buffer position to write the received data, this is because
// the data might be chunked and the event might be called multiple times.
char *current_receive_buffer_position = (char *)RECEIVE_BODY; // initialize the buffer position to the start of the buffer

void reset_received_buffer_position(void)
{
    // write the null terminator to the buffer
    *current_receive_buffer_position = '\0';
    current_receive_buffer_position = RECEIVE_BODY; // reset the buffer position to the beginning of the buffer
    
    ESP_LOGI(TAG, "Resetting received buffer position");
}

esp_err_t firestore_http_event_handler(esp_http_client_event_t *client_event)
{
    switch (client_event->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP error");
        reset_received_buffer_position();
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
    case HTTP_EVENT_ON_DATA: // note that this might be called multiple times because the data might be chunked
        ESP_LOGI(TAG, "HTTP data received");

        ESP_LOGD(TAG, "received data: %s", (char *)client_event->data);
        if (client_event->user_data)
        {
            strncpy(current_receive_buffer_position, // destination buffer
                    (char *)client_event->data,      // source buffer
                    client_event->data_len);
            current_receive_buffer_position += client_event->data_len;
        }
        ESP_LOGD(TAG, "written data to buffer, current received buffer: %s", RECEIVE_BODY);

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP session is finished");
        reset_received_buffer_position();
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP connection is closed");
        reset_received_buffer_position();
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP redirect");
        reset_received_buffer_position();
        break;
    }
    return ESP_OK;
}
