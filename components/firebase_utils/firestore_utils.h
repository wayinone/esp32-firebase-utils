#ifndef FIREBASE_AUTH_H_
#define FIREBASE_AUTH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "esp_err.h"
#include <stdbool.h>


/**
 * @brief get content from Firestore
 * 
 * @param[in] firebase_path The path to the document in Firestore.
 * e.g. "col1/doc1/subcol1", "col1/doc1", or "col1"
 * @param[out] content The content of the document.
 * TODO: should `content` be a pointer to a pointer (i.e. char **content)?
 */
esp_err_t firestore_get(char *firebase_path, char *content);

/**
 * @brief Create a document in Firestore
 * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/createDocument#query-parameters
 *
 * @param[in] firebase_path_to_collection The path to the collection in Firestore.
 * e.g. "col1", or "col1/doc1/subcol1"
 * @param[in] document_name The name of the document to create.
 * @param[in] http_body The body of the HTTP request.
 * e.g. "{\"fields\": {\"name\": {\"stringValue\": \"John\"}}}"
 */
esp_err_t firestore_createDocument(char *firebase_path_to_collection, char *document_name, char *http_body);

esp_err_t firestore_patch(char *firebase_path, char *http_body);


#ifdef __cplusplus
}
#endif

#endif /* FIREBASE_AUTH_H_ */