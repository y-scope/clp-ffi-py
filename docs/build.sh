#!/usr/bin/env bash

# Exit on any error or undefined variable
set -eu

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
pushd "$script_dir/.."

task clean && task docs:site

popd
