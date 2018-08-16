## How this test works

1. Configure and make libzbxmodbus module  
2. Create docker image of zabbix-agent + libzbxmodbus  
3. Create docker image of Modbus TCP Server (using pymodbus).  
4. Run requests with `zabbix_get` through `zabbix_agent` to modbus-server. Compare results with expected values. For example: `docker exec docker_zabbix-agent-modbus_1 sh -c "zabbix_get -s localhost -k modbus_read[172.16.238.2:5020,1,2,3,i,1,1]"`
5. Repeat 1-4 for different values of ZBX_VERSION and ZBX_HEADERS_VERSION variables in travis.  

### To add new tests  
1. (optional) Add new Modbus values in tests/docker/modbus-server/app.py by using ModbusSparseDataBlock  
2. Create new test_*.py in /tests  
3. Define tests in similar fashion to already created  
4. Add new command in .travis.yml to run new file on commit.  