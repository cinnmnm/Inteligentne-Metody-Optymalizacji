FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-pandas \
    python3-matplotlib \
    python3-networkx \ 
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1