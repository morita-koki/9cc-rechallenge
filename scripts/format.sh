#!/bin/bash


# This script is used to format the code.
# It uses clang-format to format the code.
# You can install clang-format by running `brew install clang-format` on macOS.

formatter="/usr/bin/clang-format"



if [ ! -f "$formatter" ]; then
  echo "Please install clang-format"
  exit 1
fi

script_dir=$(cd $(dirname $0); pwd)
project_dir=$(cd $script_dir/..; pwd)
src_dir=$project_dir/src

# format 
$formatter -i $src_dir/*.c $src_dir/*.h --style=Google