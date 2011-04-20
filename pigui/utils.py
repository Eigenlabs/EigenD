
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

import wx
name_part=1
details_part=2
icon_part=3

CON_ID=0
CON_RIGID=1
CON_DISPLAYNAME=2
CON_INPUT_SET=3

IO_ID=0
IO_NAME=1
IO_CONFROM=2
IO_CONTO=3


class CountObj:
    def __init__(self,name):
        self.name=name
        self.count=1
    def increment(self):
        self.count=self.count+1

    def decrement(self):
        self.count=self.count-1
    
def isTopLevel(id):
    return not '#' in id

def parentId(id):
    return id.split('#')[0]
 
def iconHandler(image):
    if image:
        input=open(image,'r')
        s=input.read()
        input.close()
        if len(s)>0:
#            print 'utils.iconHandler',image
            img=wx.Image(image)
            return img.ConvertToBitmap()
        else:
            print 'utils.iconHandler: image file length <=0'
        return None

def getDigitIndex(s):
    for i in range(len(s)):
        if s[i].isdigit():
            return i
    return -1
    
def presort(str):
    str=stripSpaces(str)
    i=getDigitIndex(str)
    numPart=str[i:]       
    namePart=str[:i]
    numPart='000000'+numPart
    return namePart+numPart[-4:]

def sortAgentNames(a,b):
    return cmp(presort(a),presort(b))

def stripSpaces(s):
    return s.replace(' ','')

def stripPrompt(prompt,str):
    if str[:len(prompt)]==prompt:
        return str[len(prompt):]
    else:
        return str

def chopString(str,dc,maxWidth,choppedList,tolerance=0,splitAtSpaces=False):
    i=getNumDisplayChars(str,dc,maxWidth)
    j=getNumDisplayChars(str,dc,maxWidth+tolerance)
    numExtraChars=j-i
    choppedOffBit=str[:i]
    leftOverBit=str[i:]
    
    if splitAtSpaces:
        spacePos=leftOverBit.find(' ')
        if spacePos!=-1 and spacePos<numExtraChars:
            i=i+(spacePos+1)
            numExtraChars=numExtraChars-(spacePos+1)

            choppedOffBit=str[:i]
            leftOverBit=str[i:]
 
    if len(leftOverBit)<=numExtraChars:
        choppedList.append(str)
    else:
        choppedList.append(choppedOffBit)
        if len(str)>i:
            chopString(leftOverBit,dc,maxWidth,choppedList,tolerance,splitAtSpaces)    

def chopString2(str,dc,maxWidth,choppedList,min=0,splitAtSpaces=False):
    i=getNumDisplayChars(str,dc,maxWidth)
   
    if len(str)<=i:
        choppedList.append(str)
    else:
        choppedOffBit=str[:i]
        leftOverBit=str[i:]
            
        if splitAtSpaces:
            spacePos=choppedOffBit.rfind(' ')
            if spacePos!=-1 and spacePos>min:
                i=spacePos
                choppedOffBit=str[:i]
                leftOverBit=str[i:]

        choppedList.append(choppedOffBit)
        chopString2(leftOverBit,dc,maxWidth,choppedList,min,splitAtSpaces)    


def getNumDisplayChars(str,dc,maxWidth):
    count=0
    width=0
    while width < maxWidth and count< len(str):
       count=count+1
       width=dc.GetTextExtent(str[:count])[0]
    return count

def isSequence(x):
    try:
        a=len(x)
        return True
    except:
        return False

def isInteger(x):
    try:
       y=int(x)
       return True
    except:
       return False

def isFloat(x):
    try:
       y=float(x)
       return True
    except:
       return False

def isBool(x):
    if x=='n' or x=='y':
        return True
    else:
        return False

def toBool(x):
    if x=='n':
        return False
    else:
        return True

def isQuoted(x):
    if len(x)<2:
        return False
    if x[0]=='\"' and x[-1]=='\"':
        return True
    else:
        return False

def stripQuotes(str):
    if isQuoted(str):
        return str[1:-1]
    else:
        return str

def SortPath(str1,str2):
    end1=False
    end2=False 

    try:
        index1=str1.index('.')
        s1=str1[:index1]
    except ValueError:
        s1=str1
        end1=True
        
    try:
        index2=str2.index('.')
        s2=str2[:index2]
    except ValueError:
        s2=str2
        end2=True
    c=intcmp(int(s1),int(s2))

    if c==0:
        if end1 and end2:
            c= 0
        elif end1 and not end2:
            c= -1
        elif end2 and not end1:
            c= 1
        else:
            c=SortPath(str1[index1+1:],str2[index2+1:])
    return c

def intcmp(int1,int2):
    if int1>int2:
        c=1
    elif int1<int2:
        c=-1
    else:
        c=0
    return c

def formatFloat(value):
    if isFloat(value):
        value="%10.4f" % float(value)
    value=str(value)
    value=value.strip() 
    if '.' in value:
        while value[len(value)-1]=='0':
            value=value.rstrip('0')
        if value[len(value)-1]=='.':
            value=value+'0'
    return value



