#!/usr/bin/env python
"""
Pymodbus Synchronous Server Example
--------------------------------------------------------------------------

The synchronous server is implemented in pure python without any third
party libraries (unless you need to use the serial protocols which require
pyserial). This is helpful in constrained or old environments where using
twisted just is not feasable. What follows is an examle of its use:
"""
# --------------------------------------------------------------------------- # 
# import the various server implementations
# --------------------------------------------------------------------------- # 
from pymodbus.server.sync import StartTcpServer
from pymodbus.server.sync import StartUdpServer
from pymodbus.server.sync import StartSerialServer

from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock, ModbusSparseDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

from pymodbus.transaction import ModbusRtuFramer, ModbusBinaryFramer
# --------------------------------------------------------------------------- # 
# configure the service logging
# --------------------------------------------------------------------------- # 
import logging
FORMAT = ('%(asctime)-15s %(threadName)-15s'
          ' %(levelname)-8s %(module)-15s:%(lineno)-8s %(message)s')
logging.basicConfig(format=FORMAT)
log = logging.getLogger()
log.setLevel(logging.DEBUG)


def run_server(modbus_type,port):
    # ----------------------------------------------------------------------- # 
    # initialize your data store
    # ----------------------------------------------------------------------- # 
    # The datastores only respond to the addresses that they are initialized to
    # Therefore, if you initialize a DataBlock to addresses of 0x00 to 0xFF, a
    # request to 0x100 will respond with an invalid address exception. This is
    # because many devices exhibit this kind of behavior (but not all)::
    #
    #     block = ModbusSequentialDataBlock(0x00, [0]*0xff)
    #
    # Continuing, you can choose to use a sequential or a sparse DataBlock in
    # your data context.  The difference is that the sequential has no gaps in
    # the data while the sparse can. Once again, there are devices that exhibit
    # both forms of behavior::
    #
    #     block = ModbusSparseDataBlock({0x00: 0, 0x05: 1})
    #     block = ModbusSequentialDataBlock(0x00, [0]*5)
    #
    # Alternately, you can use the factory methods to initialize the DataBlocks
    # or simply do not pass them to have them initialized to 0x00 on the full
    # address range::
    #
    #     store = ModbusSlaveContext(di = ModbusSequentialDataBlock.create())
    #     store = ModbusSlaveContext()
    #
    # Finally, you are allowed to use the same DataBlock reference for every
    # table or you you may use a seperate DataBlock for each table.
    # This depends if you would like functions to be able to access and modify
    # the same data or not::
    #
    #     block = ModbusSequentialDataBlock(0x00, [0]*0xff)
    #     store = ModbusSlaveContext(di=block, co=block, hr=block, ir=block)
    #
    # The server then makes use of a server context that allows the server to
    # respond with different slave contexts for different unit ids. By default
    # it will return the same context for every unit id supplied (broadcast
    # mode).
    # However, this can be overloaded by setting the single flag to False and
    # then supplying a dictionary of unit id to context mapping::
    #
    #     slaves  = {
    #         0x01: ModbusSlaveContext(...),
    #         0x02: ModbusSlaveContext(...),
    #         0x03: ModbusSlaveContext(...),
    #     }
    #     context = ModbusServerContext(slaves=slaves, single=False)
    #
    # The slave context can also be initialized in zero_mode which means that a
    # request to address(0-7) will map to the address (0-7). The default is
    # False which is based on section 4.4 of the specification, so address(0-7)
    # will map to (1-8)::
    #
    #     store = ModbusSlaveContext(..., zero_mode=True)
    # ----------------------------------------------------------------------- # 
    
    #block = ModbusSequentialDataBlock(0x00, [16]*0xff)
    coils_block = ModbusSparseDataBlock({
        1: 0,
        2: 1,
        3: 0,
        4: 1
    })

        3:0x8FC2,#PDU2 - MBE
        4:0x0DC2,

        5:0xC20D,#PDU4 - MLE
        6:0xC28F,

        7:0x0DC2,#PDU6 - LE
        8:0x8FC2,
        #
        9:0xC28F,#PDU8 - same as (PDU0 - BE)
        10:0xC20D,

        11:0xC28F,#PDU0 - same as (PDU0 - BE)
        12:0xC20D,

        # test int32
        13:0xFFFF,#PDU12
        14:0xFDCE,
        
        # this block is to test 64bits
        #BE
        15:0xBFBF,#PDU14
        16:0x9A6B,
        17:0x50B0,
        18:0xF27C,

        #LE
        19:0x7CF2,#PDU18
        20:0xB050,
        21:0x6B9A,
        22:0xBFBF,

        #MLE
        23:0xF27C,#PDU22
        24:0x50B0,
        25:0x9A6B,
        26:0xBFBF,

        #MBE
        27:0xBFBF,#PDU26
        28:0x6B9A,
        29:0xB050,
        30:0x7CF2
    })
    store = ModbusSlaveContext(
        di=coils_block, co=coils_block, hr=registers_block, ir=registers_block)

    context = ModbusServerContext(slaves=store, single=True)
    
    # ----------------------------------------------------------------------- # 
    # initialize the server information
    # ----------------------------------------------------------------------- # 
    # If you don't set this or any fields, they are defaulted to empty strings.
    # ----------------------------------------------------------------------- # 
    identity = ModbusDeviceIdentification()
    identity.VendorName = 'Pymodbus'
    identity.ProductCode = 'PM'
    identity.VendorUrl = 'http://github.com/riptideio/pymodbus/'
    identity.ProductName = 'Pymodbus Server'
    identity.ModelName = 'Pymodbus Server'
    identity.MajorMinorRevision = '1.5'

    # ----------------------------------------------------------------------- #
    # run the server you want
    # ----------------------------------------------------------------------- # 
    # Tcp:
    if (modbus_type == 'TCP'):
        StartTcpServer(context, identity=identity, address=("0.0.0.0", port))
    elif (modbus_type =='TCP_RTU'):
        # TCP with different framer
        StartTcpServer(context, identity=identity,
                    framer=ModbusRtuFramer, address=("0.0.0.0", port))


    

    # Udp:
    # StartUdpServer(context, identity=identity, address=("0.0.0.0", 5020))
    
    # Ascii:
    # StartSerialServer(context, identity=identity,
    #                    port='/dev/ttyp0', timeout=1)
    
    # RTU:
    # StartSerialServer(context, framer=ModbusRtuFramer, identity=identity,
    #                   port='/dev/ttyp0', timeout=.005, baudrate=9600)

    # Binary
    # StartSerialServer(context,
    #                   identity=identity,
    #                   framer=ModbusBinaryFramer,
    #                   port='/dev/ttyp0',
    #                   timeout=1)

from multiprocessing import Process

if __name__ == "__main__":

    p = Process(target=run_server,args=('TCP',5020))
    p.start()
    #add RTU over TCP mode for tests
    p2 = Process(target=run_server,args=('TCP_RTU',5021))
    p2.start()