# API

## HTTP API

The service API represented as a simple HTTP API.

HTTP request target value contains:
* `/message` - sending intent by text message:
```
curl -v "http://localhost:8080/message?q=turn+off+the+light" \
-H "Accept: application/json"
```
Where `q` URL option contains the message and has particular encoding.

* `/speech` - sending intent by human speech:

Sending speech data should be done by sending a bunch of chunks (see RFC 9112 - HTTP/1.1).
The example is presented in CLI tool implementation. 