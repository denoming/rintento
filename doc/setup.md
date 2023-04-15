# Setup

Configuration file represents a JSON file.
The path to this file should be specified by `JARVIS_EXECUTOR_CONFIG` environment variable.

## Description

| Param               | Description                                  |
|---------------------|----------------------------------------------|
| *.proxy.port        | The proxy (server) TCP port number to run on |
| *.proxy.threads     | The proxy (server) threads number            |
| *.recognize.host    | The backend host address                     |
| *.recognize.port    | The backend host port                        |
| *.recognize.auth    | The backend authentication token             |
| *.recognize.threads | The backend threads number                   |

## Example

```json
{
  "proxy": {
    "port": 8000,
    "threads": 2
  },
  "recognize": {
    "host": "api.wit.ai",
    "port": "https",
    "auth": "Bearer ...",
    "threads": 4
  }
}
```
