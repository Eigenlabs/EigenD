
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

import picross
import micro_native
import sys
import time

reg_config = 0
reg_odac = 1
reg_otcdac = 2
reg_fsodac = 3
reg_fsotcdac = 4

alg_out = 0
alg_bdr = 1
alg_isrc = 2
alg_vdd = 3
alg_vss = 4
alg_bias5u = 5
alg_agnd = 6
alg_fsodac = 7
alg_fsotcdac = 8
alg_odac = 9
alg_otcdac = 10
alg_vref = 11
alg_vptatp = 12
alg_vptatm = 13
alg_inp = 14
alg_inm = 15

class Upot:
    def __init__(self):
        self.dev = picross.usbdevice(picross.find(0xbeca,0x0100),0)
        self.dev.control(0x40,0xb1,0,0)

    def train(self):
        self.dev.control(0x40,0xb2,0xff,0)
        self.dev.control(0x40,0xb2,0x01,0)

    def send(self,reg,data):
        d = (((data & 0x0f)<<4)|(reg &0x0f))
        self.dev.control(0x40,0xb2,d,0)
        
    def sendrecv(self,reg,data):
        d = (((data & 0x0f)<<4)|(reg &0x0f))
        r = self.dev.control_in(0x80|0x40,0xb3,d,0,1)
        return ord(r[0])

    def eeprom_write_word(self,addr,index,data):
        self.eeprom_write(addr+2*index,data&0xff)
        self.eeprom_write(addr+2*index+1,(data>>8)&0xff)

    def eeprom_read_word(self,addr,index):
        lo = self.eeprom_read(addr+2*index)
        hi = self.eeprom_read(addr+2*index+1)
        return (hi<<8)|lo

    def eeprom_write(self,addr,data):
        self.send(0,(data)&0x0f)
        self.send(1,(data>>4)&0x0f)
        self.send(6,(addr)&0x0f)
        self.send(7,(addr>>4)&0x0f)
        self.send(8,(addr>>8)&0x03)
        self.send(9,1)

    def eeprom_read(self,addr):
        self.send(6,(addr)&0x0f)
        self.send(7,(addr>>4)&0x0f)
        self.send(8,(addr>>8)&0x03)
        self.send(9,4)
        self.send(8,0)
        return self.sendrecv(9,5)

    def eeprom_erase_page(self,addr):
        self.send(6,(addr)&0x0f)
        self.send(7,(addr>>4)&0x0f)
        self.send(8,(addr>>8)&0x03)
        self.send(9,7)
        time.sleep(0.01)

    def eeprom_erase(self):
        self.send(9,2)
        time.sleep(0.01)

    def read_register(self,reg):
        self.send(6,(reg)&0x0f)
        self.send(9,3)
        self.send(8,0)
        lo = self.sendrecv(9,5)
        self.send(8,1)
        hi = self.sendrecv(9,5)
        return (hi<<8)|lo

    def write_odac_table(self,index,data):
        self.eeprom_write_word(0x00,index,data)

    def write_fsodac_table(self,index,data):
        if index < 0x80:
            self.eeprom_write_word(0x200,index,data)
        else:
            self.eeprom_write_word(0x1a0,index-0x80,data)

    def write_config(self,data):
        self.eeprom_write_word(0x160,0,data)

    def write_otcdac(self,data):
        self.eeprom_write_word(0x164,0,data)

    def write_fsotcdac(self,data):
        self.eeprom_write_word(0x168,0,data)

    def write_control(self,data):
        self.eeprom_write_word(0x16a,0,data)

    def write_register(self,reg,data):
        self.send(0,(data)&0x0f)
        self.send(1,(data>>4)&0x0f)
        self.send(2,(data>>8)&0x0f)
        self.send(3,(data>>12)&0x0f)
        self.send(6,(reg)&0x0f)
        self.send(9,0)

    def set_output(self,which):
        self.send(10,15)
        self.send(11,which)
        time.sleep(0.5)

    def set_iro(self, value):
        config = self.read_register(reg_config)
        config &= ~(0x07 << 6)
        config &= ~(0x01 << 9)
        if value >= 0:
            value = -value
            config |= (0x01 << 9)
        config |= ((value&0x07) << 6)
        self.write_register(reg_config,config)

    def set_pga(self,value):
        config = self.read_register(reg_config)
        config &= ~(0x0f << 2)
        config |= ((value&0x0f) << 2)
        self.write_register(reg_config,config)

    def set_odacsign(self,value):
        config = self.read_register(reg_config)
        config &= ~(0x01 << 1)
        if value >= 0:
            config |= (0x01 << 1)
        self.write_register(reg_config,config)

    def set_otcdacsign(self,value):
        config = self.read_register(reg_config)
        config &= ~(0x01 << 0)
        if value >= 0:
            config |= (0x01 << 0)
        self.write_register(reg_config,config)

    def set_odac(self,value):
        self.write_register(reg_odac,value)

    def set_otcdac(self,value):
        self.write_register(reg_otcdac,value)

    def set_fsodac(self,value):
        self.write_register(reg_fsodac,value)

    def set_fsotcdac(self,value):
        self.write_register(reg_fsotcdac,value)

    def get_temp(self):
        self.send(8,7)
        return self.sendrecv(9,5)

    def temp2celcius(self,ival):
        return (float(ival)-47.58)/0.69

    def celcius2temp(self,cval):
        return int(0.69*float(cval)+47.58)

def test():
    p = Upot()
    p.train()
    p.send(8,10)
    if p.sendrecv(9,5) != 0xca:
        raise RuntimeError('communication error')
    p.set_fsodac(0xffff)
    p.set_fsotcdac(0)
    p.set_otcdac(0x1400)
    p.set_odac(0)
    p.set_otcdacsign(1)
    p.set_iro(7)
    p.set_pga(15)
    p.set_output(alg_out)

    print 'temp is',p.temp2celcius(p.get_temp())
    print 'config',hex(p.read_register(reg_config))
