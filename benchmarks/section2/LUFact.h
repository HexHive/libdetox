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

double matgen (double **, int, int, double *);
int dgefa(double **, int, int, int *);
void dgesl( double **, int, int, int *, double *, int); 
void daxpy( int, double, double *, int, int, double *, int, int);
void dscal( int, double, double *, int, int);
double ddot( int n, double *, int, int, double *, int, int);
int idamax( int, double *, int, int);
double epslon (double);
void dmxpy ( int, double *, int, int, double *, double **);
void initialise(void);
void kernel(void);
void validate(void);
void tidyup(void);
void run(int); 



