import pytest
from zabbix_get import zabbix_get


class TestModbusShortCase(object):

    host = "172.16.238.2:5020"
    # test coils(bits)

    def test_modbus_datatype_short_bit_0(self):
        key = "modbus_read["+self.host+",1,1,1,b]"
        assert zabbix_get(key) == '1'

    # 16bit, all tests are PDU (start from 0 address)
    # INT16,Big Endian
    def test_modbus_datatype_short_signed_1(self):
        key = "modbus_read["+self.host+",1,1,3,s,1]"
        assert zabbix_get(key) == '-15859.000000'

    # UINT16,Big Endian, i
    def test_modbus_datatype_short_unsigned_2(self):
        key = "modbus_read["+self.host+",1,1,3,i,1]"
        assert zabbix_get(key) == '49677'

    # 32bit.
    # Float. In 32bit float we are looking for "-71.879005"
    # Big Endian (ABCD)
    def test_modbus_datatype_short_float_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,f,1]"
        assert zabbix_get(key) == '-71.879005'
    # Little Endian with Wordswap or Mid-Little Endian (CDAB)

    def test_modbus_datatype_short_float_32bit_mle_cdab_0(self):
        key = "modbus_read["+self.host+",1,4,3,f,0]"
        assert zabbix_get(key) == '-71.879005'
    # Big Endian with WordSwap or Mid-Big Endian (BADC)

    def test_modbus_datatype_short_float_32bit_mbe_badc_0(self):
        key = "modbus_read["+self.host+",1,2,3,f,2]"
        assert zabbix_get(key) == '-71.879005'
    # Little Endian (DCBA)

    def test_modbus_datatype_short_float_32bit_le_dcba_0(self):
        key = "modbus_read["+self.host+",1,6,3,f,3]"
        assert zabbix_get(key) == '-71.879005'

    # UINT32,unsigned,long:
    # Big Endian (ABCD)
    def test_modbus_datatype_short_long_32bit_be_abcd_0(self):
        key = "modbus_read["+self.host+",1,0,3,l,1]"
        assert zabbix_get(key) == '3264201229'

    # INT32(Signed)
    # Big Endian (ABCD)
    def test_modbus_datatype_short_int32_be_0(self):
        key = "modbus_read["+self.host+",1,12,3,S,1]"
        assert zabbix_get(key) == '-562.000000'

    # UNSIGNED INT64
    # Big Endian

    def test_modbus_datatype_short_uint64_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,I,1]"
        assert zabbix_get(key) == '13816931967501922940'

    # FLOAT64(double)
    # Big Endian
    def test_modbus_datatype_short_double_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,d,1]"
        assert zabbix_get(key) == '-0.123450'
