#include "pico.h"
#include <stdio.h>
#include <math.h>

Pico::Pico()
{
	location = { 0.0d, 0.0d };

	tx_power	= 0;

	int i;

	for ( i = 0; i < MACRO_NUM; i++ )
	{
		macro_neighbor[i]	= -1;
	}

	for ( i = 0; i < MOBILE_NUM; i++ )
	{
		mobile[i]			= -1;
		service_mobile[i]	= -1;
	}
}

Pico::Pico(point location, double t_pow)
{
	this->location = location;

	tx_power	= t_pow;

	int i;

	for ( i = 0; i < MACRO_NUM; i++ )
	{
		macro_neighbor[i]	= -1;
	}

	for ( i = 0; i < MOBILE_NUM; i++ )
	{
		mobile[i]			= -1;
		service_mobile[i]	= -1;
	}

}

void Pico::pico_set_initial(point location, double t_pow)
{
	this->location = location;

	tx_power	= t_pow;
}

void Pico::set_neighbor()
{
	int mobile_temp	= -1;
	double mobile_dist_temp = 0.0;
	double mobile_dist_best_temp = AREA_DIST * 2;

	for ( int j = 0; j < num_service_mobile; j++ )
	{
		mobile_temp	= -1;
		mobile_dist_best_temp = AREA_DIST * 2;
		for (int k = 0; k < MOBILE_NUM; k++ )
		{
			if ( ( distance_mobile[k] > mobile_dist_temp )  && ( distance_mobile[k] < mobile_dist_best_temp ) && ( service_mobile_01[k] == 1 ) )
			{
				mobile_dist_best_temp	= distance_mobile[k];
				mobile_temp				= k;
			}
		}
		if (mobile_temp == -1) ;
		else
		{
			service_mobile[j]	= mobile_temp;
			mobile_dist_temp	= mobile_dist_best_temp;
		}
	}
}

void Pico::set_neighbor_1()
{
	int num_mobile_temp = 0;
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		if (service_mobile_01[i] == 1)
		{
			service_mobile[num_mobile_temp] = i;
			num_mobile_temp++;
		}
	}
	num_service_mobile = num_mobile_temp;
}

void Pico::set_user_PA1(int _ABS_user_PA1,int _ABS_user2_PA1, int _nA_user1_PA1, int _nA_user2_PA1, int _nA_mode)
{
	ABS_user2_PA1 = _ABS_user2_PA1;
	ABS_user_PA1	= _ABS_user_PA1;
	nA_user1_PA1	= _nA_user1_PA1;
	nA_user2_PA1	= _nA_user2_PA1;
	nA_mode			= _nA_mode;
}

