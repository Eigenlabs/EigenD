#!/usr/bin/env python
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


import unittest
import pip
import distutils.core
import distutils.dist
import os
import imp
import sys

test_spec = """
    {{{
    #include <iostream>
    #include <string>
    #include <sstream>
    #include <cstring>

    struct Msg
    {
        Msg() { rep<<std::fixed; rep.precision(5); }
        Msg &operator<<(void (*manipulator)(const Msg &)) { manipulator(*this); return *this; }
        template <class D> Msg &operator<<(const D &d) { rep<<d; return *this; }
        template <class D> Msg &operator<<(D &d) { rep<<d; return *this; }

        std::ostringstream rep;
    };

    struct Reporter
    {
        virtual ~Reporter() {}
        virtual void report(const std::string &msg) { std::cout << "reporter default report: " << msg << std::endl; }
    };

    static Reporter *reporter__;
    static void set_reporter(Reporter *r) { reporter__ = r; }
    static Reporter *get_reporter() { return reporter__; }
    static void report(const std::string &msg) { if(reporter__) reporter__->report(msg); }
    inline void msg_report(const Msg &m) { report(m.rep.str()); }

    static short function_s(short arg) { Msg()<<arg<<msg_report; return arg; }
    static int function_i(int arg) { Msg()<<arg<<msg_report; return arg; }
    static long function_l(long arg) { Msg()<<arg<<msg_report; return arg; }
    static long long function_ll(long long arg) { Msg()<<arg<<msg_report; return arg; }
    static unsigned short function_us(unsigned short arg) { Msg()<<arg<<msg_report; return arg; }
    static unsigned int function_ui(unsigned int arg) { Msg()<<arg<<msg_report; return arg; }
    static unsigned long function_ul(unsigned long arg) { Msg()<<arg<<msg_report; return arg; }
    static unsigned long long function_ull(unsigned long long arg) { Msg()<<arg<<msg_report; return arg; }
    static unsigned function_u(unsigned arg) { Msg()<<arg<<msg_report; return arg; }
    static bool function_b(bool arg) { Msg()<<arg<<msg_report; return arg; }
    static double function_d(double arg) { Msg()<<arg<<msg_report; return arg; }
    static float function_f(float arg) { Msg()<<arg<<msg_report; return arg; }
    static std::string function_stdstr1(std::string arg) { Msg()<<arg<<msg_report; return arg; }
    static std::string function_stdstr2(const std::string &arg) { Msg()<<arg<<msg_report; return arg; }
    static const char *function_cstr(const char *arg) { Msg()<<arg<<msg_report; return strdup(arg); }

    struct TestClass1
    {
        TestClass1(int v): value_(v) {}

        std::string __repr__() { return "TestClass1 repr"; }
        const char *__str__() { return "TestClass1 str"; }
        long __hash__() { return value_; }
        int __cmp__(TestClass1 *t) { return value_-t->value_; }
        int __call__(int v) { return v+1; }

        short function_s(short arg) { Msg()<<arg<<msg_report; return arg; }
        int function_i(int arg) { Msg()<<arg<<msg_report; return arg; }
        long function_l(long arg) { Msg()<<arg<<msg_report; return arg; }
        long long function_ll(long long arg) { Msg()<<arg<<msg_report; return arg; }
        unsigned short function_us(unsigned short arg) { Msg()<<arg<<msg_report; return arg; }
        unsigned int function_ui(unsigned int arg) { Msg()<<arg<<msg_report; return arg; }
        unsigned long function_ul(unsigned long arg) { Msg()<<arg<<msg_report; return arg; }
        unsigned long long function_ull(unsigned long long arg) { Msg()<<arg<<msg_report; return arg; }
        unsigned function_u(unsigned arg) { Msg()<<arg<<msg_report; return arg; }
        bool function_b(bool arg) { Msg()<<arg<<msg_report; return arg; }
        double function_d(double arg) { Msg()<<arg<<msg_report; return arg; }
        float function_f(float arg) { Msg()<<arg<<msg_report; return arg; }
        std::string function_stdstr1(std::string arg) { Msg()<<arg<<msg_report; return arg; }
        std::string function_stdstr2(const std::string &arg) { Msg()<<arg<<msg_report; return arg; }
        const char *function_cstr(const char *arg) { Msg()<<arg<<msg_report; return strdup(arg); }

        int value_;
    };

    struct TestClass2
    {
        TestClass2(int v): value_(v) {}
        virtual ~TestClass2() {}

        std::string myrepr() { return "TestClass2 repr"; }
        const char *mystr() { return "TestClass2 str"; }
        long myhash() { return value_; }
        int mycmp(const TestClass2 &t) { return value_-t.value_; }
        float mycall(float v1,float v2) { return v1+v2; }

        virtual short vfunction_s(short arg) { Msg()<<arg<<msg_report; return arg; }
        virtual int vfunction_i(int arg) { Msg()<<arg<<msg_report; return arg; }
        virtual long vfunction_l(long arg) { Msg()<<arg<<msg_report; return arg; }
        virtual long long vfunction_ll(long long arg) { Msg()<<arg<<msg_report; return arg; }
        virtual unsigned short vfunction_us(unsigned short arg) { Msg()<<arg<<msg_report; return arg; }
        virtual unsigned int vfunction_ui(unsigned int arg) { Msg()<<arg<<msg_report; return arg; }
        virtual unsigned long vfunction_ul(unsigned long arg) { Msg()<<arg<<msg_report; return arg; }
        virtual unsigned long long vfunction_ull(unsigned long long arg) { Msg()<<arg<<msg_report; return arg; }
        virtual unsigned vfunction_u(unsigned arg) { Msg()<<arg<<msg_report; return arg; }
        virtual bool vfunction_b(bool arg) { Msg()<<arg<<msg_report; return arg; }
        virtual double vfunction_d(double arg) { Msg()<<arg<<msg_report; return arg; }
        virtual float vfunction_f(float arg) { Msg()<<arg<<msg_report; return arg; }
        virtual std::string vfunction_stdstr1(std::string arg) { Msg()<<arg<<msg_report; return arg; }
        virtual std::string vfunction_stdstr2(const std::string &arg) { Msg()<<arg<<msg_report; return arg; }
        virtual const char *vfunction_cstr(const char *arg) { Msg()<<arg<<msg_report; return strdup(arg); }

        short function_s(short arg) { return vfunction_s(arg); }
        int function_i(int arg) { return vfunction_i(arg); }
        long function_l(long arg) { return vfunction_l(arg); }
        long long function_ll(long long arg) { return vfunction_ll(arg); }
        unsigned short function_us(unsigned short arg) { return vfunction_us(arg); }
        unsigned int function_ui(unsigned int arg) { return vfunction_ui(arg); }
        unsigned long function_ul(unsigned long arg) { return vfunction_ul(arg); }
        unsigned long long function_ull(unsigned long long arg) { return vfunction_ull(arg); }
        unsigned function_u(unsigned arg) { return vfunction_u(arg); }
        bool function_b(bool arg) { return vfunction_b(arg); }
        double function_d(double arg) { return vfunction_d(arg); }
        float function_f(float arg) { return vfunction_f(arg); }
        std::string function_stdstr1(std::string arg) { return vfunction_stdstr1(arg); }
        std::string function_stdstr2(const std::string &arg) { return vfunction_stdstr2(arg); }
        const char *function_cstr(const char *arg) { return vfunction_cstr(arg); }

        int value_;
    };

    struct TestClass3
    {
        TestClass3() {}
        virtual ~TestClass3() {}
        const char *func() { return "TestClass3::func"; }
        virtual const char *vfunc() { return "TestClass3::vfunc"; }
        virtual const char *pvfunc() = 0;

        const char *call_func() { return func(); }
        const char *call_vfunc() { return vfunc(); }
        const char *call_pvfunc() { return pvfunc(); }
    };

    struct TestClass4: TestClass3
    {
        TestClass4() {}
    };

    struct TestClass5: TestClass3
    {
        TestClass5() {}

        const char *func() { return "TestClass5::func"; }
        virtual const char *vfunc() { return "TestClass5::vfunc"; }
        virtual const char *pvfunc() { return "TestClass5::pvfunc"; }
    };

    struct TestFactory
    {
        TestClass3 *create() { return new TestClass5(); }
    };

    }}}

    \""" Module Documentation \"""

    class Reporter
        \""" Reporter Class Documentation \"""
    {
        Reporter()
        virtual void report(const stdstr &)
        \""" Report Function \"""
    }

    void set_reporter(Reporter *)
    Reporter *get_reporter()
    void report(const stdstr &)

    short function_s(short) \""" Short Function Test \"""
    int function_i(int) \""" Int Function Test \"""
    long function_l(long) \""" Long Function Test \"""
    long long function_ll(long long) \""" Long Long Function Test \"""
    unsigned short function_us(unsigned short) \""" Unsigned Short Function Test \"""
    unsigned int function_ui(unsigned int) \""" Unsigned Int Function Test \"""
    unsigned long function_ul(unsigned long) \""" Unsigned Long Function Test \"""
    unsigned long long function_ull(unsigned long long) \""" Unsigned Long Long Function Test \"""
    unsigned function_u(unsigned) \""" Unsigned Function Test \"""
    bool function_b(bool) \""" Bool Function Test \"""
    double function_d(double) \""" Double Function Test \"""
    float function_f(float) \""" Float Function Test \"""
    stdstr function_stdstr1(stdstr) \""" stdstr1 Function Test \"""
    stdstr function_stdstr2(const stdstr &) \""" stdstr2 Function Test \"""
    const char *function_cstr(const char *) \""" cstr Function Test \"""

    class TestClass1
        \""" TestClass1 Dokmuentation \"""
    {
        TestClass1(int)
        const char * __str__()
        stdstr __repr__()
        long __hash__()
        int __cmp__(TestClass1 *)
        int __call__(int)

        short function_s(short) \""" Short Function Test \"""
        int function_i(int) \""" Int Function Test \"""
        long function_l(long) \""" Long Function Test \"""
        long long function_ll(long long) \""" Long Long Function Test \"""
        unsigned short function_us(unsigned short) \""" Unsigned Short Function Test \"""
        unsigned int function_ui(unsigned int) \""" Unsigned Int Function Test \"""
        unsigned long function_ul(unsigned long) \""" Unsigned Long Function Test \"""
        unsigned long long function_ull(unsigned long long) \""" Unsigned Long Long Function Test \"""
        unsigned function_u(unsigned) \""" Unsigned Function Test \"""
        bool function_b(bool) \""" Bool Function Test \"""
        double function_d(double) \""" Double Function Test \"""
        float function_f(float) \""" Float Function Test \"""
        stdstr function_stdstr1(stdstr) \""" stdstr1 Function Test \"""
        stdstr function_stdstr2(const stdstr &) \""" stdstr2 Function Test \"""
        const char *function_cstr(const char *) \""" cstr Function Test \"""
    }

    class TestClass2
        \""" TestClass2 Dokmuentation \"""
    {
        TestClass2(int)
        const char * __str__[mystr]()
        stdstr __repr__[myrepr]()
        long __hash__[myhash]()
        int __cmp__[mycmp](const TestClass2 &)
        float __call__[mycall](float,float)

        virtual short vfunction_s(short) \""" Short Function Test \"""
        virtual int vfunction_i(int) \""" Int Function Test \"""
        virtual long vfunction_l(long) \""" Long Function Test \"""
        virtual long long vfunction_ll(long long) \""" Long Long Function Test \"""
        virtual unsigned short vfunction_us(unsigned short) \""" Unsigned Short Function Test \"""
        virtual unsigned int vfunction_ui(unsigned int) \""" Unsigned Int Function Test \"""
        virtual unsigned long vfunction_ul(unsigned long) \""" Unsigned Long Function Test \"""
        virtual unsigned long long vfunction_ull(unsigned long long) \""" Unsigned Long Long Function Test \"""
        virtual unsigned vfunction_u(unsigned) \""" Unsigned Function Test \"""
        virtual bool vfunction_b(bool) \""" Bool Function Test \"""
        virtual double vfunction_d(double) \""" Double Function Test \"""
        virtual float vfunction_f(float) \""" Float Function Test \"""
        virtual stdstr vfunction_stdstr1(stdstr) \""" stdstr1 Function Test \"""
        virtual stdstr vfunction_stdstr2(const stdstr &) \""" stdstr2 Function Test \"""

        short function_s(short) \""" Short Function Test \"""
        int function_i(int) \""" Int Function Test \"""
        long function_l(long) \""" Long Function Test \"""
        long long function_ll(long long) \""" Long Long Function Test \"""
        unsigned short function_us(unsigned short) \""" Unsigned Short Function Test \"""
        unsigned int function_ui(unsigned int) \""" Unsigned Int Function Test \"""
        unsigned long function_ul(unsigned long) \""" Unsigned Long Function Test \"""
        unsigned long long function_ull(unsigned long long) \""" Unsigned Long Long Function Test \"""
        unsigned function_u(unsigned) \""" Unsigned Function Test \"""
        bool function_b(bool) \""" Bool Function Test \"""
        double function_d(double) \""" Double Function Test \"""
        float function_f(float) \""" Float Function Test \"""
        stdstr function_stdstr1(stdstr) \""" stdstr1 Function Test \"""
        stdstr function_stdstr2(const stdstr &) \""" stdstr2 Function Test \"""
        const char *function_cstr(const char *) \""" cstr Function Test \"""
    }

    class TestClass3 \""" TestClass3 Dokmuentation \"""
    {
        TestClass3()

        virtual const char *vfunc() \"""TestClass3::vfunc\"""
        virtual const char *pvfunc() = 0 \"""TestClass3::pvfunc\"""
        const char *func() \"""TestClass3::func\"""

        const char *call_func()
        const char *call_vfunc()
        const char *call_pvfunc()
    }

    class TestClass4: TestClass3 \""" TestClass4 Dokmuentation \"""
    {
        TestClass4()
    }

    class TestClass5: TestClass3 \""" TestClass5 Dokmuentation \"""
    {
        TestClass5()

        virtual const char *vfunc() \"""TestClass5::vfunc\"""
        virtual const char *pvfunc() \"""TestClass5::pvfunc\"""
        const char *func() \"""TestClass5::func\"""
    }

    class TestFactory
    {
        TestFactory()
        TestClass3 *create()
    }


"""

def safe_remove(f):
    try: os.remove(f)
    except: pass

def compile_module(module,specification,include_path=[]):
    spec_file = 'testtmp_%s.pip' % module
    code_file = 'testtmp_%s.cpp' % module
    extra = []

    if sys.platform == 'darwin':
        extra.append('/usr/lib/libstdc++.6.dylib')

    ext = distutils.core.Extension(module,sources=[code_file],extra_objects=extra,extra_link_args=['-g'],extra_compile_args=['-g'])
    dist = distutils.dist.Distribution(dict(ext_modules=[ext]))
    bin_file = None

    try: os.mkdir('build')
    except: pass

    try:
        f=file(spec_file,"w")
        f.write(specification)
        f.close()

        pip.generate(module,spec_file,code_file,include_path)

        dist.get_option_dict('build_ext')['inplace']=('script',True)
        dist.get_option_dict('build_ext')['verbose']=('script',True)
        dist.run_command('build_ext')
        bin_file=dist.get_command_obj('build_ext').get_ext_filename(module)
        return imp.load_dynamic(module,bin_file)

    finally:
        if bin_file is not None:
            safe_remove(bin_file)
        dist.run_command('clean')
        safe_remove(spec_file)
        safe_remove(code_file)


def install_module():
    global test_module, test_reporter

    if test_module is not None: return

    test_module=compile_module('pip_test',test_spec)

    class Reporter(test_module.Reporter):
        def __init__(self):
            test_module.Reporter.__init__(self)
            self.reports=[]
        def report(self,msg):
            self.last_report=msg
            self.reports.append(msg)
        def clear_reports(self):
            self.last_report=None
            self.reports=[]

    test_reporter = Reporter()
    test_module.set_reporter(test_reporter)

test_module=None
test_reporter=None

class pip_test(unittest.TestCase):

    def setUp(self):
        install_module()

    def check_function(self,name,doc=None):
        self.assertTrue(hasattr(test_module,name))
        obj=getattr(test_module,name)
        if doc:
            self.assertTrue(hasattr(obj,'__doc__'))
            self.assertTrue(doc in obj.__doc__)

    def check_namespace(self,obj,doc,**kwds):
        self.assertTrue(hasattr(obj,'__doc__'))
        self.assertTrue(doc in obj.__doc__)
        for (mname,mdoc) in kwds.iteritems():
            self.assertTrue(hasattr(obj,mname))
            method=getattr(obj,mname)
            if mdoc != None:
                self.assertTrue(hasattr(method,'__doc__'))
                self.assertTrue(mdoc in method.__doc__)
        self.assertTrue(hasattr(test_module.Reporter,'__init__'))

    def check_arith(self,func,tipe,conv=None,valid=(),invalid=()):
        conv=conv or tipe
        for (i,o) in valid:
            try:
                test_reporter.clear_reports()
                self.assertEquals(tipe(o),func(i),"arg: %s" % i)
                self.assertEquals(conv(o), test_reporter.last_report)
            except: self.fail("arg: %s threw %s: %s" % (i,sys.exc_info()[0],sys.exc_info()[1]))
        for v in invalid:
            try: r=func(v)
            except (OverflowError,TypeError): return
            except: self.fail("arg: %s threw %s: %s" % (v,sys.exc_info()[0],sys.exc_info()[1]))
            self.fail("arg: %s returns %s" % (v,r))

    def check_simple_set(self,namespace, conv=str):
        self.check_arith(namespace.function_s,int,conv,valid=((-2.0,-2),(-1L,-1),(-1,-1),(0,0),(1,1),(1L,1),(2.0,2),(32767,32767),(-32768,-32768),(True,1),(False,0)),invalid=("hello",))
        self.check_arith(namespace.function_i,int,conv,valid=((-2.0,-2),(-1L,-1),(-1,-1),(0,0),(1,1),(1L,1),(2.0,2),(32767,32767),(-32768,-32768),(True,1),(False,0)),invalid=("hello",))
        self.check_arith(namespace.function_l,int,conv,valid=((-2.0,-2),(-1L,-1),(-1,-1),(0,0),(1,1),(1L,1),(2.0,2),(2147483647,2147483647),(-2147483648,-2147483648),(True,1),(False,0)),invalid=("hello",))
        self.check_arith(namespace.function_ll,long,conv,valid=((-2.0,-2),(-1L,-1),(-1,-1),(0,0),(1,1),(1L,1),(2.0,2),(9223372036854775807,9223372036854775807),(-9223372036854775808,-9223372036854775808L),(True,1),(False,0)),invalid=("hello",))
        self.check_arith(namespace.function_us,int,conv,valid=((0,0),(1,1),(1L,1),(2.0,2),(65535,65535),(True,1),(False,0)),invalid=(-1,-1L,-2.0,"hello"))
        self.check_arith(namespace.function_ui,int,conv,valid=((0,0),(1,1),(1L,1),(2.0,2),(65535,65535),(True,1),(False,0)),invalid=(-1,-1L,-2.0,"hello"))
        self.check_arith(namespace.function_u,int,conv,valid=((0,0),(1,1),(1L,1),(2.0,2),(65535,65535),(True,1),(False,0)),invalid=(-1,-1L,-2.0,"hello"))
        self.check_arith(namespace.function_ul,long,conv,valid=((0,0),(1,1),(1L,1),(2.0,2),(4294967295,4294967295),(True,1),(False,0)),invalid=(-1,-1L,-2.0,"hello"))
        self.check_arith(namespace.function_ull,long,conv,valid=((0,0),(1,1),(1L,1),(2.0,2),(18446744073709551615,18446744073709551615),(True,1),(False,0)),invalid=(-1,-1L,-2.0,"hello"))
        self.check_arith(namespace.function_f,float,conv,valid=((0,'0.00000'),(1,'1.00000'),(1L,'1.00000'),(2.0,'2.00000'),(True,'1.00000'),(False,'0.00000')),invalid=("hello",))
        self.check_arith(namespace.function_d,float,conv,valid=((0,'0.00000'),(1,'1.00000'),(1L,'1.00000'),(2.0,'2.00000'),(True,'1.00000'),(False,'0.00000')),invalid=("hello",))
        self.check_arith(namespace.function_b,lambda s: bool(int(s)),conv,valid=((True,'1'),(False,'0')),invalid=("hello",1,1L,1.0,-1,-1L,-1.0))
        self.check_arith(namespace.function_stdstr1,str,conv,valid=(('he\0llo','he\0llo'),),invalid=(0,1,-1,1L,-1L,0.0))
        self.check_arith(namespace.function_stdstr2,str,conv,valid=(('hello','hello'),),invalid=(0,1,-1,1L,-1L,0.0))
        self.check_arith(namespace.function_cstr,str,conv,valid=(('hello','hello'),),invalid=(0,1,-1,1L,-1L,0.0))

    def test_reporter_function(self):
        self.assertTrue(hasattr(test_module,'set_reporter'))
        self.assertTrue(hasattr(test_module,'get_reporter'))
        self.assertTrue(hasattr(test_module,'report'))

        self.assertTrue(test_module.get_reporter()==test_reporter)

        test_reporter.clear_reports()
        self.assertEquals(0,len(test_reporter.reports))
        self.assertEquals(None,test_reporter.last_report)
        test_module.report('hello')
        self.assertEquals('hello',test_reporter.last_report)
        self.assertEquals(1,len(test_reporter.reports))
        self.assertEquals('hello',test_reporter.reports[0])
        test_module.report('goodbye')
        self.assertEquals('goodbye',test_reporter.last_report)
        self.assertEquals(2,len(test_reporter.reports))
        self.assertEquals('hello',test_reporter.reports[0])
        self.assertEquals('goodbye',test_reporter.reports[1])
        test_reporter.clear_reports()
        self.assertEquals(0,len(test_reporter.reports))
        self.assertEquals(None,test_reporter.last_report)

    def test_module(self):

        self.check_namespace(test_module,'Module Documentation', function_s='Short Function Test', function_i='Int Function Test', function_l='Long Function Test', function_ll='Long Long Function Test', function_us='Unsigned Short Function Test', function_ui='Unsigned Int Function Test', function_ul='Unsigned Long Function Test', function_ull='Unsigned Long Long Function Test', function_u='Unsigned Function Test', function_b='Bool Function Test', function_d='Double Function Test', function_f='Float Function Test', function_stdstr1='stdstr1 Function Test', function_stdstr2='stdstr2 Function Test',function_cstr="cstr Function Test", Reporter="Reporter Class Documentation", TestClass1='TestClass1 Dokmuentation', TestClass2='TestClass2 Dokmuentation', TestClass3='TestClass3 Dokmuentation')
        self.check_namespace(test_module.Reporter,'Reporter Class Documentation',report='Report Function')
        self.check_namespace(test_module.TestClass1,'TestClass1 Dokmuentation',__str__=None,__repr__=None,__hash__=None,__cmp__=None, function_s='Short Function Test', function_i='Int Function Test', function_l='Long Function Test', function_ll='Long Long Function Test', function_us='Unsigned Short Function Test', function_ui='Unsigned Int Function Test', function_ul='Unsigned Long Function Test', function_ull='Unsigned Long Long Function Test', function_u='Unsigned Function Test', function_b='Bool Function Test', function_d='Double Function Test', function_f='Float Function Test', function_stdstr1='stdstr1 Function Test', function_stdstr2='stdstr2 Function Test',function_cstr="cstr Function Test")
        self.check_namespace(test_module.TestClass2,'TestClass2 Dokmuentation',__str__=None,__repr__=None,__hash__=None,__cmp__=None, function_s='Short Function Test', function_i='Int Function Test', function_l='Long Function Test', function_ll='Long Long Function Test', function_us='Unsigned Short Function Test', function_ui='Unsigned Int Function Test', function_ul='Unsigned Long Function Test', function_ull='Unsigned Long Long Function Test', function_u='Unsigned Function Test', function_b='Bool Function Test', function_d='Double Function Test', function_f='Float Function Test', function_stdstr1='stdstr1 Function Test', function_stdstr2='stdstr2 Function Test',function_cstr="cstr Function Test")

        self.assertTrue(hasattr(test_module,'pip_Reporter_dispatch'))
        self.assertTrue(hasattr(test_module,'pip_TestClass1_dispatch'))
        self.assertTrue(hasattr(test_module,'pip_TestClass2_dispatch'))

    def test_simple_functions(self):
        self.check_simple_set(test_module)
        self.check_simple_set(test_module.TestClass1(0))

    def test_virtual_functions(self):

        class TestClass2d(test_module.TestClass2):
                def __init__(self):
                    test_module.TestClass2.__init__(self,0)
                def vfunction_s(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_i(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_l(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_ll(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_us(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_ui(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_ul(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_ull(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_u(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_b(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_d(self,arg):
                    test_reporter.report(arg)
                    return arg
                def vfunction_f(self,arg):
                    test_reporter.report(arg)
                    return arg

        self.check_simple_set(test_module.TestClass2(0))
        self.check_simple_set(TestClass2d(),conv=None)

    def test_repr_str(self):
        m1 = test_module.TestClass1(1)
        m2 = test_module.TestClass2(2)
        self.assertEquals('TestClass1 repr',repr(m1))
        self.assertEquals('TestClass1 str',str(m1))
        self.assertEquals('TestClass2 repr',repr(m2))
        self.assertEquals('TestClass2 str',str(m2))

    def test_call(self):
        m1 = test_module.TestClass1(1)
        m2 = test_module.TestClass2(2)
        self.assertEquals(101,m1(100))
        self.assertAlmostEquals(4.5,m2(1.25,3.25),4)

    def test_cmp_hash(self):

        m11 = test_module.TestClass1(1)
        m12 = test_module.TestClass1(2)
        m13 = test_module.TestClass1(3)

        self.assertTrue(m11<m12)
        self.assertTrue(m11<m13)
        self.assertTrue(m12>m11)
        self.assertTrue(m12<m13)
        self.assertTrue(m13>m11)
        self.assertTrue(m13>m12)
        self.assertEquals(1,hash(m11))
        self.assertEquals(2,hash(m12))
        self.assertEquals(3,hash(m13))

        m21 = test_module.TestClass2(1)
        m22 = test_module.TestClass2(2)
        m23 = test_module.TestClass2(3)

        self.assertTrue(m21<m22)
        self.assertTrue(m21<m23)
        self.assertTrue(m22>m21)
        self.assertTrue(m22<m23)
        self.assertTrue(m23>m21)
        self.assertTrue(m23>m22)
        self.assertEquals(1,hash(m21))
        self.assertEquals(2,hash(m22))
        self.assertEquals(3,hash(m23))

    def test_virtual_inheritance(self):
        TestClass3 = test_module.TestClass3
        TestClass4 = test_module.TestClass4
        TestClass5 = test_module.TestClass5

        class TestClass3d1(TestClass3):
            "TestClass3d1 Dokmuentation"
            def __init__(self):
                TestClass3.__init__(self)

        class TestClass3d2(TestClass3):
            "TestClass3d2 Dokmuentation"
            def __init__(self):
                TestClass3.__init__(self)
            def func(self):
                """ TestClass3d2::func """
                return "TestClass3d2::func"
            def vfunc(self):
                """ TestClass3d2::vfunc """
                return "TestClass3d2::vfunc"
            def pvfunc(self):
                """ TestClass3d2::pvfunc """
                return "TestClass3d2::pvfunc"

        class TestClass4d1(TestClass4):
            "TestClass4d1 Dokmuentation"
            def __init__(self):
                TestClass4.__init__(self)

        class TestClass4d2(TestClass4):
            "TestClass4d2 Dokmuentation"
            def __init__(self):
                TestClass4.__init__(self)
            def func(self):
                """ TestClass4d2::func """
                return "TestClass4d2::func"
            def vfunc(self):
                """ TestClass4d2::vfunc """
                return "TestClass4d2::vfunc"
            def pvfunc(self):
                """ TestClass4d2::pvfunc """
                return "TestClass4d2::pvfunc"

        class TestClass5d1(TestClass5):
            "TestClass5d1 Dokmuentation"
            def __init__(self):
                TestClass5.__init__(self)

        class TestClass5d2(TestClass5):
            "TestClass5d2 Dokmuentation"
            def __init__(self):
                TestClass5.__init__(self)
            def func(self):
                """ TestClass5d2::func """
                return "TestClass5d2::func"
            def vfunc(self):
                """ TestClass5d2::vfunc """
                return "TestClass5d2::vfunc"
            def pvfunc(self):
                """ TestClass5d2::pvfunc """
                return "TestClass5d2::pvfunc"

        self.check_namespace(TestClass3,"TestClass3 Dokmuentation", func='TestClass3::func', vfunc='TestClass3::vfunc', pvfunc='TestClass3::pvfunc')
        self.check_namespace(TestClass3d1,"TestClass3d1 Dokmuentation", func='TestClass3::func', vfunc='TestClass3::vfunc', pvfunc='TestClass3::pvfunc')
        self.check_namespace(TestClass3d2,"TestClass3d2 Dokmuentation", func='TestClass3d2::func', vfunc='TestClass3d2::vfunc', pvfunc='TestClass3d2::pvfunc')
        self.check_namespace(TestClass4,"TestClass4 Dokmuentation", func='TestClass3::func', vfunc='TestClass3::vfunc', pvfunc='TestClass3::pvfunc')
        self.check_namespace(TestClass5,"TestClass5 Dokmuentation", func='TestClass5::func', vfunc='TestClass5::vfunc', pvfunc='TestClass5::pvfunc')
        self.check_namespace(TestClass4d1,"TestClass4d1 Dokmuentation", func='TestClass3::func', vfunc='TestClass3::vfunc', pvfunc='TestClass3::pvfunc')
        self.check_namespace(TestClass4d2,"TestClass4d2 Dokmuentation", func='TestClass4d2::func', vfunc='TestClass4d2::vfunc', pvfunc='TestClass4d2::pvfunc')

        self.check_namespace(TestClass5d1,"TestClass5d1 Dokmuentation", func='TestClass5::func', vfunc='TestClass5::vfunc', pvfunc='TestClass5::pvfunc')
        self.check_namespace(TestClass5d2,"TestClass5d2 Dokmuentation", func='TestClass5d2::func', vfunc='TestClass5d2::vfunc', pvfunc='TestClass5d2::pvfunc')

        test3 = TestClass3()
        test3d1 = TestClass3d1()
        test3d2 = TestClass3d2()
        test4 = TestClass4()
        test4d1 = TestClass4d1()
        test4d2 = TestClass4d2()
        test5 = TestClass5()
        test5d1 = TestClass5d1()
        test5d2 = TestClass5d2()

        self.assertEquals("TestClass3::func",test3.func())
        self.assertEquals("TestClass3::vfunc",test3.vfunc())
        self.assertEquals("TestClass3::func",test3.call_func())
        self.assertEquals("TestClass3::vfunc",test3.call_vfunc())
        self.assertRaises(RuntimeError,test3.call_pvfunc)

        self.assertEquals("TestClass3::func",test3d1.func())
        self.assertEquals("TestClass3::vfunc",test3d1.vfunc())
        self.assertEquals("TestClass3::func",test3d1.call_func())
        self.assertEquals("TestClass3::vfunc",test3d1.call_vfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test3d1))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test3d1))
        self.assertRaises(RuntimeError,test3d1.call_pvfunc)

        self.assertEquals("TestClass3::func",test4.func())
        self.assertEquals("TestClass3::vfunc",test4.vfunc())
        self.assertEquals("TestClass3::func",test4.call_func())
        self.assertEquals("TestClass3::vfunc",test4.call_vfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test4))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test4))
        self.assertRaises(RuntimeError,test4.call_pvfunc)

        self.assertEquals("TestClass3::func",test4d1.func())
        self.assertEquals("TestClass3::vfunc",test4d1.vfunc())
        self.assertEquals("TestClass3::func",test4d1.call_func())
        self.assertEquals("TestClass3::vfunc",test4d1.call_vfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test4d1))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test4d1))
        self.assertEquals("TestClass3::func",TestClass4.func(test4d1))
        self.assertEquals("TestClass3::vfunc",TestClass4.vfunc(test4d1))
        self.assertRaises(RuntimeError,test4d1.call_pvfunc)

        self.assertEquals("TestClass4d2::func",test4d2.func())
        self.assertEquals("TestClass4d2::vfunc",test4d2.vfunc())
        self.assertEquals("TestClass3::func",test4d2.call_func())
        self.assertEquals("TestClass4d2::vfunc",test4d2.call_vfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test4d2))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test4d2))
        self.assertEquals("TestClass3::func",TestClass4.func(test4d2))
        self.assertEquals("TestClass3::vfunc",TestClass4.vfunc(test4d2))
        self.assertEquals("TestClass4d2::pvfunc",test4d2.call_pvfunc())

        self.assertEquals("TestClass5::func",test5.func())
        self.assertEquals("TestClass5::vfunc",test5.vfunc())
        self.assertEquals("TestClass5::pvfunc",test5.pvfunc())
        self.assertEquals("TestClass3::func",test5.call_func())
        self.assertEquals("TestClass5::vfunc",test5.call_vfunc())
        self.assertEquals("TestClass5::pvfunc",test5.call_pvfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test5))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test5))

        self.assertEquals("TestClass3d2::func",test3d2.func())
        self.assertEquals("TestClass3d2::vfunc",test3d2.vfunc())
        self.assertEquals("TestClass3d2::pvfunc",test3d2.pvfunc())
        self.assertEquals("TestClass3::func",test3d2.call_func())
        self.assertEquals("TestClass3d2::vfunc",test3d2.call_vfunc())
        self.assertEquals("TestClass3d2::pvfunc",test3d2.call_pvfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test3d2))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test3d2))

        self.assertEquals("TestClass5::func",test5d1.func())
        self.assertEquals("TestClass5::vfunc",test5d1.vfunc())
        self.assertEquals("TestClass5::pvfunc",test5d1.pvfunc())
        self.assertEquals("TestClass3::func",test5d1.call_func())
        self.assertEquals("TestClass5::vfunc",test5d1.call_vfunc())
        self.assertEquals("TestClass5::pvfunc",test5d1.call_pvfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test5d1))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test5d1))
        self.assertEquals("TestClass5::func",TestClass5.func(test5d1))
        self.assertEquals("TestClass5::vfunc",TestClass5.vfunc(test5d1))

        self.assertEquals("TestClass5d2::func",test5d2.func())
        self.assertEquals("TestClass5d2::vfunc",test5d2.vfunc())
        self.assertEquals("TestClass5d2::pvfunc",test5d2.pvfunc())
        self.assertEquals("TestClass3::func",test5d2.call_func())
        self.assertEquals("TestClass5d2::vfunc",test5d2.call_vfunc())
        self.assertEquals("TestClass5d2::pvfunc",test5d2.call_pvfunc())
        self.assertEquals("TestClass3::func",TestClass3.func(test5d2))
        self.assertEquals("TestClass3::vfunc",TestClass3.vfunc(test5d2))
        self.assertEquals("TestClass5::func",TestClass5.func(test5d2))
        self.assertEquals("TestClass5::vfunc",TestClass5.vfunc(test5d2))

    def test_own(self):
        tf = test_module.TestFactory()
        t3 = tf.create()
        self.assertEquals("TestClass5::pvfunc", t3.pvfunc())
        t3 = test_module.TestClass3()
        self.assertRaises(RuntimeError,t3.call_pvfunc)


if __name__ == '__main__':
    unittest.main()
