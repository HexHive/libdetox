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

extern double jgf_get_time(void);
extern void jgf_init_time(void);


double starttime = 0; 


double jgf_get_time()
{


SYSTEMTIME ts, te;
double t;

  GetSystemTime(&ts);

  t = (double) (3600*ts.wHour + 60*ts.wMinute + ts.wSecond - starttime) + (double) ts.wMilliseconds * 1.0e-3; 

  return t; 


}

void jgf_init_time()
{

SYSTEMTIME ts, te;

  GetSystemTime(&te);

  starttime = 3600*te.wHour +60*te.wMinute + te.wSecond;

} 
