FROM python:3.12-bookworm

ARG UNAME=dev
ARG UID=1000
ARG GID=1000
ARG FILES_DIR="/"
ARG SHARE_DIR="/"

ENV VCPKG_ROOT="/home/$UNAME/.vcpkg"

USER root

RUN apt update && \
    apt install -y build-essential sudo vim git cmake ninja-build gdb curl tar zip unzip

# Create default user
RUN groupadd -f -g $GID $UNAME
RUN useradd -l -g $GID --uid $UID -ms /bin/bash $UNAME
RUN echo $UNAME:$UNAME | chpasswd
RUN echo $UNAME 'ALL=(ALL) NOPASSWD:SETENV: ALL' > /etc/sudoers.d/$UNAME || true

# Install component tests packages
ADD --chown=$UNAME test/component/requirements.txt $SHARE_DIR/test/component/requirements.txt
RUN pip install -r $SHARE_DIR/test/component/requirements.txt

USER $UNAME

RUN git clone https://github.com/Microsoft/vcpkg.git $HOME/.vcpkg && \
    bash $HOME/.vcpkg/bootstrap-vcpkg.sh && \
    mkdir -p $HOME/.local/bin && \
    ln -s $HOME/.vcpkg/vcpkg $HOME/.local/bin/vcpkg

CMD bash --rcfile "$HOME/.profile"