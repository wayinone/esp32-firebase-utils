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

