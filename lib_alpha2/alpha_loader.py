
#
# Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com
#
# This file is part of EigenD.
#
# EigenD is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# EigenD is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
#

import os
import sys
import time
import optparse
import picross
import ezload

FRM_UPDATE_MSG = '0x13'
FRM_COMMIT_MSG = '0x14'
ERASE_CAL_DATA_MSG = '0x12'
CAL_DATA_MSG = '0x10'
PADDED_BYTE_MSG = '0x00'
FRM_UPDATE_LEN = 268 #including header
FRM_COMMIT_LEN = 2
CAL_DATA_LEN = 260 #including header
ERASE_CAL_DATA_LEN = 2 #no payload
FRM_REC_EOF = 1
FRM_REC_EL = 4 #extended linear address record
VENDOR_ID = 0x2139
PRODUCT_ID_BSP = 0x0102
PRODUCT_ID_PSU = 0x0103
CAL_ERASE_DELAY = 5 #in seconds
FLASH_PAGE_DELAY = 0.08 #in seconds 

inst_types = {
    1: ('alpha',(1,7),'a2_logic_0107.mcs'),
    2: ('tau',(1,4),'t2m_logic_0104.mcs')
}

def find_LS_MS( dw ):
    LS = dw % 256
    MS = dw / 256
    
    return LS , MS 

class usb_interface():

    def __init__( self,device,ep ):
        self.end_point = ep
        picross.pic_set_interrupt()
        self.interface = picross.usbdevice( device, 0 )
        self.interface.start_pipes()
        self.interface.control_out(0x40,0xb1,0,0,'')
        time.sleep( 4 )

    def reset( self ):
        print "Reseting USB"

    def bulk_transfer( self,packet ):
        bytes = packet.get_buffer()        
        strpacket =  "".join( map(chr,bytes))
        self.interface.bulk_write( self.end_point,strpacket )

    def control(self,*args):
        return self.interface.control(*args)

    def control_in(self,*args):
        return self.interface.control_in(*args)

    def control_out(self,*args):
        return self.interface.control_out(*args)

class usb_packet():
    max_packet_size = 0
  
    def __init__( self , msg, size = 512 ):
        self.max_packet_size = size
        self.packet_buff = []
        self.add_msg( msg )    
            
    def add_frm_commit( self ):
        flash_page_cap = FRM_UPDATE_LEN - len( self.packet_buff )
        if flash_page_cap > 0:
            for x in range( 0, flash_page_cap ):
                self.packet_buff.append( 0 )

        self.packet_buff.append( int( FRM_COMMIT_MSG,16 ) )
        self.packet_buff.append( int( PADDED_BYTE_MSG,16 ) )
        
    def add_frm_update( self ):
        self.packet_buff.append( int( FRM_UPDATE_MSG,16 ) )
        self.packet_buff.append( int( PADDED_BYTE_MSG,16 ) )

    def add_page_dw( self,dw ):
        LS,MS = find_LS_MS( dw )
        self.packet_buff.append( LS )
        self.packet_buff.append( MS )

    def add_erase_cal_data( self ):
        self.packet_buff.append( int( ERASE_CAL_DATA_MSG,16 ) )
        self.packet_buff.append( int( PADDED_BYTE_MSG,16 ) )

    def add_cal_data( self ):
        self.packet_buff.append( int( CAL_DATA_MSG,16 ) )
        self.packet_buff.append( int ( PADDED_BYTE_MSG,16 ) )
        
    def add_msg( self, msg ):

        if msg == FRM_UPDATE_MSG:
            self.add_frm_update()
        elif msg == FRM_COMMIT_MSG:
            self.add_frm_commit()
        elif msg == ERASE_CAL_DATA_MSG:
            self.add_erase_cal_data()
        elif msg == CAL_DATA_MSG:
            self.add_cal_data()
            self.keynum = -1
        else:
            print "Invalid msg"
            
    def add_frm_update_pdu( self,data ): # return left over string
        cap = FRM_UPDATE_LEN - len( self.packet_buff ) 
        dataLen = len( data )
        
        if dataLen <= cap * 2:
            limit = dataLen
        else:
            limit = cap * 2 

        bytes = [ int(data[x:x+2],16) for x in range(0,limit,2) ]     
        self.packet_buff.extend( bytes ) 

        if limit == dataLen:
            return ""
        else:
            return data[limit:]

    def add_frm_commit_pdu( self,checksum ):
        max_dw_val = 65536
        dwchecksum = checksum % max_dw_val
        LS,MS = find_LS_MS( dwchecksum )
        self.packet_buff.append( LS )
        self.packet_buff.append( MS )
        return ""

    def add_cal_data_pdu( self,data ):
        data_list = data.split( " " )
        key = int( data_list.pop( 0 ) )
        if key != self.keynum:
            print "Data for KEY!" , key
            self.keynum = key
            LS,MS = find_LS_MS( key )
            self.packet_buff.append( LS )
            self.packet_buff.append( MS )

        data_list.pop( 0 )

        for x in range( 0,len(data_list) ):
            LS,MS = find_LS_MS( int(data_list[x]) )
            self.packet_buff.append( LS )
            self.packet_buff.append( MS )
            
    def add_payload( self,msg,data ): 
        if msg == FRM_UPDATE_MSG:
            return self.add_frm_update_pdu( data )
        elif msg == FRM_COMMIT_MSG:
            self.add_frm_commit_pdu( data )
        elif msg == CAL_DATA_MSG:
            self.add_cal_data_pdu( data )       
        else:
            print "Data belongs to invalid msg"

    def add_padding( self ):
        cap = self.max_packet_size - len( self.packet_buff ) 
        for i in range(0,cap):
            self.packet_buff.append( 0 )

    def get_size( self ):
        return len( self.packet_buff )
    
    def get_buffer( self ):
        return self.packet_buff


def alpha_download(usb,filename,progress=None):
    data = ""
    page_num = 0
    check_sum = 0

    filedata = open(filename).read().splitlines()
    filelen = len(filedata)
    packet = usb_packet( FRM_UPDATE_MSG )
    packet.add_page_dw( page_num )

    for index,line in enumerate(filedata):
        if progress:
            progress(index,filelen)

        rectype = int(line[7:9],16)

        if rectype == FRM_REC_EL:
            continue

        if rectype == FRM_REC_EOF:
            print "EOF is reached"
            packet.add_msg( FRM_COMMIT_MSG )
            packet.add_payload( FRM_COMMIT_MSG, check_sum )
            packet.add_padding()
            usb.bulk_transfer( packet )
            break

        hexdata = line[9:-2]

        for x in range(0,len(hexdata),4):
            first = int( hexdata[x:x+2],16 )           
            second = int( hexdata[x+2:x+4],16 )
            check_sum = check_sum + first + ( second * 256 )

        data = data + line[9:-2] 

        if packet.get_size() < FRM_UPDATE_LEN:
            data = packet.add_payload( FRM_UPDATE_MSG, data )
            continue

        packet.add_padding()
        usb.bulk_transfer( packet )
        time.sleep( FLASH_PAGE_DELAY )
        page_num = page_num + 1
        packet = usb_packet( FRM_UPDATE_MSG )
        packet.add_page_dw( page_num )
        data = packet.add_payload( FRM_UPDATE_MSG, data )

    time.sleep(1)

    if progress:
        progress(filelen,filelen)


def calibration_erase(usb):
    packet = usb_packet( ERASE_CAL_DATA_MSG )
    packet.add_padding()
    usb.bulk_transfer( packet )
    time.sleep( CAL_ERASE_DELAY )

def calibration_download(usb,filename): 
    filedata = open(filename).read() #read all data until EOF
    packet = usb_packet( CAL_DATA_MSG )

    for line in filedata.splitlines():
        if packet.get_size() < CAL_DATA_LEN:
            packet.add_payload( CAL_DATA_MSG,line )
        else:
            packet.add_padding()
            usb.bulk_transfer( packet )
            time.sleep( FLASH_PAGE_DELAY )                        
            packet = usb_packet( CAL_DATA_MSG )
            packet.add_payload( CAL_DATA_MSG,line )        
    
    packet.add_padding()
    usb.bulk_transfer( packet )
    time.sleep( FLASH_PAGE_DELAY )                        

def progress(a,b):
    if a<b:
        print a,b,'\r',
    else:
        print 'done'

    sys.stdout.flush()

def build_version(major,minor):
    return (major&0x07,minor)

def main():
    parser = optparse.OptionParser(usage=sys.argv[0]+' [options] [driver] [args]')

    parser.add_option('-f',action='store',dest='firmware',default=None,help='firmware file')
    parser.add_option('-c',action ='store',dest='calibration',default=None,help ='calibration data file')
    parser.add_option('-e',action ='store_true',dest='erase',default=False,help ='erase calibration data ')

    (opts,args) = parser.parse_args(sys.argv)

    base_name = picross.find2(ezload.vendor,ezload.bs_product,False)
    if base_name:
        print 'booting base station'
        ezload.download(base_name,ezload.bs_firmware())
        time.sleep(5)

    base_name = picross.find2(ezload.vendor,ezload.psu_product,False)
    if base_name:
        print 'booting base station'
        ezload.download(base_name,ezload.psu_firmware())
        time.sleep(5)

    dev_name = picross.find2(VENDOR_ID,PRODUCT_ID_BSP,False)
    if not dev_name:
        dev_name = picross.find2(VENDOR_ID,PRODUCT_ID_PSU,False)
        if not dev_name:
            print 'no base station or psu connected'
            return

    dev_usb = usb_interface(dev_name,4)
    inst_cfg = dev_usb.control_in(0x40|0x80,0xc6,0,0,64)
    inst_type = ord(inst_cfg[0])
    inst_ver = build_version(ord(inst_cfg[3]),ord(inst_cfg[2]))
    inst_ver_hw = ord(inst_cfg[1])

    inst = inst_types.get(inst_type)

    if not inst:
        print 'no recognised instrument connected'
        return

    print 'instrument: %s version %d firmware version: %s.%s latest firmware: %s.%s' % (inst[0],inst_ver_hw,inst_ver[0],inst_ver[1],inst[1][0],inst[1][1])

    if opts.calibration:
        calibration_download(dev_usb,opts.calibration);

    if opts.erase:
        calibration_erase(dev_usb)

    if opts.firmware:
        alpha_download(dev_usb,opts.firmware,progress);
        return

    if inst_ver < inst[1]:
        file = ezload.find_release_resource('firmware',inst[2])
        print 'downloading',file
        alpha_download(dev_usb,file,progress);
