import unittest
import json

from zabbix_get import zabbix_get

class ModbusBulkTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

    # 16bit, all tests are PDU (start from 0 address)
    def test_modbus_bulk_BE(self):
        formula = "2*float+1*int32+1*uint64"
        first_reg = "8"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "8":-71.879005,
                "10":-71.879005,
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)

    
    def test_modbus_bulk_int16_with_skip(self):
        formula = "2*skip+2*int16+1*skip+1*int16"
        first_reg = "0"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+"]"
        expected_json = json.loads("""
            {
                "2":-28734,
                "3":3522,
                "5":-15729
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)

    def test_modbus_bulk_uint16_with_skip(self):
        formula = "2*skip+2*uint16+1*skip+1*uint16"
        first_reg = "0"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+"]"
        expected_json = json.loads("""
            {
                "2":36802,
                "3":3522,
                "5":49807
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)        
    
    
    def test_modbus_bulk_BE_with_skip(self):
        formula = "6*skip+2*float+1*int32+1*uint64"
        first_reg = "2"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "8":-71.879005,
                "10":-71.879005,
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)
    
        
    
    def test_modbus_bulk_BE_with_whitespace(self):
        formula = "\' 1*int32 + 1*uint64 \'"
        first_reg = "12"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)
    
    
    def test_modbus_bulk_BE_no_multiplier(self):
        formula = "int32+uint64"
        first_reg = "12"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)

    #must work with other endianess too:
    
    def test_modbus_bulk_LE_with_skip(self):
        formula = "1*float+10*skip+1*double"
        first_reg = "6"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "6":-71.879005,
                "18":-0.123450
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)

    
    def test_modbus_bulk_MLE_with_skip(self):
        formula = "1*float+16*skip+1*double"
        first_reg = "4"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "4":-71.879005,
                "22":-0.123450
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)

    
    def test_modbus_bulk_MBE_with_skip(self):
        formula = "1*float+22*skip+1*double"
        first_reg = "2"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "2":-71.879005,
                "26":-0.123450
            }
            """)
        str_from_zabbix = zabbix_get(key)
        self.assertNotIn("ZBX_NOTSUPPORTED",str_from_zabbix)
        self.assertDictEqual(json.loads(str_from_zabbix), expected_json)

    

    # test that 'bit' is not supported:
    
    def test_modbus_bulk_bit_not_allowed(self):
        formula = "1*bit+10*skip+1*double"
        first_reg = "12"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        self.assertEqual(zabbix_get(key),'ZBX_NOTSUPPORTED: Check bulk formula: usage of \'bit\' datatype is not allowed.')
    

if __name__ == '__main__':
    unittest.main(verbosity=2)