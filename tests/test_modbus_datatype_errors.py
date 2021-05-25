import pytest
import json

from zabbix_get import zabbix_get


class TestModbusDatatypeErrors(object):

    

    def test_modbus_datatype_no_multiplier_sign(self, host):
        formula = "2int16"
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert 'ZBX_NOTSUPPORTED: There must be "*" sign after a multiplier and before a type in the datatype expression:' in  zabbix_get(key)

    def test_modbus_datatype_negative_multiplier(self, host):
        formula = "-2*int16"
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert 'ZBX_NOTSUPPORTED: Multiplier must be positive in the datatype exression:' in  zabbix_get(key)

    def test_modbus_datatype_unknown(self, host):
        formula = "2*bad"
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression:' in  zabbix_get(key)

    def test_modbus_datatype_unexpected_end_0(self, host):
        formula = "2*int16+"
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert zabbix_get(key) ==  'ZBX_NOTSUPPORTED: Unexpected end of the datatype expression.'

    def test_modbus_datatype_skip_0(self, host):
        formula = "skip"
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert zabbix_get(key) ==  'ZBX_NOTSUPPORTED: Datatype expression can neither begin nor end with skipping.'

    def test_modbus_datatype_skip_1(self, host):
        formula = "2*skip"
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert zabbix_get(key) ==  'ZBX_NOTSUPPORTED: Datatype expression can neither begin nor end with skipping.'

    def test_modbus_datatype_skip_2(self, host):
        formula = "  skip   "
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert zabbix_get(key) ==  'ZBX_NOTSUPPORTED: Datatype expression can neither begin nor end with skipping.'

    def test_modbus_datatype_whitespace_0(self, host):
        formula = "\"    \""
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert zabbix_get(key) ==  'ZBX_NOTSUPPORTED: Unexpected end of the datatype expression.'
    
    def test_modbus_no_datatype_for_read_registers_0(self, host):
        """This test checks that if no datatype is provided(with ',') then 'uint16' must be used for functions 3 and 4"""
        formula = ""
        key = "modbus_read_registers["+host+",1,2,3,"+formula+"]"
        assert zabbix_get(key) ==  '36802'
    
    def test_modbus_no_datatype_for_read_registers_1(self, host):
        """This test checks that if no datatype is provided then 'uint16' must be used for functions 3 and 4"""
        key = "modbus_read_registers["+host+",1,2,3]"
        assert zabbix_get(key) ==  '36802'

    def test_modbus_no_datatype_for_read_registers_2(self, host):
        """This test checks that if no datatype is provided then 'uint16' must be used for functions 3 and 4"""
        key = "modbus_read_registers["+host+",1,2,3,,1,0]"
        assert zabbix_get(key) ==  '36802'
    
    def test_modbus_no_datatype_for_read_coils_0(self, host):
        """This test checks that if no datatype is provided(with ',') then 'bit' must be used for functions 1 and 2"""
        formula = ""
        key = "modbus_read_registers["+host+",1,0,1,"+formula+"]"
        assert zabbix_get(key) ==  '0'

    def test_modbus_no_datatype_for_read_coils_1(self, host):
        """This test checks that if no datatype is provided then 'bit' must be used for functions 1 and 2"""
        formula = ""
        key = "modbus_read_registers["+host+",1,0,1]"
        assert zabbix_get(key) ==  '0'

    # test that 'bit' is not supported:
    def test_modbus_datatype_expression_bit_not_allowed(self, host):
        formula = "1*bit+10*skip+1*double"
        first_reg = "12"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",BE]"
        assert 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression:' in  zabbix_get(key)

    def test_bad_datatype(self, host):
        key = "modbus_read_registers["+host+",3,14,3,bad]"
        assert 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression:' in  zabbix_get(key)

    def test_int64_datatype(self, host):
        key = "modbus_read_registers["+host+",3,14,3,int64]"
        assert 'ZBX_NOTSUPPORTED: Invalid type in the datatype expression:' in  zabbix_get(key)