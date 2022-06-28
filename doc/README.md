
# Useful

## Run mock server with delay for each request

```shell
$ docker run --publish 0.0.0.0:8888:8888 alpine nc -lk -p 8888 -e sleep 5s
$ wget 127.0.0.1:8888
--2019-09-27 06:27:22--  http://127.0.0.1:8888/
Connecting to 127.0.0.1:8888... connected.
HTTP request sent, awaiting response... No data received.
Retrying.
...
```
