
/*
 Copyright 2009 Eigenlabs Ltd.  http://www.eigenlabs.com

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

#ifdef _WIN32  
#ifdef _DEBUG
#define __REDEF_DEBUG__
#undef _DEBUG
#pragma message ("Undef _DEBUG") 
#endif
#endif

#include <Python.h>

#ifdef __REDEF_DEBUG__
#define _DEBUG
#undef __REDEF_DEBUG__
#pragma message ("redefine _DEBUG")
#endif


#include <picross/pic_log.h>
#include <picross/pic_error.h>
#include <piw/piw_tsd.h>

#include <cstdlib>
#include <cstring>
#include <vector>
#include <list>

#include "pis_python.h"

#if PY_VERSION_HEX >= 0x02050000
    typedef Py_ssize_t my_ssize_t;
#else
    typedef int my_ssize_t;
#endif

namespace
{
    class interp_t : public pisession::interpreter_it
    {
        public:
            interp_t(const char *zip, const char *boot, const char *setup): tstate_(0)
            {
                PyEval_AcquireLock();
                tstate_ = Py_NewInterpreter();

                bool ok = initPython(zip,boot,setup);

                PyEval_ReleaseThread(tstate_);

                if(!ok)
                {
                    PIC_THROW("Can't initialise interpreter");
                }
            }

            void enter()
            {
                PyEval_AcquireThread(tstate_);
            }

            void exit()
            {
                PyEval_ReleaseThread(tstate_);
            }

            virtual pisession::agent_t create_agent(const pia::context_t &env, const char *module, const char *name);

            virtual ~interp_t()
            {
                if(tstate_)
                {
                    PyEval_AcquireThread(tstate_);
                    Py_EndInterpreter(tstate_);
                    PyEval_ReleaseLock();
                }
            }

            bool initPython(const char *zip, const char *boot, const char *setup)
            {
                PyObject *sys=0, *ppath=0, *spath=0, *smodule = 0, *bmodule=0, *bfunc=0, *bargs=0, *sfunc=0;

                if(!(sys=PyImport_AddModule("sys"))) goto err;
                if(!(spath=PyObject_GetAttrString(sys,"path"))) goto err;
                if(!(ppath = PyString_FromString(zip))) goto err;

                PyList_Append(spath,ppath);

                if(!(bmodule = PyImport_ImportModule((char *)boot))) goto err;
                if(!(bfunc = PyObject_GetAttrString(bmodule,"bootstrap")) || !PyCallable_Check(bfunc)) goto err;
                if(!(bargs = PyTuple_New(0))) goto err;

                if(!PyObject_CallObject(bfunc,bargs)) goto err;

                if(!(smodule = PyImport_ImportModule((char *)setup))) goto err;
                if(!(sfunc = PyObject_GetAttrString(smodule,"setup")) || !PyCallable_Check(sfunc)) goto err;

                if(!PyObject_CallObject(sfunc,bargs)) goto err;

                Py_DECREF(spath);
                Py_DECREF(ppath);
                Py_DECREF(bmodule);
                Py_DECREF(smodule);
                Py_DECREF(bfunc);
                Py_DECREF(sfunc);
                Py_DECREF(bargs);

                return true;

            err:

                if(spath) { Py_DECREF(spath); }
                if(ppath) { Py_DECREF(ppath); }
                if(bmodule) { Py_DECREF(bmodule); }
                if(smodule) { Py_DECREF(smodule); }
                if(bfunc) { Py_DECREF(bfunc); }
                if(sfunc) { Py_DECREF(sfunc); }
                if(bargs) { Py_DECREF(bargs); }

                PyErr_Print();
                return false;
            }

        PyThreadState *tstate_;
    };

    typedef pic::ref_t<interp_t> rinterpreter_t;

    class module_t : public pisession::agent_it
    {
        public:
            module_t(const rinterpreter_t &interp, const pia::context_t &env, const char *module,const char *name): interp_(interp), env_(env)
            {
                if(!initPython(module,name))
                {
                    pic::msg_t m;
                    m << "Can't initialise script for " << module << " (" << name << ")";
                    pic::hurl(m);
                }
            }

            virtual ~module_t()
            {
            }

            bool initPython(const char *agent, const char *name)
            {
                PyObject *module=0, *func=0, *args=0, *env=0, *pyname=0, *unload=0, *fini=0;

                python_=0;

                interp_->enter();

                if(!(pyname = PyString_FromString(name))) goto err;
                if(!(module = PyImport_ImportModule((char *)agent))) goto err;
                if(!(func = PyObject_GetAttrString(module,"main")) || !PyCallable_Check(func)) goto err;
                if(!(unload = PyObject_GetAttrString(module,"unload")) || !PyCallable_Check(unload)) goto err;
                if(!(fini = PyObject_GetAttrString(module,"fini")) || !PyCallable_Check(fini)) goto err;
                if(!(args = PyTuple_New(2))) goto err;
                if(!(env = PyCObject_FromVoidPtr(env_.entity(),0))) goto err;

                PyTuple_SetItem(args,0,env); Py_INCREF(env);
                PyTuple_SetItem(args,1,pyname); Py_INCREF(pyname);

                if(!(python_ = PyObject_CallObject(func,args))) goto err;

                Py_DECREF(module);
                Py_DECREF(func);
                Py_DECREF(args);
                Py_DECREF(env);
                Py_DECREF(pyname);

                unload_ = unload;
                fini_ = fini;
                interp_->exit();
                return true;

            err:

                if(module) { Py_DECREF(module); }
                if(func) { Py_DECREF(func); }
                if(args) { Py_DECREF(args); }
                if(env) { Py_DECREF(env); }
                if(pyname) { Py_DECREF(pyname); }
                if(python_) { Py_DECREF(python_); }
                if(unload) { Py_DECREF(unload); }
                if(fini) { Py_DECREF(fini); }

                python_=0;
                unload_=0;
                fini_=0;

                PyErr_Print();
                interp_->exit();
                return false;
            }

            std::string __unload()
            {
                PyObject *env = PyCObject_FromVoidPtr(env_.entity(),0);
                PyObject *args = PyTuple_New(2);
                PyTuple_SetItem(args,0,env);
                PyTuple_SetItem(args,1,python_); Py_INCREF(python_);
                PyObject *rv = PyObject_CallObject(unload_,args);
                Py_DECREF(args);
                Py_DECREF(unload_); unload_=0;

                if(!rv)
                {
                    PyErr_Print();
                    return "";
                }

                my_ssize_t rvl;
                char *rvs;
                std::string rrv;

                if(PyString_AsStringAndSize(rv,&rvs,&rvl)>=0)
                {
                    rrv=std::string(rvs,rvl);
                }
                else
                {
                    PyErr_Print();
                }

                Py_DECREF(rv);
                return rrv;
            }

            std::string unload()
            {
                bct_entity_t x = piw::tsd_getcontext();
                //pia::context_t env(env_);

                try
                {
                    if(python_)
                    {
                        //piw::tsd_setcontext(env.entity());
                        piw::tsd_setcontext(env_.entity());
                        piw::tsd_lock();


                        //printf("locking interp for unload\n"); fflush(stdout);
                        //printf("locked interp for unload\n"); fflush(stdout);
                        std::string rv = __unload();
                        //printf("unlocking interp for unload\n"); fflush(stdout);
                        //printf("unlocked interp for unload\n"); fflush(stdout);

                        //printf("enter interp\n"); fflush(stdout);
                        //interp_->enter();

                        Py_DECREF(python_);
                        python_=0;

                        //printf("exit interp\n"); fflush(stdout);
                        //interp_->exit();

                        //printf("unlocking and restoring ctx\n"); fflush(stdout);
                        piw::tsd_unlock();
                        piw::tsd_setcontext(x);

                        env_.release();
                        __fini();

                        return rv;
                    }
                }
                catch(...)
                {
                    piw::tsd_unlock();
                    piw::tsd_setcontext(x);
                    throw;
                }

                return "";
            }

            pia::context_t env()
            {
                return env_;
            }

            void __fini()
            {
                PyObject *args = PyTuple_New(0);
                PyObject *rv = PyObject_CallObject(fini_,args);
                Py_DECREF(rv);
                Py_DECREF(args);
                Py_DECREF(fini_); fini_=0;
            }


        PyObject *python_;
        PyObject *unload_;
        PyObject *fini_;
        rinterpreter_t interp_;
        pia::context_t env_;
    };

}

pisession::agent_t interp_t::create_agent(const pia::context_t &env, const char *module, const char *name)
{
    return pic::ref(new module_t(rinterpreter_t::from_lent(this), env, module, name));
}

pisession::interpreter_t pisession::create_interpreter(const char *zip, const char *boot, const char *setup)
{
    return pic::ref(new interp_t(zip,boot,setup));
}
