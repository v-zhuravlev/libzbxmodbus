# libzbxmodbus
Loadable module to integrate Modbus (RTU and TCP) protocol into Zabbix  
libmodbus.org library is used in this integration of version 3.1.1 or higher.  
Your Linux distro may contain lower version and this won't work.  

RTU version also features:
  - uses serial ports (/dev/tty*). As Zabbix forked pollers tend to use it simultaneously, IPC semaphores are used.

## 1. Download and install libmodbus first  
  - wget http://libmodbus.org/releases/libmodbus-3.1.2.tar.gz
  - `tar zxvpf libmodbus-3.1.2.tar.gz`
  - `cd libmodbus-3.1.2`
  - `./configure`
  - `make`
  - `make install`
  - (if /usr/local/lib is not included in ld.so.conf.d/ (eg Centos6) ):  
      `echo '/usr/local/lib'>> /etc/ld.so.conf`  
  - `ldconfig`  
  - (if /usr/local/lib/pkgconfig is not included in pkg-config default path):  
`export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig`
  
## 2.A Install from github sources  
  - Tools required:
    - autoconf
    - automake
    - libtool
    - pkg-config
  - Prereq: Download and install libmodbus
  - Run `autogen.sh`
  - `./configure --enable-zabbix-3` or `./configure --enable-zabbix-2`
  - `make`
  - `make install`  
  
## 2.B Install from tar.gz sources  
Download from  https://share.zabbix.com/dir-libraries/zabbix-loadable-modules/modbus-loadable-module
  - `tar zxvpf libzbxmodbus-0.1.tar.gz`
  - `cd libzbxmodbus-0.1`
  - `./configure --enable-zabbix-3` or `./configure --enable-zabbix-2`
  - `make`
  - `make install`

## 3.Integrate into Zabbix  
  - Module libzbxmodule.so can be loaded into zabbix_server, zabbix_proxy or zabbix_agent(use passive mode).  
    Depending where you want to load you module edit appropriate zabbix configuration file:
```
      LoadModulePath = /usr/local/lib
      LoadModule = libzbxmodbus.so
```
  - (optional) If you plan to use Modbus RTU over serial port then add zabbix user to dialout group to gain proper access to ports:  
      `usermod -a -G dialout zabbix`
  - Restart Zabbix daemon  
  
##  4. Configure Modbus polling in Zabbix
  Configure the new item with the following key:
  
`modbus_read_registers[<connection>,<slave_id>, <reg_to_read>, <modbus_function>, [<datatype>],[<endiannes>],[<first_reg>]]`  

where:  

* **connection:**  
    IPv4 of Modbus client, for example: 192.168.1.1  
      or  
    Serial connection parameters in a form of:  
      `portname [baudrate] [parity:N|E|O] [databits] [stopbits]`  
    for example  
     `/dev/ttyS0 9600 N 8 2`  
     `/dev/ttyUSB0 115200 E 8 1`  
    or simply use portname, then defaults for the rest will be used:  
      `/dev/ttyS1 `  
    defaults are: *9600 N 8 1*  
    
* **slave_id:**  
    Modbus slave ID  

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
      b - for MODBUS_BIT  
      i - for MODBUS_INTEGER (unsigned)  
      s - for MODBUS_SIGNED_INT (NOTE: in Zabbix use 'Type of information' Numeric(float) )  
      l - for MODBUS_LONG  
      f - for MODBUS_FLOAT  
    otherwise, defaults will be used:  
      MODBUS_BIT if modbus function 1 or 2.  
      MODBUS_INTEGER if modbus_function 3 or 4.  
  
* **endianness(optional):**   
    Modbus endianness for long and float 32bit datatypes:  
      0 - for MODBUS_16BIT_LE (16 bit little endian)  
      1 - for MODBUS_16BIT_BE (16 bit big endian)  
    default is LE  
    
* **first_reg(optional):**  
    Modbus addressing scheme  
      0 - MODBUS_PDU_ADDRESSING is used. First data object has address 0. This is default value.  
      1 - MODBUS_PROTOCOL_ADDRESSING is used. First data object has address of 1.  
      
    
**Example keys:**  
```
    modbus_read_registers[{$MODBUS_PORT},{$MODBUS_SLAVE},59,3,f,1,0]
    modbus_read_registers[{HOST.CONN},{$MODBUS_SLAVE},59,3,f,1,0]
    modbus_read_registers[/dev/ttyS0 9600 N,32,4,3,f,1,0]
    modbus_read_registers[192.168.1.1,1,6,1]
    modbus_read_registers[192.168.1.1,1,5,1]
    modbus_read_registers[{$MODBUS_PORT},32,4,3,f,1,0]
```
  
**Additional item parameters:**  
Item type:  
  
* *Simple check* if you load module on zabbix_server or zabbix_proxy  
* *Zabbix Agent* or *Zabbix Agent(active)* if you load module on zabbix_agent  

  
    
## 5. Troubleshooting and Testing Modbus

Testing Modbus connectivity is easy with *modpoll* command utility.  
You may try to grab some modbus registers with it before you try to do it with Zabbix.  
http://www.modbusdriver.com/modpoll.html
  
## 6. Known Issues and Limitations  
 - a) Semaphoreset is mapped to path '.' so this module could possible lead to the conflict with other applications using semaphores same reckless way  
 - b) Because of *a)* If module is loaded to the Zabbix server and Zabbix agent on the same host - locking won't work properly  
 - c) Any libmodbus error returned including CRC would lead to Unsupported Item in Zabbix, so minimizing 'Refresh unsupported items' parameter in Zabbix is recommended because CRC could be common in RS-485 env.  
 - d) Single semaphoreset is created no matter how many physical ports are used. So if /dev/ttyS0 is in use by the module then another Zabbix poller cannot use /dev/ttyS1 simultaneously - it has to wait for unlock of another poller which is working with /dev/ttyS0.  
