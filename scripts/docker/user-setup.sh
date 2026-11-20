#!/bin/bash
# Build STOKE's external dependencies inside the container image.
set -e

cd /home/stoke/stoke

./configure.sh
make external NTHREADS=$(nproc)

# fix PATH
echo "PATH=\"\$HOME/stoke/bin:\$PATH\"" >> ~/.bashrc
