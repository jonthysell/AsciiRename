#!/bin/bash

pushd $(dirname $0)/..

mkdir -p build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=Debug/
cmake --build . --config Debug

popd