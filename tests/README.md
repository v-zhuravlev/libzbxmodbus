## How this test works

1. Configure and make libzbxmodbus module  
2. Use docker image of zabbix-agent and mount libzbxmodbus into it
3. Create docker image of Modbus TCP Server (using pymodbus).  
4. Run requests with `zabbix_get` through `zabbix_agent` to modbus-server.
Compare results with expected values.
For example: `docker-compose exec zabbix-agent-modbus sh -c "zabbix_get -s localhost -k modbus_read[172.16.238.2:5020,1,2,3,i,1,1]"`
5. Repeat 1-4 for different values of ZBX_VERSION and ZBX_HEADERS_VERSION variables in travis.  

### To add new tests  
1. (optional) Add new Modbus values in [app.py](./docker/modbus-server/app.py) by using `ModbusSparseDataBlock`
2. Create new test_*.py in /tests  
3. Define tests in similar fashion to already created  

## Run locally
You can run tests locally:
1. Build libzbxmodbus `./configure --enable-zabbix-3.2 && make`
1. Make sure Docker is running. In Gitpod run `sudo docker-up` and open a new terminal.
2. Build and run the rest with docker-compose `export ZBX_VERSION=3.4.12 ; pushd tests/docker && docker-compose build && docker-compose up -d --force-recreate`
3. Run sample test from shell `docker-compose exec zabbix-agent-modbus sh -c "zabbix_get -s localhost -k modbus_read[172.16.238.2:5020,1,2,3,i,1,1]"` \
Note that by default services do not expose open ports.
If you have `zabbix_get` and/or Modbus utilities installed
and you want to query `zabbix_agent` or Modbus server directly,
then you can add docker-compose.override.yml file alongside [docker-compose.yml](./docker/docker-compose.yml)
with the contents like these:
```yaml
services:
  zabbix-agent-modbus:
    ports:
    - "10050:10050"

  modbus-server:
    ports:
    - "5020:5020" # TCP
    - "5021:5021" # TCP RTU
```