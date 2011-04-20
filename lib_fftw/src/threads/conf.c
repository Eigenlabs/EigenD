/*
 * Copyright (c) 2003, 2007-8 Matteo Frigo
 * Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include "threads.h"

static const solvtab s =
{
#if defined(HAVE_THREADS) || defined(HAVE_OPENMP)

     SOLVTAB(X(dft_thr_vrank_geq1_register)),
     SOLVTAB(X(rdft_thr_vrank_geq1_register)),
     SOLVTAB(X(rdft2_thr_vrank_geq1_register)),

#endif 

     SOLVTAB_END
};

void X(threads_conf_standard)(planner *p)
{
     X(solvtab_exec)(s, p);
}
