/*
    Copyright (C) 2006-2007 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef __CONVOLVER_H
#define __CONVOLVER_H

#define ZITA_THREADING 0

#include <picross/pic_config.h>
#include <plg_convolver/src/convolver_exports.h>

#if ZITA_THREADING
#include <pthread.h>
#include <semaphore.h>
#define DEFAULT_PRIORITY SCHED_FIFO
#else
#define DEFAULT_PRIORITY 0
#endif

#define CALLING_FFTW
#include <lib_fftw/fftw3.h>


class CONVOLVER_DECLSPEC_CLASS Inpnode
{
private:

    friend class Convlevel;

    Inpnode        *_next;
    fftwf_complex **_ffta;
    float          *_buff;
    unsigned int    _inp;
};


class CONVOLVER_DECLSPEC_CLASS Macnode
{
private:

    friend class Convlevel;

    Macnode        *_next;
    Inpnode        *_inpn;
    fftwf_complex **_fftb;
    bool            _copy;
};


class CONVOLVER_DECLSPEC_CLASS Outnode
{
private:

    friend class Convlevel;

    Outnode        *_next;
    Macnode        *_list;
    float          *_buff [3];
    unsigned int    _out;
};


class CONVOLVER_DECLSPEC_CLASS Converror
{
public:

    enum
    {
        BAD_STATE = -1,
        BAD_PARAM = -2,
        MEM_ALLOC = -3
    };

private:

    friend class Convlevel;
    friend class Convproc;

    Converror (int error) : _error (error) {}

    int _error;
};


class CONVOLVER_DECLSPEC_CLASS Convlevel
{
#if ZITA_THREADING==1
private:

    friend class Convproc;
#else
public:
#endif // ZITA_THREADING==1

    enum { ST_IDLE, ST_TERM, ST_PROC };

    Convlevel (void);
    ~Convlevel (void);

    void *alloc_aligned (size_t size);
    void start (int priority, int schclass);
    void stop (void);
    void configure (unsigned int offs,
                    unsigned int npar,
                    unsigned int partsize,
                    unsigned int outstep,
                    unsigned int fftwopt,
                    unsigned int vectopt);
    void impdata_create (unsigned int inp,
                            unsigned int out,
                            unsigned int step,
                            float *data,
                            int ind0,
                            int ind1);
    void impdata_update (unsigned int inp,
                            unsigned int out,
                            unsigned int step,
                            float *data,
                            int ind0,
                            int ind1);
    void impdata_copy (unsigned int inp1,
                        unsigned int out1,
                        unsigned int inp2,
                        unsigned int out2);
    void prepare (unsigned int inpsize, float **inpbuff);
    void fftswap (fftwf_complex *p);
    void cleanup (void);
    int  readout (float **outbuff, bool skip);
    void print (void);
    bool idle (void) const { return _stat == ST_IDLE; }

    static void *static_main (void *arg);
    void main (void);
    void process (bool skip);
    Macnode *findmacnode (unsigned int inp, unsigned int out, bool create);

    volatile unsigned int  _stat; // current processing state

    float            _norm;       // normalisation factor
    unsigned int     _offs;       // offset from start of impulse response
    unsigned int     _npar;       // number of partitions
    unsigned int     _parsize;    // partition and outbut buffer size
    unsigned int     _outoffs;    // offset into  output buffer
    unsigned int     _outstep;    // step size for output buffer
    unsigned int     _inpsize;    // size of shared input buffer
    unsigned int     _inpoffs;    // offset into input buffer
    unsigned int     _vectopt;    // vector optimisation options
    unsigned int     _ipar;       // rotating partition index
    unsigned int     _opi1;       // rotating output buffer indices
    unsigned int     _opi2;
    unsigned int     _opi3;
    int              _bits;
    int              _late;
#if ZITA_THREADING==1
    pthread_t        _pthr;
    sem_t            _trig;
#endif

    Inpnode         *_inp_list;
    Outnode         *_out_list;
    fftwf_plan       _plan_r2c;
    fftwf_plan       _plan_c2r;
    float           *_time_data;
    float           *_prep_data;
    fftwf_complex   *_freq_data;
};


class CONVOLVER_DECLSPEC_CLASS Convproc
{
public:

    Convproc (void);
    ~Convproc (void);

    enum
    {
        ST_IDLE,
        ST_STOP,
        ST_WAIT,
        ST_PROC
    };

    enum
    {
        FL_LATE = 0x0000FFFF,
        FL_LOAD = 0x01000000
    };

    enum
    {
        MAXINP   = 64,
        MAXOUT   = 64,
        MINPART  = 64,
        MAXPART  = 8192,
        MAXDIVIS = 16,
        MINQUANT = 16,
        MAXQUANT = 8192
    };

    float *inpdata (unsigned int inp)
    {
        return _inpbuff [inp] + _inpoffs;
    }

    float *outdata (unsigned int out)
    {
        return _outbuff [out] + _outoffs;
    }

    unsigned int state (void) const
    {
        return _state;
    }

    unsigned int flags (void)
    {
        unsigned int f = _flags;
        _flags = 0;
        return f;
    }

    void set_fftwopt (unsigned int fftwopt)
    {
        _fftwopt = fftwopt;
    }

    void set_vectopt (unsigned int vectopt)
    {
        _vectopt = vectopt;
    }

    int configure (unsigned int ninp,
                    unsigned int nout,
                    unsigned int maxsize,
                    unsigned int quantum,
                    unsigned int minpart,
                    unsigned int maxpart);

    int impdata_create (unsigned int inp,
                        unsigned int out,
                        unsigned int step,
                        float       *data,
                        int          ind0,
                        int          ind1);

    int impdata_update (unsigned int inp,
                        unsigned int out,
                        unsigned int step,
                        float       *data,
                        int          ind0,
                        int          ind1);

    int impdata_copy (unsigned int inp1,
                        unsigned int out1,
                        unsigned int inp2,
                        unsigned int out2);

    int start_process (int priority, int schclass = DEFAULT_PRIORITY);

    int stop_process (void);

    int cleanup (void);

    void check (void);

    void print (void);

    void process (bool skip = false);

private:

    enum
    {
        NPROC  = 5,
        PMASK2 = 0x1540,
        PMASK4 = 0x2A80
    };

    unsigned int    _state;
    unsigned int    _flags;
    unsigned int    _ninp;
    unsigned int    _nout;
    unsigned int    _nproc;
    unsigned int    _quantum;
    unsigned int    _minpart;
    unsigned int    _maxpart;
    unsigned int    _inpsize;
    unsigned int    _inpoffs;
    unsigned int    _outoffs;
    unsigned int    _fftwopt;
    unsigned int    _vectopt;
    unsigned int    _procdel;
    Convlevel       _procs [NPROC];
    float          *_inpbuff [MAXINP];
    float          *_outbuff [MAXOUT];
};


#endif

