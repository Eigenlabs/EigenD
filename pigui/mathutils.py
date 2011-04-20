
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

import math,types

class vector:

    def __init__(self,p1,p2):
        self.p1=p1
        self.p2=p2
        self.delta=p2-p1
        
    def getP1(self):
        return self.p1

    def getP2(self):
        return self.p2
    
    def isHorizontal(self):
        return self.delta.getY()==0.0

    def isVertical(self):
        return self.delta.getX()==0.0 

    def getLength(self):
        return self.delta.getLength()
    
    def getUnitVector(self):
        len=self.getLength()
        return point(self.delta.getX()/len,self.delta.getY()/len)

    def getNormalUnitVector(self):
        len=self.getLength()
        return point(-1*self.delta.getY()/len,self.delta.getX()/len)

    def getX(self):
        return self.delta.getX()    
    
    def getY(self):
        return self.delta.getY()
        
    def getMidPoint(self):
        return self.getPoint(0.5)
    
    def getPoint(self,proportion):
        return self.p1+self.delta*proportion
        
    def isZero(self):
        return self.p1==self.p2

    def asTuple(self):
        return self.p1.asTuple()+self.p2.asTuple()

class point:

    def __init__(self,x,y):
        self.x=x
        self.y=y
    
    def __add__(self,other):
        return point(self.x+other.x,self.y+other.y)

    def __mul__(self,other):
        if isinstance(other,types.IntType) or isinstance(other,types.FloatType):
            return point(self.x*other,self.y*other)

    def __sub__(self,other):
        return point(self.x-other.x,self.y-other.y)
    
    def getX(self):
        return self.x
    
    def __eq__(self,other):
        return self.x==other.x and self.y ==other.y

    def getY(self):
        return self.y

    def getPosition(self):
        return (self.x,self.y)

    def getLength(self):
        return math.pow((self.x*self.x)+(self.y*self.y),0.5)
    
    def asTuple(self):
        return (self.getX(),self.getY())

class point_t(point):
    def __init__(self,pos):
        point.__init__(self,pos[0],pos[1])




