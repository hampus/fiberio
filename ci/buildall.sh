#!/bin/bash
#
# Run this from the parent directory to build in all the testing environments
#

function buildenv() {
    DOCKERENV=$1
    printf "Building Docker image based on $DOCKERENV\n\n"
    if docker build -t fiberio-$DOCKERENV -f ci/$DOCKERENV/Dockerfile . ; then
        printf "\nSucceeded to build Docker image based on $DOCKERENV\n\n"
    else
        printf "\nFAILED to build Docker image based on $DOCKERENV\n\n"
        exit 1
    fi
}

function buildcode() {
    DOCKERENV=$1
    CXX=$2
    COVERAGE=$3
    printf "Building project in $DOCKERENV with $CXX\n\n"
    if docker run --rm -e CXX=$CXX -e COVERAGE=$COVERAGE \
            fiberio-$DOCKERENV ; then
        printf "\nSucceeded to build source code in $DOCKERENV with $CXX\n\n"
    else
        printf "\nFAILED to build source code in $DOCKERENV with $CXX\n\n"
    fi
}

buildenv ubuntu18.04
buildenv archlinux

buildcode ubuntu18.04 g++ 1
buildcode ubuntu18.04 clang++ 0
buildcode archlinux g++ 0
buildcode archlinux clang++ 0
