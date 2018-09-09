#!/bin/bash
if [[ ${TRAVIS_JOB_TYPE} == "FORMAT" ]]; then
   echo "Going to do clang-format check..."
   ./clang-format.sh
   git diff --exit-code
else 
    # run tests
    echo "Going to build and then test with docker..."
    ./configure --enable-zabbix-${ZBX_HEADERS_VERSION} -q
    make
    pushd tests/docker && docker-compose up -d
    popd
    docker logs docker_zabbix-agent-modbus_1
    docker exec docker_zabbix-agent-modbus_1 sh -c "zabbix_get -s localhost -k agent.ping"
    docker exec docker_zabbix-agent-modbus_1 sh -c "zabbix_get -s localhost -k modbus_read[172.16.238.2:5020,1,2,3,uint16,1,1]"
    pytest -v
fi