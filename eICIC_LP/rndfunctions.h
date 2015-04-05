#ifndef _RNDFUNCTIONS_H_
#define _RNDFUNCTIONS_H_

double uniform(void);
double gaussian(double mu, double sigma);

#define RAYLEIGH    sqrt(pow(gaussian(0, 1/1.2533), 2) + pow(gaussian(0, 1/1.2533), 2))

#define LOG_NORMAL  pow(10, gaussian(0, LN_SHAD) * 0.1)

#endif /*_RNDFUNCTIONS_H_*/
