AC_INIT([libzbxmodbus], [0.4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/modbus.c])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
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

#Zabbix 2.4 / Zabbix 3.0 / 3.2 chooser
AC_ARG_ENABLE([zabbix-2],
[  --enable-zabbix-2    Build with Zabbix 2.4 headers],
[case "${enableval}" in
  yes) zabbix_2=true ;;
  no)  zabbix_2=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-zabbix-2]) ;;
esac],[zabbix_2=false])
AM_CONDITIONAL([ZABBIX_2], [test x$zabbix_2 = xtrue])

AC_ARG_ENABLE([zabbix-3],
[  --enable-zabbix-3    Build with Zabbix 3.0 headers],
[case "${enableval}" in
  yes) zabbix_3=true ;;
  no)  zabbix_3=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-zabbix-3]) ;;
esac],[zabbix_3=false])
AM_CONDITIONAL([ZABBIX_3], [test x$zabbix_3 = xtrue])

AC_ARG_ENABLE([zabbix-3.2],
[  --enable-zabbix-3.2    Build with Zabbix 3.2 headers],
[case "${enableval}" in
  yes) zabbix_3_2=true ;;
  no)  zabbix_3_2=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-zabbix-3.2]) ;;
esac],[zabbix_3_2=false])
AM_CONDITIONAL([ZABBIX_3_2], [test x$zabbix_3_2 = xtrue])

if [test "x${zabbix_2}" = xfalse && test "x${zabbix_3}" = xfalse && test "x${zabbix_3_2}" = xfalse] ; then
AC_MSG_ERROR([
------------------------------------------
Please specify the version of Zabbix module will be used with: by providing either
--enable-zabbix-2 or --enable-zabbix-3 or --enable-zabbix-3.2 key but no more than one.
------------------------------------------])
fi



LT_INIT
AC_DISABLE_STATIC([yes])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
