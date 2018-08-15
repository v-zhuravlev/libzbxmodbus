import subprocess

def zabbix_get(key):
    batchcmd = "docker exec docker_zabbix-agent-modbus_1 sh -c \"zabbix_get -s localhost -k"+key +"\""
    result = subprocess.check_output(batchcmd, shell=True)
    return result.rstrip()