#!/bin/bash
if [[ ${TRAVIS_JOB_TYPE} == "FORMAT" ]]; then
   echo "Going to do clang-format check..."
   ./clang-format.sh
   git diff --exit-code
else 
    # run tests
    if [[ ${MODBUS_HOST} ]]; then
        echo "Going to test with docker and modbus-slave emulation, host=${MODBUS_HOST}"
        pytest -v --host=$MODBUS_HOST
    else 
        echo "Going to test with docker and modbus-slave emulation..."
        pytest -v
    fi
fi