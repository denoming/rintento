#!/usr/bin/env bash

set -e

DOCKER_IMAGE_NAME=my/jarvis-dev-image:user

PROJECT_DIR=$(dirname "$(dirname "$(realpath -s $0)")")
USER_NAME=${USER}
USER_UID=$(id -u)
USER_GID=$(id -g)

command -v docker > /dev/null
if [ $? != 0 ]; then
    echo -e "Error:"
    echo -e "Docker is not installed"
    exit 1
fi

build_image() {
  BUILD_CMD="docker build --tag ${DOCKER_IMAGE_NAME} \
  --build-arg UNAME=${USER_NAME} \
  --build-arg UID=${USER_UID} \
  --build-arg UID=${USER_GID} \
  --build-arg PULSE_SERVER="unix:${XDG_RUNTIME_DIR}/pulse/native" \
  --file ${PROJECT_DIR}/Dockerfile ${PROJECT_DIR}
  "

  if [ -z "$(docker images -q ${DOCKER_IMAGE_NAME})" ]; then
    echo -e "Building <${DOCKER_IMAGE_NAME}> image"
    echo "${BUILD_CMD}"
    ${BUILD_CMD}
  fi
}

run_image() {
  RUN_CMD=(docker run -it \
  --hostname "${USER_NAME}" \
  --rm \
  --user="${USER_UID}:${USER_GID}"
  --volume="${PROJECT_DIR}:${PROJECT_DIR}:rw" \
  --volume="${XDG_RUNTIME_DIR}/pulse:${XDG_RUNTIME_DIR}/pulse" \
  --network=bridge \
  --workdir="${PROJECT_DIR}" \
  --env PULSE_SERVER="unix:${XDG_RUNTIME_DIR}/pulse/native" \
  "${DOCKER_IMAGE_NAME}" /bin/bash)

  if [ -n "$(docker images -q ${DOCKER_IMAGE_NAME})" ]; then
    "${RUN_CMD[@]}"
  else
    echo "Docker image <${DOCKER_IMAGE_NAME}> is absent"
  fi
}

build_image
run_image