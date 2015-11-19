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
int datasizes[]={1000,1500,2000};
int JACOBI_NUM_ITER = 100;

double start_time, end_time, kernel_time; 
int ops; 

double **G; 
double Gtotal = 0.0;

void  SORrun(double omega, double ** G, int num_iterations, int M, int N)
{

    double omega_over_four, one_minus_omega;
    double *Gi, *Gim1, *Gip1; 
    int Mm1, Nm1, i, j, p;

    omega_over_four = omega * 0.25;
    one_minus_omega = 1.0 - omega;
               

    /* update interior points */ 
    Mm1 = M-1;
    Nm1 = N-1;

    start_time = getclock();    
  
#ifdef ENABLE_BT
    fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif

    for (p=0; p<num_iterations; p++)
      {
	for (i=1; i<Mm1; i++)
	  {
	    Gi = G[i];
	    Gim1 = G[i-1];
	    Gip1 = G[i+1];
	    for (j=1; j<Nm1; j++)
	      Gi[j] = omega_over_four * (Gim1[j] + Gip1[j] + Gi[j-1] 
					 + Gi[j+1]) + one_minus_omega * Gi[j];
	  }
      }
   
#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif

    end_time = getclock();
    kernel_time = end_time - start_time;   

    for (i=1; i<Mm1; i++) {
      for (j=1; j<Nm1; j++) {
	Gtotal += G[i][j];
      }
    }               
    

}
			
double** RandomMatrix(int M, int N)
{
        double **A; 
        int i,j;  
        
        rinit(10101); 
        A = (double **) malloc (sizeof(double*) * N); 
        for (i=0; i<N; i++)
          {
          A[i] = (double *) malloc(sizeof(double) * M); 
	  for (j=0; j<M; j++)
	    {
	      A[i][j] = (double) uni() * 1e-6;
	    }
	  }      
	  return A;
	  
}


void initialise(){

   G = RandomMatrix(datasizes[size], datasizes[size]);

  }
 
void kernel(){

   SORrun(1.25, G, JACOBI_NUM_ITER, datasizes[size], datasizes[size]);
  }

void validate(){

  double refval[] = {0.49801945271476516508,1.12160004916842459544,1.99586884671435171867};
  double dev; 
    
  dev = fabs(Gtotal - refval[size]);
  
  if (dev > 1.0e-12 ){
    printf("Validation failed\n");
    printf("Gtotal = %.20f  %.20f   %d\n",Gtotal,dev,size);
    }
  }

void tidyup(){
   free (G);     
  }  



  void run(int sizein){

   size = sizein; 

   initialise(); 
    
   kernel(); 
   
   validate(); 
   tidyup(); 

   ops = JACOBI_NUM_ITER;     
   printf("Section2:SOR:Kernel    %f (s)       %f      (Iterations/s)\n", kernel_time, (double) ops / kernel_time);   

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







