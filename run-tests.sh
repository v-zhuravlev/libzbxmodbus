#!/bin/bash
if [[ ${TRAVIS_JOB_TYPE} == "FORMAT" ]]; then
   echo "Going to do clang-format check..."
   ./clang-format.sh
   git diff --exit-code
else 
    # run tests
    echo "Going to test with docker and modbus-slave emulation..."
    pytest -v
fi