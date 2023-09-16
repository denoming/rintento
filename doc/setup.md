# Setup

Configuration file represents a simple JSON file.
The path to this file should is specified by `RINTENTO_EXECUTOR_CONFIG` environment variable.

## Description

| Param                   | Description                                        |
|-------------------------|----------------------------------------------------|
| server.port             | The server TCP port number                         |
| server.threads          | The server threads number                          |
| recognition.server.host | The backend host address                           |
| recognition.server.port | The backend host port                              |
| recognition.server.auth | The backend authentication token                   |
| automations             | The pre-configured actions with associated intents |

## Example

```json
{
  "server": {
    "port": 8080,
    "threads": 8
  },
  "recognition": {
    "server": {
      "host": "api.wit.ai",
      "port": "https",
      "auth": "<token>"
    }
  },
  "automations": [
    {
      "alias": "Turn on the light",
      "intent": "light_on",
      "actions": [
        {
          "type": "script",
          "exec": "/path/to/program",
          "args": [
            "-arg1",
            "-arg2",
            "value"
          ],
          "home": "/path/to/home/dir",
          "env": {
            "ENV3": "VAR3",
            "ENV4": "VAR4"
          },
          "inheritParentEnv": false,
          "timeout": 3000
        }
      ]
    }
  ]
}
```
