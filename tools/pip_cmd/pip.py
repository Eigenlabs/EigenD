#!/usr/bin/python
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


import sys
import re
import string
import types
import copy
import os.path

sys.path.append(os.path.dirname(os.path.dirname(__file__)))

from pip_cmd import process,expand,error

from error import PipError

def get_template():
    if '__loader__' in globals():
        d = os.path.join(os.path.dirname(__file__),'template')
        return __loader__.get_data(d)
    else:
        d = os.path.join(os.path.dirname(__file__),'template')
        return open(d).read()

def generate(module, ifile, ofile, path):
    tree=process.process(path,module,ifile)
    text=expand.expand(get_template(),tree)

    output=file(ofile,"w")
    output.write(text)
    output.close()

def cli():
    if len(sys.argv) < 4:
        print "usage: pip module in-file out-file [includes]"
        sys.exit(-1)

    try:
        generate(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4:])
        sys.exit(0)
    except PipError,e:
        print e

    sys.exit(-1)
        
if __name__ == '__main__':
    cli()
