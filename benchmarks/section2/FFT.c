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
*             see below for previous history of this code                 *
*                                                                         *
*      This version copyright (c) The University of Edinburgh, 2001.      *
*                         All rights reserved.                            *
*                                                                         *
**************************************************************************/


/** Computes FFT's of complex, double precision data where n is an integer power of 2.
  * This appears to be slower than the Radix2 method,
  * but the code is smaller and simpler, and it requires no extra storage.
  * <P>
  *
  * @author Bruce R. Miller bruce.miller@nist.gov,
  * @author Derived from GSL (Gnu Scientific Library), 
  * @author GSL's FFT Code by Brian Gough bjg@vvv.lanl.gov
  */

  /* See {@link ComplexDoubleFFT ComplexDoubleFFT} for details of data layout.
   */
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include <libfastbt.h>
#include "FFT.h" 
#include "getclock.h"

#define PI 3.14159265358979323

int size; 
int datasizes[]={2097152,8388608,16777216};
int data_length;
double *data; 

double start_time, end_time, kernel_time; 
int ops; 

double total = 0.0;
double totali = 0.0;


double* RandomVector(int N) {
                
  double *A; 
  int i; 

  A = (double *) malloc (sizeof(double) * N); 
 
  rinit (1010101); 

  for (i=0; i<N; i++)
    A[i] = (double) uni() * 1e-6;

  return A;
}

/** Compute Fast Fourier Transform of (complex) data, in place.*/
void transform (void) {
    
    int i; 
    start_time = getclock();    
    
#ifdef ENABLE_BT
    fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif

    transform_internal(data, -1); 
    
#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif
    end_time = getclock();
    kernel_time = end_time - start_time;   

    for (i=0; i<data_length; i++) {
      total += data[i];
    }

  }

  /** Compute Inverse Fast Fourier Transform of (complex) data, in place.*/
void inverse (void) {

    int i,n; 
    double norm; 

    start_time = getclock();  
    
#ifdef ENABLE_BT
    //fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif

    transform_internal(data, +1);  

    n = data_length/2;
    norm=1/((double) n);
    for(i=0; i<data_length; i++)
      data[i] *= norm;

#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif
     
    /* included measuring end time. was this a bug?.. Marcel Wirth */
    end_time = getclock();
    
    
    kernel_time += end_time - start_time; 

    for(i=0; i<data_length; i++) {
      totali += data[i];
    }

  }


  /** Make a random array of n (complex) elements. */


int log2 (int n){
    int log = 0, k;

    for(k=1; k < n; k *= 2, log++);
    if (n != (1 << log))
      printf("FFT: Data length %d is not a power of 2!\n",n);
    return log; 
}

void transform_internal (double * data, int direction) {
   
    int n, logn, bit, dual, i, j, a, b;
    double  w_real, w_imag, wd_real, wd_imag, s, s2, t, theta; 
    double  tmp_real, tmp_imag, z1_real, z1_imag;     

    n = data_length/2;
    if (n == 1) return;	
    logn = log2(n);

    /* bit reverse the input data for decimation in time algorithm */
    bitreverse(data) ;

    /* apply fft recursion */
    for (bit = 0, dual = 1; bit < logn; bit++, dual *= 2) {
      w_real = 1.0;
      w_imag = 0.0;

      theta = 2.0 * direction * PI / (2.0 * (double) dual);
      s = sin(theta);
      t = sin(theta / 2.0);
      s2 = 2.0 * t * t;

      /* a = 0 */
      for (b = 0; b < n; b += 2 * dual) {
	i = 2*b ;
	j = 2*(b + dual);

	wd_real = data[j] ;
	wd_imag = data[j+1] ;
	  
	data[j]   = data[i]   - wd_real;
	data[j+1] = data[i+1] - wd_imag;
	data[i]  += wd_real;
	data[i+1]+= wd_imag;
      }
      
      /* a = 1 .. (dual-1) */
      for (a = 1; a < dual; a++) {
	/* trignometric recurrence for w-> exp(i theta) w */
	{
	  tmp_real = w_real - s * w_imag - s2 * w_real;
	  tmp_imag = w_imag + s * w_real - s2 * w_imag;
	  w_real = tmp_real;
	  w_imag = tmp_imag;
	}
	for (b = 0; b < n; b += 2 * dual) {
	  i = 2*(b + a);
	  j = 2*(b + a + dual);

	  z1_real = data[j];
	  z1_imag = data[j+1];
	      
	  wd_real = w_real * z1_real - w_imag * z1_imag;
	  wd_imag = w_real * z1_imag + w_imag * z1_real;

	  data[j]   = data[i]   - wd_real;
	  data[j+1] = data[i+1] - wd_imag;
	  data[i]  += wd_real;
	  data[i+1]+= wd_imag;
	}
      }
    }
  }


void bitreverse(double * data) {
    /* This is the Goldrader bit-reversal algorithm */
    int n,i,j,k,ii,jj; 
    double tmp_real, tmp_imag; 

    n=data_length/2;

    for (i = 0, j=0; i < n - 1; i++) {
      ii = 2*i;
      jj = 2*j;
      k = n / 2 ;
      if (i < j) {
	tmp_real    = data[ii];
	tmp_imag    = data[ii+1];
	data[ii]   = data[jj];
	data[ii+1] = data[jj+1];
	data[jj]   = tmp_real;
	data[jj+1] = tmp_imag; }

      while (k <= j) {
	j = j - k ;
	k = k / 2 ; }
      j += k ;
    }
  
}


void initialise(){

    data_length = 2*datasizes[size];
    data = RandomVector(data_length);

}
 
void kernel(){


    transform();      
    inverse();  

  }

void validate(){

  double refval[] = {1.29100650000014338659,5.16402600000193157115,10.32805200000056444765};
  double refvali[] = {2.09727980315876161299,8.39187810210538209788,16.77969092468124401307};
  double dev,devi; 

  dev = fabs(total - refval[size]);
  devi = fabs(totali - refvali[size]);
  if (dev > 1.0e-12 ){
    printf("Validation failed\n");
    printf("total = %.20f  %.20f  %d\n",total, dev, size);
  }
  if (devi > 1.0e-12 ){
    printf("Validation failed\n");
    printf("total = %.20f  %.20f  %d\n",totali, devi, size);
  }


}

void tidyup(){
   free(data);
  }  


void run(int sizein){

   size = sizein; 

   initialise();
   
   kernel(); 

   validate(); 
   tidyup(); 
   
   ops = datasizes[size];     
   printf("Section2:FFT:Kernel    %f (s)       %f      (Samples/s)\n", kernel_time, (double) ops / kernel_time);   

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






