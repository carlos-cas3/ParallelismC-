#!/bin/bash
set -e

sudo apt update

sudo apt install -y \
    build-essential \
    cmake \
    libopencv-dev \
    libzmq3-dev \
    libssl-dev

