#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "rndfunctions.h"
#include "parameters.h"


double uniform(void)
{
	return ((double)(rand() & RAND_MAX)/(double)RAND_MAX);
}

double uniform1(void)								// uniform random variable(0,1)
{
	int ran_num = RAND_MAX+1;
	double v0 = (double)(rand())+0.5;
	double v1 = (double)(rand())*(double)(ran_num);
	double x = (v0+v1)/pow((double)(ran_num),2);
	return x;
}

double uniform_1()
{
	return (((double)rand()*(double)RAND_MAX + (double)rand()) / pow((double)RAND_MAX, 2) );
}

double gaussian(void)
{
	static int ready = 0;
	static double qstore;
	double v1, v2, r, fac, gaus;

	if (ready == 0)
	{
		do
		{
			v1 = 2. * uniform();
			v2 = 2. * uniform();
			r = v1 * v1 + v2 * v2;
		} while (r > 1.0);
		fac = sqrt(-2. * log(r)/r);
		qstore = v1 * fac;
		gaus = v2 * fac;
		ready = 1;
	}
	else
	{
		ready = 0;
		gaus = qstore;
	}
	return(gaus);
}

double gaussian_1(double mean, double var){
	double x=0,tmp1,tmp2;

	while(1){
		tmp1=(double)uniform1();
		if (tmp1!=0.0)
			break;
	}
	tmp2=(double)uniform1();

	x=sqrt((-2*log(tmp1))) *cos(2*PI*tmp2) *  sqrt(var) + mean;

	return x;
}

double rayleigh()
{
	double a = sqrt(pow(gaussian_1(0,1/1.2533), 2) + pow(gaussian_1(0,1/1.2533), 2));
	return a;
}

double log_normal()
{
	double a = pow(10, gaussian_1(0, LN_SHAD)*0.1);
	return a;
}

