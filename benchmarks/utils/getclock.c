/**************************************************************************
*                                                                         *
*                 Java Grande Forum Benchmark Suite                       *
*                                                                         *
*                           C Version 1.0                                 *
*                                                                         *
*                            produced by                                  *
*                                                                         *
*                  Java Grande Benchmarking Project                       *
*                                                                         *
*                                at                                       *
*                                                                         *
*                Edinburgh Parallel Computing Centre                      *
*                                                                         *
*                email: epcc-javagrande@epcc.ed.ac.uk                     *
*                                                                         *
*      This version copyright (c) The University of Edinburgh, 2001.      *
*                         All rights reserved.                            *
*                                                                         *
**************************************************************************/

#include "getclock.h"
static int firstcall = 1; 

double getclock()
{
      double time;
      double jgf_get_time(void);  
      void jgf_init_time(void);      

      if (firstcall) {
         jgf_init_time(); 
         firstcall = 0;
      }

      time = jgf_get_time(); 
      return time;
} 


