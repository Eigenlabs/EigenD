
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

#include "epython.h"
#include <iostream>

#include <picross/pic_resources.h>
#include <picross/pic_thread.h>

void epython::PythonInterface::py_startup()
{
    char *pyhome = strdup(pic::python_prefix_dir().c_str());
    Py_SetPythonHome(pyhome);

    pic_init_dll_path();

    Py_Initialize();

    {
      std::string root = pic::release_root_dir();
      char cmdbuffer[4096];
      char escbuffer[4096];

      char *q = escbuffer;
      const char *p = root.c_str();

      while(*p)
      {
        if(*p=='\\')
        {
            *q++ = '\\';
        }

        *q++ = *p++;
      }

      *q = 0;

      sprintf(cmdbuffer,
         "import sys,os\n"
         "if '' in sys.path: sys.path.remove('')\n"
         "if os.getcwd() in sys.path: sys.path.remove(os.getcwd())\n"
         "sys.path.insert(0,os.path.join('%s','modules'))\n"
         "sys.path.insert(0,os.path.join('%s','bin'))\n"
            ,escbuffer,escbuffer
      );

      PyRun_SimpleString(cmdbuffer);
    }

    thread_ = PyEval_SaveThread();
}

void epython::PythonInterface::py_shutdown()
{
    if(!thread_)
        return;

    PyEval_RestoreThread((PyThreadState *)thread_);
    Py_Finalize();
}

bool epython::PythonInterface::get_thread()
{
    if(!thread_)
    {
        return false;
    }

    PyEval_RestoreThread((PyThreadState *)thread_);
    thread_ = 0;
    return true;
}

void epython::PythonInterface::set_thread()
{
    thread_ = PyEval_SaveThread();
}

bool epython::PythonBackend::init_python(const char *module, const char *method)
{
    if(!interface_->get_thread())
        return false;

    PyObject *o_module = 0, *o_func = 0, *o_args = 0;
    PyObject *o_object;

    bool error = true;

    if(!(o_module = PyImport_ImportModule((char *)module))) goto err;
    if(!(o_func = PyObject_GetAttrString(o_module,method)) || !PyCallable_Check(o_func)) goto err;
    if(!(o_args = PyTuple_New(0))) goto err;
    if(!(o_object = PyObject_CallObject(o_func,o_args))) goto err;

    object_ = o_object;
    error = false;
err:

    if(o_module) { Py_DECREF(o_module); }
    if(o_func) { Py_DECREF(o_func); }
    if(o_args) { Py_DECREF(o_args); };


    if(error)
        handle_error();

    interface_->set_thread();
    return !error;
}

void epython::PythonBackend::handle_error()
{
    last_error_ = std::string();

    PyErr_Print();

    PyObject *o_main = 0, *o_main_dict = 0, *o_output = 0;

    if(!(o_main = PyImport_AddModule("__main__"))) goto err2;
    if(!(o_main_dict = PyModule_GetDict(o_main))) goto err2;
    if(!(o_output = PyRun_String("sys.stdout.data", Py_eval_input, o_main_dict, o_main_dict))) goto err2;

    if(o_output && PyString_Check(o_output))
    {
        char* output_str = PyString_AsString(o_output);
        std::cerr << output_str << std::endl;
        last_error_ = std::string(output_str);
    }

err2:

    if(o_output) { Py_DECREF(o_output); }
    if(o_main_dict) { Py_DECREF(o_main_dict); }
    if(o_main) { Py_DECREF(o_main); }
}

epython::PythonBackend::~PythonBackend()
{
    if(object_)
    {
        Py_DECREF((PyObject *)object_);
        object_ = 0;
    }
}

void *epython::PythonBackend::mediator()
{
    if(!object_)
        return 0;

    if(!interface_->get_thread())
        return 0;

    void *m = 0;
    PyObject *o_func = 0, *o_args = 0, *o_object = 0;
    bool error = true;

    if(!(o_func = PyObject_GetAttrString((PyObject *)object_,"mediator")) || !PyCallable_Check(o_func)) goto err;
    if(!(o_args = PyTuple_New(0))) goto err;
    if(!(o_object = PyObject_CallObject(o_func,o_args))) goto err;

    m = PyCObject_AsVoidPtr(o_object);
    error = false;

err:

    if(error)
        handle_error();

    if(o_object) { Py_DECREF(o_object); }
    if(o_func) { Py_DECREF(o_func); }
    if(o_args) { Py_DECREF(o_args); }

    interface_->set_thread();
    return m;
}

std::string epython::PythonBackend::last_error()
{
    return last_error_;
}
