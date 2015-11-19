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
*      adapted from SciMark 2.0, author Roldan Pozo (pozo@cam.nist.gov)   *
*                                                                         *
*      This version copyright (c) The University of Edinburgh, 2001.      *
*                         All rights reserved.                            *
*                                                                         *
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include <libfastbt.h>
#include "SOR.h" 
#include "getclock.h"

int size; 
int datasizes_M[] = {50000,100000,500000};
int datasizes_N[] = {50000,100000,500000};
int datasizes_nz[] = {250000,500000,2500000};
int SPARSE_NUM_ITER = 200;

double start_time, end_time, kernel_time; 
int ops; 

double *x; 
double *y; 
double *val; 
int *col;
int *row;

double ytotal = 0.0;
int nz; 


double* RandomVector(int N) {
                
  double *A; 
  int i; 
  A = (double *) malloc (sizeof(double) * N); 
 
  rinit(1010101); 

  for (i=0; i<N; i++)
    A[i] = (double) uni() * 1e-6;

  return A;
}


void test()
{
		
    int reps,i; 

    start_time = getclock();

#ifdef ENABLE_BT
    fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif

    for (reps=0; reps<SPARSE_NUM_ITER; reps++)
      {
	for (i=0; i<nz; i++)
	  {
	    y[ row[i] ] += x[ col[i] ] * val[i];
	  }
      }
   
#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif

    end_time = getclock();
    kernel_time = end_time - start_time;   


    for (i=0; i<nz; i++) {
      ytotal += y[ row[i] ];
    }

}





void initialise(){

  int i; 

  x = RandomVector(datasizes_N[size]);
  y = (double *) malloc(sizeof(double)*datasizes_M[size]);


  nz = datasizes_nz[size]; 
  val = (double *) malloc(sizeof(double)*nz);
  col = (int *) malloc(sizeof(int)*nz);
  row = (int *) malloc(sizeof(int)*nz);

  rinit(1966); 

  for (i=0; i<nz; i++) {

    /* generate random row index (0, M-1) */ 
    row[i] = (int) (uni() * datasizes_M[size]);
    
    /* generate random column index (0, N-1) */ 
    col[i] = (int) (uni() * datasizes_N[size]);
    
    val[i] = (double) uni();
    
  }
  
}
 
void kernel(){

    test();

  }

void validate(){

  double refval[] = {74.88852099218826197102,150.30633831746624196057,749.54537470282355116069};
  double dev = fabs(ytotal - refval[size]);
  if (dev > 1.0e-12 ){
    printf("Validation failed\n");
    printf("ytotal =  %.20f  %.20f %d\n",ytotal, dev, size);
  }

}

void tidyup(){
  free(y);
  free(x); 
  free(val); 
  free(row);
  free(col);
}  



void run(int sizein){

   size = sizein; 

   initialise(); 
   
   
   kernel(); 
   
   validate(); 
   tidyup();     

   ops = SPARSE_NUM_ITER;     
   printf("Section2:SparseMatmult:Kernel    %f (s)       %f      (Iterations/s)\n", kernel_time, (double) ops / kernel_time);   

}

       



/*
 *	Global variables for rstart & uni
 */

float uni_u[98];	
float uni_c, uni_cd, uni_cm;
int uni_ui, uni_uj;

  float uni(void) {

	float luni;			/* local variable for uni */

	luni = uni_u[uni_ui] - uni_u[uni_uj];
	if (luni < 0.0)
		luni += 1.0;
	uni_u[uni_ui] = luni;
	if (--uni_ui == 0)
		uni_ui = 97;
	if (--uni_uj == 0)
		uni_uj = 97;
	if ((uni_c -= uni_cd) < 0.0)
		uni_c += uni_cm;
	if ((luni -= uni_c) < 0.0)
		luni += 1.0;
	return (float) luni;
}

  void rstart(int i, int j, int k, int l) {

	int ii, jj, m;
	float s, t;

	for (ii = 1; ii <= 97; ii++) {
		s = 0.0;
		t = 0.5;
		for (jj = 1; jj <= 24; jj++) {
			m = ((i*j % 179) * k) % 179;
			i = j;
			j = k;
			k = m;
			l = (53*l+1) % 169;
			if (l*m % 64 >= 32)
				s += t;
			t *= 0.5;
		}
		uni_u[ii] = s;
	}
	uni_c  = 362436.0   / 16777216.0;
	uni_cd = 7654321.0  / 16777216.0;
	uni_cm = 16777213.0 / 16777216.0;
	uni_ui = 97;	/*  There is a bug in the original Fortran version */
	uni_uj = 33;	/*  of UNI -- i and j should be SAVEd in UNI()     */
}



void rinit(int ijkl) {
	int i, j, k, l, ij, kl;

	/* check ijkl is within range */
	if( (ijkl < 0) || (ijkl > 900000000) )
		{
		printf("rinit: ijkl = %d -- out of range\n\n", ijkl);
		exit(3);
               	}

	/* decompose the long integer into the the equivalent four
	 * integers for rstart. This should be a 1-1 mapping
 	 *	ijkl <--> (i, j, k, l)
	 * though not quite all of the possible sets of (i, j, k, l)
	 * can be produced.
	 */

	ij = ijkl/30082;
	kl = ijkl - (30082 * ij);

	i = ((ij/177) % 177) + 2;
	j = (ij % 177) + 2;
	k = ((kl/169) % 178) + 1;
	l = kl % 169;

	if( (i <= 0) || (i > 178) )
		{
		printf("rinit: i = %d -- out of range\n\n", i);
		exit(3);
               	}

	if( (j <= 0) || (j > 178) )
		{
		printf("rinit: j = %d -- out of range\n\n", j);
		exit(3);
               	}

	if( (k <= 0) || (k > 178) )
		{
		printf("rinit: k = %d -- out of range\n\n", k);
		exit(3);
               	}

	if( (l < 0) || (l > 168) )
		{
		printf("rinit: l = %d -- out of range\n\n", l);
		exit(3);
               	}

	if (i == 1 && j == 1 && k == 1)
		{
                printf("rinit: 1 1 1 not allowed for 1st 3 seeds\n\n");
		exit(4);
                }


        rstart(i, j, k, l);

}








