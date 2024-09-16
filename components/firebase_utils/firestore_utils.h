#ifndef FIRESTORE_UTILS_H_
#define FIRESTORE_UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "esp_err.h"
#include <stdbool.h>

#define FIRESTORE_DB_ROOT CONFIG_FIRESTORE_DB_ROOT
#define FIREBASE_PROJECT_ID CONFIG_FIREBASE_PROJECT_ID

    /**
     * @brief get content from Firestore
     *
     * @param[in] firebase_path The path to the document in Firestore.
     * e.g. "col1/doc1/subcol1", "col1/doc1", or "col1"
     * @param[in] token The token to authenticate the request.
     * @param[out] content The content of the document.
     * TODO: should `content` be a pointer to a pointer (i.e. char **content)?
     */
    esp_err_t firestore_get(char *firebase_path, char *token, char *content);

    /**
     * @brief Create a document in Firestore
     * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/createDocument#query-parameters
     *
     * @param[in] firebase_path_to_collection The path to the collection in Firestore.
     * e.g. "col1", or "col1/doc1/subcol1"
     * @param[in] document_name The name of the document to create.
     * @param[in] http_body The body of the HTTP request. i.e. the content of the document to be sent. *
     * e.g. "{\"fields\": {\"name\": {\"stringValue\": \"John\"}}}" Note that this needs to conform to the format required by the Firestore API.
     * See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * @param[in] token The token to authenticate the request.
     */
    esp_err_t firestore_createDocument(char *firebase_path_to_collection, char *document_name, char *http_body, char *token);

    /**
     * @brief Patch (update or inserts) a document in Firestore
     * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/patch
     * 
     * @param[in] firebase_path The path to the document in Firestore.
     * e.g. "col1/doc1" or "col1/doc1/subcol2/doc2"
     * @param[in] http_body The body of the HTTP request. i.e. the content of the document to be sent. 
     * e.g. "{\"fields\": {\"name\": {\"stringValue\": \"John\"}}}" Note that this needs to conform to the format required by the Firestore API.
     * See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * @param[in] token The token to authenticate the request.
     */
    esp_err_t firestore_patch(char *firebase_path, char *http_body, char *token);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTORE_UTILS_H_ */