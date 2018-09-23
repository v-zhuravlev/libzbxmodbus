import pytest
from zabbix_get import zabbix_get


class TestModbus64BitCase(object):

    host = "172.16.238.2:5020"

    # INT64(Signed) NOT IMLEMENTED
    # Big Endian
    @pytest.mark.skip("Implement first")
    def test_modbus_datatype_int64_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,int64]"
        assert zabbix_get(key) == '-4629812106207628676'
    # Mid-Little Endian

    @pytest.mark.skip("Implement first")
    def test_modbus_datatype_int64_64bit_mle_0(self):
        key = "modbus_read["+self.host+",1,22,3,int64,MLE]"
        assert zabbix_get(key) == '-4629812106207628676'

    # Little Endian
    @pytest.mark.skip("Implement first")
    def test_modbus_datatype_int64_64bit_le_0(self):
        key = "modbus_read["+self.host+",1,18,3,int64,LE]"
        assert zabbix_get(key) == '-4629812106207628676'

    # Mid-Big Endian
    @pytest.mark.skip("Implement first")
    def test_modbus_datatype_int64_64bit_mbe_0(self):
        key = "modbus_read["+self.host+",1,26,3,int64,MBE]"
        assert zabbix_get(key) == '-4629812106207628676'

    # UNSIGNED INT64
    # Big Endian
    def test_modbus_datatype_uint64_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,uint64]"
        assert zabbix_get(key) == '13816931967501922940'
    # Mid-Little Endian

    def test_modbus_datatype_uint64_64bit_mle_0(self):
        key = "modbus_read["+self.host+",1,22,3,uint64,MLE]"
        assert zabbix_get(key) == '13816931967501922940'

    # Little Endian
    def test_modbus_datatype_uint64_64bit_le_0(self):
        key = "modbus_read["+self.host+",1,18,3,uint64,LE]"
        assert zabbix_get(key) == '13816931967501922940'

    # Mid-Big Endian
    def test_modbus_datatype_uint64_64bit_mbe_0(self):
        key = "modbus_read["+self.host+",1,26,3,uint64,MBE]"
        assert zabbix_get(key) == '13816931967501922940'

    # FLOAT64(double)
    # Big Endian

    def test_modbus_datatype_double_64bit_be_0(self):
        key = "modbus_read["+self.host+",1,14,3,double]"
        assert zabbix_get(key) == '-0.123450'
    # Mid-Little Endian

    def test_modbus_datatype_double_64bit_mle_0(self):
        key = "modbus_read["+self.host+",1,22,3,double,MLE]"
        assert zabbix_get(key) == '-0.123450'

    # Little Endian
    def test_modbus_datatype_double_64bit_le_0(self):
        key = "modbus_read["+self.host+",1,18,3,double,LE]"
        assert zabbix_get(key) == '-0.123450'

    # Mid-Big Endian
    def test_modbus_datatype_double_64bit_mbe_0(self):
        key = "modbus_read["+self.host+",1,26,3,double,MBE]"
        assert zabbix_get(key) == '-0.123450'
