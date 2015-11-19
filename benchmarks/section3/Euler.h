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

struct Statevector
{
  double a;   /* Storage for Statevectors */
  double b;
  double c;
  double d;
};

struct Vector2
{
  double ihat;   /* Storage for 2-D vector */
  double jhat; 
};

void init(void); 
void doIteration(void);
void calculateStateVar(double **, double **, struct Statevector **);
void calculateR(void);  
void calculateG(double **, double **,struct Statevector **); 
void calculateF(double **, double **,struct Statevector **); 
void calculateDamping(double **,struct Statevector **); 
void calculateDeltaT(void);
void calculateDummyCells(double **, double **,struct Statevector **);
void runiters(void);
double** newdoublearray(int, int);
struct Statevector** newStatevectorarray(int, int);
double magnitude(struct Vector2*);
double dot(struct Vector2 *, struct Vector2 *);  
