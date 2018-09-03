import unittest

from zabbix_get import zabbix_get

class ModbusTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

    # 16bit, all tests are PDU (start from 0 address)
    # INT16,Big Endian
    def test_modbus_datatype_signed_0(self):
        key = "modbus_read["+self.host+",1,0,3,int16]"
        self.assertEqual(zabbix_get(key),'-15729.000000')
    # INT16,Big Endian
    def test_modbus_datatype_signed_1(self):
        key = "modbus_read["+self.host+",1,1,3,int16]"
        self.assertEqual(zabbix_get(key),'-15859.000000')


    # UINT16,Big Endian
    def test_modbus_datatype_unsigned_0(self):
        key = "modbus_read["+self.host+",1,0,3,uint16]"
        self.assertEqual(zabbix_get(key),'49807')
    # UINT16,Big Endian
    def test_modbus_datatype_unsigned_1(self):
        key = "modbus_read["+self.host+",1,1,3,uint16]"
        self.assertEqual(zabbix_get(key),'49677')

    # 32bit. 
    # Float. In 32bit float we are looking for "-71.879005"
    # Big Endian (ABCD)
    def test_modbus_datatype_float_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,float]"
        self.assertEqual(zabbix_get(key),'-71.879005')
    # Little Endian with Wordswap or Mid-Little Endian (CDAB)
    def test_modbus_datatype_float_32bit_mle_cdab_0(self):
        key = "modbus_read["+self.host+",1,4,3,float,MLE]"
        self.assertEqual(zabbix_get(key),'-71.879005')
    # Big Endian with WordSwap or Mid-Big Endian (BADC)
    def test_modbus_datatype_float_32bit_mbe_badc_0(self):
        key = "modbus_read["+self.host+",1,2,3,float,MBE]"
        self.assertEqual(zabbix_get(key),'-71.879005')
    # Little Endian (DCBA)
    def test_modbus_datatype_float_32bit_le_dcba_0(self):
        key = "modbus_read["+self.host+",1,6,3,float,LE]"
        self.assertEqual(zabbix_get(key),'-71.879005')

    # UINT32,unsigned,long:
    # Big Endian (ABCD)
    def test_modbus_datatype_long_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,uint32]"
        self.assertEqual(zabbix_get(key),'3264201229')
    # Mid-Little Endian (CDAB)
    def test_modbus_datatype_long_32bit_mle_cdab_0(self):
        key = "modbus_read["+self.host+",1,0,3,uint32,MLE]"
        self.assertEqual(zabbix_get(key),'3255681679')

    # Little Endian (DCBA)
    def test_modbus_datatype_long_32bit_le_dcba_0(self):
        key = "modbus_read["+self.host+",1,0,3,uint32,LE]"
        self.assertEqual(zabbix_get(key),'230854594')
    # Mid-Big Endian (BADC)
    def test_modbus_datatype_long_32bit_mbe_badc_0(self):
        key = "modbus_read["+self.host+",1,0,3,uint32,MBE]"
        self.assertEqual(zabbix_get(key),'2411859394')    

    # INT32(Signed)
    # Big Endian (ABCD)
    def test_modbus_datatype_int32_be_0(self):
        key = "modbus_read["+self.host+",1,12,3,int32]"
        self.assertEqual(zabbix_get(key),'-562.000000')
    # Mid-Little Endian (CDAB)
    def test_modbus_datatype_int32_mle_0(self):
        key = "modbus_read["+self.host+",1,12,3,int32,MLE]"
        self.assertEqual(zabbix_get(key),'-36765697.000000')

    # Little Endian (DCAB)
    def test_modbus_datatype_int32_le_0(self):
        key = "modbus_read["+self.host+",1,12,3,int32,LE]"
        self.assertEqual(zabbix_get(key),'-822214657.000000')

    # Mid-Big Endian (BADC)
    def test_modbus_datatype_int32_mbe_0(self):
        key = "modbus_read["+self.host+",1,12,3,int32,MBE]"
        self.assertEqual(zabbix_get(key),'-12547.000000')
            


if __name__ == '__main__':
    unittest.main(verbosity=2)