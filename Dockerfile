# syntax=docker/dockerfile:1

ARG BASE_CONTAINER=python:3.12-bookworm
FROM $BASE_CONTAINER
ARG TARGETARCH

ARG USERNAME=bender
ARG USER_UID=1000
ARG USER_GID=1000

COPY scripts/install-$TARGETARCH.sh /tmp
RUN chmod +x /tmp/install-$TARGETARCH.sh
RUN /tmp/install-$TARGETARCH.sh

RUN groupadd -f -g $USER_GID $USERNAME \
 && useradd -l -g $USER_GID -G sudo --uid $USER_UID -ms /bin/bash $USERNAME \
 && echo $USERNAME:$USERNAME | chpasswd \
 && echo $USERNAME 'ALL=(ALL) NOPASSWD:SETENV: ALL' > /etc/sudoers.d/010_$USERNAME || true
