# GCP Auth

Provides help command `gcp-sa-utils` for generate refresh token from a private key download from a service account.


## Installation

Run 
```
cd gcp_auth
. script/create_venv.py
. environment/bin/activate
```

## Commend

For the cli help, you can run

```cmd
gcp-sa-utils --help
```

To use the get refresh key function, see

```cmd
gcp-sa-utils get_refresh_token --help
```

## Acquire a private key to get refresh token

 1. Start by creating a service account under the GCP project (that is linked to the Firebase project). The service account should have these roles:
    1. `Service Account OpenID Connect Identity Token Creator` , and
    2. `Service Account Token Creator`.
 2. Then, you can download a JSON private key from the service account.
