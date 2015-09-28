AC_INIT([libzbxmodbus], [0.1])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/modbus.c])
# Checks for libraries.
have_modbus=no
AC_SEARCH_LIBS([modbus_get_float], [modbus], [have_modbus=yes])
if test "x${have_modbus}" = xno; then
AC_MSG_ERROR([
------------------------------------------
The modbus library and header file
required to build zbxmodbus. Stopping...
Check 'config.log' for more information.
------------------------------------------])
fi
PKG_CHECK_MODULES([LIBMODBUS], [libmodbus >= 3.1.1], [],
[AC_MSG_ERROR([
    ------------------------------------------
    libmodbus 3.1.1 or higher is required.
    Versions below that can cause serious performance issues with pollers
    because of bugged timeout handling. Please download and build latest
    libmodbus from http://libmodbus.org/
    ------------------------------------------])
])
LT_INIT
AC_DISABLE_STATIC([yes])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
