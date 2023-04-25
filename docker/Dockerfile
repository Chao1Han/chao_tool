FROM ubuntu:22.04

RUN ln -snf /bin/bash /bin/sh
ENV LANG=C.UTF-8
ARG DEBIAN_FRONTEND=noninteractive

HEALTHCHECK NONE
RUN useradd -d /home/ipex -m -s /bin/bash ipex

RUN apt-get update && \
    apt-get install -y --no-install-recommends --fix-missing \
    apt-utils \
    build-essential \
    ca-certificates \
    clinfo \
    curl \
    git \
    gnupg2 \
    gpg-agent \
    rsync \
    sudo \
    unzip \
    wget && \
    apt-get clean && \
    rm -rf  /var/lib/apt/lists/*

RUN https_proxy=$https_proxy wget -qO - https://repositories.intel.com/graphics/intel-graphics.key | \
    gpg --dearmor --output /usr/share/keyrings/intel-graphics.gpg
RUN echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/intel-graphics.gpg] https://repositories.intel.com/graphics/ubuntu jammy flex' | \
    tee  /etc/apt/sources.list.d/intel.gpu.jammy.list

RUN apt-get update && \
    apt-get install -y --no-install-recommends --fix-missing \
    intel-opencl-icd=23.05.25593.18-601~20.04\
    intel-level-zero-gpu=1.3.25593.18-601~20.04 \
    level-zero=1.9.4+i589~20.04 \
    level-zero-dev=1.9.4+i589~20.04 && \
    apt-get clean && \
    rm -rf  /var/lib/apt/lists/*

RUN https_proxy=$https_proxy wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \
   | gpg --dearmor | tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null && \
   echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" \
   | tee /etc/apt/sources.list.d/oneAPI.list
   
RUN apt-get update && apt-get install -y --no-install-recommends --fix-missing \
    python3.10 libpython3.10 python3-pip && \
    apt-get clean && \
    rm -rf  /var/lib/apt/lists/*

RUN pip --no-cache-dir install --upgrade \
    pip \
    setuptools

RUN ln -sf $(which python3.10) /usr/local/bin/python && \
    ln -sf $(which python3.10) /usr/local/bin/python3 && \
    ln -sf $(which python3.10) /usr/bin/python && \
    ln -sf $(which python3.10) /usr/bin/python3


WORKDIR /home/gta