#!/usr/bin/env bash

set -e

make_image=false
push_image=false
platform="arm64"

while getopts "mpe:" flag; do
  case "$flag" in
    m)
      make_image=true
      ;;
    p)
      push_image=true
      ;;
    e)
      platform="$OPTARG"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

root_dir="$(dirname "$(dirname "$(realpath -s $0)")")"
user_uid="$(id -u)"
user_gid="$(id -g)"
image="denoming/rintento:latest"

command -v docker > /dev/null
if [ $? != 0 ]; then
    echo -e "Error:"
    echo -e "Docker is not installed"
    exit 1
fi

make_image() {
  CMD=(docker build \
  --platform "linux/amd64,linux/arm64" \
  --tag "${image}" \
  --build-arg "BASE_CONTAINER=python:3.12-bookworm" \
  --build-arg "USERNAME=bender" \
  --build-arg "USER_UID=${user_uid}" \
  --build-arg "USER_GID=${user_gid}" \
  --build-arg "SHARE_DIR="/usr/share/rintento"" \
  --file "${root_dir}/Dockerfile"
  "${root_dir}")

  if [ -z "$(docker images -q ${image})" ]; then
    echo -e "Building <${image}> image"
    echo "${CMD[@]}"
    "${CMD[@]}"
  fi
}

run_image() {
  if ! [ -n "$(docker images -q ${image})" ]; then
    echo "Pull <${image}> docker image"
    docker pull ${image}
  fi

  CMD=(docker run -it \
  --platform "linux/${platform}" \
  --rm \
  --user "${user_uid}:${user_gid}" \
  --env RINTENTO_CONFIG="${HOME}/.local/share/rintento/rintento-config.cfg" \
  --volume "${HOME}/.ssh:${HOME}/.ssh" \
  --volume "${root_dir}:${root_dir}" \
  --volume "${HOME}/.local/share/rintento:${HOME}/.local/share/rintento" \
  --network "host" \
  --workdir "${root_dir}" \
  "${image}" /bin/bash)

  echo -e "Running <${image}> image"
  echo "${CMD[@]}"
  "${CMD[@]}"
}

push_image() {
  CMD=(docker image push ${image})
  if [ -n "$(docker images -q ${image})" ]; then
    echo -e "Pushing <${image}> image"
    echo "${CMD[@]}"
    "${CMD[@]}"
  else
    echo "Docker image <${image}> is absent"
  fi
}

if [ "$make_image" == "true" ]; then
  make_image
fi
if [ "$push_image" == "true" ]; then
  push_image
fi
run_image
