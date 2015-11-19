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

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <libfastbt.h>
#include "MolDyn.h"
#include "getclock.h"

    int size;
    int datasizes[] = {8,13};

    int mm, npart;

    long interactions;  
    double start_time, end_time, kernel_time; 


    double *x, *vh, *f;
    double ekin;
    double vel;
    double sc;

    double epot;
    double vir;
    double count;
    double side;
    double rcoff;
    double a;      
    double hsq;    
    double hsq2;   
    double tscale; 
    double vaver;  
        
  /*
   *  Parameter definitions
   */

    double den    = 0.83134;
    double tref   = 0.722;  
    double h      = 0.064;
    int    irep   = 10;
    int    istop  = 20;
    int    iprint = 5;
    int    movemx = 50;


void initialise(){

   int ijk=0;
   int i,j,k,lg;  

   mm  = datasizes[size]; 
   npart = 4*mm*mm*mm; 

   side   = pow((double)npart/den,0.3333333);
   rcoff  = (double)mm/4.0;
   a      = side/(double)mm;
   hsq    = h*h;
   hsq2   = hsq*0.5;
   tscale = 16.0/((double)npart-1.0);
   vaver  = 1.13*sqrt(tref/24.0);

   x = (double *) malloc(3*npart*sizeof(double)); 
   vh = (double *) malloc(3*npart*sizeof(double)); 
   f = (double *) malloc(3*npart*sizeof(double)); 

  /*
   *  Generate fcc lattice for atoms inside box
   */
    for (lg=0; lg<2; lg++)
      for (i=0; i<mm; i++)
        for (j=0; j<mm; j++)
          for (k=0; k<mm; k++) {
            x[ijk]   = i*a+lg*a*0.5;
            x[ijk+1] = j*a+lg*a*0.5;
            x[ijk+2] = k*a;
            ijk += 3;
          }

    for (lg=1; lg<3; lg++)
      for (i=0; i<mm; i++)
        for (j=0; j<mm; j++)
          for (k=0; k<mm; k++) {
            x[ijk]   = i*a+(2-lg)*a*0.5;
            x[ijk+1] = j*a+(lg-1)*a*0.5;
            x[ijk+2] = k*a+a*0.5;
            ijk += 3;
          }
   
  /*
   *  Initialise velocities and forces (which are zero in fcc positions)
   */
    mxwell();
   
    for (i=0; i<npart*3; i++) {
      f[i] = 0.0; 
    }

  }


void runiters(){
  /*
   *  Start of md
   */
    int i,move; 

    for (move=1; move<=movemx; move++) {

    /*
     *  Move the particles and partially update velocities
     */
      domove();

    /*
     *  Compute forces in the new positions and accumulate the virial
     *  and potential energy.
     */
      forces();

    /*
     *  Scale forces, complete update of velocities and compute k.e.
     */
      ekin=mkekin();

    /*
     *  Average the velocity and temperature scale if desired
     */
      vel=velavg();
      if (move<istop && fmod(move, irep)==0) {
        sc=sqrt(tref/(tscale*ekin));
        for (i=0; i<npart*3; i++) {
	  vh[i]*=sc;
	}
        ekin=tref/tscale;
      }
    }

}

 



/*
 *  Sample Maxwell distribution at temperature tref
 */
  void mxwell(){
    int i;
    int n3 = npart*3;
    double r, tscale, v1, v2, s, ekin=0.0, sp=0.0, sc;
    
    rinit(4711);
    tscale=16.0/((double)npart-1.0);

    for (i=0; i<n3; i+=2) {
      s=2.0;
      while (s>=1.0) {
        v1=2.0*uni()-1.0;
        v2=2.0*uni()-1.0;
        s=v1*v1+v2*v2;
      }
      r=sqrt(-2.0*log(s)/s);
      vh[i]=v1*r;
      vh[i+1]=v2*r;
    }

    for (i=0; i<n3; i+=3) sp+=vh[i];
    sp/=(double)npart;
    for(i=0; i<n3; i+=3) {
      vh[i]-=sp;
      ekin+=vh[i]*vh[i];
    }

    sp=0.0;
    for (i=1; i<n3; i+=3) sp+=vh[i];
    sp/=(double)npart;
    for(i=1; i<n3; i+=3) {
      vh[i]-=sp;
      ekin+=vh[i]*vh[i];
    }

    sp=0.0;
    for (i=2; i<n3; i+=3) sp+=vh[i];
    sp/=(double)npart;
    for(i=2; i<n3; i+=3) {
      vh[i]-=sp;
      ekin+=vh[i]*vh[i];
    }

    sc=h*sqrt(tref/(tscale*ekin));
    for (i=0; i<n3; i++) vh[i]*=sc;
  }


/*
 *  Move particles
 */
  void domove(){
    int i, n3=3*npart;

    for (i=0; i<n3; i++) {
      x[i] += vh[i]+f[i];
  /*
   *  Periodic boundary conditions
   */
      if (x[i] < 0.0)  x[i] += side;
      if (x[i] > side) x[i] -= side;
  /*
   *  Partial velocity updates
   */
      vh[i] += f[i];
  /*
   *  Initialise forces for the next iteration
   */
      f[i] = 0.0;
    }
  }


/*
 *  Compute forces and accumulate the virial and the potential
 */

  void forces(){

    int i, j;
    double sideh, rcoffs;
    double xi,yi,zi,fxi,fyi,fzi,xx,yy,zz;
    double rd, rrd, rrd2, rrd3, rrd4, rrd6, rrd7, r148;
    double forcex, forcey, forcez;

    vir    = 0.0;
    epot   = 0.0;
    sideh  = 0.5*side;
    rcoffs = rcoff*rcoff;

    for (i=0; i<npart*3; i+=3) {
      xi  = x[i];
      yi  = x[i+1];
      zi  = x[i+2];
      fxi = 0.0;
      fyi = 0.0;
      fzi = 0.0;

      for (j=i+3; j<npart*3; j+=3) {
        xx = xi-x[j];
        yy = yi-x[j+1];
        zz = zi-x[j+2];
        if (xx<-sideh) xx += side;
        if (xx> sideh) xx -= side;
        if (yy<-sideh) yy += side;
        if (yy> sideh) yy -= side;
        if (zz<-sideh) zz += side;
        if (zz> sideh) zz -= side;
        rd = xx*xx+yy*yy+zz*zz;

        if (rd<=rcoffs) {
          rrd      = 1.0/rd;
          rrd2     = rrd*rrd;
          rrd3     = rrd2*rrd;
          rrd4     = rrd2*rrd2;
          rrd6     = rrd2*rrd4;
          rrd7     = rrd6*rrd;
          epot    += (rrd6-rrd3);
          r148     = rrd7-0.5*rrd4;
          vir     -= rd*r148;
          forcex   = xx*r148;
          fxi     += forcex;
          f[j]    -= forcex;
          forcey   = yy*r148;
          fyi     += forcey;
          f[j+1]  -= forcey;
          forcez   = zz*r148;
          fzi     += forcez;
          f[j+2]  -= forcez;
          interactions++; 
        }
      }
      f[i]     += fxi;
      f[i+1]   += fyi;
      f[i+2]   += fzi;
    }
  }

/*
 *  Scale forces, update velocities and compute K.E.
 */
  double mkekin(){
    int i;
    double sum=0.0, ekin;

    for (i=0; i<3*npart; i++) {
      f[i]*=hsq2;
      vh[i]+=f[i];
      sum+=vh[i]*vh[i];
    }
    ekin=sum/hsq;

    return(ekin);
  }

/*
 *  Compute average velocity
 */
  double velavg(){
    int i;
    double vaverh=vaver*h;
    double vel=0.0;
    double sq;

    count=0.0;
    for (i=0; i<npart*3; i+=3){
      sq=sqrt(vh[i]*vh[i]+vh[i+1]*vh[i+1]+vh[i+2]*vh[i+2]);
      if (sq>vaverh) count++;
      vel+=sq;
    }
    vel/=h;

    return(vel);
  }



  void application(){ 

    start_time = getclock();      

#ifdef ENABLE_BT
    fbt_init(NULL);
    fbt_start_transaction(&fbt_commit_transaction);
#endif

    runiters();

#ifdef ENABLE_BT
    fbt_commit_transaction();
#endif

    end_time = getclock();
    kernel_time = end_time - start_time;   


  } 

  void validate(){
    double refval[] = {72.70548090623722714554,305.40449631297553878539};
    double dev; 

    dev = fabs(ekin - refval[size]); 
    if (dev > 1.0e-10){
      printf("Validation failed\n"); 
      printf("Computed kinetic energy = %.20f\n", ekin);
      printf("Reference value = %.20f\n", refval[size]); 
    }
  }

  void tidyup(){    

    free(x);
    free(vh); 
    free(f); 

  }


  void run(int sizein){
 

    size = sizein; 

    initialise(); 

    application(); 

    validate(); 
    tidyup(); 

    printf("Section3:MolDyn:Run    %f (s)       %f      (Interactions/s)\n", kernel_time, (double) interactions / kernel_time);   
    
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

