FROM zabbix/zabbix-agent:ubuntu-trunk

RUN apt-get update -q=2
RUN apt-get install autoconf automake libtool pkg-config -q=2
COPY . ./
RUN ./autogen.sh

RUN ./configure --enable-zabbix-3.2
WORKDIR ./libmodbus
# RUN ./configure 
# WORKDIR .
RUN make && make install
#force LLD cache refresh
RUN ldconfig

ENV ZBX_SERVER_HOST=0.0.0.0/0
ENV ZBX_LOADMODULE=libzbxmodbus.so

#since parent image has VOLUME for modules predefined, it is necessary to load libzbxmodule.so from outside using:
#-v "${pwd}/lib:/var/lib/zabbix/modules"

EXPOSE 10050