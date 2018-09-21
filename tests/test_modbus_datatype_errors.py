import unittest
import json

from zabbix_get import zabbix_get


class ModbusDatatypeErrorsTestCase(unittest.TestCase):

    host = "172.16.238.2:5020"

    def test_modbus_datatype_no_multiplier_sign(self):
        formula = "2int16"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: There must be "*" sign after a multiplier and before a type in the datatype expression.')

    def test_modbus_datatype_negative_multiplier(self):
        formula = "-2*int16"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Multiplier must be positive in the datatype exression.')

    def test_modbus_datatype_unknown(self):
        formula = "2*bad"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression.')

    def test_modbus_datatype_unexpected_end_0(self):
        formula = "2*int16+"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Unexpected end of the datatype expression.')

    def test_modbus_datatype_skip_0(self):
        formula = "skip"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Datatype expression can neither begin nor end with skipping.')

    def test_modbus_datatype_skip_1(self):
        formula = "2*skip"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Datatype expression can neither begin nor end with skipping.')

    def test_modbus_datatype_skip_2(self):
        formula = "'  skip   '"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Datatype expression can neither begin nor end with skipping.')

    @unittest.skip("Requires shell escaping")
    def test_modbus_datatype_whitespace_0(self):
        formula = "\'\"    \"\'"
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Unexpected end of the datatype expression.')
    
    def test_modbus_no_datatype_for_read_registers_0(self):
        """This test checks that if no datatype is provided(with ',') then 'uint16' must be used for functions 3 and 4"""
        formula = ""
        key = "modbus_read_registers["+self.host+",1,2,3,"+formula+"]"
        self.assertEqual(zabbix_get(key), '36802')
    
    def test_modbus_no_datatype_for_read_registers_1(self):
        """This test checks that if no datatype is provided then 'uint16' must be used for functions 3 and 4"""
        key = "modbus_read_registers["+self.host+",1,2,3]"
        self.assertEqual(zabbix_get(key), '36802')

    def test_modbus_no_datatype_for_read_registers_2(self):
        """This test checks that if no datatype is provided then 'uint16' must be used for functions 3 and 4"""
        key = "modbus_read_registers["+self.host+",1,2,3,,1,0]"
        self.assertEqual(zabbix_get(key), '36802')
    
    @unittest.skip("Implement this first")
    def test_modbus_no_datatype_for_read_coils_0(self):
        """This test checks that if no datatype is provided(with ',') then 'bit' must be used for functions 1 and 2"""
        formula = ""
        key = "modbus_read_registers["+self.host+",1,2,1,"+formula+"]"
        self.assertEqual(zabbix_get(key), 'some coil value')

    @unittest.skip("Implement this first")
    def test_modbus_no_datatype_for_read_coils_1(self):
        """This test checks that if no datatype is provided then 'bit' must be used for functions 1 and 2"""
        formula = ""
        key = "modbus_read_registers["+self.host+",1,2,1]"
        self.assertEqual(zabbix_get(key), 'some coil value')

    # test that 'bit' is not supported:
    def test_modbus_datatype_expression_bit_not_allowed(self):
        formula = "1*bit+10*skip+1*double"
        first_reg = "12"
        key = "modbus_read["+self.host+",1,"+first_reg+",3,"+formula+",BE]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Usage of \'bit\' is not allowed in the datatype expression.')

    def test_bad_datatype(self):
        key = "modbus_read_registers["+self.host+",3,14,3,bad]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression.')

    def test_int64_datatype(self):
        key = "modbus_read_registers["+self.host+",3,14,3,int64]"
        self.assertEqual(zabbix_get(key), 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression.')


if __name__ == '__main__':
    unittest.main(verbosity=2)
