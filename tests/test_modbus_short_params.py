import unittest

from zabbix_get import zabbix_get

class ModbusTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

    # 16bit, all tests are PDU (start from 0 address)
    # INT16,Big Endian
    def test_modbus_datatype_short_signed_1(self):
        key = "modbus_read["+self.host+",1,1,3,s]"
        self.assertEqual(zabbix_get(key),'-15859.000000')

    # UINT16,Big Endian, i
    def test_modbus_datatype_short_unsigned_2(self):
        key = "modbus_read["+self.host+",1,1,3,i]"
        self.assertEqual(zabbix_get(key),'49677')

    # 32bit. 
    # Float. In 32bit float we are looking for "-71.879005"
    # Big Endian (ABCD)
    def test_modbus_datatype_short_float_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,f]"
        self.assertEqual(zabbix_get(key),'-71.879005')
    
    # UINT32,unsigned,long:
    # Big Endian (ABCD)
    def test_modbus_datatype_short_long_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,l]"
        self.assertEqual(zabbix_get(key),'3264201229')
    
    # INT32(Signed)
    # Big Endian (ABCD)
    def test_modbus_datatype_short_int32_be_0(self):
        key = "modbus_read["+self.host+",1,12,3,S]"
        self.assertEqual(zabbix_get(key),'-562.000000')


#UNSIGNED INT64
    # Big Endian
    def test_modbus_datatype_short_uint64_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,I]"
        self.assertEqual(zabbix_get(key),'13816931967501922940')

    # FLOAT64(double)
    # Big Endian
    def test_modbus_datatype_short_double_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,d]"
        self.assertEqual(zabbix_get(key),'-0.123450')


if __name__ == '__main__':
    unittest.main(verbosity=2)