# libzbxmodbus
[![Build Status](https://travis-ci.org/v-zhuravlev/libzbxmodbus.svg?branch=master)](https://travis-ci.org/v-zhuravlev/libzbxmodbus)  
This is the [Loadable module](https://www.zabbix.com/documentation/4.0/manual/config/items/loadablemodules) that adds support for Modbus (TCP, RTU and "RTU over TCP" (encapsulated)) in Zabbix.

This module features:
- [libmodbus](https://github.com/stephane/libmodbus) as a core library
- Support of Modbus functions: `READ COILS`, `READ DISCRETE INPUT STATUS`, `READ HOLDING REGISTERS` and `READ INPUT REGISTERS`
- Support of 16bit, 32bit, 64bit datatypes like unsigned integers, signed integers, floats, and doubles.  
- Support of four endianess types for 32bit, 64bit datatypes: `Big Endian`, `Little Endian`, `Mid-Big Endian`, `Mid-Little Endian`
- **Bulk data collection support**. You can now get as many Modbus registers as needed with just a single command and return it as JSON object to Zabbix. Huge thanks to @i-ky for implementing it.
- Resource locking (via IPC semaphores). It is used when using RTU or Encapsulated Modbus. So two or more Zabbix pollers don't poll the same serial port at the same time. No locking is used for Modbus TCP.  

## 1. Install
Download sources from [here](https://share.zabbix.com/dir-libraries/zabbix-loadable-modules/modbus-loadable-module), then  do:
  - `tar zxvpf libzbxmodbus-0.9.tar.gz`
  - `cd libzbxmodbus-0.9`
  - `./configure --prefix=/etc/zabbix --enable-zabbix-[2|3|3.2]`
  - `make`
  - `make install`  

Note: If you want to install from Github sources, see hints [here](https://github.com/v-zhuravlev/libzbxmodbus/wiki/Install-from-Github-repo).

## 2. Load into Zabbix  
  - Module libzbxmodule.so can be loaded into zabbix_server, zabbix_proxy or zabbix_agent(use passive mode).  
    Depending on where you want to load your module edit appropriate Zabbix configuration file:
```
      LoadModulePath = /etc/zabbix/lib
      LoadModule = libzbxmodbus.so
```
  - Because of the resource locking and that networks might be slow to respond, it is also recommended to tune `Timeout` to 10 seconds or more:
```
      Timeout = 10
```  
  - (optional) If you plan to use Modbus RTU over serial port then add Zabbix user to dialout group to gain proper access to ports:  
      `usermod -a -G dialout zabbix`
  - Restart Zabbix daemon  
  
## 3. Configure Modbus Polling in Zabbix

  Configure the new item with the following type:  
  
* *Simple check* if the module is on zabbix_server or zabbix_proxy  
* *Zabbix Agent* or *Zabbix Agent (active)* if the module is on zabbix_agentd
  
  and then input the key:  
  
`modbus_read[<connection>,<slave_id>,<reg_to_read>,<modbus_function>[,<datatype_expression>][,<endiannes>][,<first_reg>]]`  

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
    or enter only the portname, defaults for the rest will be used:  
      `/dev/ttyS1 `  
    defaults are: *9600 N 8 1*  
    
* **slave_id:**  
    Modbus slave ID or Unit Identifier(for TCP)

* **reg_to_read:**  
    First Modbus register to read
    
* **modbus_function:**  
    Modbus function in integer form:  
    `1` - for READ COILS  
    `2` - for READ DISCRETE INPUT STATUS    
    `3` - for READ HOLDING REGISTERS  
    `4` - for READ INPUT REGISTERS  
    
* **datatype_expression (optional):**  
    
    Provide datatype:  
      `b` or `bit` - for MODBUS_BIT  
      `i` or `uint16` - for MODBUS_UINT16, 16bit (unsigned)  
      `s` or `int16` - for MODBUS_SIGNED_INT, 16bit (NOTE: in Zabbix use 'Type of information' Numeric (float))  
      `l` or `uint32` - for MODBUS_UINT32, 32bit (unsigned)  
      `S` or `int32` - for MODBUS_SIGNED_INT32, 32bit (NOTE: in Zabbix use 'Type of information' Numeric (float))  
      `f` or `float` - for MODBUS_FLOAT, 32bit  
      `I` or `uint64`- for MODBUS_UINT64, 64bit (unsigned) (NOTE: in Zabbix use 'Type of information' Numeric (unsigned))  
      `d` or `double`- for MODBUS_FLOAT64, 64bit  
    
    otherwise, defaults will be used:  
      MODBUS_BIT if modbus_function is `1` or `2`.  
      MODBUS_UINT16 if modbus_function is `3` or `4`.  

    Note: Datatypes can be combined in the datatype expression here to request more than one register at once. See [section (5)](#5-bulk-data-collection) below for how to do this.
    
    
  
* **endianness(optional):**   
    Modbus endianness for 32bit/64bit datatypes:  
      `0` or `MLE` - for MODBUS_MLE_CDAB (Mid-Little Endian (CDAB))  
      `1` or `BE` - for MODBUS_BE_ABCD (Big Endian (ABCD))  
      `2` or `MBE` - for MODBUS_MBE_BADC (Mid-Big Endian (BADC))  
      `3` or `LE` - for MODBUS_LE_DCBA (Little Endian (DCBA))  
    Default is BE(1). Normally, you don't need to change this.  
    
* **first_reg(optional):**  
    Modbus addressing scheme:  
      `0` - MODBUS_PDU_ADDRESSING is used. First data object has address 0. This is the default value.  
      `1` - MODBUS_PROTOCOL_ADDRESSING is used. First data object has an address of 1.  
      
    
**Example keys:**  
```
    modbus_read[/dev/ttyS0,32,4,3]
    modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},59,3,float,BE,0]
    modbus_read[{HOST.CONN},{$MODBUS_SLAVE},59,3,float,BE,0]
    modbus_read[/dev/ttyS0 9600 N,32,4,3,float,BE,0]
    modbus_read[192.168.1.1,1,6,1]
    modbus_read[192.168.1.1:514,1,5,1]
    modbus_read[{$MODBUS_PORT},32,4,3,uint32,BE,0]
    modbus_read[enc://192.168.1.1,1,6,1]
    modbus_read[tcp://192.168.1.1:5000,1,5,1]
    modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,1,100*bit]
    modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,1,10*double]
    modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,3,2*uint16+skip+float]
    modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,3,2*uint16+double+float*3,MLE]
```


## 5. Bulk Data Collection
It is now possible to get as many Modbus registers as needed with just a single command and return it as the JSON object. JSON object is then processed by Zabbix (version 3.4 or newer is required) and all single values routed to different dependent items.   

In order to get data in bulk, you first need to define which registers you want to retrieve using datatype expression. 

### 5.1. The Simplest Example
The simplest datatype expression would be 
`2*uint16` that you write in the key: `modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,3,2*uint16]`  
This expression is quite simple - it just retrieves two unsigned integers starting from register 10. Here is what you would get:
```json
{
  "10":123,
  "11":321
}
```
To process it by Zabbix, first create master item:
- Name: Modbus bulk request
- Key: `modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,3,2*uint16]`  
- Type: Simple check or Zabbix agent
- Type of information: Text  
- History: 0 (recommended)  
![image](https://user-images.githubusercontent.com/14870891/45901178-c3d03800-bdea-11e8-811e-f2fcee702beb.png)



Then, create two [dependent items](https://www.zabbix.com/documentation/4.0/manual/config/items/itemtypes/dependent_items):
The first item:  
- Key: register10    
- Type: Dependent item
- Master item: Modbus bulk request
- Type of information: Numeric (unsigned)
- In Preprocessing tab: add JSONPath step with parameter: `$.10`
![image](https://user-images.githubusercontent.com/14870891/45901353-507af600-bdeb-11e8-9350-c7a0c5ab8e7f.png)
![image](https://user-images.githubusercontent.com/14870891/45901388-67b9e380-bdeb-11e8-850c-e95c3bb4aa7e.png)



And the second item:  
- Key: register11    
- Type: Dependent item
- Master item: Modbus bulk request
- Type of information: Numeric (unsigned)
- In Preprocessing tab: add JSONPath step with parameter: `$.11`

Congratulations! You've just collected two metrics with the single Modbus command.  
Note: You can find examples how mass data collection works in Zabbix [here](https://blog.zabbix.com/zabbix-3-4-mass-data-collection-using-mercury-and-smartmontools-as-an-example/5784/).

### 5.2. Another Example
But datatype expressions can be more advanced. First, you can combine together different datatypes -   
`uint16+double+float`, and you can define how many times each datatype should be repeated - 
`2*uint16+double+float*3`. So in a key `modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,3,2*uint16+double+float*3]` you would get a JSON:
```json
{
  "10":123,
  "11":321,
  "12":0.12345,
  "16":100.10,
  "18":100.10,
  "20":100.10
}  
```
Note two things here. First, since `double` is 4 words long (64bit/16bit = 4), next request register address is actually `16`. Same applies to the following registers: since `float` datatype is 2 words long (32bit/16bit = 2) you would see next keys as `18` and `20`.

### 5.3. Example with skip
Modbus protocol supports only sequential read of registers. But what if you don't need all of them? For that, there is a special keyword `skip` you can use in order to retrieve multiple registers that are not located together. Example: Let's retrieve two `int16` from registers with addresses of `10` and `20`. Zabbix key 
`modbus_read[{$MODBUS_PORT},{$MODBUS_SLAVE},10,3,int16+10*skip+int16]` would give you:
```json
{
  "10":123,
  "20":-123
}
```
As you may noticed, `skip` is one word (16bit) long.  


    
## 6. Troubleshooting and Testing Modbus

Testing Modbus connectivity is easy with [*modpoll*](http://www.modbusdriver.com/modpoll.html) command utility.  
You may try to grab some Modbus registers with it before you try to do it with Zabbix.  


If you use libzbxmodbus with Zabbix agent, then you can also save time and test responses first with `zabbix_get`, for example:
`zabbix_get -s localhost -kmodbus_read["/dev/ttyS1 9600 N",9,0x1518,3,l,1,0]`
  
## 7. Known Issues and Limitations  
 - Any libmodbus error returned including CRC errors would lead to Unsupported Item in Zabbix, so minimizing 'Refresh unsupported items' parameter in Zabbix is recommended because CRC could be common in RS-485 env.  
 - If you have many different TCP gateways or serials ports in use then there are chances that Gate A will be locked while Gate B is being polled. This happens because the hash generated and assigned to the resource is too small and might not always be unique. That should not be an issue though.
 - This module doesn't work with the upstream libmodbus lib, so patched libmodbus v3.1.4 library is used underneath and shipped as a submodule. 
Two patches are applied:
    - Encapsulated Modbus RTU over TCP ([libmodbus PR](https://github.com/stephane/libmodbus/pull/385))
    - Fix for MODBUS_GET_* macro ([libmodbus PR](https://github.com/stephane/libmodbus/pull/441))  
- Also, note that if you already have a libmodbus library installed then patched libmodbus 3.1.4 inside this distribution might overwrite it.   
Once these PRs are accepted - [upstream libmodbus](https://github.com/stephane/libmodbus) can be used instead.  

 
 ## 8. Read More  
 Use case example in IoT project: https://www.zabbix.com/files/zabconf2017/fabrizio_fantoni-zabbix_in_iot_architecture.pdf
 More examples in Habrhabr article (RU): https://habrahabr.ru/company/zabbix/blog/268119/  
 About Modbus: http://www.ni.com/white-paper/52134/en/  
