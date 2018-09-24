import pytest
from zabbix_get import zabbix_get


@pytest.mark.parametrize("modbus_func", [
    1,
    2
], ids=["read coils", "read discrete inputs"])
class TestModbusBit(object):
    """This tests coils(bits)"""

    def test_modbus_datatype_bit_0(self, host, modbus_func):
        key = "modbus_read[{},1,0,{},bit]".format(host, modbus_func)
        assert zabbix_get(key) == '0'

    def test_modbus_datatype_bit_1(self, host, modbus_func):
        key = "modbus_read[{},1,1,{},bit]".format(host, modbus_func)
        assert zabbix_get(key) == '1'


@pytest.mark.parametrize("modbus_func", [
    3,
    4
], ids=["read holding registers", "read input registers"])
class TestModbus(object):
    # 16bit, all tests are PDU (start from 0 address)
    # INT16,Big Endian

    def test_modbus_datatype_signed_0(self, host, modbus_func):
        key = "modbus_read[{},1,0,{},int16]".format(host, modbus_func)
        assert zabbix_get(key) == '-15729.000000'
    # INT16,Big Endian

    def test_modbus_datatype_signed_1(self, host, modbus_func):
        key = "modbus_read[{},1,1,{},int16]".format(host, modbus_func)
        assert zabbix_get(key) == '-15859.000000'

    # UINT16,Big Endian

    def test_modbus_datatype_unsigned_0(self, host, modbus_func):
        key = "modbus_read[{},1,0,{},uint16]".format(host, modbus_func)
        assert zabbix_get(key) == '49807'
    # UINT16,Big Endian

    def test_modbus_datatype_unsigned_1(self, host, modbus_func):
        key = "modbus_read[{},1,1,{},uint16]".format(host, modbus_func)
        assert zabbix_get(key) == '49677'

    # 32bit.
    # Float. In 32bit float we are looking for "-71.879005"
    def test_modbus_datatype_float_32bit_default(self, host, modbus_func):
        key = "modbus_read[{},1,0,{},float]".format(host, modbus_func)
        assert zabbix_get(key) == '-71.879005'

    # UINT32,unsigned,long:
    def test_modbus_datatype_long_32bit_default(self, host, modbus_func):
        key = "modbus_read[{},1,0,{},uint32]".format(host, modbus_func)
        assert zabbix_get(key) == '3264201229'

    # INT32(Signed)
    def test_modbus_datatype_int32_default(self, host, modbus_func):
        key = "modbus_read[{},1,12,{},int32]".format(host, modbus_func)
        assert zabbix_get(key) == '-562.000000'

    @pytest.mark.parametrize(("reg", "datatype", "endianess",  "expected"), [
        ("0", "float", "BE", "-71.879005"),
        ("4", "float", "MLE", "-71.879005"),
        ("2", "float", "MBE", "-71.879005"),
        ("6", "float", "LE", "-71.879005"),
        ("0", "uint32", "BE", "3264201229"),
        ("0", "uint32", "MLE", "3255681679"),
        ("0", "uint32", "MBE", "2411859394"),
        ("0", "uint32", "LE", "230854594"),
        ("12", "int32", "BE", "-562.000000"),
        ("12", "int32", "MLE", "-36765697.000000"),
        ("12", "int32", "MBE", "-12547.000000"),
        ("12", "int32", "LE", "-822214657.000000")
    ], ids=["float-BE", "float-MLE", "float-MBE", "float-LE", "uint32-BE", "uint32-MLE", "uint32-MBE", "uint32-LE", "int32-BE", "int32-MLE", "int32-MBE", "int32-LE"])
    def test_modbus_datatype_32bit(self, host, modbus_func, reg, datatype, endianess,  expected):
        key = "modbus_read[{},1,{},{},{},{}]".format(
            host, reg, modbus_func, datatype, endianess)
        assert zabbix_get(key) == expected
