/**
 * @file firestore_utils.cc
 * @brief Firestore utility functions based on
 * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/
 */

#include "firebase_common.h" // import firestore_http_event_handler
#include "firestore_utils.h"
#include <string.h>
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "FIRESTORE";

static const int MAX_PATCH_FIELDS = 10;

#define FIRESTORE_HOSTNAME "firestore.googleapis.com"

/**
 * The following are defined by embedded data in CMakeLists.txt
 * and are used to verify the server's certificate.
 * One can download the certificate from:
 *  Firefox -> put website `FIRESTORE_HOSTNAME` -> click on the lock icon -> More Information -> View Certificate -> Details -> Export
 */
extern const char firestore_api_pem_start[] asm("_binary_googleapis_com_chain_pem_start");
extern const char firestore_api_pem_end[] asm("_binary_googleapis_com_chain_pem_end");

static const int HTTP_PATH_SIZE = 256;

static const char BASE_PATH_FORMAT[] = "/v1/projects/" FIREBASE_PROJECT_ID "/" FIRESTORE_DB_ROOT "/%s";
static int BASE_PATH_FORMAT_SIZE = sizeof(BASE_PATH_FORMAT);

static const int SEND_BUF_SIZE = 2048; // this is also called transmit (tx) buffer size

/**
 * @brief Get the path for Firestore REST API
 *
 * @param[in] firestore_path The path of your collection and documents.
 * e.g. "col1/doc1", "col1", "col1/doc1/subcol1", or "col1/subcol1/doc1"
 * @param[out] full_path The full path to be used in the HTTP request
 */
esp_err_t get_path(char *firestore_path, char *full_path)
{
    int s32Length = snprintf(full_path, HTTP_PATH_SIZE, BASE_PATH_FORMAT, firestore_path);

    ESP_LOGD(TAG, "(get_path) Full path: %s", full_path);

    if (s32Length > 0)
    {
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to get path, Maybe the path is too long, Consider increasing HTTP_PATH_SIZE");
        return ESP_FAIL;
    }
}

/**
 * @brief Make an abstract API request to API
 *
 * @param[in] full_path The path of your collection and documents.
 * e.g. "col1/doc1", "col1", "col1/doc1/subcol1", or "col1/subcol1/doc1"
 * @param[in] http_method The HTTP method to use. e.g. HTTP_METHOD_GET, HTTP_METHOD_POST, HTTP_METHOD_PATCH, HTTP_METHOD_DELETE
 * @param[in] http_body The body of the HTTP request. e.g. "{\"fields\": {\"name\": {\"stringValue\": \"John\"}}}"
 * if the request does not require a body, pass NULL
 * @param[in] auth_token The auth token to be used in the HTTP request. If the request does not require an auth token, pass NULL
 * @param[out] receive_http_body The body of the HTTP response (could be an error message, or returned json data)
 */
esp_err_t make_abstract_firestore_api_request(
    char *full_path,
    esp_http_client_method_t http_method,
    char *http_body,
    char *auth_token,
    char *receive_http_body)
{
    esp_http_client_handle_t firestore_client_handle;
    esp_http_client_config_t http_config = {
        .host = FIRESTORE_HOSTNAME,
        .path = full_path,
        .cert_pem = firestore_api_pem_start, // the certificate of the server (can be downloaded from the browser)
        .event_handler = firestore_http_event_handler,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .buffer_size = RECEIVE_BUF_SIZE,
        .buffer_size_tx = SEND_BUF_SIZE,
        .user_data = RECEIVE_BODY}; // note that RECEIVE_BODY should be initialized when firebase_auth.cc is called.

    if (http_method == HTTP_METHOD_POST || http_method == HTTP_METHOD_PATCH)
    {
        if (http_body == NULL)
        {
            ESP_LOGE(TAG, "HTTP method %d requires `http_body`", http_method);
            return ESP_FAIL;
        }
    }
    ESP_LOGI(TAG, "HTTP path: %s", http_config.path);
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
        // add 'BEARER ' to the auth_token
        char auth_token_with_bearer[strlen(auth_token) + 8];
        snprintf(auth_token_with_bearer, strlen(auth_token) + 8, "Bearer %s", auth_token);
        esp_http_client_set_header(firestore_client_handle, "Authorization", auth_token_with_bearer);
    }
    ESP_LOGI(TAG, "http headers set up! Making request...");
    if (esp_http_client_perform(firestore_client_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to perform HTTP request");
        esp_http_client_cleanup(firestore_client_handle);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "HTTP request performed");
    int response_code = esp_http_client_get_status_code(firestore_client_handle);
    ESP_LOGD(TAG,
             "HTTP Response code: %d, content_length: %d",
             response_code,
             (int)esp_http_client_get_content_length(firestore_client_handle));

    // get the response body
    int receive_http_body_size = strlen(RECEIVE_BODY);

    if (receive_http_body != NULL)
    {
        strncpy(receive_http_body, RECEIVE_BODY, receive_http_body_size);
    }
    if (response_code != 200)
    {
        ESP_LOGE(TAG, "Firestore REST API call failed with HTTP code: %d", response_code);
        if (receive_http_body_size > 0)
        {
            ESP_LOGE(TAG, "Error message: %s", RECEIVE_BODY);
        }
        esp_http_client_cleanup(firestore_client_handle);
        return ESP_FAIL;
    }
    esp_http_client_cleanup(firestore_client_handle);
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

esp_err_t firestore_get(char *firebase_path, char *token, char *content)
{
    esp_err_t result = ESP_OK;
    int full_path_size = BASE_PATH_FORMAT_SIZE + strlen(firebase_path) + 1;
    char *full_path = (char *)malloc(full_path_size);
    get_path(firebase_path, full_path);

    result = make_abstract_firestore_api_request(full_path, HTTP_METHOD_GET, NULL, token, content);
    free(full_path);
    return result;
}

esp_err_t firestore_createDocument(char *firebase_path_to_collection, char *document_name, char *data, char *token)
{
    if (!is_collection_path(firebase_path_to_collection))
    {
        ESP_LOGE(TAG, "Invalid path to collection. The path %s is a document path", firebase_path_to_collection);
        return ESP_FAIL;
    }

    esp_err_t result = ESP_OK;
    // the firebase_path is 'firebase_path_to_collection' + '?documentId=' + 'document_name'
    int firestore_path_size = strlen(firebase_path_to_collection) + strlen(document_name) + 18;
    char firebase_path[firestore_path_size];
    snprintf(firebase_path, firestore_path_size, "%s?documentId=%s", firebase_path_to_collection, document_name);

    ESP_LOGD(TAG, "firebase_path: %s", firebase_path);

    int full_path_size = BASE_PATH_FORMAT_SIZE + firestore_path_size + 1;
    char *full_path = (char *)malloc(full_path_size);

    get_path(firebase_path, full_path);

    result = make_abstract_firestore_api_request(full_path, HTTP_METHOD_POST, data, token, NULL);
    free(full_path);
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
void extract_keys_from_fields(char *json, char **keys, int *num_keys) {
    cJSON *root = cJSON_Parse(json);
    cJSON *fields = cJSON_GetObjectItem(root, "fields");
    cJSON *field = NULL;
    int i = 0;
    cJSON_ArrayForEach(field, fields) {
        keys[i] = field->string;
        i++;
    }
    *num_keys = i;
}

/**
 * @brief get the update mask string for patch request url query
 * 
 * @param[in] fields the array of fields extracted from the json string, e.g. fields[0] = "Oct21", fields[1] = "Oct22"
 * @param[in] num_fields the number of fields extracted from the json string
 * @param[out] update_mask_string the update mask string to be used in the patch request url
 * e.g. "updateMask.fieldPaths=Oct21&updateMask.fieldPaths=Oct22"
 */
void get_update_mask_string(char **fields, int num_fields, char *update_mask_string){
    int update_mask_string_size = 0;
    for (int i = 0; i < num_fields; i++) {
        update_mask_string_size += strlen(fields[i]) + 25;
    }
    char update_mask[update_mask_string_size];
    int offset = 0;
    for (int i = 0; i < num_fields; i++) {
        offset += snprintf(update_mask + offset, update_mask_string_size - offset, "updateMask.fieldPaths=%s&", fields[i]);
    }
    update_mask[offset - 1] = '\0';
    strcpy(update_mask_string, update_mask);
}


esp_err_t firestore_patch(char *firebase_path, char *data, char *token)
{
    if (is_collection_path(firebase_path))
    {
        ESP_LOGE(TAG, "Invalid path to document. The path %s is a collection path", firebase_path);
        return ESP_FAIL;
    }
    esp_err_t result = ESP_OK;
    int full_path_size = BASE_PATH_FORMAT_SIZE + strlen(firebase_path) + 1;
    char *full_path = (char *)malloc(full_path_size);
    get_path(firebase_path, full_path);

    // get the update mask string
    char *keys[MAX_PATCH_FIELDS];
    int num_keys = 0;
    extract_keys_from_fields(data, keys, &num_keys);

    //calculate the size of the update mask string (so that this request will only update or insert the fields in the json body)
    int update_mask_string_size = 0;
    int update_mask_query_param_size = strlen("updateMask.fieldPaths=") + 2;
    for (int i = 0; i < num_keys; i++) {
        update_mask_string_size += strlen(keys[i]) + update_mask_query_param_size;
    }

    char update_mask_string[update_mask_string_size];
    get_update_mask_string(keys, num_keys, update_mask_string);

    ESP_LOGD(TAG, "Update mask string: %s", update_mask_string);

    // append "?" + update mask string to the full path
    int full_path_size_with_update_mask = full_path_size + strlen(update_mask_string) + 2;
    char *full_path_with_update_mask = (char *)malloc(full_path_size_with_update_mask);
    snprintf(full_path_with_update_mask, full_path_size_with_update_mask, "%s?%s", full_path, update_mask_string);

    result = make_abstract_firestore_api_request(full_path_with_update_mask, HTTP_METHOD_PATCH, data, token, NULL);
    free(full_path);
    free(full_path_with_update_mask);
    return result;
}
