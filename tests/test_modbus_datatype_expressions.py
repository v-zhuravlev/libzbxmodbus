import pytest
import json

from zabbix_get import zabbix_get


class TestModbusBulk(object):

    
    # bits (coils, discrete inputs)

    def test_modbus_bulk_bits_coils(self, host):
        formula = "4*bit"
        first_reg = "0"
        key = "modbus_read["+host+",1,"+first_reg+",1,"+formula+"]"
        expected_json = json.loads("""
            {
                "0":0,
                "1":1,
                "2":0,
                "3":1
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_bits_discrete_inputs(self, host):
        formula = "4*bit"
        first_reg = "0"
        key = "modbus_read["+host+",1,"+first_reg+",2,"+formula+"]"
        expected_json = json.loads("""
            {
                "0":0,
                "1":1,
                "2":0,
                "3":1
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_bits_with_skip(self, host):
        formula = "1*bit+2*skip+1*bit"
        first_reg = "0"
        key = "modbus_read["+host+",1,"+first_reg+",1,"+formula+"]"
        expected_json = json.loads("""
            {
                "0":0,
                "3":1
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    # 16bit, all tests are PDU (start from 0 address)

    def test_modbus_bulk_BE(self, host):
        formula = "2*float+1*int32+1*uint64"
        first_reg = "8"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "8":-71.879005,
                "10":-71.879005,
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_int16_with_skip(self, host):
        formula = "2*int16+1*skip+1*int16"
        first_reg = "2"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+"]"
        expected_json = json.loads("""
            {
                "2":-28734,
                "3":3522,
                "5":-15729
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_uint16_with_skip(self, host):
        formula = "2*uint16+1*skip+1*uint16"
        first_reg = "2"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+"]"
        expected_json = json.loads("""
            {
                "2":36802,
                "3":3522,
                "5":49807
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_BE_with_skip(self, host):
        formula = "2*float+1*int32+1*uint64"
        first_reg = "8"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "8":-71.879005,
                "10":-71.879005,
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_BE_with_whitespace(self, host):
        formula = " 1*int32 + 1*uint64 "
        first_reg = "12"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    # 16bit, all tests are PDU (start from 0 address)
    def test_modbus_bulk_multiplier_after_BE(self, host):
        formula = "float*2+int32*1+uint64*1"
        first_reg = "8"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "8":-71.879005,
                "10":-71.879005,
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_BE_no_multiplier(self, host):
        formula = "int32+uint64"
        first_reg = "12"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",BE]"
        expected_json = json.loads("""
            {
                "12":-562.000000,
                "14":13816931967501922940
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    # must work with other endianess too:

    def test_modbus_bulk_LE_with_skip(self, host):
        formula = "1*float+10*skip+1*double"
        first_reg = "6"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",LE]"
        expected_json = json.loads("""
            {
                "6":-71.879005,
                "18":-0.123450
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_MLE_with_skip(self, host):
        formula = "1*float+16*skip+1*double"
        first_reg = "4"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",MLE]"
        expected_json = json.loads("""
            {
                "4":-71.879005,
                "22":-0.123450
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json

    def test_modbus_bulk_MBE_with_skip(self, host):
        formula = "1*float+22*skip+1*double"
        first_reg = "2"
        key = "modbus_read["+host+",1,"+first_reg+",3,"+formula+",MBE]"
        expected_json = json.loads("""
            {
                "2":-71.879005,
                "26":-0.123450
            }
            """)
        str_from_zabbix = zabbix_get(key)
        assert "ZBX_NOTSUPPORTED" not in str_from_zabbix
        assert json.loads(str_from_zabbix) == expected_json
