#ifndef FIREBASE_AUTH_H_
#define FIREBASE_AUTH_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

#define FIREBASE_REFRESH_TOKEN CONFIG_FIREBASE_REFRESH_TOKEN
#define FIREBASE_API_KEY CONFIG_FIREBASE_API_KEY

/**
 * @brief Exchange a refresh token for an ID token
 * https://cloud.google.com/identity-platform/docs/use-rest-api#section-refresh-token
 *
 * @param[in] refresh_token The refresh token to be used to get the access token.
 * @param[out] access_token The access token to be used in the Firebase API requests. Note that GCP a token usually
 * has 758 characters. So, we usually initialize this as `char access_token[1024]`.
 */
esp_err_t firebase_get_access_token_from_refresh_token(char *refresh_token, char *access_token);

#ifdef __cplusplus
}
#endif

#endif /* FIREBASE_AUTH_H_ */