#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Invalid number of command line arguments"
    exit
fi

cd clientMachine
./client "$1"