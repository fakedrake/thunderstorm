#! /usr/bin/env nix-shell
#! nix-shell -i bash
set -e
set -u
set -o pipefail


mkdir -p build/
cd build
cmake ..
make VERBOSE=5
./thunderstorm
