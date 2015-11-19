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

/**
* NumericSortTest
*
* Performs the numeric sort test for the Java BYTEmark
* benchmark suite. This test implements a heapsort
* algorithm, performed on an int array. Results are
* reported in number of iterations per sec.
*/

#include <stdlib.h>
#include <stdio.h>
#include <libfastbt.h>
#include "HeapSort.h" 
#include "getclock.h"

int size; 
int datasizes[]={1000000,5000000,25000000};

int array_rows; 
int * TestArray; 

double start_time, end_time, kernel_time; 
int ops; 

/*The test sorts an array of pseudo-random 32-bit  integers. */ 


/*
* buildTestData
*
* Instantiates array and fills first it with random
* 32-bit integers.
*/

void buildTestData()
{
    int  i; 

    TestArray = (int *) malloc(sizeof(int) * array_rows);


    rinit(1729);     

    for(i = 0; i < array_rows; i++) {
        TestArray [i] = (int) (uni() * 2147483647);
    }
}



/*
* Do
*
* Perform an iteration of the numeric sort benchmark. Returns
* the elapsed time in milliseconds.
*/
void Do()
{
  start_time = getclock();  
  
#ifdef ENABLE_BT
    fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif
    
  NumHeapSort();

#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif
    
  end_time = getclock();
  kernel_time = end_time - start_time; 
}

/*
* NumSift
*
* Performs the sift operation on a numeric array,
* constructing a heap in the array.
* Instructions from strsift:
* Pass this function:
*  1. The string array # being sorted
*  2. Offsets within which to sort
* This performs the core sort of the Heapsort algorithm
*/

void NumSift(int min, int max)  
{
    int k;     
    int temp;  

    while((min + min) <= max)
    {
            k = min + min;
            if (k < max)
                if (TestArray[k]
                    < TestArray[k+1])
                    ++k;
            if (TestArray[min]
                < TestArray[k])
            {
                temp = TestArray[k];
                TestArray[k]
                    = TestArray[min];
                TestArray[min] = temp;
                min = k;
            }
            else
                min = max + 1;
    }
}


/*
* NumHeapSort
*
* Sorts one of the int arrays in the array of arrays.
* This routine performs a heap sort on that array.
*/

void NumHeapSort()
{
    int temp,i;                  
    int top = array_rows - 1;  

    /* First, build a heap in the array. Sifting moves larger
       values toward bottom of array. */ 

    for (i = top/2; i > 0; --i)
        NumSift(i,top);

    /* Repeatedly extract maximum from heap and place it at the
       end of the array. When we get done, we'll have a sorted
       array. */ 

    for (i = top; i > 0; --i)
    {
        NumSift(0,i);

        /* Exchange bottom element with descending top. */ 

        temp = TestArray[0];
        TestArray[0] = TestArray[i];
        TestArray[i] = temp;
    }
}

/*
* freeTestData
*
* free up memory 
*/

void freeTestData()
{
  free (TestArray);    /* Destroy the array. */ 
}

  void initialise(){
    array_rows = datasizes[size];
    buildTestData();
  }
 
  void kernel(){
    Do(); 
  }

  void validate(){
    int error,i;

    error = 0; 
    for (i = 1; i < array_rows; i++){
      error = (TestArray[i] < TestArray[i-1]); 
      if (error){
	printf("Validation failed\n");
	printf("Item %d = %d\n",i, TestArray[i]); 
	printf("Item %d = %d\n",i-1, TestArray[i-1]); 
	break;
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

        
    ops = array_rows;     
    printf("Section2:HeapSort:Kernel    %f (s)       %f      (items/s)\n", kernel_time, (double) ops / kernel_time); 
     
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






