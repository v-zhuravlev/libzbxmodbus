import subprocess
from os.path import abspath, dirname, join

def zabbix_get(key):
    batchcmd = ["docker-compose", "exec", "-T", "zabbix-agent-modbus", "zabbix_get", "-s", "localhost", "-k", key]
    directory = join(dirname(abspath(__file__)), "docker")
    result = subprocess.check_output(batchcmd, cwd=directory)
    return result.rstrip()