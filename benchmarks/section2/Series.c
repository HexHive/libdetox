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
*                  Original version of this code by                       *
*                 Gabriel Zachmann (zach@igd.fhg.de)                      *
*                                                                         *
*      This version copyright (c) The University of Edinburgh, 2001.      *
*                         All rights reserved.                            *
*                                                                         *
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include <libfastbt.h>
#include "Series.h" 
#include "getclock.h"

int size; 
int datasizes[]={10000,100000,1000000};

double start_time, end_time, kernel_time; 
int ops; 

int array_rows; 
double **TestArray; 


void buildTestData()
{
  int j,i;

 
  TestArray = (double **) malloc(sizeof (double*) * 2); 
  TestArray[0] = (double *) malloc(sizeof(double) * array_rows);
  TestArray[1] = (double *) malloc(sizeof(double) * array_rows);
}



/*
* Do
*
* This consists of calculating the
* first n pairs of fourier coefficients of the function (x+1)^x on
* the interval 0,2. n is given by array_rows, the array size.
* NOTE: The # of integration steps is fixed at 1000. 
*/

void Do()
{
    double omega; 
    int i;

    start_time = getclock();
   
#ifdef ENABLE_BT
    fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif

    /* Calculate the fourier series. Begin by calculating A[0]. */ 

    TestArray[0][0]=TrapezoidIntegrate((double)0.0, 
                            (double)2.0,           
                            1000,                  
                            (double)0.0,           
                            0) / (double)2.0;      

    /* Calculate the fundamental frequency.
       ( 2 * pi ) / period...and since the period
       is 2, omega is simply pi. */ 

    omega = (double) 3.1415926535897932;

    for (i = 1; i < array_rows; i++)
    {
      /* Calculate A[i] terms. Note, once again, that we
         can ignore the 2/period term outside the integral
         since the period is 2 and the term cancels itself
         out. */ 

        TestArray[0][i] = TrapezoidIntegrate((double)0.0,
                          (double)2.0,
                          1000,
                          omega * (double)i,
                          1);                      

        /* Calculate the B[i] terms. */ 

        TestArray[1][i] = TrapezoidIntegrate((double)0.0,
                          (double)2.0,
                          1000,
                          omega * (double)i,
                          2);                      
    }

#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif

    end_time = getclock();
    kernel_time = end_time - start_time;   


}

/*
* TrapezoidIntegrate
*
* Perform a simple trapezoid integration on the function (x+1)**x.
* x0,x1 set the lower and upper bounds of the integration.
* nsteps indicates # of trapezoidal sections.
* omegan is the fundamental frequency times the series member #.
* select = 0 for the A[0] term, 1 for cosine terms, and 2 for
* sine terms. Returns the value.
*/

double TrapezoidIntegrate (double x0,    
			   double x1,         
			   int nsteps,           
			   double omegan,           
			   int select)               
{
    double x;      
    double dx;     
    double rvalue; 

    x = x0;

    dx = (x1 - x0) / (double)nsteps;

    rvalue = thefunction(x0, omegan, select) / (double)2.0;

    if (nsteps != 1)
    {
      --nsteps;              
      while (--nsteps > 0)
	{
	  x += dx;
	  rvalue += thefunction(x, omegan, select);
	}
    }


    rvalue=(rvalue + thefunction(x1,omegan,select) / (double)2.0) * dx;
    return(rvalue);
}

/*
* thefunction
*
* This routine selects the function to be used in the Trapezoid
* integration. x is the independent variable, omegan is omega * n,
* and select chooses which of the sine/cosine functions
* are used. Note the special case for select=0.
*/

double thefunction(double x,  
		   double omegan,
		   int select)      
{

  /* Use select to pick which function we call. */ 

    switch(select)
    {
        case 0: return(pow(x+(double)1.0,x));

        case 1: return(pow(x+(double)1.0,x) * cos(omegan*x));

        case 2: return(pow(x+(double)1.0,x) * sin(omegan*x));
    }

    /* We should never reach this point, but the following  
       keeps compilers from issuing a warning message. */ 

    return (0.0);
}

/*
* freeTestData
*
* Nulls array that is created with every run and forces garbage
* collection to free up memory.
*/

void freeTestData()
{
    free (TestArray);
}


void initialise(){
  array_rows = datasizes[size];
  buildTestData();
}
 
void kernel(){
    Do(); 
}

void validate(){
     double ref[4][2] = {{2.8729524964837996, 0.0},
                       {1.1161046676147888, -1.8819691893398025},
                       {0.34429060398168704, -1.1645642623320958},
                       {0.15238898702519288, -0.8143461113044298}};
    int i,j; 
    double error; 

    for (i = 0; i < 4; i++){
      for (j = 0; j < 2; j++){
	error = fabs(TestArray[j][i] - ref[i][j]);
	if (error > 1.0e-12 ){
	  printf("Validation failed for coefficient %d %d\n",j,i);
	  printf("Computed value = %.20f\n", TestArray[j][i]);
	  printf("Reference value =  %.20f\n", ref[i][j]);
	}
      }
    }
}

void tidyup(){
  freeTestData(); 
}  



void run(int sizein){

   size = sizein; 

   initialise(); 

   kernel(); 

   validate(); 
   tidyup(); 

   ops = array_rows * 2 - 1;   
   printf("Section2:Series:Kernel    %f (s)       %f      (Iterations/s)\n", kernel_time, (double) ops / kernel_time);
  
}
