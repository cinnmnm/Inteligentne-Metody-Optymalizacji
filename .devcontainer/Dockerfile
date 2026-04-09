FROM debian:bookworm-slim
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libomp-dev \
    git \
    python3 \
    python3-pip \
    python3-pandas \
    python3-matplotlib \
    python3-networkx \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash vscode

# alias python -> python3
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 1

USER vscode