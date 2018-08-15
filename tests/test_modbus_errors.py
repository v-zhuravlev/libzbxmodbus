import unittest

from zabbix_get import zabbix_get

class ModbusErrorsCase(unittest.TestCase):

    host = "172.16.238.2:5020"
    # test errors
    def test_empty(self):
        """Test Empty"""
        key = "modbus_read"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Invalid number of parameters.')
    def test_empty_squares(self):
        """Test empty squares"""
        key = "modbus_read[]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Invalid number of parameters.')        

    def test_no_IP(self):
        """Test no IP"""
        key = "modbus_read_registers[,3,14,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: No connection address provided.')
    def test_bad_IP(self):
        """Test bad IP"""
        key = "modbus_read_registers[badIP,3,14,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Network is unreachable')

    def test_no_slaveID(self):
        key = "modbus_read_registers[{HOST.CONN},,14,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: No slave id provided.')
    def test_no_function(self):
        key = "modbus_read_registers["+self.host+",3,14,,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: No Modbus function provided! Please provide either 1,2,3,4.')

    def test_bad_datatype(self):
        key = "modbus_read_registers["+self.host+",3,14,3,bad,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check datatype provided.')


    def test_bad_PDU_flag_integer(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,1,3]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check addressing scheme(PDU,PROTOCOL) used')
    def test_bad_PDU_flag_string(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,1,bad]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check addressing scheme(PDU,PROTOCOL) used')


    def test_bad_LE_flag_integer(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,2,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check endiannes used')

    def test_bad_LE_flag_string(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,bad,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check endiannes used')


    def test_bad_function_integer(self):
        key = "modbus_read_registers["+self.host+",3,14,5,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check function (1,2,3,4) used')
    def test_bad_function_string(self):
        key = "modbus_read_registers["+self.host+",3,14,bad,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check function (1,2,3,4) used')

    def test_bad_register_out_of_bounds_integer(self):
        key = "modbus_read_registers["+self.host+",3,1000,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Illegal data address')
    def test_bad_register_string(self):
        key = "modbus_read_registers["+self.host+",3,bad,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check register to read')

    def test_bad_slaveid_integer(self):
        key = "modbus_read_registers["+self.host+",5000,14,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Illegal data address')
    def test_bad_slaveid_string(self):
        key = "modbus_read_registers["+self.host+",bad,14,3,l,1,0]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check slaveid parameter')        
        


if __name__ == '__main__':
    unittest.main(verbosity=2)