import subprocess

def zabbix_get(key):
    batchcmd = "docker-compose exec zabbix-agent-modbus sh -c \"zabbix_get -s localhost -k" + key + "\""
    result = subprocess.check_output(batchcmd, shell=True, cwd="tests/docker/")
    return result.rstrip()