#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "rndfunctions.h"
#include "parameters.h"


double uniform(void)
{
	return ((double)(rand() & RAND_MAX)/(double)RAND_MAX);
}

// http://phoxis.org/2013/05/04/generating-random-numbers-from-normal-distribution-in-c/
double gaussian(double mu, double sigma)
{
	double U1, U2, W, mult;
	static double X1, X2;
	static int call = 0;

	if (call == 1)
	{
		call = !call;
		return (mu + sigma * (double) X2);
	}

	do
	{
		U1 = -1 + ((double) rand () / RAND_MAX) * 2;
		U2 = -1 + ((double) rand () / RAND_MAX) * 2;
		W = pow (U1, 2) + pow (U2, 2);
	}
	while (W >= 1 || W == 0);

	mult = sqrt ((-2 * log (W)) / W);
	X1 = U1 * mult;
	X2 = U2 * mult;

	call = !call;

	return (mu + sigma * (double) X1);
}

double rayleigh()
{
	return sqrt(pow(gaussian(0, 1/1.2533), 2) + pow(gaussian(0, 1/1.2533), 2));
}

double log_normal()
{
	return pow(10, gaussian(0, LN_SHAD) * 0.1);
}
