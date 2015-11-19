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
*                     David Oh (bamf@pobox.com)                           *
*                                                                         *
*      This version copyright (c) The University of Edinburgh, 2001.      *
*                         All rights reserved.                            *
*                                                                         *
**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <libfastbt.h>
#include "Euler.h" 
#include "getclock.h"



  int size;
  int datasizes[] = {8,12};

  double start_time, end_time, kernel_time; 
  int ops; 

  double machff = 0.7;    /* Inflow mach number */
  double secondOrderDamping = 1.0;
  double fourthOrderDamping = 1.0;
  int ntime = 1; /* 0 = local timestep, 1 = time accurate */ 
  int scale; /* Refine input grid by this factor */ 
  double error;

  double **a;   /* Grid cell area */
  double **deltat;   /* Timestep */
  double **opg, **pg, **pg1;                     /* Pressure */
  double **sxi, **seta;
  double **tg, **tg1;                           /* Temperature */
  double **xnode, **ynode;      /* Storage of node coordinates */

  double ***oldval, ***newval; /* Tepmoray arrays for interpolation */ 

  double cff, uff, vff, pff, rhoff, tff, jplusff, jminusff;
                                    /* Far field values */
  double datamax, datamin;
  int iter = 100; /* Number of iterations */ 
  int imax, jmax;     /* Number of nodes in x and y direction*/
  int imaxin, jmaxin; /* Number of nodes in x and y direction in unscaled data */ 
  int nf = 6; /* Number of fields in data file */ 


  struct Statevector **d;   /* Damping coefficients */
  struct Statevector **f, **g;   /* Flux Vectors */
  struct Statevector **r, **ug1;
  struct Statevector **ug;      /* Storage of data */

  double Cp = 1004.5;      /* specific heat, const pres. */
  double Cv=717.5;      /* specific heat, const vol. */
  double Gamma=1.4;   /* Ratio of specific heats */
  double rgas=287.0;       /* Gas Constant */
  double fourthOrderNormalizer = 0.02; /* Damping coefficients */
  double secondOrderNormalizer = 0.02;



void init(){

    int i, j, k, n;             /* Dummy counters */
    double scrap, scrap2;     /* Temporary storage */
    FILE *fp; 
    int iold,jold;
    double xf,yf;

    /* Set scale factor for interpolation */ 
    scale = datasizes[size]; 
 
    /* Open data file */
    fp = fopen("tunnel.dat","r");
    
    /* Read header */
    fscanf(fp,"%d %d",&imaxin,&jmaxin); 

    /* Read data into temporary array 
       note: dummy extra row and column needed to make interpolation simple */ 

    oldval = (double***) malloc(sizeof(double**)*(imaxin+1));
    for (i=0;i<imaxin+1;i++) {
      oldval[i] = (double**) malloc(sizeof(double*)*(jmaxin+1));
      for (j=0;j<jmaxin+1;j++) {
	oldval[i][j] = (double*) malloc(sizeof(double)*nf);
      }
    }

    for (i=0;i<imaxin;i++){
      for (j=0;j<jmaxin;j++){
	fscanf(fp,"%lf %lf %lf %lf %lf %lf",oldval[i][j],oldval[i][j]+1,
               oldval[i][j]+2,oldval[i][j]+3,oldval[i][j]+4,oldval[i][j]+5); 
      }
    }

    /* interpolate onto finer grid  */ 

    imax = (imaxin - 1) * scale + 1; 
    jmax = (jmaxin - 1) * scale + 1;

    newval = (double***) malloc(sizeof(double**)*imax);
    for (i=0;i<imax;i++) {
      newval[i] = (double**) malloc(sizeof(double*)*jmax);
      for (j=0;j<jmax;j++) {
	newval[i][j] = (double*) malloc(sizeof(double)*nf);
      }
    }

    
    for (k=0; k<nf; k++){
      for (i=0;i<imax;i++){
	for (j=0;j<jmax;j++){
	  iold = i/scale; 
          jold = j/scale; 
          xf = ( (double) (i%scale)) /( (double) scale); 
          yf = ( (double) (j%scale)) /( (double) scale); 
	  newval[i][j][k] = (1.0 - xf)*(1.0 - yf)* oldval[iold][jold][k]
	                +(1.0 - xf)*       yf * oldval[iold][jold+1][k]
                        +       xf *(1.0 - yf)* oldval[iold+1][jold][k]
                        +       xf *       yf * oldval[iold+1][jold+1][k];
	}
      }
    }

    

    deltat = newdoublearray(imax+1,jmax+2);
    opg = newdoublearray(imax+2,jmax+2);
    pg = newdoublearray(imax+2,jmax+2);
    pg1 = newdoublearray(imax+2,jmax+2);
    sxi = newdoublearray(imax+2,jmax+2);
    seta = newdoublearray(imax+2,jmax+2);
    tg = newdoublearray(imax+2,jmax+2);
    tg1 = newdoublearray(imax+2,jmax+2);
    ug = newStatevectorarray(imax+2,jmax+2);
    a = newdoublearray(imax,jmax);
    d =  newStatevectorarray(imax+2,jmax+2);
    f =  newStatevectorarray(imax+2,jmax+2);
    g =  newStatevectorarray(imax+2,jmax+2);
    r =  newStatevectorarray(imax+2,jmax+2);
    ug1 =  newStatevectorarray(imax+2,jmax+2);
    xnode = newdoublearray(imax,jmax);
    ynode = newdoublearray(imax,jmax);



    /* Set farfield values (we use normalized units for everything */    
    cff = 1.0;
    vff = 0.0;
    pff = 1.0 / Gamma;
    rhoff = 1.0;
    tff = pff / (rhoff * rgas);

    /* Copy the interpolated data to arrays  */ 


    for (i=0; i<imax; i++){
      for (j=0; j<jmax; j++){

	xnode[i][j] = newval[i][j][0];
	ynode[i][j] = newval[i][j][1];
        ug[i+1][j+1].a = newval[i][j][2];
        ug[i+1][j+1].b = newval[i][j][3];
        ug[i+1][j+1].c = newval[i][j][4];
        ug[i+1][j+1].d = newval[i][j][5];

	scrap = ug[i+1][j+1].c/ug[i+1][j+1].a;
	scrap2 = ug[i+1][j+1].b/ug[i+1][j+1].a;
	tg[i+1][j+1] = ug[i+1][j+1].d/ug[i+1][j+1].a 
	  - (0.5 * (scrap*scrap + scrap2*scrap2));
	tg[i+1][j+1] = tg[i+1][j+1] / Cv;
	pg[i+1][j+1] = rgas * ug[i+1][j+1].a * tg[i+1][j+1];

      }
    }
     
	
    /* Calculate grid cell areas */
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j)
	a[i][j] = 0.5 * ((xnode[i][j] - xnode[i-1][j-1]) 
			              * (ynode[i-1][j] - ynode[i][j-1])-
			 (ynode[i][j] - ynode[i-1][j-1]) 
			              * (xnode[i-1][j] - xnode[i][j-1]));

  }


  void doIteration() {
    double scrap;
    int i, j;

    /* Record the old pressure values */
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	opg[i][j] = pg[i][j];
      }


    calculateDummyCells(pg, tg, ug);
    calculateDeltaT();

    calculateDamping(pg, ug);

    /* Do the integration */
    /* Step 1 */
    calculateF(pg, tg, ug);
    calculateG(pg, tg, ug);
    calculateR();

    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	ug1[i][j].a=ug[i][j].a-0.25*deltat[i][j]/a[i][j]*(r[i][j].a-d[i][j].a);
	ug1[i][j].b=ug[i][j].b-0.25*deltat[i][j]/a[i][j]*(r[i][j].b-d[i][j].b);
	ug1[i][j].c=ug[i][j].c-0.25*deltat[i][j]/a[i][j]*(r[i][j].c-d[i][j].c);
	ug1[i][j].d=ug[i][j].d-0.25*deltat[i][j]/a[i][j]*(r[i][j].d-d[i][j].d);
      }
    calculateStateVar(pg1, tg1, ug1);

    /* Step 2 */
    calculateDummyCells(pg1, tg1, ug1);
    calculateF(pg1, tg1, ug1);
    calculateG(pg1, tg1, ug1);
    calculateR();
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	ug1[i][j].a=
	  ug[i][j].a-0.33333*deltat[i][j]/a[i][j]*(r[i][j].a-d[i][j].a);
	ug1[i][j].b=
	  ug[i][j].b-0.33333*deltat[i][j]/a[i][j]*(r[i][j].b-d[i][j].b);
	ug1[i][j].c=
	  ug[i][j].c-0.33333*deltat[i][j]/a[i][j]*(r[i][j].c-d[i][j].c);
	ug1[i][j].d=
	  ug[i][j].d-0.33333*deltat[i][j]/a[i][j]*(r[i][j].d-d[i][j].d);
      }
    calculateStateVar(pg1, tg1, ug1);
    
    /* Step 3 */
    calculateDummyCells(pg1, tg1, ug1);
    calculateF(pg1, tg1, ug1);
    calculateG(pg1, tg1, ug1);
    calculateR();
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	ug1[i][j].a=
	  ug[i][j].a-0.5*deltat[i][j]/a[i][j]*(r[i][j].a-d[i][j].a);
	ug1[i][j].b=
	  ug[i][j].b-0.5*deltat[i][j]/a[i][j]*(r[i][j].b-d[i][j].b);
	ug1[i][j].c=
	  ug[i][j].c-0.5*deltat[i][j]/a[i][j]*(r[i][j].c-d[i][j].c);
	ug1[i][j].d=
	  ug[i][j].d-0.5*deltat[i][j]/a[i][j]*(r[i][j].d-d[i][j].d);

      }
    calculateStateVar(pg1, tg1, ug1);
    
    /* Step 4 (final step) */
    calculateDummyCells(pg1, tg1, ug1);
    calculateF(pg1, tg1, ug1);
    calculateG(pg1, tg1, ug1);
    calculateR();
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	ug[i][j].a -= deltat[i][j]/a[i][j]*(r[i][j].a-d[i][j].a);
	ug[i][j].b -= deltat[i][j]/a[i][j]*(r[i][j].b-d[i][j].b);
	ug[i][j].c -= deltat[i][j]/a[i][j]*(r[i][j].c-d[i][j].c);
	ug[i][j].d -= deltat[i][j]/a[i][j]*(r[i][j].d-d[i][j].d);
      }
    calculateStateVar(pg, tg, ug);

    /* calculate RMS Pressure Error */
    error = 0.0;
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	scrap = pg[i][j]-opg[i][j];
	error += scrap*scrap;
      }
    error = sqrt(error / (double)((imax-1) * (jmax-1)) ); 
  }


  void calculateStateVar(double **localpg, double **localtg,
			 struct Statevector **localug)
    /* Calculates the new state values for range-kutta */
    /* Works for default values, 4/11 at 9:45 pm */
    {
      double temp, temp2;
      int i, j;

      for (i = 1; i < imax; ++i) {
	for (j = 1; j < jmax; ++j) {
	  temp = localug[i][j].b;
	  temp2 = localug[i][j].c;
	  localtg[i][j] = localug[i][j].d/localug[i][j].a - 0.5 *
	    (temp*temp + temp2*temp2)/(localug[i][j].a*localug[i][j].a);

	  localtg[i][j] = localtg[i][j] / Cv;
	  localpg[i][j] = localug[i][j].a * rgas * localtg[i][j];
	}
      }
    }


  void calculateR() {
    /* Works for default values, straight channel (all 0's) 4/11, 9:15 pm */
    
    double deltax, deltay;
    double temp;
    int i,j;
    struct Statevector scrap;
    
    for (i = 1; i < imax; ++i) {
      for (j = 1; j < jmax; ++j) {
	
	/* Start by clearing R */
	r[i][j].a = 0.0;
	r[i][j].b = 0.0;
	r[i][j].c = 0.0;
	r[i][j].d = 0.0;
	
	/* East Face */
	deltay = (ynode[i][j] - ynode[i][j-1]);
	deltax = (xnode[i][j] - xnode[i][j-1]);
	temp = 0.5 * deltay;
	r[i][j].a += temp*(f[i][j].a + f[i+1][j].a);
	r[i][j].b += temp*(f[i][j].b + f[i+1][j].b);
	r[i][j].c += temp*(f[i][j].c + f[i+1][j].c);
	r[i][j].d += temp*(f[i][j].d + f[i+1][j].d);

	temp = -0.5*deltax;
	r[i][j].a += temp * (g[i][j].a+g[i+1][j].a);
	r[i][j].b += temp * (g[i][j].b+g[i+1][j].b);
	r[i][j].c += temp * (g[i][j].c+g[i+1][j].c);
	r[i][j].d += temp * (g[i][j].d+g[i+1][j].d);
	
	/* South Face */
	deltay = (ynode[i][j-1] - ynode[i-1][j-1]);  
	deltax = (xnode[i][j-1] - xnode[i-1][j-1]);

	temp = 0.5 * deltay;
	r[i][j].a  += temp*(f[i][j].a+f[i][j-1].a);
	r[i][j].b  += temp*(f[i][j].b+f[i][j-1].b);
	r[i][j].c  += temp*(f[i][j].c+f[i][j-1].c);
	r[i][j].d  += temp*(f[i][j].d+f[i][j-1].d);

	temp = -0.5*deltax;
	r[i][j].a += temp * (g[i][j].a+g[i][j-1].a);
	r[i][j].b += temp * (g[i][j].b+g[i][j-1].b);
	r[i][j].c += temp * (g[i][j].c+g[i][j-1].c);
	r[i][j].d += temp * (g[i][j].d+g[i][j-1].d);
	
	/* West Face */
	deltay = (ynode[i-1][j-1] - ynode[i-1][j]);
	deltax = (xnode[i-1][j-1] - xnode[i-1][j]);

	temp = 0.5 * deltay;
	r[i][j].a  += temp*(f[i][j].a+f[i-1][j].a);
	r[i][j].b  += temp*(f[i][j].b+f[i-1][j].b);
	r[i][j].c  += temp*(f[i][j].c+f[i-1][j].c);
	r[i][j].d  += temp*(f[i][j].d+f[i-1][j].d);

	temp = -0.5*deltax;
	r[i][j].a += temp * (g[i][j].a+g[i-1][j].a);
	r[i][j].b += temp * (g[i][j].b+g[i-1][j].b);
	r[i][j].c += temp * (g[i][j].c+g[i-1][j].c);
	r[i][j].d += temp * (g[i][j].d+g[i-1][j].d);
	
	
	/* North Face */
	deltay = (ynode[i-1][j] - ynode[i][j]);
	deltax = (xnode[i-1][j] - xnode[i][j]);

	temp = 0.5 * deltay;
	r[i][j].a  += temp*(f[i][j].a+f[i+1][j].a);
	r[i][j].b  += temp*(f[i][j].b+f[i+1][j].b);
	r[i][j].c  += temp*(f[i][j].c+f[i+1][j].c);
	r[i][j].d  += temp*(f[i][j].d+f[i+1][j].d);
	
	temp = -0.5*deltax;
	r[i][j].a += temp * (g[i][j].a+g[i][j+1].a);
	r[i][j].b += temp * (g[i][j].b+g[i][j+1].b);
	r[i][j].c += temp * (g[i][j].c+g[i][j+1].c);
	r[i][j].d += temp * (g[i][j].d+g[i][j+1].d);

      }
    }
  }


  void calculateG(double **localpg, double **localtg,
		  struct Statevector **localug) {
    /* Works for default values 4/10: 5:15 pm */    
    double temp, temp2, temp3;
    double v;
    int i, j;
    
    for (i = 0; i < imax + 1; ++i) {
      for (j = 0; j < jmax + 1; ++j) {
	v = localug[i][j].c / localug[i][j].a;
	g[i][j].a = localug[i][j].c;
	g[i][j].b = localug[i][j].b * v;
	g[i][j].c = localug[i][j].c*v + localpg[i][j];
	temp = localug[i][j].b * localug[i][j].b;
	temp2 = localug[i][j].c * localug[i][j].c;
	temp3 = localug[i][j].a * localug[i][j].a;
	g[i][j].d = localug[i][j].c * (Cp * localtg[i][j]+ 
		 (0.5 * (temp + temp2)/(temp3)));
      }
    }
  }


  void calculateF(double **localpg, double **localtg, 
		  struct Statevector **localug) {
     /* Works for default values 4/10: 4:50 pm */
    {
      double u;
      double temp1, temp2, temp3;
      int i, j;
      
      for (i = 0; i < imax + 1; ++i) {
	for (j = 0; j < jmax + 1; ++j) {	  
	  u = localug[i][j].b/ localug[i][j].a;
	  f[i][j].a = localug[i][j].b;
	  f[i][j].b = localug[i][j].b *u + localpg[i][j];
	  f[i][j].c = localug[i][j].c * u;
	  temp1 = localug[i][j].b * localug[i][j].b;
	  temp2 = localug[i][j].c * localug[i][j].c;
	  temp3 = localug[i][j].a * localug[i][j].a;
	  f[i][j].d = localug[i][j].b * (Cp * localtg[i][j] + 
	      	 (0.5 * (temp1 + temp2)/(temp3)));
	}
      }
    }
  }

  void calculateDamping(double **localpg, struct Statevector **localug) {
      double adt, sbar;
      double nu2;
      double nu4;
      double tempdouble;
      int ascrap, i, j;
      struct Statevector temp,temp2,scrap2,scrap4;
      
      nu2 = secondOrderDamping * secondOrderNormalizer;
      nu4 = fourthOrderDamping * fourthOrderNormalizer;

      /* First do the pressure switches */
      /* Checked and works with defaults, 4/12 at 1:20 am */
      /* The east and west faces have been checked numerically vs.John's old */
      /* Scheme, and work! 4/13 @ 2:20 pm */
      for (i = 1; i < imax; ++i)
	for (j = 1; j < jmax; ++j) {
	  sxi[i][j] = fabs(localpg[i+1][j] -
		2.0 * localpg[i][j] + localpg[i-1][j])/ localpg[i][j];
	  seta[i][j] = fabs(localpg[i][j+1] -
	       2.0 * localpg[i][j] + localpg[i][j-1]) / localpg[i][j];
	}
      
      /* Then calculate the fluxes */
      for (i = 1; i < imax; ++i) {
	for (j = 1; j < jmax; ++j) {
	  
	  /* Clear values */
	  /* East Face */
	  if (i > 1 && i < imax-1) {
	    adt = (a[i][j] + a[i+1][j]) / (deltat[i][j] + deltat[i+1][j]);
	    sbar = (sxi[i+1][j] + sxi[i][j]) * 0.5;
	  }
	  else {
	    adt = a[i][j]/deltat[i][j];
	    sbar = sxi[i][j];
	  }
	  tempdouble = nu2*sbar*adt;
	  scrap2.a = tempdouble * (localug[i+1][j].a-localug[i][j].a);
	  scrap2.b = tempdouble * (localug[i+1][j].b-localug[i][j].b);
	  scrap2.c = tempdouble * (localug[i+1][j].c-localug[i][j].c);
	  scrap2.d = tempdouble * (localug[i+1][j].d-localug[i][j].d);

	  if (i > 1 && i < imax-1) {
	    temp.a = localug[i+2][j].a-localug[i-1][j].a;
	    temp.b = localug[i+2][j].b-localug[i-1][j].b;
	    temp.c = localug[i+2][j].c-localug[i-1][j].c;
	    temp.d = localug[i+2][j].d-localug[i-1][j].d;

	    temp2.a = 3.0*(localug[i][j].a-localug[i+1][j].a);
	    temp2.b = 3.0*(localug[i][j].b-localug[i+1][j].b);
	    temp2.c = 3.0*(localug[i][j].c-localug[i+1][j].c);
	    temp2.d = 3.0*(localug[i][j].d-localug[i+1][j].d);

	    tempdouble = -nu4*adt;
	    scrap4.a = tempdouble*(temp.a+temp2.a);
	    scrap4.b = tempdouble*(temp.a+temp2.b);
	    scrap4.c = tempdouble*(temp.a+temp2.c);
	    scrap4.d = tempdouble*(temp.a+temp2.d);
	  }
	  else {
	    scrap4.a = 0.0;
	    scrap4.b = 0.0;
	    scrap4.c = 0.0;
	    scrap4.d = 0.0;
	  }

	  temp.a = scrap2.a + scrap4.a;
	  temp.b = scrap2.b + scrap4.b;
	  temp.c = scrap2.c + scrap4.c;
	  temp.d = scrap2.d + scrap4.d;
	  d[i][j] = temp;
	  
	  /* West Face */ 
	  if(i > 1 && i < imax-1) {
	    adt = (a[i][j] + a[i-1][j]) / (deltat[i][j] + deltat[i-1][j]);
	    sbar = (sxi[i][j] + sxi[i-1][j]) *0.5;
	  }
	  else {
	    adt = a[i][j]/deltat[i][j];
	    sbar = sxi[i][j];
	  }

	  tempdouble = -nu2*sbar*adt;
	  scrap2.a = tempdouble * (localug[i][j].a-localug[i-1][j].a);
	  scrap2.b = tempdouble * (localug[i][j].b-localug[i-1][j].b);
	  scrap2.c = tempdouble * (localug[i][j].c-localug[i-1][j].c);
	  scrap2.d = tempdouble * (localug[i][j].d-localug[i-1][j].d);


	  if (i > 1 && i < imax-1) {
	    temp.a = localug[i+1][j].a-localug[i-2][j].a;
	    temp.b = localug[i+1][j].b-localug[i-2][j].b;
	    temp.c = localug[i+1][j].c-localug[i-2][j].c;
	    temp.d = localug[i+1][j].d-localug[i-2][j].d;

	    temp2.a = 3.0*(localug[i-1][j].a-localug[i][j].a);
	    temp2.b = 3.0*(localug[i-1][j].b-localug[i][j].b);
	    temp2.c = 3.0*(localug[i-1][j].c-localug[i][j].c);
	    temp2.d = 3.0*(localug[i-1][j].d-localug[i][j].d);

	    tempdouble = nu4*adt;
	    scrap4.a = tempdouble*(temp.a+temp2.a);
	    scrap4.b = tempdouble*(temp.a+temp2.b);
	    scrap4.c = tempdouble*(temp.a+temp2.c);
	    scrap4.d = tempdouble*(temp.a+temp2.d);
	  }
	  else {
	    scrap4.a = 0.0;
	    scrap4.b = 0.0;
	    scrap4.c = 0.0;
	    scrap4.d = 0.0;
	  }

	  d[i][j].a += scrap2.a + scrap4.a;
	  d[i][j].b += scrap2.b + scrap4.b;
	  d[i][j].c += scrap2.c + scrap4.c;
	  d[i][j].d += scrap2.d + scrap4.d;

	  /* North Face */
	  if (j > 1 && j < jmax-1) {
	    adt = (a[i][j] + a[i][j+1]) / (deltat[i][j] + deltat[i][j+1]);
	    sbar = (seta[i][j] + seta[i][j+1]) * 0.5;
	  }
	  else {
	    adt = a[i][j]/deltat[i][j];
	    sbar = seta[i][j];
	  }
	  tempdouble = nu2*sbar*adt;
	  scrap2.a = tempdouble * (localug[i][j+1].a-localug[i][j].a);
	  scrap2.b = tempdouble * (localug[i][j+1].b-localug[i][j].b);
	  scrap2.c = tempdouble * (localug[i][j+1].c-localug[i][j].c);
	  scrap2.d = tempdouble * (localug[i][j+1].d-localug[i][j].d);

	  if (j > 1 && j < jmax-1) {
	    temp.a = localug[i][j+2].a-localug[i][j-1].a;
	    temp.b = localug[i][j+2].b-localug[i][j-1].b;
	    temp.c = localug[i][j+2].c-localug[i][j-1].c;
	    temp.d = localug[i][j+2].d-localug[i][j-1].d;

	    temp2.a = 3.0*(localug[i][j].a-localug[i][j+1].a);
	    temp2.b = 3.0*(localug[i][j].b-localug[i][j+1].b);
	    temp2.c = 3.0*(localug[i][j].c-localug[i][j+1].c);
	    temp2.d = 3.0*(localug[i][j].d-localug[i][j+1].d);

	    tempdouble = -nu4*adt;
	    scrap4.a = tempdouble*(temp.a+temp2.a);
	    scrap4.b = tempdouble*(temp.a+temp2.b);
	    scrap4.c = tempdouble*(temp.a+temp2.c);
	    scrap4.d = tempdouble*(temp.a+temp2.d);
	  }
	  else {
	    scrap4.a = 0.0;
	    scrap4.b = 0.0;
	    scrap4.c = 0.0;
	    scrap4.d = 0.0;
	  }
	  d[i][j].a += scrap2.a + scrap4.a;
	  d[i][j].b += scrap2.b + scrap4.b;
	  d[i][j].c += scrap2.c + scrap4.c;
	  d[i][j].d += scrap2.d + scrap4.d;
	  
	  /* South Face */
	  if (j > 1 && j < jmax-1) {
	    adt = (a[i][j] + a[i][j-1]) / (deltat[i][j] + deltat[i][j-1]);
	    sbar = (seta[i][j] + seta[i][j-1]) * 0.5;
	  }
	  else {
	    adt = a[i][j]/deltat[i][j];
	    sbar = seta[i][j];
	  }
	  tempdouble = -nu2*sbar*adt;
	  scrap2.a = tempdouble * (localug[i][j].a-localug[i][j-1].a);
	  scrap2.b = tempdouble * (localug[i][j].b-localug[i][j-1].b);
	  scrap2.c = tempdouble * (localug[i][j].c-localug[i][j-1].c);
	  scrap2.d = tempdouble * (localug[i][j].d-localug[i][j-1].d);

	  if (j > 1 && j < jmax-1) {
	    temp.a = localug[i][j+1].a-localug[i][j-2].a;
	    temp.b = localug[i][j+1].b-localug[i][j-2].b;
	    temp.c = localug[i][j+1].c-localug[i][j-2].c;
	    temp.d = localug[i][j+1].d-localug[i][j-2].d;

	    temp2.a = 3.0*(localug[i][j-1].a-localug[i][j].a);
	    temp2.b = 3.0*(localug[i][j-1].b-localug[i][j].b);
	    temp2.c = 3.0*(localug[i][j-1].c-localug[i][j].c);
	    temp2.d = 3.0*(localug[i][j-1].d-localug[i][j].d);

	    tempdouble = nu4*adt;
	    scrap4.a = tempdouble*(temp.a+temp2.a);
	    scrap4.b = tempdouble*(temp.a+temp2.b);
	    scrap4.c = tempdouble*(temp.a+temp2.c);
	    scrap4.d = tempdouble*(temp.a+temp2.d);
	  }
	  else {
	    scrap4.a = 0.0;
	    scrap4.b = 0.0;
	    scrap4.c = 0.0;
	    scrap4.d = 0.0;
	  }
	  d[i][j].a += scrap2.a + scrap4.a;
	  d[i][j].b += scrap2.b + scrap4.b;
	  d[i][j].c += scrap2.c + scrap4.c;
	  d[i][j].d += scrap2.d + scrap4.d;
	}
      }
  }
  
  void calculateDeltaT() {
    double xeta, yeta, xxi, yxi;              /* Local change in x and y */
    int i, j;
    double mint;
    double c, q, r;
    double safety_factor = 0.7;
    
    for (i = 1; i < imax; ++i)
      for (j = 1; j < jmax; ++j) {
	xxi = (xnode[i][j] - xnode[i-1][j] 
	                         + xnode[i][j-1] - xnode[i-1][j-1]) * 0.5;
	yxi = (ynode[i][j] - ynode[i-1][j] 
	                         + ynode[i][j-1] - ynode[i-1][j-1]) * 0.5;
	xeta = (xnode[i][j] - xnode[i][j-1] 
		                 + xnode[i-1][j] - xnode[i-1][j-1]) * 0.5;
	yeta = (ynode[i][j] - ynode[i][j-1] 
		                 + ynode[i-1][j] - ynode[i-1][j-1]) * 0.5;
	
	q = (yeta * ug[i][j].b - xeta * ug[i][j].c);
	r = (-yxi * ug[i][j].b + xxi * ug[i][j].c);
	c = sqrt (Gamma * rgas * tg[i][j]);
	
	deltat[i][j] = safety_factor * 2.8284 * a[i][j] /

	  ( (fabs(q) + fabs(r))/ug[i][j].a + c * 
	   sqrt(xxi*xxi + yxi*yxi + xeta*xeta + yeta*yeta +
				  2.0 * fabs(xeta*xxi + yeta*yxi)));
      }
    
    /* If that's the user's choice, make it time accurate */
    if (ntime == 1) {
      mint = 100000.0;
      for (i = 1; i < imax; ++i)
	for (j = 1; j < jmax; ++j)
	  if (deltat[i][j] < mint)
	    mint = deltat[i][j];
      
      for (i = 1; i < imax; ++i)
	for (j = 1; j < jmax; ++j)
	  deltat[i][j] = mint;
    }
  }

  void calculateDummyCells(double **localpg,
			   double **localtg, struct Statevector **localug) {
    double c;
    double jminus;
    double jplus;
    double s;
    double rho, temp, u, v;
    double scrap, scrap2;
    double theta;
    double uprime;
    int i, j;
    struct Vector2 norm,tan,u1;

    uff = machff;
    jplusff = uff + 2.0 / (Gamma - 1.0) * cff;
    jminusff = uff - 2.0 / (Gamma - 1.0) * cff;
    
    for (i = 1; i < imax; ++i) {
      /* Bottom wall boundary cells */
      /* Routine checked by brute force for initial conditions, 4/9; 4:30 */
      /* Routine checked by brute force for random conditions, 4/13, 4:40 pm */
      /* Construct tangent vectors */
      tan.ihat = xnode[i][0] - xnode[i-1][0];
      tan.jhat = ynode[i][0] - ynode[i-1][0];
      norm.ihat = - (ynode[i][0] - ynode[i-1][0]);
      norm.jhat = xnode[i][0] - xnode[i-1][0];
      
      scrap = magnitude(&tan);
      tan.ihat = tan.ihat / scrap;
      tan.jhat = tan.jhat / scrap;
      scrap = magnitude(&norm);
      norm.ihat = norm.ihat / scrap;
      norm.jhat = norm.jhat / scrap;
      
      /* now set some state variables */
      rho = localug[i][1].a;
      localtg[i][0] = localtg[i][1];
      u1.ihat = localug[i][1].b / rho;
      u1.jhat = localug[i][1].c / rho;
      
      u = dot(&u1,&tan) + dot(&u1,&norm) * tan.jhat /norm.jhat;
      u = u / (tan.ihat - (norm.ihat * tan.jhat / norm.jhat));
      
      v = - (dot(&u1,&norm) + u * norm.ihat) / norm.jhat;
      
      /* And construct the new state vector */
      localug[i][0].a = localug[i][1].a;
      localug[i][0].b = rho * u;
      localug[i][0].c = rho * v;
      localug[i][0].d = rho * (Cv * localtg[i][0] + 0.5 * (u*u + v*v));
      localpg[i][0] = localpg[i][1];
      
      /* Top Wall Boundary Cells */
      /* Checked numerically for default conditions, 4/9 at 5:30 pm */
      /* Construct normal and tangent vectors */
      /* This part checked and works; it produces the correct vectors */
      tan.ihat = xnode[i][jmax-1] - xnode[i-1][jmax-1];
      tan.jhat = ynode[i][jmax-1] - ynode[i-1][jmax-1];
      norm.ihat = ynode[i][jmax-1] - ynode[i-1][jmax-1];
      norm.jhat = -(xnode[i][jmax-1] - xnode[i-1][jmax-1]);
      
      scrap = magnitude(&tan);
      tan.ihat = tan.ihat / scrap;
      tan.jhat = tan.jhat / scrap;
      scrap = magnitude(&norm);
      norm.ihat = norm.ihat / scrap;
      norm.jhat = norm.jhat / scrap;
     
      /* now set some state variables */
      rho = localug[i][jmax-1].a;
      temp = localtg[i][jmax-1];
      u1.ihat = localug[i][jmax-1].b / rho;
      u1.jhat = localug[i][jmax-1].c / rho;
     
      u = dot(&u1,&tan) + dot(&u1,&norm) * tan.jhat /norm.jhat;
      u = u / (tan.ihat - (norm.ihat * tan.jhat / norm.jhat));
     
      v = - (dot(&u1,&norm) + u * norm.ihat) / norm.jhat;
      
      /* And construct the new state vector */
      localug[i][jmax].a = localug[i][jmax-1].a;
      localug[i][jmax].b = rho * u;
      localug[i][jmax].c = rho * v;
      localug[i][jmax].d = rho * (Cv * temp + 0.5 * (u*u + v*v));
      localtg[i][jmax] = temp;
      localpg[i][jmax] = localpg[i][jmax-1];
    }
   
   for (j = 1; j < jmax; ++j) {
     /* Inlet Boundary Cells: unchecked */
     /* Construct the normal vector; This works, 4/10, 2:00 pm */
     norm.ihat = ynode[0][j-1] - ynode[0][j];
     norm.jhat = xnode[0][j] - xnode[0][j-1];
     scrap = magnitude(&norm);
     norm.ihat = norm.ihat / scrap;
     norm.jhat = norm.jhat / scrap;
     theta = acos((ynode[0][j-1] - ynode[0][j]) / 
      sqrt((xnode[0][j] - xnode[0][j-1])*(xnode[0][j] - xnode[0][j-1]) 
	   + (ynode[0][j-1] - ynode[0][j]) * (ynode[0][j-1] - ynode[0][j])));
     
     u1.ihat = localug[1][j].b / localug[1][j].a;
     u1.jhat = localug[1][j].c / localug[1][j].a;
     uprime = u1.ihat * cos(theta);
     c = sqrt(Gamma * rgas * localtg[1][j]);
     /* Supersonic inflow; works on the initial cond, 4/10 at 3:10 pm */
     if (uprime < -c) {
       /* Use far field conditions */
       localug[0][j].a = rhoff;
       localug[0][j].b = rhoff * uff;
       localug[0][j].c = rhoff * vff;
       localug[0][j].d = rhoff * (Cv * tff + 0.5 * (uff*uff + vff*vff));
       localtg[0][j] = tff;
       localpg[0][j] = pff;
     }
     /* Subsonic inflow */
     /* This works on the initial conditions 4/10 @ 2:20 pm */
     else if(uprime < 0.0) {
       /* Calculate Riemann invarients here */
       jminus = u1.ihat - 2.0/(Gamma-1.0) * c;
       s = log(pff) - Gamma * log(rhoff);
       v = vff;
       
       u = (jplusff + jminus) / 2.0;
       scrap = (jplusff - u) * (Gamma-1.0) * 0.5;
       localtg[0][j] = (1.0 / (Gamma * rgas)) * scrap * scrap;
       localpg[0][j] = exp(s) / pow((rgas * localtg[0][j]), Gamma);
       localpg[0][j] = pow(localpg[0][j], 1.0 / (1.0 - Gamma));
       
       /* And now: construct the new state vector */
       localug[0][j].a = localpg[0][j] / (rgas * localtg[0][j]);
       localug[0][j].b = localug[0][j].a * u;
       localug[0][j].c = localug[0][j].a * v;
       localug[0][j].d = localug[0][j].a * (Cv * tff + 0.5 * (u*u + v*v));
     }
     /* Other options */
     /* We should throw an exception here */
     else {
       printf("You have outflow at the inlet, which is not allowed.\n");
     }
     
     /* Outlet Boundary Cells */
     /* Construct the normal vector; works, 4/10 3:10 pm */
     norm.ihat = ynode[0][j] - ynode[0][j-1];
     norm.jhat = xnode[0][j-1] - xnode[0][j];
     scrap = magnitude(&norm);
     norm.ihat = norm.ihat / scrap;
     norm.jhat = norm.jhat / scrap;
     scrap = xnode[0][j-1] - xnode[0][j];
     scrap2 = ynode[0][j] - ynode[0][j-1];
     theta = acos((ynode[0][j] - ynode[0][j-1]) / 
		       sqrt(scrap*scrap + scrap2*scrap2));
     
     u1.ihat = localug[imax-1][j].b / localug[imax-1][j].a;
     u1.jhat = localug[imax-1][j].c / localug[imax-1][j].a;
     uprime = u1.ihat * cos(theta);
     c = sqrt(Gamma * rgas * localtg[imax-1][j]);
     /* Supersonic outflow; works for defaults cond, 4/10: 3:10 pm */
     if (uprime > c){
       /* Use a backward difference 2nd order derivative approximation */
       /* To set values at exit */
       localug[imax][j].a = 2.0 * localug[imax-1][j].a - localug[imax-2][j].a;
       localug[imax][j].b = 2.0 * localug[imax-1][j].b - localug[imax-2][j].b;
       localug[imax][j].c = 2.0 * localug[imax-1][j].c - localug[imax-2][j].c;
       localug[imax][j].d = 2.0 * localug[imax-1][j].d - localug[imax-2][j].d;
       localpg[imax][j] = 2.0 * localpg[imax-1][j] - localpg[imax-2][j];
       localtg[imax][j] = 2.0 * localtg[imax-1][j] - localtg[imax-2][j];
     }
     /* Subsonic Outflow; works for defaults cond, 4/10: 3:10 pm */
     else if (uprime < c && uprime > 0) {
       jplus = u1.ihat + 2.0/(Gamma - 1) * c;
       v = localug[imax-1][j].c / localug[imax-1][j].a;
       s = log(localpg[imax-1][j]) -
	                         Gamma * log(localug[imax-1][j].a);
       
       u = (jplus + jminusff) / 2.0;
       scrap =(jplus - u)* (Gamma-1.0) * 0.5;
       localtg[imax][j] = (1.0 / (Gamma * rgas)) * scrap * scrap;
       localpg[imax][j] = exp(s) / 
	                       pow((rgas * localtg[imax][j]), Gamma);
       localpg[imax][j] = pow(localpg[imax][j], 1.0 / (1.0-Gamma));
       rho = localpg[imax][j]/ (rgas * localtg[imax][j]);
       
       /* And now, construct the new state vector */
       localug[imax][j].a = rho;
       localug[imax][j].b = rho * u;
       localug[imax][j].c = rho * v;
       localug[imax][j].d = rho * (Cv * localtg[imax][j] + 0.5 * (u*u + v*v));
       
     }
     /* Other cases that shouldn't have to be used. */
     else if (uprime < -c) {
       /* Supersonic inflow */
       /* Use far field conditions */
       localug[0][j].a = rhoff;
       localug[0][j].b = rhoff * uff;
       localug[0][j].c = rhoff * vff;
       localug[0][j].d = rhoff * (Cv * tff + 0.5 * (uff*uff + vff*vff));
       localtg[0][j] = tff;
       localpg[0][j] = pff;
     }
     /* Subsonic inflow */
     /* This works on the initial conditions 4/10 @ 2:20 pm */
     else if(uprime < 0.0) {
       /* Debug: throw exception here? */
       /* Calculate Riemann invarients here */
       jminus = u1.ihat - 2.0/(Gamma-1.0) * c;
       s = log(pff) - Gamma * log(rhoff);
       v = vff;
       
       u = (jplusff + jminus) / 2.0;
       scrap = (jplusff - u)* (Gamma-1.0) * 0.5;
       localtg[0][j] = (1.0 / (Gamma * rgas)) * scrap * scrap;
       localpg[0][j] = exp(s) / pow((rgas * localtg[0][j]), Gamma);
       localpg[0][j] = pow(localpg[0][j], 1.0 / (1.0 - Gamma));
       
       /* And now: construct the new state vector */
       localug[0][j].a = localpg[0][j] / (rgas * localtg[0][j]);
       localug[0][j].b = localug[0][j].a * u;
       localug[0][j].c = localug[0][j].a * v;
       localug[0][j].d = localug[0][j].a * (Cv * tff + 0.5 * (u*u + v*v));
     }
     /* Other Options */
     /* Debug: throw exception here? */
     else {
       printf("You have inflow at the outlet, which is not allowed.\n");
     }
   } 
   /* Do something with corners to avoid division by zero errors */
   /* What you do shouldn't matter */
   localug[0][0] = localug[1][0];
   localug[imax][0] = localug[imax][1];
   localug[0][jmax] = localug[1][jmax];
   localug[imax][jmax] = localug[imax][jmax-1];
  }

  void runiters(){

  int i; 

  for (i = 0; i<iter; i++){
    doIteration();
  }
  
  }


double** newdoublearray(int nx, int ny)
  {
    int i; 
    double **a; 

    a = (double**) malloc(sizeof(double*)*nx); 
    for (i=0; i<nx; i++){
      a[i] =(double*) malloc(sizeof(double)*ny); 
    }
   
    return a;
  }

struct Statevector** newStatevectorarray(int nx, int ny)
  {
    int i; 
    struct Statevector **a; 

    a = (struct Statevector**) malloc(sizeof(struct Statevector*)*nx); 
    for (i=0; i<nx; i++){
      a[i] = (struct Statevector*) malloc(sizeof(struct Statevector)*ny); 
    }
   
    return a;
  }

  double magnitude(struct Vector2* this) {
    double mag;

    mag = sqrt(this->ihat*this->ihat + this->jhat * this->jhat);
    return mag;
  }

  double dot(struct Vector2 *this, struct Vector2* that) {
    /* Calculates dot product of two 2-d vector */
    double answer;
    
    answer = this->ihat * that->ihat + this->jhat * that->jhat;
    
    return answer;
  }



  void initialise(){

      init();

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
    double refval[] = {0.0033831416599344965,0.006812543658280322};
    double dev; 

    dev = fabs(error - refval[size]); 
    if (dev > 1.0e-10){
      printf("Validation failed\n"); 
      printf("Computed RMS pressure error = %.20f\n",error);
      printf("Reference value = %.20f\n", refval[size]); 
    }
  }

  void tidyup(){    

    /* should do lots of tedious frees in here */

  }


  void run(int sizein){
 

    size = sizein; 

    initialise(); 

    application(); 

    validate(); 
    tidyup(); 

    ops = iter;     
    printf("Section3:Euler:Run    %f (s)       %f      (Iterations/s)\n", kernel_time, (double) ops / kernel_time);   
    
  }



 
