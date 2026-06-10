#!/bin/bash -e
handle_error() {
    echo "An error occurred on line $1"
    read -p "Press enter to continue"
    exit 1
}

trap 'handle_error $LINENO' ERR

if [ "$(uname)" == "Darwin" ]; then                                                                                                                                      
  TOOLCHAIN="xcode"                                                                                                                                                    
elif [ "$(uname)" == "Linux" ]; then                                                                                                                                     
  TOOLCHAIN="ninja-clang"
else
  TOOLCHAIN="vs"                                                                                                                                                       
fi  

cmake --preset $TOOLCHAIN -DBUILD_EXTRAS=OFF
