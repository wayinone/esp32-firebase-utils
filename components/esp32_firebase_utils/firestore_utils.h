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

    // create an enum for patch type: OVERWRITE_ENTIRE_DOCUMENT, UPSET_DOCUMENT
    typedef enum
    {
        FIRESTORE_DOC_OVERWRITE, // this will overwrite the entire document
        FIRESTORE_DOC_UPSERT     // this will update the keys if they exist, and insert them if they do not exist (the use of `updateMask`)
    } firestore_patch_type_t;

    /**
     * @brief Create a document in Firestore
     * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/createDocument#query-parameters
     * Note that if the document already exists, it will throw an error.
     *
     * @param[in] path_to_collection The path to the collection in Firestore.
     * e.g. "col1", or "col1/doc1/subcol1"
     * @param[in] document_name The name of the document to create.
     * @param[in] data The body of the HTTP request. i.e. the content of the document to be sent. *
     * e.g. "{\"fields\": {\"name\": {\"stringValue\": \"John\"}}}" Note that this needs to conform to the format required by the Firestore API.
     * See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * @param[in] token The token to authenticate the request.
     */
    esp_err_t firestore_createDocument(char *path_to_collection, char *document_name, char *data, char *token);

    /**
     * @brief Patch (update or inserts) a document in Firestore
     * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/patch
     * If the document does not exist, it will be created.
     *
     * @param[in] path_to_document The path to the document in Firestore.
     * e.g. "col1/doc1" or "col1/doc1/subcol2/doc2"
     * @param[in] data The body of the HTTP request. i.e. the content of the document to be sent.
     * e.g."{\"fields\": { \"Oct23\": {\"integerValue\": \"500\"}, \"Oct22\": {\"integerValue\": \"500\"}}}"
     * Note that this needs to conform to the format required by the Firestore API.
     * See https://firebase.google.com/docs/firestore/reference/rest/v1/projects.databases.documents#Document
     * Note that in this example, with UPSERT mode, the fields "Oct23" and "Oct22" will be updated (or inserted if not exists) in the document.
     * However, with OVERWRITE mode, the entire document will be replaced with the new data.
     * @param[in] token The token to authenticate the request.
     * @param[in] patch_type The type of patch to be done.
     */
    esp_err_t firestore_patch(char *path_to_document, char *data, char *token, firestore_patch_type_t patch_type);

    /**
     * @brief Get a field value from a document in Firestore
     * https://firebase.google.com/docs/firestore/reference/rest/v1beta1/projects.databases.documents/get
     * Here, we use the query parameter `mask.fieldPaths=field` to get only the field value.
     * e.g. The following will obtain `val` from the value of field `YOUR_FAVORITE_FIELD` in the document `col1/doc1`
     * char *val; 
     * firestore_get_a_field_value("col1/doc1", "YOUR_FAVORITE_FIELD", NULL, val);
     * 
     * @param[in] path_to_document The path to the document in Firestore.
     * e.g. "col1/doc1" or "col1/doc1/subcol2/doc2"
     * @param[in] field The field to get the value from.
     * @param[in] token The token to authenticate the request.
     * @param[out] value The value of the field.
     * @return esp_err_t
     */
    esp_err_t firestore_get_a_field_value(char *path_to_document, char *field, char *token, char *value);


#ifdef __cplusplus
}
#endif

#endif /* FIRESTORE_UTILS_H_ */