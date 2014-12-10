/*
 Copyright 2012-2014 Eigenlabs Ltd.  http://www.eigenlabs.com

 This file is part of EigenD.

 EigenD is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 EigenD is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with EigenD.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PROPED__
#define __PROPED__

#include "Workspace.h"
#include "AtomEditor.h"
#include "StringPropertyEditor.h"
#include "BoolPropertyEditor.h"
#include "FloatPropertyEditor.h"
#include "IntPropertyEditor.h"
#include "EnumPropertyEditor.h"
#include "NamePropertyEditor.h"
#include "BrowseEditor.h"
#include "StringMappingPropertyEditor.h"
#include "KeyToCoursePropertyEditor.h"
#include "CoursesPropertyEditor.h"
#include "PropertySeparator.h"
#include <set>
#include "juce.h"
#include <vector>
#include "ToolManager.h"

class PropertyEditor  : public Component,
                        public ValueMonitorListener
{
public:
    PropertyEditor (Atom* atom, ToolManager* tm, ValueMonitor* vm, bool showNames);
    ~PropertyEditor();
    void refresh();
    void resized();
    void childBoundsChanged(Component* child);
    void enumerate_updated(String id,String path, int nf,int nc);
    void activated(String id);
    void current(String id, String cookie);
    void finfo_updated(String id, String path,const std::set<std::string> & finfo);
    void cinfo_updated(String id, String path,const std::set<std::string> & cinfo);
    void sourcekeys_updated( String id, String keys);
    virtual void value_changed(String id); 
    Atom* getAtom();
    bool getShowNames();
private:

    Atom* atom_;
    bool showNames_;
    String section_;
    void addPropertyEditor(Atom* atom, int indentLevel);
//    void checkSection(Atom* atom);
    void setupProperties(Atom* atom, bool topLevel, int indentLevel, bool showNames);
    void addSeparator(String name, int indentLevel);
    void addBoolPropertyEditor(Atom* atom,String name,String value, String domain, bool disabled, int indentLevel);
    void addFloatPropertyEditor(Atom* atom,String name,String value, String domain,bool disabled, int indentLevel);
    void addIntPropertyEditor(Atom* atom,String name,String value, String domain,bool disabled,int indentLevel);
    void addStringPropertyEditor(Atom* atom,String name,String value, String domain,bool disabled,int indentLevel);
    void addEnumPropertyEditor(Atom* atom,String name,String value, String domain, bool disabled, int indentLevel);
    void addNamePropertyEditor(Atom* atom,int indentLevel);
    void addBrowseEditor(Atom* atom, String name, int indentLevel);
    void addMappingEditor(Atom* atom, int indentLevel);
    void addKeyToCourseEditor(Atom* atom, int indentLevel);
    void addCoursesEditor(Atom* atom, int indentLevel);
    Component* getEditor(String id);
    PropertyEditor (const PropertyEditor&);
    const PropertyEditor& operator= (const PropertyEditor&);
    std::vector<Component*> editors_;
    std::map<String,Component*> oldEditors_;
    float getMin(String domain);
    float getMax(String domain);
    void layout();
    ToolManager* tm_;
    ValueMonitor* vm_;
};

#endif 
