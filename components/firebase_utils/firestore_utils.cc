/**
 * @file firestore_utils.cc
 * @brief Firestore utility functions based on
 * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/
 * Note that I deliberately remove the requirement for website certificate verification
 * So that I can make a request to the Firestore API without having to use the root certificate
 * Remember to go to `idf.py menuconfig` and set
 * Component config->ESP LTS-> (enable these options) "Allow potentially insecure options" and
 * then "Skip server verification by default"
 */

#include "firestore_utils.h"
#include <string.h>
#include "esp_log.h"
#include "cJSON.h"
#include "esp_http_client.h"

#define FIRESTORE_HOSTNAME "firestore.googleapis.com"
#define FIRESTORE_BASE_PATH_FORMAT "/v1/projects/" FIREBASE_PROJECT_ID "/" FIRESTORE_DB_ROOT "/%s"
#define FIRESTORE_DUMMY_RETURN_MASK "mask.fieldPaths=z" // this is used to prevent the whole document from being returned when using patch request

#define MAX_PATCH_UPDATE_MASK_BUFFER 256 // this buffer will hold string like "updateMask.fieldPaths=Oct21&updateMask.fieldPaths=Oct22"
#define MAX_PATCH_UPSERT_FIELDS 5

static const char *TAG = "FIRESTORE";
static const char *TAG_EVENT_HANDLER = "FIRESTORE_AUTH_HTTP_EVENT";

static const int BASE_PATH_FORMAT_SIZE = strlen(FIRESTORE_BASE_PATH_FORMAT) + 1;

static const int SEND_BUF_SIZE = 4096; // this is also called transmit (tx) buffer size
static const int RECEIVE_BUF_SIZE = 4096;

static char *RECEIVE_BODY = NULL;
static int receive_body_len = 0;

static char *PATCH_UPSERT_QUERY_BUFFER = NULL;

void firestore_utils_init()
{
    // initialize the receive body buffer over SPIRAM
    RECEIVE_BODY = (char *)heap_caps_malloc(RECEIVE_BUF_SIZE, MALLOC_CAP_SPIRAM);
    PATCH_UPSERT_QUERY_BUFFER = (char *)heap_caps_malloc(MAX_PATCH_UPDATE_MASK_BUFFER, MALLOC_CAP_SPIRAM);
}

void firestore_utils_cleanup()
{
    heap_caps_free(RECEIVE_BODY);
    RECEIVE_BODY = NULL;
    heap_caps_free(PATCH_UPSERT_QUERY_BUFFER);
    PATCH_UPSERT_QUERY_BUFFER = NULL;
}

static esp_err_t firestore_http_event_handler(esp_http_client_event_t *client_event);

/**
 * @brief Make an abstract API request to API
 *
 * @param[in] full_path The path of your collection and documents.
 * e.g. "col1/doc1", "col1", "col1/doc1/subcol1", or "col1/subcol1/doc1"
 * @param[in] queries queries The query parameters to be used in the HTTP request.
 *  e.g. "mask.fieldPaths=z&updateMask.fieldPaths=Oct21&updateMask.fieldPaths=Oct22"
 * @param[in] http_method The HTTP method to use. e.g. HTTP_METHOD_GET, HTTP_METHOD_POST, HTTP_METHOD_PATCH, HTTP_METHOD_DELETE
 * @param[in] http_body The body of the HTTP request. e.g. "{\"fields\": {\"name\": {\"stringValue\": \"John\"}}}"
 * if the request does not require a body, pass NULL
 * @param[in] auth_token The auth token to be used in the HTTP request. If the request does not require an auth token, pass NULL
 * @param[out] receive_http_body The body of the HTTP response (could be an error message, or returned json data)
 */
esp_err_t make_abstract_firestore_api_request(
    char *full_path,
    char *queries,
    esp_http_client_method_t http_method,
    char *http_body,
    char *auth_token)
{

    esp_http_client_handle_t firestore_client_handle;
    esp_http_client_config_t http_config = {
        .host = FIRESTORE_HOSTNAME,
        .path = full_path,
        .query = queries,
        .event_handler = firestore_http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = RECEIVE_BUF_SIZE,
        .buffer_size_tx = SEND_BUF_SIZE,
        .user_data = RECEIVE_BODY // note that this is the response body buffer
    };

    if (http_method == HTTP_METHOD_POST || http_method == HTTP_METHOD_PATCH)
    {
        if (http_body == NULL)
        {
            ESP_LOGE(TAG, "HTTP method %d requires `http_body`", http_method);
            return ESP_FAIL;
        }
    }
    ESP_LOGI(TAG, "HTTP path: %s", http_config.path);
    ESP_LOGI(TAG, "HTTP query: %s", http_config.query);
    firestore_client_handle = esp_http_client_init(&http_config);
    ESP_LOGI(TAG, "http config initialized");

    if (http_body != NULL)
    {
        esp_http_client_set_header(firestore_client_handle, "Content-Type", "application/json");
        esp_http_client_set_post_field(firestore_client_handle, http_body, strlen(http_body));
    }
    esp_http_client_set_method(firestore_client_handle, http_method);

    if (auth_token != NULL)
    {
        char auth_token_with_bearer[strlen(auth_token) + 8];
        snprintf(auth_token_with_bearer, strlen(auth_token) + 8, "Bearer %s", auth_token);
        esp_http_client_set_header(firestore_client_handle, "Authorization", auth_token_with_bearer);
    }
    ESP_LOGI(TAG, "http headers set up! Making request...");
    if (esp_http_client_perform(firestore_client_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to perform HTTP request");
        esp_http_client_cleanup(firestore_client_handle);
        receive_body_len = 0; // reset the receive body length

        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "HTTP request performed");
    int response_code = esp_http_client_get_status_code(firestore_client_handle);
    ESP_LOGI(TAG,
             "HTTP Response code: %d, content_length: %d",
             response_code,
             (int)esp_http_client_get_content_length(firestore_client_handle));

    // get the response body
    int receive_http_body_size = strlen(RECEIVE_BODY);

    if (response_code != 200)
    {
        ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", response_code);
        if (receive_http_body_size > 0)
        {
            ESP_LOGE(TAG, "Error message: %s", RECEIVE_BODY);
        }
        esp_http_client_cleanup(firestore_client_handle);
        receive_body_len = 0; // reset the receive body length

        return ESP_FAIL;
    }

    esp_http_client_cleanup(firestore_client_handle);
    receive_body_len = 0; // reset the receive body length
    ESP_LOGI(TAG, "HTTP request cleanup");
    return ESP_OK;
}

/**
 * @brief Check if the path is a collection path
 */
bool is_collection_path(char *firebase_path)
{
    int num_slashes = 0;
    for (int i = 0; i < strlen(firebase_path); i++)
    {
        if (firebase_path[i] == '/')
        {
            num_slashes++;
        }
    }
    return num_slashes % 2 == 0;
}

esp_err_t firestore_createDocument(char *firebase_path_to_collection, char *document_name, char *data, char *token)
{
    if (!is_collection_path(firebase_path_to_collection))
    {
        ESP_LOGE(TAG, "Invalid path to collection. The path %s is a document path", firebase_path_to_collection);
        return ESP_FAIL;
    }

    esp_err_t result = ESP_OK;
    // the query is "documentID=document_name"
    int query_size = strlen("documentId=") + strlen(document_name) + 1;
    char query[query_size];
    snprintf(query, query_size, "documentId=%s", document_name);

    int full_path_size = BASE_PATH_FORMAT_SIZE + strlen(firebase_path_to_collection) + 1;
    char full_path[full_path_size];
    snprintf(full_path, full_path_size, FIRESTORE_BASE_PATH_FORMAT, firebase_path_to_collection);

    result = make_abstract_firestore_api_request(full_path, query, HTTP_METHOD_POST, data, token);
    ESP_LOGI(TAG, "Firestore patch request done");
    return result;
}

/**
 * @brief Extracts the keys from a json string that contains "fields" key
 *
 * @param[in] json the json string e.g.
 * {"fields": { "Oct21": {"integerValue": "100"},
 *              "Oct22": {"integerValue": "100"}}}
 *
 * @param[out] keys the array of keys extracted from the json string.
 * e.g. keys[0] = "Oct21", keys[1] = "Oct22"
 * @param[out] num_keys the number of keys extracted from the json string
 */
void extract_keys_from_fields(cJSON *json_object, char **keys, int *num_keys)
{
    cJSON *fields = cJSON_GetObjectItem(json_object, "fields");
    cJSON *field = NULL;
    int i = 0;
    cJSON_ArrayForEach(field, fields)
    {
        if (i >= MAX_PATCH_UPSERT_FIELDS)
        {
            ESP_LOGW(TAG, "The number of fields in the json string exceeds the maximum number of fields allowed");
            break;
        }
        keys[i] = field->string;
        i++;
    }
    *num_keys = i;
}

/**
 * @brief get the mask and update mask string for patch request url query, for upsert method
 * It will update the PATCH_UPSERT_QUERY_BUFFER static variable
 *
 * @param[in] json_data the json string that contains the fields to be updated
 * e.g. "{\"fields\": { \"Oct21\": {\"integerValue\": \"100\"}, \"Oct22\": {\"integerValue\": \"100\"}}"
 * @return void* the update mask string to be used in the patch request url
 * e.g. "updateMask.fieldPaths=Oct21&updateMask.fieldPaths=Oct22"
 */
void get_query_for_upsert(char *json_data)
{
    char *keys[MAX_PATCH_UPSERT_FIELDS];
    int num_keys = 0;

    cJSON *json_object = cJSON_Parse(json_data);
    extract_keys_from_fields(json_object, keys, &num_keys);

    // put FIRESTORE_DUMMY_RETURN_MASK  in the beginning of the PATCH_UPSERT_QUERY_BUFFER
    strcpy(PATCH_UPSERT_QUERY_BUFFER, FIRESTORE_DUMMY_RETURN_MASK);

    int offset = strlen(FIRESTORE_DUMMY_RETURN_MASK);
    for (int i = 0; i < num_keys; i++)
    {
        offset += snprintf(PATCH_UPSERT_QUERY_BUFFER + offset, MAX_PATCH_UPDATE_MASK_BUFFER - offset, "&updateMask.fieldPaths=%s", keys[i]);
    }
    PATCH_UPSERT_QUERY_BUFFER[offset] = '\0'; // add null terminator
    cJSON_Delete(json_object);    
}

esp_err_t firestore_patch(char *firebase_path, char *data, char *token, firestore_patch_type_t patch_type)
{
    if (is_collection_path(firebase_path))
    {
        ESP_LOGE(TAG, "Invalid path to document. The path %s is a collection path", firebase_path);
        return ESP_FAIL;
    }
    esp_err_t result = ESP_OK;
    int full_path_size = BASE_PATH_FORMAT_SIZE + strlen(firebase_path) + 1;
    char full_path[full_path_size];
    snprintf(full_path, full_path_size, FIRESTORE_BASE_PATH_FORMAT, firebase_path);

    // the following query parameters prevent whole document from being returned

    int update_mask_string_size = 0;
    if (patch_type == FIRESTORE_DOC_UPSERT)
    {
        get_query_for_upsert(data); // This will update the PATCH_UPSERT_QUERY_BUFFER static variable
        result = make_abstract_firestore_api_request(full_path, PATCH_UPSERT_QUERY_BUFFER, HTTP_METHOD_PATCH, data, token);
    }
    else // patch_type == OVERWRITE
    {
        result = make_abstract_firestore_api_request(full_path, FIRESTORE_DUMMY_RETURN_MASK, HTTP_METHOD_PATCH, data, token);
    }

    ESP_LOGI(TAG, "Firestore patch request done");
    return result;
}

/**
 * @brief Extracts a field value from a json string that returned from Firestore API
 * For example "{\"fields\": { \"YOUR_FAVORITE_KEY\": {\"integerValue\": \"1000\"}}}";
 * The value of the field "YOUR_FAVORITE_KEY" ("1000") will be extracted and stored in the `value` buffer
 *
 * @param[in] json The json string returned from Firestore API
 * @param[in] field The field to extract the value from
 * @param[out] value The buffer to store the value of the field
 */
esp_err_t extract_a_field_value_from_firestore_response(char *json, char *field, char *value)
{
    cJSON *root = cJSON_Parse(json);
    cJSON *fields = cJSON_GetObjectItem(root, "fields");
    cJSON *field_json_object = cJSON_GetObjectItem(fields, field); // e.g. {\"integerValue\": \"1000\"}
    if (field_json_object == NULL)
    {
        ESP_LOGE(TAG, "Field %s not found in the json string", field);
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    //  extract the first value of the object
    cJSON *type_key = NULL;
    cJSON_ArrayForEach(type_key, field_json_object)
    {
        strcpy(value, type_key->valuestring);
    }
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t firestore_get_a_field_value(char *firebase_path_to_document, char *field, char *token, char *value)
{
    esp_err_t result = ESP_OK;

    // ensure that the path is a document path
    if (is_collection_path(firebase_path_to_document))
    {
        ESP_LOGE(TAG, "Invalid path to document. The path %s is a collection path", firebase_path_to_document);
        return ESP_FAIL;
    }

    int full_path_size = BASE_PATH_FORMAT_SIZE + strlen(firebase_path_to_document) + 2;
    char full_path[full_path_size];
    snprintf(full_path, full_path_size, FIRESTORE_BASE_PATH_FORMAT, firebase_path_to_document);

    // use mask.fieldPaths=field to get only the field value
    int query_size = strlen("mask.fieldPaths=") + strlen(field) + 2;
    char query[query_size];
    snprintf(query, query_size, "mask.fieldPaths=%s", field);
    ESP_LOGI(TAG, "query: %s", query);

    result = make_abstract_firestore_api_request(full_path, query, HTTP_METHOD_GET, NULL, token); // the response body will be stored in RECEIVE_BODY until firestore_utils_cleanup is called
    /**
     * A typical receive_http_body, for example, could be
     * "{\"fields\": { \"Sep30\": {\"integerValue\": \"1000\"}}}";
     */
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get the response from Firestore API about field %s", field);
        ESP_LOGE(TAG, "The response body: %s", RECEIVE_BODY);

        return ESP_FAIL;
    }

    result = extract_a_field_value_from_firestore_response(RECEIVE_BODY, field, value);
    return result;
}

/**
 * @brief HTTP event handler for Firestore API
 *
 * TODO: this is exactly copy-pasted from firebase_auth.cc, I can't find a way to make this a shared function
 * (because there is a static variable `receive_body_len` that is used in the function)
 */
static esp_err_t firestore_http_event_handler(esp_http_client_event_t *client_event)
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