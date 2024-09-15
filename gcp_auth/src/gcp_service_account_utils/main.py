"""
command line tool for google serive account

From the private key files downloaded from the google cloud console, under a service account
, we can get the resfresh token for the service account.

"""
import firebase_admin
from firebase_admin import credentials, auth
import click
import urllib3
import json


@click.group()
def cli():
    """
    google-sa-utils is a command line tool for the google service account.
    """
    pass

@cli.command()
@click.option(
    "--private-key-json", "-j",
    type=click.Path(exists=True, resolve_path=True),
    help="The path to the service account public key json file",
)
@click.option(
    "--device-id", "-d",
    type=str,
    help="This is the sub (subject) in the JWT token, usually this is unique"
     "identifier for the device, user or service account",
)
@click.option(
    "--api-key", "-k",
    type=str,
    help="The api key for the firebase project",
)
def get_refresh_token(private_key_json: str, device_id: str, api_key: str) -> str:
    """
    This function gets the custom token for the service account.
    """
    
    custom_token = get_custom_token_from_private_key_json(private_key_json, device_id)
    refresh_token = get_refresh_token_from_custom_token(custom_token, api_key)
    return refresh_token

def get_private_key_id(private_key_json: str) -> str:
    """
    This function gets the private key id from the private key json file.
    """
    with open(private_key_json, 'r') as f:
        private_key_json = json.load(f)
    return private_key_json['private_key_id']

def get_custom_token_from_private_key_json(private_key_json: str, device_id: str) -> str:
    """
    This function gets the custom token from the private key json file.
    """
    cred = credentials.Certificate(private_key_json)

    if firebase_admin._apps.get('[DEFAULT]', None):
        default_app = firebase_admin._apps['[DEFAULT]']
        firebase_admin.delete_app(default_app)

    firebase_admin.initialize_app(credential=cred)  
    custom_token = auth.create_custom_token(device_id)
    # check if this succeeds
    if not custom_token:
        raise ValueError("Error:Custom token creation failed")
    return custom_token.decode('utf-8')

def get_refresh_token_from_custom_token(custom_token: str, api_key: str) -> str:
    """ 
    This function gets the refresh token from the custom token. To do this in curl:
    ```
    curl 'https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key=[API_KEY]' \
    -H 'Content-Type: application/json' \
    --data-binary '{"token":"[CUSTOM_TOKEN]","returnSecureToken":true}'
    ```
    """

    http = urllib3.PoolManager()
    url = f'https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key={api_key}'
    data = json.dumps(
        {"token": custom_token,
        "returnSecureToken": "true"})
    headers = {'Content-Type': 'application/json', 'Content-Length': len(data)}
    response = http.request(
        method='POST', 
        url=url, 
        body=data, 
        headers=headers
    )
    dict_response = json.loads(response.data)
    # check if this succeeds
    if 'refreshToken' not in dict_response:
        print(dict_response)
        return None

    return dict_response['refreshToken']


"""
curl -X POST -H "Content-Type: application/json" \
-d '{"payload": { \"iss\": \"firebase-adminsdk-mllr2@meter-monitor-a2d29.iam.gserviceaccount.com\", \"sub\": \"firebase-adminsdk-mllr2@meter-monitor-a2d29.iam.gserviceaccount.com\", \"aud\": \"https://firestore.googleapis.com/\", \"iat\": 1529350000, \"exp\": 1800 }
}' \
POST https://iamcredentials.googleapis.com/v1/projects/meter-monitor-a2d29/serviceAccounts/firebase-adminsdk-mllr2@meter-monitor-a2d29.iam.gserviceaccount.com:signJwt

"""

""" custom token by user credentials with ADC with default on a service account
curl -X POST \
-H 'Content-Type: application/json' \
--data-binary '{"token":"eyJhbGciOiAiUlMyNTYiLCAidHlwIjogIkpXVCJ9.eyJpc3MiOiAiZmlyZWJhc2UtYWRtaW5zZGstbWxscjJAbWV0ZXItbW9uaXRvci1hMmQyOS5pYW0uZ3NlcnZpY2VhY2NvdW50LmNvbSIsICJzdWIiOiAiZmlyZWJhc2UtYWRtaW5zZGstbWxscjJAbWV0ZXItbW9uaXRvci1hMmQyOS5pYW0uZ3NlcnZpY2VhY2NvdW50LmNvbSIsICJhdWQiOiAiaHR0cHM6Ly9pZGVudGl0eXRvb2xraXQuZ29vZ2xlYXBpcy5jb20vZ29vZ2xlLmlkZW50aXR5LmlkZW50aXR5dG9vbGtpdC52MS5JZGVudGl0eVRvb2xraXQiLCAidWlkIjogImRldmljZS0xIiwgImlhdCI6IDE3MjYxNDk3NjYsICJleHAiOiAxNzI2MTUzMzY2fQ.jBRMIv8xCZ71pYPUHpyM8R6NsAuVETPYq6IhOBsKQR6MjwKrDkxug5AqcKIpQF9uKwFitwITspsAye5N0jlfDRs8jnWaLud6iV8S8W41dW2GyjLApahfB9TtkIxLYeQYUiWVjevmlzIiyDGtbotf8vh8F700HO2SPJqqpT-ASJeLzShaFwq0sQTe6pGC0A0jOlslzyOUrZdN9B4hNrggktAgoCR34MtkmxkRJTbFIsC1W0SEQ-xQQccJ6oNzm4_uyXV3PjB6y_cW_1X4EwqrfTssSo5MZrrpFP4KZuoxfuwrkY2M4Y_AHONJ2yq6H5uIkPnjAkDiTlyQF74pja3zfg","returnSecureToken":true}' \
'https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'
"""

""" custom token by service account credentials
curl -X POST \
-H 'Content-Type: application/json' \
--data-binary '{"token":"eyJhbGciOiAiUlMyNTYiLCAidHlwIjogIkpXVCIsICJraWQiOiAiMWE2MTRlMDZmZTUxMzAyZGRhZTgxNmEwMTU0NGVmNmVhMTE1MDQzNiJ9.eyJpc3MiOiAiZmlyZWJhc2UtYWRtaW5zZGstbWxscjJAbWV0ZXItbW9uaXRvci1hMmQyOS5pYW0uZ3NlcnZpY2VhY2NvdW50LmNvbSIsICJzdWIiOiAiZmlyZWJhc2UtYWRtaW5zZGstbWxscjJAbWV0ZXItbW9uaXRvci1hMmQyOS5pYW0uZ3NlcnZpY2VhY2NvdW50LmNvbSIsICJhdWQiOiAiaHR0cHM6Ly9pZGVudGl0eXRvb2xraXQuZ29vZ2xlYXBpcy5jb20vZ29vZ2xlLmlkZW50aXR5LmlkZW50aXR5dG9vbGtpdC52MS5JZGVudGl0eVRvb2xraXQiLCAidWlkIjogImRldmljZS0yIiwgImlhdCI6IDE3MjYxNTIwNzcsICJleHAiOiAxNzI2MTU1Njc3fQ.dmykahXdouX_ocGbPdV9PsYG-9eAwBne11NRhj_H5MDfoxIhMmrQbMRSco3tfpURcTaXTF7d2bBKUa_X5CQWt3CHlywnCLw516tIXloHhYca-ZTKWmH3gFJdtCgU-oqUvSkAcaZ1fWDK88fnBGD5NZ5SkTlIoOTMSxV_6PdE63tJ4Vi98fIDLt0z4B41r1-TCatsXpOFra9uy5fvMNz1kSVEytaa1uG2TMVVzBS8pc2HsNcdKYBV9ePCadLo6TeQ_27oOpxmM2O-qBTTE15T4lWAQa6NQstYz5und8J-4n7rzHrdnsj-19pJ9HK-uDoUeb9s4IJjMre0fl7VJbbDFg","returnSecureToken":true}' \
'https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'
"""

""" custom token by another service account credentials 
curl -X POST \
-H 'Content-Type: application/json' \
--data-binary '{"token":"eyJhbGciOiAiUlMyNTYiLCAidHlwIjogIkpXVCIsICJraWQiOiAiYWQ2YTNhNDUzODhiMTdiMjEzMmY0OGEyZjYwYjk4NjZjN2IzNWFjMCJ9.eyJpc3MiOiAiZmlyZXN0b3JlLXNlcnZpY2UtdG9rZW4tY3JlYXRlQG1ldGVyLW1vbml0b3ItYTJkMjkuaWFtLmdzZXJ2aWNlYWNjb3VudC5jb20iLCAic3ViIjogImZpcmVzdG9yZS1zZXJ2aWNlLXRva2VuLWNyZWF0ZUBtZXRlci1tb25pdG9yLWEyZDI5LmlhbS5nc2VydmljZWFjY291bnQuY29tIiwgImF1ZCI6ICJodHRwczovL2lkZW50aXR5dG9vbGtpdC5nb29nbGVhcGlzLmNvbS9nb29nbGUuaWRlbnRpdHkuaWRlbnRpdHl0b29sa2l0LnYxLklkZW50aXR5VG9vbGtpdCIsICJ1aWQiOiAiZGV2aWNlLTMiLCAiaWF0IjogMTcyNjE1MzM3NiwgImV4cCI6IDE3MjYxNTY5NzZ9.kZFFCSstlYWyGZDjNeViNzLd4laxo7Y2ogoBXRIK-jN9zMLznXZ7IQT23EWCVT3bzhTm0MCVBTBe7r96TIDK8iZ6fEbu349BCbJQGgQj-MZ87stQ1NC6V5KwpKC-u6Jw6kb_NTFi-IPaZze_QkN_rdsJMNOt_k42iUirruncdBgV4PQKKnKNIq-9dzZNeJuReEmr-ME1MK1LaWsjp25rFHQxWiWW-WWitrk3PoRw4csPUWfRomchd7iHU0K02RdOxUaH-5UCgRSJALRIYDBitkocaRuJ5F8TwpF74Yc_2LrqbyoNcPFu73R3QFMfO007jCu0LXAqzlowlYWIPTVa-A","returnSecureToken":true}' \
'https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'

"""
"""
curl -X POST \
-H 'Content-Type: application/json' \
--data-binary '{"token":"eyJhbGciOiAiUlMyNTYiLCAidHlwIjogIkpXVCIsICJraWQiOiAiYjU3YzMzZGIzNzllNDNlMWUwNDgwYmUyZDE1ZGE3MDA3OGJkZTcwOSJ9.eyJpc3MiOiAiZmlyZXN0b3JlLXNlcnZpY2UtdG9rZW4tY3JlYXRlQG1ldGVyLW1vbml0b3ItYTJkMjkuaWFtLmdzZXJ2aWNlYWNjb3VudC5jb20iLCAic3ViIjogImZpcmVzdG9yZS1zZXJ2aWNlLXRva2VuLWNyZWF0ZUBtZXRlci1tb25pdG9yLWEyZDI5LmlhbS5nc2VydmljZWFjY291bnQuY29tIiwgImF1ZCI6ICJodHRwczovL2lkZW50aXR5dG9vbGtpdC5nb29nbGVhcGlzLmNvbS9nb29nbGUuaWRlbnRpdHkuaWRlbnRpdHl0b29sa2l0LnYxLklkZW50aXR5VG9vbGtpdCIsICJ1aWQiOiAiZGV2aWNlLTQiLCAiaWF0IjogMTcyNjE1ODI5MywgImV4cCI6IDE3MjYxNjE4OTN9.JE6N0XfvxvwqVtfZSdx8tUZOlZdKLIqI5Wci8RZHSSD6rgarAEKjoEfOX01D0I3GH_u6XX7mcyIpzXjfYzv5q3t50Gg7DsRJSXe0ZnUAbb_poJuSX2H8PDekXK1gClU8LzlHzXbGVar0c_qjdk9hWcam7LQQ4Q6fiLL2OXKCTv5oji_X9X11XxnfNhbJgeBx82E2aPK2TBg5rQFhabFI2v_RN4K9Gkuk99E6HjFIOFXI8iPgFXdyiP93hCz1XIBXTWm6wC1xl6IR2_nxTNS32f6g3N_FekDwC5I7Ot_IqmTlepGDOumQKak_DqZ3YXKqFpLxDsv0SjWnMfEkPsQI9w","returnSecureToken":true}' \
'https://identitytoolkit.googleapis.com/v1/accounts:signInWithCustomToken?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'
"""

"""
curl -X POST \
-H 'Content-Type: application/x-www-form-urlencoded' \
--data 'grant_type=refresh_token&refresh_token=AMf-vBy0hq44kCqGEr9eKqaypbsDRAprd1-xDZM84mmGzBamH97J8qhJ_pqV6zvpDu5LIU8UkV1WFkPDLdtkvkze035g-8e2VjldMo7EKSZo02Zh5EgmkFReaQi-fj0vD5RLIgXhCW_aRTWUr-MC53cwKkL2DzS-xkIhIV2_wlDx9zk4lDzm03Y' \
'https://securetoken.googleapis.com/v1/token?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'
"""

"""refresh token: AMf-vBxKbd-cXjxFrhwy1NvFn2dfAKM2kbTECVtsBhcn_4VNISVM8TpKjM9QUsOnRIZkLbiKbOiysdk4nyJSphSB9wFMA-dIPQFBO81npN5XZwB1tBhp_3L1Fe4bnkg7hRnqkgh3NJL1zdeimFjwJaXjMCPqu4qggXO4-X7k5O6Pxwq8ZRl6Oyc
    curl -X POST \
    -H 'Content-Type: application/x-www-form-urlencoded' \
    --data 'grant_type=refresh_token&refresh_token=AMf-vBxKbd-cXjxFrhwy1NvFn2dfAKM2kbTECVtsBhcn_4VNISVM8TpKjM9QUsOnRIZkLbiKbOiysdk4nyJSphSB9wFMA-dIPQFBO81npN5XZwB1tBhp_3L1Fe4bnkg7hRnqkgh3NJL1zdeimFjwJaXjMCPqu4qggXO4-X7k5O6Pxwq8ZRl6Oyc' \
    'https://securetoken.googleapis.com/v1/token?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'
"""

"""refresh token: AMf-vBy7FDIgIfGzGEZJ5mYY0SHinJtU--GGeOIwOxhQahZDsFavE5Ay4_Kde0ibfOc1MRUOeHw0dxhNDV-itiVSIAQpLr5sTuvoVONHHKbfA4ypT5OJsHN2HH6hWjaTLVNQr6vkEpmq4DNe4W-fqzUgi0Un6l1Qoartn1yWeL82wX-IZbgLjPo
    curl -X POST \
    -H 'Content-Type: application/x-www-form-urlencoded' \
    --data 'grant_type=refresh_token&refresh_token=AMf-vBx0oTKzm8IFfvzJaDrZxTMDzfES9jWp6WUNoRhZg6UJSUqTfOOGqoJTqi3u6Y8NBZ8LXKkUImos5Td7HO4CZM0iq00e9IHoeMJDpWtS8dezgtA_nfS09h34n-0dsMGq95EKHWt96iyta0oyNbYKbGqIHTzoq7yYKmotEoJLBJkFQB9lNkE' \
    'https://securetoken.googleapis.com/v1/token?key=AIzaSyA9vmUgXbyst8TBa9xq91Vo3rfPE0dC5WE'
"""