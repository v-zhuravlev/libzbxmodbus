lib_LTLIBRARIES = libzbxmodbus.la

libzbxmodbus_la_SOURCES = \
	modbus.c
libzbxmodbus_la_CFLAGS = $(LIBMODBUS_CFLAGS)
libzbxmodbus_la_LDFLAGS = $(LIBMODBUS_LIBS) \
    -module \
    -shared \
    -avoid-version