#!/bin/bash

# Build the Docker image
docker build -t rpi-kernel-module .

# Run the Docker container and compile the module
docker run --rm -v $(pwd):/usr/src/kernel rpi-kernel-module bash -c "make M=/usr/src/kernel"

# The compiled module will be available in the current directory