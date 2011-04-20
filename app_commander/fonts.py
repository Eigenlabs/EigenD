
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

DEFAULT_PTS=11
TITLE_BAR_PTS=11
DICT_LABEL_PTS=28
STAFF_MAX_PTS=18

TABLE_TITLE_PT_FACTOR=1.2
STAFF_NUMBERS_PT_FACTOR=0.75
STAFF_FACTOR=1.3

TABLE_TITLE_WEIGHT=wx.FONTWEIGHT_BOLD
TABLE_ENTRY_SELECTED_WEIGHT=wx.FONTWEIGHT_BOLD
TITLE_BAR_WEIGHT=wx.FONTWEIGHT_BOLD

def setFont(dc,weight=None,ptfactor=None,abs_pts=None,max_pts=None):
    font=dc.GetFont()
    oldWeight=font.GetWeight()
    oldPts=font.GetPointSize()
    newWeight=oldWeight
    newPts=oldPts

    if weight:
        newWeight=weight
    if abs_pts:
        newPts=abs_pts
    elif ptfactor:
        newPts=int(oldPts*ptfactor)
    if max_pts:
        newPts=min(newPts,max_pts)
    resetFont(dc,newPts,newWeight,font)

    return (oldPts,oldWeight)

def setTitleBarFont(dc):
    return setFont(dc,weight=TITLE_BAR_WEIGHT,abs_pts=TITLE_BAR_PTS)

def setTableTitleFont(dc):
    return setFont(dc,weight=TABLE_TITLE_WEIGHT,ptfactor=TABLE_TITLE_PT_FACTOR)

def setSubTableTitleFont(dc):
    return setFont(dc,weight=TABLE_TITLE_WEIGHT)

def setTableEntrySelectedFont(dc):
    return setFont(dc,weight=TABLE_ENTRY_SELECTED_WEIGHT)

def setDictionaryLabelFont(dc):
    return setFont(dc,weight=wx.FONTWEIGHT_NORMAL,abs_pts=DICT_LABEL_PTS)

def setStaffNumbersFont(dc):
    return setFont(dc,ptfactor=STAFF_NUMBERS_PT_FACTOR)

def setStaffFont(dc):
    return setFont(dc,ptfactor=STAFF_FACTOR,max_pts=STAFF_MAX_PTS)

def setStaffViewHistoryFont(dc,pts):
    return setFont(dc,abs_pts=pts)

def setStaffViewFont(dc,pts):
    return setFont(dc,abs_pts=pts)

def resetFont(dc,pts,weight,font=None):
    if not font:
        font=dc.GetFont()
    font.SetWeight(weight)
    font.SetPointSize(pts)
    dc.SetFont(font)

   
