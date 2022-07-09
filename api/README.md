# Wit.AI API

## Message recognition request

```text
curl --http1.1 'https://api.wit.ai/message?v=20220618&q=turn%20off%20the%20light' \
-i -L \
-H "Authorization: Bearer $TOKEN"
```

## Speech recognition request

```text
curl --http1.1 'https://api.wit.ai/speech?v=20220618' \
-i -L \
-H "Authorization: Bearer $TOKEN" \
-H "Transfer-Encoding: chunked" \
-H "Content-Type: audio/raw; encoding=signed-integer; bits=16; rate=16000; endian=little" \
--data-binary "@turn-on-the-light.raw"
```

## Options

* Specify local proxy: `--proxy http://localhost:8080`
* Enable full traceability: `--trace`

# J.A.R.V.I.S API

```text
curl -v "http://localhost:8080/message?q=turn+off+the+light" \
-H "Accept: application/json"
```
