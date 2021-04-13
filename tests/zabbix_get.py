import subprocess
from os.path import abspath, dirname, join

def zabbix_get(key):
    batchcmd = "docker-compose exec zabbix-agent-modbus sh -c \"zabbix_get -s localhost -k" + key + "\""
    directory = join(dirname(abspath(__file__)), "tests", "docker")
    result = subprocess.check_output(batchcmd, shell=True, cwd=directory)
    return result.rstrip()