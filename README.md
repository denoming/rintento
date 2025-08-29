# Rintento

[![CI](https://github.com/denoming/rintento/actions/workflows/build.yaml/badge.svg)](https://github.com/denoming/rintento/actions/workflows/build.yaml)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Introduction

The service is intended for running an action upon client intent. Recognizing intent is made by configured backend.
Upon client request service initiates recognizing and running previously configured actions.

![](asset/misc/overview.png)

The main purpose of the service is running an action upon client requests.
Where each client request represented by:
* text message (e.g. "turn on the light")
* human speech in particular audio format

After recognizing intent the service perform matching obtained intents and previously configured action.
After matching procedure the service runs particular actions.

For more information please visit [documentation file](doc%2Findex.md).

## Building

### Locally

Debug configuration:
```shell
$ cmake --preset debug
$ cmake --build --preset build-debug
```
Release configuration:
```shell
$ cmake --preset release
$ cmake --build --preset build-release
```

### Inside docker

Debug configuration:
```shell
$ bash scripts/run-test-env.sh
$ cmake --preset debug-docker
$ cmake --build --preset build-debug-docker
```
Release configuration:
```shell
$ bash scripts/run-test-env.sh
$ cmake --preset release-docker
$ cmake --build --preset build-release-docker
```

## Testing

Prerequisites:
* application at [wit.ai](https://wit.ai/apps) service is present (`asset/wit.ai` dir contains config and exported data); 
* test preset in `CMakeUserPresets.json` file at the root of the project.

### Component tests

Custom test preset template for component tests:
```
{
  "version": 6,
  "testPresets": [
    {
      "name": "my-component-tests",
      "inherits": ["component-tests"],
      "environment": {
        "RINTENTO_WIT_AUTH": "PUT-YOUR-AUTH-TOKEN-HERE"
      }
    }
  ]
}
```

Running component tests:
```shell
$ bash scripts/run-test-env.sh
$ cmake --preset debug-docker
$ cmake --build --preset build-debug-docker
$ ctest --preset "my-component-tests"
```

## License

See the [LICENSE](LICENSE.md) file for license rights and limitations (MIT).