# libzbxmodbus
[![Build Status](https://travis-ci.org/v-zhuravlev/libzbxmodbus.svg?branch=master)](https://travis-ci.org/v-zhuravlev/libzbxmodbus)  
Loadable module to integrate Modbus (RTU and TCP) protocol into Zabbix  
'Encapsulated Modbus' (RTU over TCP) also supported since 0.6).  

Patched libmodbus 3.1.4 library is used underneath and shipped as nested module.  
Two patches are applied:
- Encapsulated Modbus RTU over TCP ([libmodbus PR](https://github.com/stephane/libmodbus/pull/385))
- Fix for MODBUS_GET_* macro ([libmodbus PR](https://github.com/stephane/libmodbus/pull/441))  

Once these PRs are accepted - [upstream libmodbus](https://github.com/stephane/libmodbus) can be used instead.  

Resource locking (via IPC semaphores) is used when using RTU or Encapsulated Modbus. So two or more zabbix pollers don't poll same serial port at the same time. No locking is used for Modbus TCP.  

## 1.A Install from Github  
  - Tools required:
    - autoconf
    - automake
    - libtool
    - pkg-config
  - `git clone --recursive https://github.com/v-zhuravlev/libzbxmodbus.git`
  - Run `autogen.sh`
  - `./configure --prefix=/etc/zabbix --enable-zabbix-[2|3|3.2]`
  - `make`
  - `make install`  
  
## 1.B Install from tar.gz
Download from  https://share.zabbix.com/dir-libraries/zabbix-loadable-modules/modbus-loadable-module
  - `tar zxvpf libzbxmodbus-0.7.tar.gz`
  - `cd libzbxmodbus-0.7`
  - `./configure --prefix=/etc/zabbix --enable-zabbix-[2|3|3.2]`
  - `make`
  - `make install`

## 2.Integrate into Zabbix  
  - Module libzbxmodule.so can be loaded into zabbix_server, zabbix_proxy or zabbix_agent(use passive mode).  
    Depending where you want to load you module edit appropriate zabbix configuration file:
```
      LoadModulePath = /etc/zabbix/lib
      LoadModule = libzbxmodbus.so
```
  - Because of resource locking and that networks might be slow to respond, it is also recommended to tune `Timeout` to 10 seconds or more:
```
      Timeout = 10
```  
  - (optional) If you plan to use Modbus RTU over serial port then add zabbix user to dialout group to gain proper access to ports:  
      `usermod -a -G dialout zabbix`
  - Restart Zabbix daemon  
  
## 3. Configure Modbus polling in Zabbix
  Configure the new item with the following key:
  
`modbus_read[<connection>,<slave_id>, <reg_to_read>, <modbus_function>, [<datatype>],[<endiannes>],[<first_reg>]]`  

where:  

* **connection:**  
    for Modbus TCP: 
    IPv4 of Modbus TCP client/gate, for example: `192.168.1.1`  
    you may also use this form: `tcp://192.168.1.1`  
    TCP port may also be redefined (from Modbus TCP default 502) if needed: `192.168.1.1:5000` 
    
    for Modbus Encapsulated (RTU over TCP): 
    IPv4 of Modbus gate, for example: `enc://192.168.1.1`  
    TCP port may also be redefined (from Modbus default 502) if needed: `enc://192.168.1.1:5000`
    
    Note: DNS names are not supported for TCP and RTU over TCP
    
      
    for Modbus RTU over serial:
    Serial connection parameters in a form of:  
      `portname [baudrate] [parity:N|E|O] [databits] [stopbits]`  
    for example  
     `/dev/ttyS0 9600 N 8 2`  
     `/dev/ttyUSB0 115200 E 8 1`  
    or simply use portname, then defaults for the rest will be used:  
      `/dev/ttyS1 `  
    defaults are: *9600 N 8 1*  
    
* **slave_id:**  
    Modbus slave ID or Unit Identifier(for TCP)

* **reg_to_read:**  
    First modbus register to read
    
* **modbus_function:**  
    Modbus function in integer form:  
    1 - for READ COILS  
    2 - for READ DISCRETE INPUT STATUS    
    3 - for READ HOLDING REGISTERS  
    4 - for READ INPUT REGISTERS  
    
and some optional params can be provided as well:  
  
* **datatype(optional):**  
    provide datatype as single char:  
      `b` - for MODBUS_BIT  
      `i` - for MODBUS_INTEGER (unsigned)  
      `s` - for MODBUS_SIGNED_INT (NOTE: in Zabbix use 'Type of information' Numeric(float) )  
      `l` - for MODBUS_LONG, 32bit (unsigned)
      `f` - for MODBUS_FLOAT, 32bit  
      
    There is also experimental support added for 64bit Modbus datatypes:  
      `S` - for MODBUS_SIGNED_INT64 (NOTE: in Zabbix use 'Type of information' Numeric(float) )  
      `I` - for MODBUS_INT64 (unsigned) (NOTE: in Zabbix use 'Type of information' Numeric(unsigned) )  
      `d` - for MODBUS_FLOAT64, 64 bit  
    
    otherwise, defaults will be used:  
      MODBUS_BIT if modbus function 1 or 2.  
      MODBUS_INTEGER if modbus_function 3 or 4.  
    
    Please also note Zabbix datatypes constraints when working with 64bit:  
    https://www.zabbix.com/documentation/3.4/manual/config/items  
  
* **endianness(optional):**   
    Modbus endianness for 32bit/64bit datatypes:  
      0 - for MODBUS_MLE_CDAB (Mid-Little Endian (CDAB))  
      1 - for MODBUS_BE_ABCD (Big Endian (ABCD))  
      2 - for MODBUS_MBE_BADC (Mid-Big Endian (BADC))  
      3 - for MODBUS_LE_DCBA (Little Endian (DCBA))  
    default is BE(1)  
    
* **first_reg(optional):**  
    Modbus addressing scheme  
      0 - MODBUS_PDU_ADDRESSING is used. First data object has address 0. This is default value.  
      1 - MODBUS_PROTOCOL_ADDRESSING is used. First data object has address of 1.  
      
    
**Example keys:**  
```
    modbus_read[/dev/ttyS0,32,4,3]
    modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},59,3,f,1,0]
    modbus_read[{HOST.CONN},{$MODBUS_SLAVE},59,3,f,1,0]
    modbus_read[/dev/ttyS0 9600 N,32,4,3,f,1,0]
    modbus_read[192.168.1.1,1,6,1]
    modbus_read[192.168.1.1:514,1,5,1]
    modbus_read[{$MODBUS_PORT},32,4,3,f,1,0]
    modbus_read[enc://192.168.1.1,1,6,1]
    modbus_read[tcp://192.168.1.1:5000,1,5,1]
```
  
**Additional item parameters:**  
Item type:  
  
* *Simple check* if you load module on zabbix_server or zabbix_proxy  
* *Zabbix Agent* or *Zabbix Agent(active)* if you load module on zabbix_agent  

  
    
## 4. Troubleshooting and Testing Modbus

Testing Modbus connectivity is easy with *modpoll* command utility.  
You may try to grab some modbus registers with it before you try to do it with Zabbix.  
http://www.modbusdriver.com/modpoll.html

If using libzbxmodbus with Zabbix agent, you can also save time and test responses first with `zabbix_get`, for example:
`zabbix_get -s localhost -kmodbus_read["/dev/ttyS1 9600 N",9,0x1518,3,l,1,0]`
  
## 5. Known Issues and Limitations  
 - Any libmodbus error returned including CRC errors would lead to Unsupported Item in Zabbix, so minimizing 'Refresh unsupported items' parameter in Zabbix is recommended because CRC could be common in RS-485 env.  
 - If you have many different TCP gateways or serials ports in use then there are chances that Gate A will be locked while Gate B is being polled. This happens because the hash generated and assigned to resource is too small and might not always be unique. That should not be an issue though.
 - If you already have libmodbus library installed then patched libmodbus 3.1.4 inside this ditribution might overwrite it.  
 
 ## 6. Read more  
 Use case example in IoT project: https://www.zabbix.com/files/zabconf2017/fabrizio_fantoni-zabbix_in_iot_architecture.pdf
 More examples in Habrhabr article (RU): https://habrahabr.ru/company/zabbix/blog/268119/
 
