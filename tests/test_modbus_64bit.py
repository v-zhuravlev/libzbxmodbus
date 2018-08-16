import unittest

from zabbix_get import zabbix_get

class Modbus64BitTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

    #64bit signed
    #https://github.com/v-zhuravlev/libzbxmodbus/issues/28#issuecomment-390718491
    # 64bit signed integers(long):
    # long,BigEndian
    # testing for value 0xFFFFFFFFFFFFFDCE
    def test_modbus_datatype_long_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,10,3,S]"
        self.assertEqual(zabbix_get(key),'-562.000000')
    # 64int signed,(LittleEndian with Wordswap),(CDAB)
    def test_modbus_datatype_long_64bit_le_cdab_0(self):
        key = "modbus_read["+self.host+",1,10,3,S,0]"
        self.assertEqual(zabbix_get(key),'-157907461934678016.000000')

    #IMPLEMENT:
    # 64int signed (LittleEndian)(DCBA)
    @unittest.skip("implement this first")
    def test_modbus_datatype_long_64bit_le_dcba_0(self):
            key = "modbus_read["+self.host+",1,10,3,S,0]"
            self.assertEqual(zabbix_get(key),'-3531385057811890177')
    #IMPLEMENT: 
    # 64int signed (Big Endian with WordSwap) Mid-Big Endian (BADC)
    @unittest.skip("implement this first")
    def test_modbus_datatype_long_64bit_be_badc_0(self):
            key = "modbus_read["+self.host+",1,10,3,S]"
            self.assertEqual(zabbix_get(key),'-12547')


if __name__ == '__main__':
    unittest.main(verbosity=2)