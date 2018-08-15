import unittest
import subprocess

class TestModbus(unittest.TestCase):

    host = "172.16.238.2:5020"
    def zabbix_get(self, key):
        batchcmd = "docker exec docker_zabbix-agent-modbus_1 sh -c \"zabbix_get -s localhost -k"+key +"\""
        result = subprocess.check_output(batchcmd)
        return result.rstrip()


    def test_modbus_simple(self):
        key = "modbus_read["+self.host+",1,2,3,i,1,1]"
        self.assertEqual(self.zabbix_get(key),'49677')
    
    def test_upper(self):
        self.assertEqual('foo'.upper(), 'FOO')

    def test_isupper(self):
        self.assertTrue('FOO'.isupper())
        self.assertFalse('Foo'.isupper())

    def test_split(self):
        s = 'hello world'
        self.assertEqual(s.split(), ['hello', 'world'])
        # check that s.split fails when the separator is not a string
        with self.assertRaises(TypeError):
            s.split(2)

if __name__ == '__main__':
    unittest.main()