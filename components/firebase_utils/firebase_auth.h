#ifndef FIREBASE_AUTH_H_
#define FIREBASE_AUTH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

#define FIREBASE_REFRESH_TOKEN CONFIG_FIREBASE_REFRESH_TOKEN
#define FIREBASE_API_KEY CONFIG_FIREBASE_API_KEY

#define FIREBASE_API_KEY_SIZE 64        // original 40
#define FIREBASE_REFRESH_TOKEN_SIZE 200 // original 184

/**
 * @brief Get the path for Firestore REST API
 * https://cloud.google.com/identity-platform/docs/use-rest-api
 *
 * @param[in] refresh_token The refresh token to be used to get the access token.
 * @param[out] access_token The access token to be used in the Firebase API requests.
 */
esp_err_t firebase_get_access_token_from_refresh_token(char *refresh_token, char *access_token);

#ifdef __cplusplus
}
#endif

#endif /* FIREBASE_AUTH_H_ */