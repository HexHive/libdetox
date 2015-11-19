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

#include<sys/time.h> 

double jgf_get_time(void);
void jgf_init_time(void);


double jgf_get_time()
{

  long long ts; 

  double t;

  ts = gethrtime();

  t = (double) ts * 1.0e-9; 

  return t; 


}

void jgf_init_time()
{

} 

