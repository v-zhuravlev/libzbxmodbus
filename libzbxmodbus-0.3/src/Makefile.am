lib_LTLIBRARIES = libzbxmodbus.la
libzbxmodbus_la_SOURCES = \
	modbus.c
libzbxmodbus_la_CFLAGS = $(LIBMODBUS_CFLAGS)

if ZABBIX_2
libzbxmodbus_la_CFLAGS += -I../include/zabbix-2.4
endif
if ZABBIX_3
libzbxmodbus_la_CFLAGS += -I../include/zabbix-3.0
endif

libzbxmodbus_la_LDFLAGS = $(LIBMODBUS_LIBS) \
    -module \
    -shared \
    -avoid-version
