#!/usr/bin/env bash

JARVISTO_URL1=https://github.com/denoming/jarvisto/releases/download/v0.3.11/libjarvisto_0.3.11_arm64.deb
JARVISTO_URL2=https://github.com/denoming/jarvisto/releases/download/v0.3.11/libjarvisto-dev_0.3.11_arm64.deb

apt update
apt install -y build-essential sudo vim git cmake ninja-build gdb curl tar zip unzip \
               libgtest-dev libgmock-dev libspdlog-dev libhowardhinnant-date-dev nlohmann-json3-dev \
               libboost-json1.81-dev libboost-program-options1.81-dev libboost-filesystem1.81-dev libboost-url1.81-dev \
               libssl-dev libmosquittopp-dev libsndfile1-dev libconfig++-dev libsigc++-3.0-dev

# Install libjarvisto dependency
wget -O /tmp/jarvisto1.deb $JARVISTO_URL1
apt install -y /tmp/jarvisto1.deb
rm /tmp/jarvisto1.deb
wget -O /tmp/jarvisto2.deb $JARVISTO_URL2
apt install -y /tmp/jarvisto2.deb
rm /tmp/jarvisto2.deb