import unittest
import subprocess

class ModbusTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

    def zabbix_get(self, key):
        batchcmd = "docker exec docker_zabbix-agent-modbus_1 sh -c \"zabbix_get -s localhost -k"+key +"\""
        result = subprocess.check_output(batchcmd, shell=True)
        return result.rstrip()

    # test errors
    def test_empty(self):
        key = "modbus_read"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Invalid number of parameters.')
    def test_empty_squares(self):
        key = "modbus_read[]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Invalid number of parameters.')        

    def test_no_IP(self):
        key = "modbus_read_registers[,3,14,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: No connection address provided.')
    def test_bad_IP(self):
        key = "modbus_read_registers[badIP,3,14,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Network is unreachable')

    def test_no_slaveID(self):
        key = "modbus_read_registers[{HOST.CONN},,14,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: No slave id provided.')
    def test_no_function(self):
        key = "modbus_read_registers["+self.host+",3,14,,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: No Modbus function provided! Please provide either 1,2,3,4.')

    def test_bad_datatype(self):
        key = "modbus_read_registers["+self.host+",3,14,3,bad,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check datatype provided.')


    def test_bad_PDU_flag_integer(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,1,3]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check addressing scheme(PDU,PROTOCOL) used')
    def test_bad_PDU_flag_string(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,1,bad]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check addressing scheme(PDU,PROTOCOL) used')


    def test_bad_LE_flag_integer(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,2,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check endiannes used')

    def test_bad_LE_flag_string(self):
        key = "modbus_read_registers["+self.host+",3,14,3,l,bad,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check endiannes used')


    def test_bad_function_integer(self):
        key = "modbus_read_registers["+self.host+",3,14,5,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check function (1,2,3,4) used')
    def test_bad_function_string(self):
        key = "modbus_read_registers["+self.host+",3,14,bad,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check function (1,2,3,4) used')

    def test_bad_register_out_of_bounds_integer(self):
        key = "modbus_read_registers["+self.host+",3,1000,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Illegal data address')
    def test_bad_register_string(self):
        key = "modbus_read_registers["+self.host+",3,bad,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check register to read')

    def test_bad_slaveid_integer(self):
        key = "modbus_read_registers["+self.host+",5000,14,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Illegal data address')
    def test_bad_slaveid_string(self):
        key = "modbus_read_registers["+self.host+",bad,14,3,l,1,0]"
        self.assertEqual(self.zabbix_get(key),'ZBX_NOTSUPPORTED: Check slaveid parameter')        
        
  # test holding_registers03.mbs set
  # 16bit, all tests are PDU (start from 0 address)
  # signed int
    def test_modbus_datatype_signed_0(self):
        key = "modbus_read["+self.host+",1,0,3,s]"
        self.assertEqual(self.zabbix_get(key),'-15729.000000')
  # signed int
    def test_modbus_datatype_signed_1(self):
        key = "modbus_read["+self.host+",1,1,3,s]"
        self.assertEqual(self.zabbix_get(key),'-15859.000000')

  # unsigned int
    def test_modbus_datatype_unsigned_0(self):
        key = "modbus_read["+self.host+",1,0,3,i]"
        self.assertEqual(self.zabbix_get(key),'49807')
  # unsigned int
    def test_modbus_datatype_unsigned_1(self):
        key = "modbus_read["+self.host+",1,1,3,i]"
        self.assertEqual(self.zabbix_get(key),'49677')

   # 32bit
   # float,BigEndian (ABCD)
    def test_modbus_datatype_float_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,f]"
        self.assertEqual(self.zabbix_get(key),'-71.879005')
    # float,(LittleEndian with Wordswap),(CDAB)
    def test_modbus_datatype_float_32bit_le_cdab_0(self):
        key = "modbus_read["+self.host+",1,0,3,f,0]"
        self.assertEqual(self.zabbix_get(key),'-35.439999')

    #IMPLEMENT: 
    # float (LittleEndian)(DCBA)
    @unittest.skip("implement this first")
    def test_modbus_datatype_float_32bit_le_dcba_0(self):
            key = "modbus_read["+self.host+",1,0,3,f,0]"
            self.assertEqual(self.zabbix_get(key),'1.19907815e-30')
    #IMPLEMENT: 
    # float (Big Endian with WordSwap) Mid-Big Endian (BADC)
    @unittest.skip("implement this first")
    def test_modbus_datatype_float_32bit_be_badc_0(self):
            key = "modbus_read["+self.host+",1,0,3,f]"
            self.assertEqual(self.zabbix_get(key),'-1.91351763e-29')

    
    # 32bit unsigned integers(long):
    # long,BigEndian (ABCD)
    @unittest.skip("Seems to be broken 32bit long")
    def test_modbus_datatype_long_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,l]"
        self.assertEqual(self.zabbix_get(key),'3264201229')
    # long,(LittleEndian with Wordswap),(CDAB)
    @unittest.skip("Seems to be broken with 32bit long")
    def test_modbus_datatype_long_32bit_le_cdab_0(self):
        key = "modbus_read["+self.host+",1,0,3,l,0]"
        self.assertEqual(self.zabbix_get(key),'3255681679')

    #IMPLEMENT:
    # long (LittleEndian)(DCBA)
    @unittest.skip("implement this first")
    def test_modbus_datatype_long_32bit_le_dcba_0(self):
            key = "modbus_read["+self.host+",1,0,3,l,0]"
            self.assertEqual(self.zabbix_get(key),'230854594')
    #IMPLEMENT: 
    # long (Big Endian with WordSwap) Mid-Big Endian (BADC)
    @unittest.skip("implement this first")
    def test_modbus_datatype_long_32bit_be_badc_0(self):
            key = "modbus_read["+self.host+",1,0,3,l]"
            self.assertEqual(self.zabbix_get(key),'2411859394')    




    # test other things
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