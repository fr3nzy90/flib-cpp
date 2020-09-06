#!/bin/bash

count=${1:-1}

folders=(
  "../../out/build/Linux-x64-Debug"
  "../../out/build/Linux-x64-Release"
)
for folder in "${folders[@]}"; do
  echo "Folder: $folder"
  script_directory="`pwd`"
  cd $folder
  for i in $(seq 1 $count); do 
    "./tests"
  done
  cd $script_directory
done