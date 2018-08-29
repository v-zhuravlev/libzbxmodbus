import unittest

from zabbix_get import zabbix_get

class ModbusTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

        
  # test holding_registers03.mbs set
  # 16bit, all tests are PDU (start from 0 address)
  # signed int
    def test_modbus_datatype_signed_0(self):
        key = "modbus_read["+self.host+",1,0,3,s]"
        self.assertEqual(zabbix_get(key),'-15729.000000')
  # signed int
    def test_modbus_datatype_signed_1(self):
        key = "modbus_read["+self.host+",1,1,3,s]"
        self.assertEqual(zabbix_get(key),'-15859.000000')

  # unsigned int
    def test_modbus_datatype_unsigned_0(self):
        key = "modbus_read["+self.host+",1,0,3,i]"
        self.assertEqual(zabbix_get(key),'49807')
  # unsigned int
    def test_modbus_datatype_unsigned_1(self):
        key = "modbus_read["+self.host+",1,1,3,i]"
        self.assertEqual(zabbix_get(key),'49677')

   # 32bit
   # float,BigEndian (ABCD)
    def test_modbus_datatype_float_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,f]"
        self.assertEqual(zabbix_get(key),'-71.879005')
    # float,(LittleEndian with Wordswap),Mid-Little Endian (CDAB)
    def test_modbus_datatype_float_32bit_mle_cdab_0(self):
        key = "modbus_read["+self.host+",1,0,3,f,0]"
        self.assertEqual(zabbix_get(key),'-35.439999')
    #IMPLEMENT: 
    # float (Big Endian with WordSwap) Mid-Big Endian (BADC)
    def test_modbus_datatype_float_32bit_mbe_badc_0(self):
            key = "modbus_read["+self.host+",1,0,3,f,2]"
            self.assertEqual(zabbix_get(key),'-1.91351763e-29')
    #IMPLEMENT: 
    #float (LittleEndian)(DCBA)
    def test_modbus_datatype_float_32bit_le_dcba_0(self):
            key = "modbus_read["+self.host+",1,0,3,f,3]"
            self.assertEqual(zabbix_get(key),'1.19907815e-30')            

    
    # 32bit unsigned integers(long):
    # long,BigEndian (ABCD)
    @unittest.skip("Seems to be broken 32bit long")
    def test_modbus_datatype_long_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,l]"
        self.assertEqual(zabbix_get(key),'3264201229')
    # long,(LittleEndian with Wordswap),Mid-Little Endian (CDAB)
    @unittest.skip("Seems to be broken with 32bit long")
    def test_modbus_datatype_long_32bit_mle_cdab_0(self):
        key = "modbus_read["+self.host+",1,0,3,l,0]"
        self.assertEqual(zabbix_get(key),'3255681679')

    #IMPLEMENT:
    # long (LittleEndian)(DCBA)
    @unittest.skip("implement this first")
    def test_modbus_datatype_long_32bit_le_dcba_0(self):
            key = "modbus_read["+self.host+",1,0,3,l,3]"
            self.assertEqual(zabbix_get(key),'230854594')
    #IMPLEMENT:
    # long (Big Endian with WordSwap) Mid-Big Endian (BADC)
    @unittest.skip("implement this first")
    def test_modbus_datatype_long_32bit_mbe_badc_0(self):
            key = "modbus_read["+self.host+",1,0,3,l,2]"
            self.assertEqual(zabbix_get(key),'2411859394')    



if __name__ == '__main__':
    unittest.main(verbosity=2)