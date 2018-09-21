ARG ZBX_VERSION=3.4.12
FROM zabbix/zabbix-agent:ubuntu-${ZBX_VERSION}

RUN apt-get update -q=2
RUN apt-get install make autoconf automake libtool pkg-config -q=2
COPY ./libmodbus .

#only libmodbus(without libzbxmodbus) is installed
RUN ./configure -q

RUN make && make install
#force LLD cache refresh
RUN ldconfig

ENV ZBX_SERVER_HOST=127.0.0.1
ENV ZBX_LOADMODULE=libzbxmodbus.so
ENV ZBX_DEBUGLEVEL=4

EXPOSE 10050