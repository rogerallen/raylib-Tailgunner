FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-22.04

# Install system dependencies
# - Raylib dependencies for Linux
# - libcurl for leaderboard networking
# - zip for packaging
# - Analysis tools (cppcheck, valgrind, clang-tools) used by Makefile
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    build-essential \
    git \
    cmake \
    python3 \
    python3-pip \
    wget \
    unzip \
    zip \
    libasound2-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxcursor-dev \
    libxinerama-dev \
    libwayland-dev \
    libxkbcommon-dev \
    libcurl4-openssl-dev \
    cppcheck \
    valgrind \
    clang-tools \
    && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# ------------------------------------------------------------------------------
# Setup Environment Variables for Makefile
# ------------------------------------------------------------------------------
ENV RAYLIB_NATIVE_PATH=/opt/raylib/native
ENV EMSDK_PATH=/opt/emsdk
ENV RAYLIB_EMSCRIPTEN_PATH=/opt/raylib/web

# ------------------------------------------------------------------------------
# Install Raylib Native (Linux AMD64)
# ------------------------------------------------------------------------------
# Using 5.5 release to match project requirements
RUN mkdir -p ${RAYLIB_NATIVE_PATH} \
    && wget -q https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz \
    && tar -xzf raylib-5.5_linux_amd64.tar.gz -C ${RAYLIB_NATIVE_PATH} --strip-components=1 \
    && rm raylib-5.5_linux_amd64.tar.gz

# ------------------------------------------------------------------------------
# Install Emscripten SDK
# ------------------------------------------------------------------------------
RUN git clone https://github.com/emscripten-core/emsdk.git ${EMSDK_PATH} \
    && cd ${EMSDK_PATH} \
    && ./emsdk install 3.1.56 \
    && ./emsdk activate 3.1.56

# ------------------------------------------------------------------------------
# Install Raylib for Web (Emscripten)
# ------------------------------------------------------------------------------
# 1. Clone Raylib source (tag 5.5)
# 2. Build for Web using Emscripten
# 3. Organize output to match Makefile expectations (libraylib.a in /raylib subdir)
RUN git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git ${RAYLIB_EMSCRIPTEN_PATH} \
    && cd ${RAYLIB_EMSCRIPTEN_PATH}/src \
    && /bin/bash -c "source ${EMSDK_PATH}/emsdk_env.sh && make PLATFORM=PLATFORM_WEB -j$(nproc)" \
    && mkdir -p ${RAYLIB_EMSCRIPTEN_PATH}/raylib \
    && cp libraylib.a ${RAYLIB_EMSCRIPTEN_PATH}/raylib/

# Ensure the vscode user owns the tools
RUN chown -R vscode:vscode /opt/raylib /opt/emsdk
