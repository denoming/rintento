#!/usr/bin/env bash

set -e

DOCKER_IMAGE_NAME=my/jarvis-dev-image

FILE_IMAGE_NAME=jarvis-dev-image-qemux86-64.tar.bz2
FILE_IMAGE_PATH=$1/${FILE_IMAGE_NAME}

command -v docker > /dev/null
if [ $? != 0 ]; then
    echo -e "Error:"
    echo -e "Docker is not installed"
    exit 1
fi

if [ -d "${FILE_IMAGE_PATH}" ]; then
    echo -e "Error:"
    echo -e "File image <${FILE_IMAGE_PATH}> is absent"
    exit 1
fi

echo -e "Import <${FILE_IMAGE_NAME}> as <${DOCKER_IMAGE_NAME}> docker image"
docker import - ${DOCKER_IMAGE_NAME} < "${FILE_IMAGE_PATH}" > /dev/null
if [ -n "$(docker images -q ${DOCKER_IMAGE_NAME})" ]; then
  echo -e "Importing of docker image is complete"
fi
