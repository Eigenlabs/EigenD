
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

import yacc
import lex
import types

from error import PipError
def merge(d1,d2):
    r = d1.copy()
    for (k,v) in d2.items():
        if r.has_key(k):
            r[k]=r[k]+v
        else:
            r[k]=v
    return r

def f(d,f):
    for k in f.keys():
        d[k]=1
    return d

tokens = (
    'CLASS', 'OPENCURLY', 'CLOSECURLY', 'CALLMETHOD', 'STRSET',
    'OPENROUND', 'CLOSEROUND', 'VIRTUAL', 'SEMICOLON',
    'COMMA', 'NAME', 'UNSIGNED', 'CHAR', 'SHORT', 'INT',
    'LONG', 'FLOAT', 'DOUBLE', 'VOID', 'COLON', 'TILDE',
    'EQUALS', 'ZERO', 'CONST', 'STAR', 'AMPERSAND', 'LONGTEXT',
    'INLINE', 'OPENSQUARE', 'CLOSESQUARE', 'BOOL', 'FROM',
    'PYOBJECT', 'SLASH', 'IMPORT', 'DOT', 'STDSTR','CMPMETHOD',
    'VERBATIM', 'EPILOG', 'STRMETHOD', 'REPRMETHOD','HASHMETHOD'
)

reserved = {
    'class': 'CLASS',
    'unsigned': 'UNSIGNED',
    'const': 'CONST',
    'char': 'CHAR',
    'short': 'SHORT',
    'int': 'INT',
    'long': 'LONG',
    'float': 'FLOAT',
    'double': 'DOUBLE',
    'bool': 'BOOL',
    'void': 'VOID',
    'virtual': 'VIRTUAL',
    'const': 'CONST',
    'inline': 'INLINE',
    'PyObject': 'PYOBJECT',
    'import' : 'IMPORT',
    'from' : 'FROM',
    'stdstr' : 'STDSTR',
    'strset' : 'STRSET',
    'epilog' : 'EPILOG',
    '__str__' : 'STRMETHOD',
    '__repr__' : 'REPRMETHOD',
    '__hash__' : 'HASHMETHOD',
    '__cmp__' : 'CMPMETHOD',
    '__call__': 'CALLMETHOD'
}

def t_LONGTEXT(t):
    r'\"\"\"(.|\n)*?\"\"\"'
    t.lineno += t.value.count('\n')
    t.value='\n'.join(map(lambda s: '"%s"' % s, t.value[3:-3].split('\n')))+'\n""\n'
    t.type = 'LONGTEXT'
    return t;

def t_VERBATIM(t):
    r'(\{\{\{|\<\<\<)(.|\n)*?(\}\}\}|\>\>\>)'
    t.lineno += t.value.count('\n')
    t.value=t.value[3:-3]
    t.type = 'VERBATIM'
    return t;

def t_NAME(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value,'NAME')
    return t

t_OPENCURLY = r'\{'
t_CLOSECURLY = r'\}'
t_OPENROUND = r'\('
t_CLOSEROUND = r'\)'
t_OPENSQUARE = r'\['
t_CLOSESQUARE = r'\]'
t_SEMICOLON = r'\;'
t_COMMA = r'\,'
t_COLON = r'\:'
t_SLASH = r'\/'
t_EQUALS = r'\='
t_ZERO = r'0'
t_TILDE = r'\~'
t_STAR = r'\*'
t_AMPERSAND = r'\&'
t_DOT = r'\.'
t_ignore = ' \t'

def t_newline(t):
    r'\n+'
    t.lineno += len(t.value)

def t_error(t):
    print "illegal input '%s'" % t.value[0]
    t.skip(1)

def p_statementlist_stmt(p):
    'statementlist : '
    p[0] = { 'moddoc' : '""' ,'inlines': (),'prolog': "",'epilog': "",'classes': (),'functions': (),'imports': () }

def p_statementlist_list(p):
    'statementlist : statementlist statement'
    p[0] = merge(p[1],p[2])

def p_statement_moddoc(p):
    'statement : moddoc'
    p[0] = { 'moddoc': p[1] }

def p_statement_prolog(p):
    'statement : prolog'
    p[0] = { 'prolog': p[1] }

def p_statement_epilog(p):
    'statement : epilog'
    p[0] = { 'epilog': p[1] }

def p_statement_class(p):
    'statement : class'
    p[0] = { 'classes': (p[1],) }

def p_statement_function(p):
    'statement : function'
    p[0] = { 'functions': (p[1],) }

def p_statement_inline(p):
    'statement : inline'
    p[0] = { 'inlines': (p[1],) }

def p_statement_import(p):
    'statement : import'
    p[0] = { 'imports': (p[1],) }

def p_import(p):
    'import : FROM modname optfilename IMPORT classlist'
    p[0] = { 'name': p[2], 'spec': p[3] or p[2]+'.pip', 'classes': p[5] }

def p_moddoc(p):
    'moddoc : LONGTEXT optsemi'
    p[0] = p[1]

def p_prolog(p):
    'prolog : VERBATIM optsemi'
    p[0] = p[1]

def p_epilog(p):
    'epilog : EPILOG VERBATIM optsemi'
    p[0] = p[2]

def p_class(p):
    'class : CLASS NAME implname base classflags opttext OPENCURLY methodlist CLOSECURLY optsemi'
    p[0]= { 'name': p[2], 'implname': p[3], 'ctors': p[8]['ctors'], 'methods': p[8]['methods'], 'docstring': p[6],
            'inlines': p[8]['inlines'], 'handlers': p[8]['handlers'], 'base': p[4], 'dtors': p[8]['dtors'], 'classflags': p[5] }
    for special in p[8]['specials']: p[0][special['name']] = special

def p_function(p):
    'function : c2pvoidtype NAME implname OPENROUND p2ctypelist CLOSEROUND optflags opttext optsemi'
    p[0]= { 'name': p[2], 'implname': p[3], 'returns': p[1], 'args': p[5], 'docstring': p[8], 'flags': p[7] }

def p_inlinefunction(p):
    'inline : INLINE NAME OPENROUND NAME CLOSEROUND optflags opttext VERBATIM  optsemi'
    p[0]= { 'name': p[2], 'body': p[8], 'args': p[4], 'docstring': p[7], 'flags': p[6] }

def p_base_empty(p):
    'base : '
    p[0] = None

def p_base_base(p):
    'base : COLON NAME'
    p[0] = p[2]

def p_methodlist_empty(p):
    'methodlist : '
    p[0]={ 'dtors': (), 'ctors': (), 'inlines': (), 'methods': (), 'handlers': (), 'specials': () }

def p_methodlist_list(p):
    'methodlist : methodlist method'
    p[0]=merge(p[1],p[2])

def p_method_dtors(p):
    'method : TILDE NAME OPENROUND CLOSEROUND VERBATIM optsemi'
    p[0]= { 'dtors': ({ 'body': p[5] },)}

def p_method_ctor(p):
    'method : NAME OPENROUND p2ctypelist CLOSEROUND opttext optsemi'
    p[0]= { 'ctors': ({ 'args': p[3], 'docstring': p[5] },)}

def p_method_simple(p):
    'method : c2pvoidtype NAME OPENROUND p2ctypelist CLOSEROUND optflags opttext optsemi'
    p[0]= { 'methods': ({ 'name': p[2], 'returns': p[1], 'args': p[4], 'docstring': p[7], 'isvirtual':0, 'flags': p[6] },)}

def p_method_inline(p):
    'method : INLINE NAME OPENROUND NAME COMMA NAME CLOSEROUND optflags opttext VERBATIM optsemi'
    p[0]= { 'inlines': ({ 'name': p[2], 'body': p[10], 'self': p[4], 'args': p[6], 'docstring': p[9], 'flags': p[7] },)}

def p_method_virtual(p):
    'method : VIRTUAL p2cvoidtype NAME OPENROUND c2ptypelist CLOSEROUND optflags opttext optsemi'
    p[0]= { 'handlers': ({'name': p[3], 'returns': p[2], 'args': p[5], 'flags': p[7] },),
            'methods': ({'name': p[3], 'returns': p[2], 'args': p[5], 'docstring': p[8], 'isvirtual': 1, 'flags': p[7] },)}

def p_method_purevirtual(p):
    'method : VIRTUAL p2cvoidtype NAME OPENROUND c2ptypelist CLOSEROUND optflags EQUALS ZERO opttext optsemi'
    p[0]= { 'handlers': ({'name': p[3], 'returns': p[2], 'args': p[5], 'flags': p[7] },),
            'methods': ({'name': p[3], 'returns': p[2], 'args': p[5], 'docstring': p[10], 'isvirtual': 1, 'ispure': 1, 'flags': p[7] },)}

def p_method_str(p):
    'method : basetype_str STRMETHOD implname OPENROUND CLOSEROUND opttext optflags optsemi'
    p[0]= { 'specials': ({ 'name': '__str__', 'implname': p[3] or '__str__', 'returns': p[1], 'flags': p[6], 'docstring': p[7] },)}

def p_method_repr(p):
    'method : basetype_str REPRMETHOD implname OPENROUND CLOSEROUND opttext optflags optsemi'
    p[0]= { 'specials': ({ 'name': '__repr__', 'implname': p[3] or '__repr__', 'returns': p[1], 'flags': p[6], 'docstring': p[7] },)}

def p_method_hash(p):
    'method : basetype_long HASHMETHOD implname OPENROUND CLOSEROUND opttext optflags optsemi'
    p[0]= { 'specials': ({ 'name': '__hash__', 'implname': p[3] or '__hash__', 'returns': p[1], 'flags': p[6], 'docstring': p[7] },)}

def p_method_cmp(p):
    'method : basetype_int CMPMETHOD implname OPENROUND c2ptype CLOSEROUND optflags opttext optsemi'
    p[0]= { 'specials': ({ 'name': '__cmp__', 'implname': p[3] or '__cmp__', 'returns': p[1], 'arg': p[5], 'flags': p[7], 'docstring': p[8] },)}

def p_method_call(p):
    'method : c2pvoidtype CALLMETHOD implname OPENROUND p2ctypelist CLOSEROUND optflags opttext optsemi'
    p[0]= { 'specials': ({ 'name': '__call__', 'implname': p[3] or '__call__', 'returns': p[1], 'args': p[5], 'flags': p[7], 'docstring': p[8] },)}

def p_p2cvoidtype_p2ctype(p):
    'p2cvoidtype : p2ctype'
    p[0]=p[1]

def p_p2cvoidtype_void(p):
    'p2cvoidtype : void'
    p[0]=p[1]

def p_c2pvoidtype_c2ptype(p):
    'c2pvoidtype : c2ptype'
    p[0]=p[1]

def p_c2pvoidtype_void(p):
    'c2pvoidtype : void'
    p[0]=p[1]

def p_p2ctypelist_empty(p):
    'p2ctypelist : '
    p[0]=()

def p_p2ctypelist_p2ctype(p):
    'p2ctypelist : p2ctype'
    p[0]=(p[1],)

def p_p2ctypelist_list(p):
    'p2ctypelist : p2ctypelist COMMA p2ctype'
    p[0]=p[1]+(p[3],)

def p_c2ptypelist_empty(p):
    'c2ptypelist : '
    p[0]=()

def p_c2ptypelist_c2ptype(p):
    'c2ptypelist : c2ptype'
    p[0]=(p[1],)

def p_c2ptypelist_list(p):
    'c2ptypelist : c2ptypelist COMMA c2ptype'
    p[0]=p[1]+(p[3],)

def p_c2ptype_p2ctype(p):
    'c2ptype : basetype'
    p[0]=p[1]

def p_p2ctype_basetype(p):
    'p2ctype : basetype'
    p[0] = p[1]

def p_void(p):
    'void : VOID'
    p[0] = { 'rtype': 'void', 'isvoid': 1 }

def p_p2ctype_a2(p):
    'p2ctype : CONST NAME STAR'
    p[0] =  { 'rtype': 'const '+p[2]+'_type_ *', 'ctype': p[2]+'_type_ *',   'c2r': '' , 'cvt': p[2], 'r2c': '--invalid-type-compile-error--' }

def p_p2ctype_a5(p):
    'p2ctype : NAME AMPERSAND'
    p[0] =  { 'rtype': p[1]+'_type_ &', 'ctype': p[1]+'_type_ *',   'c2r': '*' , 'cvt': p[1], 'r2c': '--invalid-type-compile-error--' }

def p_p2ctype_ptr(p):
    'p2ctype : NAME STAR'
    p[0] =  { 'rtype': p[1]+'_type_ *', 'ctype': p[1]+'_type_ *',    'c2r': '', 'r2c': ''  , 'cvt': p[1] }

def p_c2ptype_ptr(p):
    'c2ptype : NAME STAR'
    p[0] =  { 'rtype': p[1]+'_type_ *', 'ctype': p[1]+'_type_ *',    'c2r': '', 'r2c': ''  , 'cvt': p[1] }

def p_basetype_constref(p):
    'basetype : CONST NAME AMPERSAND'
    p[0] =  { 'rtype': 'const '+p[2]+'_type_ &', 'ctype': p[2]+'_type_ *',   'c2r': '*' , 'cvt': p[2]+'_copy', 'r2c': '&' }

def p_basetype_copy(p):
    'basetype : NAME'
    p[0] =  { 'rtype': p[1]+'_type_', 'ctype': p[1]+'_type_ *',    'c2r': '*', 'r2c': '&'  , 'cvt': p[1]+'_copy' }

def p_basetype_str(p):
    'basetype : basetype_str'
    p[0] = p[1]

def p_basetype_long(p):
    'basetype : basetype_long'
    p[0] = p[1]

def p_basetype_int(p):
    'basetype : basetype_int'
    p[0] = p[1]

def p_basetype_stdstr_ref(p):
    'basetype_str : CONST STDSTR AMPERSAND'
    p[0] =  { 'rtype': 'const std::string &', 'ctype': 'std::string',   'c2r': '' , 'cvt': 'stdstr', 'r2c': '' }

def p_basetype_strset_ref(p):
    'basetype_str : CONST STRSET AMPERSAND'
    p[0] =  { 'rtype': 'const std::set<std::string> &', 'ctype': 'std::set<std::string>',   'c2r': '' , 'cvt': 'strset', 'r2c': '' }

def p_basetype_str_c(p):
    'basetype_str : CONST CHAR STAR'
    p[0] =  { 'rtype': 'const char *', 'ctype': 'const char *',  'r2c': '', 'c2r': '', 'cvt': 'str' }

def p_basetype_str_uc(p):
    'basetype_str : CONST UNSIGNED CHAR STAR'
    p[0] =  { 'rtype': 'const unsigned char *', 'ctype': 'const unsigned char *',  'r2c': '', 'c2r': '', 'cvt': 'ustr' }

def p_basetype_stdstr_copy(p):
    'basetype_str : STDSTR'
    p[0] =  { 'rtype': 'std::string', 'ctype': 'std::string',    'c2r': '', 'r2c': ''  , 'cvt': 'stdstr' }

def p_basetype_strset_copy(p):
    'basetype_str : STRSET'
    p[0] =  { 'rtype': 'std::set<std::string>', 'ctype': 'std::set<std::string>',    'c2r': '', 'r2c': ''  , 'cvt': 'strset' }

def p_basetype_long_long(p):
    'basetype_long : LONG'
    p[0] =  { 'rtype': 'long', 'ctype': 'long',    'c2r': '', 'r2c': ''  , 'cvt': 'l' }

def p_basetype_char(p):
    'basetype : CHAR'
    p[0] =  { 'rtype': 'char', 'ctype': 'char',    'c2r': '' , 'r2c': ''  , 'cvt': 'c' }

def p_basetype_uchar(p):
    'basetype : UNSIGNED CHAR'
    p[0] =  { 'rtype': 'unsigned char', 'ctype': 'unsigned char',    'c2r': '',  'r2c': ''  , 'cvt': 'uc' }

def p_basetype_short(p):
    'basetype : SHORT'
    p[0] =  { 'rtype': 'short', 'ctype': 'short',    'c2r': '',  'r2c': ''  , 'cvt': 's' }

def p_basetype_ushort(p):
    'basetype : UNSIGNED SHORT'
    p[0] =  { 'rtype': 'unsigned short', 'ctype': 'unsigned short',    'c2r': '',  'r2c': ''  , 'cvt': 'us' }

def p_basetype_int_int(p):
    'basetype_int : INT'
    p[0] =  { 'rtype': 'int', 'ctype': 'int',    'c2r': '',  'r2c': ''  , 'cvt': 'i' }

def p_basetype_uint(p):
    'basetype : UNSIGNED INT'
    p[0] =  { 'rtype': 'unsigned int', 'ctype': 'unsigned int',    'c2r': '', 'r2c': ''  , 'cvt': 'ui' }

def p_basetype_ustandalone(p):
    'basetype : UNSIGNED'
    p[0] =  { 'rtype': 'unsigned int', 'ctype': 'unsigned int',    'c2r': '', 'r2c': ''  , 'cvt': 'ui' }

def p_basetype_ulong(p):
    'basetype : UNSIGNED LONG'
    p[0] =  { 'rtype': 'unsigned long', 'ctype': 'unsigned long',    'c2r': '', 'r2c': ''  , 'cvt': 'ul' }

def p_basetype_longlong(p):
    'basetype : LONG LONG'
    p[0] =  { 'rtype': 'long long', 'ctype': 'long long',    'c2r': '', 'r2c': ''  , 'cvt': 'll' }

def p_basetype_ulonglong(p):
    'basetype : UNSIGNED LONG LONG'
    p[0] =  { 'rtype': 'unsigned long long', 'ctype': 'unsigned long long',    'c2r': '', 'r2c': ''  , 'cvt': 'ull' }

def p_basetype_float(p):
    'basetype : FLOAT'
    p[0] =  { 'rtype': 'float', 'ctype': 'float',    'c2r': '', 'r2c': ''  , 'cvt': 'f' }

def p_basetype_double(p):
    'basetype : DOUBLE'
    p[0] =  { 'rtype': 'double', 'ctype': 'double',    'c2r': '', 'r2c': ''  , 'cvt': 'd' }

def p_basetype_bool(p):
    'basetype : BOOL'
    p[0] =  { 'rtype': 'bool', 'ctype': 'bool',    'c2r': '', 'r2c': ''  , 'cvt': 'b' }

def p_basetype_pyobjectptr(p):
    'basetype : PYOBJECT STAR'
    p[0] =  { 'rtype': 'PyObject *', 'ctype': 'PyObject *', 'r2c': '',  'c2r': '', 'cvt': 'py' }

def p_classflags_empty(p):
    'classflags : '
    p[0] = {}

def p_classflags_list(p):
    'classflags : SLASH flaglist SLASH'
    p[0] = p[2]

def p_flags_empty(p):
    'optflags : '
    p[0] = {}

def p_flags_list(p):
    'optflags : OPENSQUARE flaglist CLOSESQUARE'
    p[0] = p[2]

def p_flags_list2(p):
    'optflags : SLASH flaglist SLASH'
    p[0] = p[2]

def p_flaglist_empty(p):
    'flaglist :'
    p[0] = {}

def p_flaglist_name(p):
    'flaglist : NAME'
    p[0] = { p[1]: 1 }

def p_flaglist_list(p):
    'flaglist : flaglist COMMA NAME'
    p[0] = p[1].copy()
    p[0][p[3]] = 1

def p_implname_empty(p):
    'implname : '
    p[0] = None

def p_implname_name(p):
    'implname : OPENSQUARE bigname CLOSESQUARE'
    p[0] = p[2]

def p_bigelem_name(p):
    'bigelem : NAME'
    p[0] = p[1]

def p_bigelem_colon(p):
    'bigelem : COLON'
    p[0] = ':'

def p_bigname_elem(p):
    'bigname : bigelem'
    p[0] = p[1]

def p_bigname_list(p):
    'bigname : bigname bigelem'
    p[0] = p[1]+p[2]

def p_modname_single(p):
    'modname : NAME'
    p[0] = p[1]

def p_modname_multi(p):
    'modname : modname DOT NAME'
    p[0] = p[1]+'.'+p[3]

def p_classlist_class(p):
    'classlist : NAME'
    p[0] = ({'name':p[1]},)

def p_classlist_list(p):
    'classlist : classlist COMMA NAME'
    p[0] = p[1]+({'name':p[3]},)

def p_optfilename_empty(p):
    'optfilename : '
    p[0] = None

def p_optfilename_filename(p):
    'optfilename : OPENSQUARE filename CLOSESQUARE'
    p[0] = p[2]

def p_filename_element(p):
    'filename : filename_element'
    p[0] = p[1]

def p_filename_list(p):
    'filename : filename filename_element'
    p[0] = p[1]+p[2]

def p_filename_element_name(p):
    'filename_element : NAME'
    p[0] = p[1]

def p_filename_element_dot(p):
    'filename_element : DOT'
    p[0] = '.'

def p_filename_element_slash(p):
    'filename_element : SLASH'
    p[0] = '/'

def p_optsemi_missing(p):
    'optsemi : '
    p[0] = None

def p_optsemi_semicolon(p):
    'optsemi : SEMICOLON'
    p[0] = ';'

def p_opttext_empty(p):
    'opttext : '
    p[0] = '""'

def p_opttext_text(p):
    'opttext : LONGTEXT'
    p[0] = p[1]

def p_error(p):
    raise PipError("syntax error at token %s(%s) line %d" % (p.type,p.value,p.lineno))

lex.lex()
yacc.yacc()

def parse(module,input):
    tree=yacc.parse(input)
    tree['module']=module
    return tree
