import pytest
from zabbix_get import zabbix_get


@pytest.mark.parametrize("modbus_func", [
    3,
    4
], ids=["read holding registers", "read input registers"])
class TestModbus64BitCase(object):

    # INT64(Signed) NOT IMLEMENTED
    @pytest.mark.skip("Implement first")
    def test_modbus_datatype_int64_64bit_default(self, host, modbus_func):
        key = "modbus_read[{},1,14,{},int64]".format(host, modbus_func)
        assert zabbix_get(key) == '-4629812106207628676'

    @pytest.mark.skip("Implement first")
    @pytest.mark.parametrize(("reg", "endianess", "expected"), [
        ("14", "BE",  "-4629812106207628676"),
        ("22", "MLE", "-4629812106207628676"),
        ("26", "MBE", "-4629812106207628676"),
        ("18", "LE",  "-4629812106207628676")
    ], ids=["BE", "MLE", "MBE", "LE"])
    def test_modbus_datatype_int64_64bit(self, host, modbus_func, reg, endianess, expected):
        key = "modbus_read[{},1,{},{},uint64,{}]".format(
            host, reg, modbus_func, endianess)
        assert zabbix_get(key) == expected

    # UNSIGNED INT64
    # Big Endian
    def test_modbus_datatype_uint64_64bit_default(self, host, modbus_func):
        key = "modbus_read[{},1,14,{},uint64]".format(host, modbus_func)
        assert zabbix_get(key) == '13816931967501922940'

    # FLOAT64(double)
    def test_modbus_datatype_double_64bit_default(self, host, modbus_func):
        key = "modbus_read[{},1,14,{},double]".format(host, modbus_func)
        assert zabbix_get(key) == '-0.123450'

    @pytest.mark.parametrize(("reg", "datatype", "endianess",  "expected"), [
        ("14", "uint64", "BE",   "13816931967501922940"),
        ("22", "uint64", "MLE",  "13816931967501922940"),
        ("26", "uint64", "MBE",  "13816931967501922940"),
        ("18", "uint64", "LE",   "13816931967501922940"),
        ("14", "double", "BE",   "-0.123450"),
        ("22", "double", "MLE",  "-0.123450"),
        ("26", "double", "MBE",  "-0.123450"),
        ("18", "double", "LE",   "-0.123450")
    ], ids=["uint64-BE", "uint64-MLE", "uint64-MBE", "uint64-LE", "double-BE", "double-MLE", "double-MBE", "double-LE"])
    def test_modbus_datatype_64bit(self, host, modbus_func, reg, datatype, endianess,  expected):
        key = "modbus_read[{},1,{},{},{},{}]".format(
            host, reg, modbus_func, datatype, endianess)
        assert zabbix_get(key) == expected
