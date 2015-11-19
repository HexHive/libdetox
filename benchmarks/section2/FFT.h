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

double* RandomVector(int); 
void transform (void);
void inverse (void);
int log2 (int); 
void transform_internal(double *, int); 
void bitreverse(double *); 
void initialise();
void kernel(); 
void validate(); 
void tidyup();
void run(int);


float uni(void); 
void rstart(int,int,int,int);
void rinit(int); 
