FROM ubuntu:20.04

RUN ln -snf /bin/bash /bin/sh
ENV LANG=C.UTF-8
ARG DEBIAN_FRONTEND=noninteractive

RUN echo "Acquire::ftp::Proxy::linux-ftp.sh.intel.com \"DIRECT\";" >> /etc/apt/apt.conf
RUN echo "Acquire::http::Proxy \"http://child-prc.intel.com:913\";" >> /etc/apt/apt.conf
RUN echo "Acquire::https::Proxy \"http://child-prc.intel.com:913\";" >> /etc/apt/apt.conf
RUN echo "Acquire::ftp::Proxy \"http://child-prc.intel.com:913\";" >> /etc/apt/apt.conf

RUN apt-get update && apt-get install -y --no-install-recommends --fix-missing git ninja-build libglib2.0-0 libsm6 libxext6 wget net-tools git software-properties-common \
build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev wget libbz2-dev curl vim nano pciutils git \
clang-format opencl-headers clinfo initramfs-tools libdrm2 libz3-4 libigdfcl1 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

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

RUN wget -r http://mlpc.intel.com/downloads/gpu-new/components/driver/UBUNTU/agama-ci-devel-602/
RUN dpkg -i ./mlpc.intel.com/downloads/gpu-new/components/driver/UBUNTU/agama-ci-devel-602/*.deb
RUN rm -rf ./mlpc.intel.com

WORKDIR /home/gta