FROM ubuntu:26.04
LABEL maintainer="Berkeley Churchill (berkeley@cs.stanford.edu)"

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Create stoke user first
RUN useradd -ms /bin/bash stoke

# Toolchain + STOKE dependencies. The system compiler (GCC 15) and system
# Boost/jsoncpp/GMP are used directly; Z3, cvc5 and googletest are built from
# the pinned submodules under src/ext by `make external`.
RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf \
    bison \
    build-essential \
    ca-certificates \
    ccache \
    cmake \
    flex \
    g++ \
    g++-multilib \
    gcc \
    ghc \
    git \
    libboost-all-dev \
    libcln-dev \
    libghc-regex-compat-dev \
    libghc-regex-tdfa-dev \
    libghc-split-dev \
    libgmp-dev \
    libiml-dev \
    libjsoncpp-dev \
    libtool \
    m4 \
    ninja-build \
    openssh-server \
    pkg-config \
    python3 \
    python3-pip \
    python3-tomli \
    python3-venv \
    time \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Install additional development tools for devcontainer
RUN apt-get update && apt-get install -y --no-install-recommends \
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

RUN usermod -a -G sudo stoke

# Ensure stoke user has sudo access without password for development
RUN echo "stoke ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set up the workspace directory first
WORKDIR /home/stoke
RUN mkdir -p /home/stoke/stoke

# Copy the current workspace content. Submodules (x64asm, cpputil, z3, cvc5,
# googletest) must be checked out on the host first:
#   git submodule update --init
COPY . /home/stoke/stoke/

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

# Build STOKE's external dependencies (x64asm, Z3, cvc5, googletest)
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
#
# ssh in with ssh -i stoke_dockerkey -p 2000 stoke@localhost
# you may need to change permissions on stoke_dockerkey first: chmod 0600 stoke_dockerkey
#
# once done, run `make release` to avoid stoke trying to build git hooks
