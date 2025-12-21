FROM ubuntu:20.04
LABEL maintainer="Berkeley Churchill (berkeley@cs.stanford.edu)"

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Create stoke user first
RUN useradd -ms /bin/bash stoke

# Install software-properties-common first, then add PPA for newer GCC versions
RUN apt-get update && apt-get install -y \
    software-properties-common \
    && add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && apt-get update

# Add Ubuntu 16.04 (Xenial) repositories to install GCC 4.9
RUN echo "deb http://dk.archive.ubuntu.com/ubuntu/ xenial main" >> /etc/apt/sources.list && \
    echo "deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe" >> /etc/apt/sources.list && \
    apt-get update

# Install GCC 4.9 from Xenial repositories
RUN apt-get install -y --allow-unauthenticated \
    gcc-4.9 \
    g++-4.9 \
    gcc-4.9-multilib \
    g++-4.9-multilib

# Install all STOKE dependencies
RUN apt-get install -y \
    antlr \
    autoconf \
    bison \
    ccache \
    cmake \
    doxygen \
    exuberant-ctags \
    flex \
    g++-multilib \
    ghc \
    git \
    libantlr3c-dev \
    libboost-all-dev \
    libcln-dev \
    libghc-regex-compat-dev \
    libghc-regex-tdfa-dev \
    libghc-split-dev \
    libgmp-dev \
    libjsoncpp-dev \
    libiml-dev \
    libpqxx-dev \
    libtool \
    openssh-server \
    pccts \
    pkg-config \
    python3 \
    subversion \
    time \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Install additional development tools for devcontainer
RUN apt-get update && apt-get install -y \
    gdb \
    gdbserver \
    valgrind \
    strace \
    ltrace \
    curl \
    wget \
    unzip \
    zip \
    tree \
    htop \
    nano \
    less \
    man-db \
    bash-completion \
    sudo \
    && rm -rf /var/lib/apt/lists/*

# Set up GCC 4.9 as default and configure environment variables
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.9

# Set environment variables for STOKE compilation
ENV CC=gcc-4.9
ENV CXX=g++-4.9
ENV NOCVC4=1

# Compile custom jsoncpp with old ABI for GCC 4.9 compatibility
# This fixes JSON linking issues by ensuring ABI compatibility between STOKE and jsoncpp
# The system libjsoncpp is compiled with new C++11 ABI, but GCC 4.9 uses old ABI by default
RUN cd /tmp && \
    wget -q https://github.com/open-source-parsers/jsoncpp/archive/1.7.4.tar.gz && \
    tar xf 1.7.4.tar.gz && \
    cd jsoncpp-1.7.4 && \
    mkdir build && \
    cd build && \
    CC=gcc-4.9 CXX=g++-4.9 CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0" cmake .. && \
    make jsoncpp_lib_static -j$(nproc) && \
    mkdir -p /usr/local/stoke/lib && \
    cp src/lib_json/libjsoncpp.a /usr/local/stoke/lib/ && \
    cd /tmp && \
    rm -rf jsoncpp-1.7.4 1.7.4.tar.gz


RUN usermod -a -G sudo stoke

# Ensure stoke user has sudo access without password for development
RUN echo "stoke ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set up the workspace directory first
WORKDIR /home/stoke
RUN mkdir -p /home/stoke/stoke

# Copy the current workspace content
COPY . /home/stoke/stoke/

# Set up the STOKE environment and build dependencies
RUN chown -R stoke:stoke /home/stoke/stoke && \
    chmod +x /home/stoke/stoke/scripts/docker/user-setup.sh

# setup ssh _server_ keys
RUN ssh-keygen -A -t ed25519 -v


# Switch to stoke user for the setup
USER stoke
WORKDIR /home/stoke/stoke

# copy over the ssh _user_ keys
# These keys have no passphrase
RUN mkdir ../.ssh/
RUN cp stoke_dockerkey.pub ../.ssh/authorized_keys


# Set environment variables for stoke user
RUN echo 'export CC=gcc-4.9' >> ~/.bashrc \
    && echo 'export CXX=g++-4.9' >> ~/.bashrc \
    && echo 'export NOCVC4=1' >> ~/.bashrc

# Run the user setup script to build STOKE dependencies
RUN /home/stoke/stoke/scripts/docker/user-setup.sh

# Set up development environment
RUN echo 'export PATH="/home/stoke/stoke/bin:$PATH"' >> ~/.bashrc \
    && echo 'cd /home/stoke/stoke' >> ~/.bashrc \
    && echo 'alias ll="ls -la"' >> ~/.bashrc \
    && echo 'alias la="ls -A"' >> ~/.bashrc \
    && echo 'alias l="ls -CF"' >> ~/.bashrc

# Default to bash shell
SHELL ["/bin/bash", "-c"]

# Start ssh and hang (start bash)
ENTRYPOINT sudo service ssh start && bash

# build with docker build -t $CONTAINER_NAME .
# run with  docker run --name stoke-test -d -it -p 2000:22 $CONTAINER_NAME
# that will detach it and leave it running (kill with docker kill stoke-test)
# you may need to then fully remove it with docker container rm stoke-test)
# ssh in with ssh -i stoke_dockerkey -p 2000 stoke@localhost