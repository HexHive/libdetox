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

#include <sys/time.h>
#include <stdio.h>
extern double jgf_get_time(void);
extern void jgf_init_time(void);

time_t starttime = 0; 

double jgf_get_time()
{

  struct timeval ts; 

  double t;

  int err; 

  err = gettimeofday(&ts, NULL); 

  t = (double) (ts.tv_sec - starttime)  + (double) ts.tv_usec * 1.0e-6; 
 
  return t; 

}

void jgf_init_time()
{
  struct  timeval  ts;
  int err; 

  err = gettimeofday(&ts, NULL);
  starttime = ts.tv_sec; 
}


