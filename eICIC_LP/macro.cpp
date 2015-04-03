#include "macro.h"



macro::macro()
{
	location_x	= 0;
	location_y	= 0;

	tx_power	= 0;

	int i;

	for ( i = 0; i < PICO_NUM; i++ )
	{
		pico_neighbor[i]	= -1;
	}

	for ( i = 0; i < MOBILE_NUM; i++ )
	{		
		mobile[i]			= -1;
		mobile_service[i]	= -1;
		mobile_service_01[i]= 0;
	}
}

macro::macro(double loc_x, double loc_y, double t_pow)
{
	location_x	= loc_x;
	location_y	= loc_y;

	tx_power	= t_pow;

	int i;

	for ( i = 0; i < PICO_NUM; i++ )
	{
		pico_neighbor[i]	= -1;
	}

	for ( i = 0; i < MOBILE_NUM; i++ )
	{		
		mobile[i]			= -1;
	}
}

void macro::macro_set_initial(double loc_x, double loc_y, double t_pow)
{
	location_x	= loc_x;
	location_y	= loc_y;

	tx_power	= t_pow;
}

void macro::macro_set_neighbor()
{
	int num_mobile_temp = 0;
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		if (mobile_service_01[i] == 1)
		{
			mobile_service[num_mobile_temp] = i;
			num_mobile_temp++;
		}
	}
	num_mobile = num_mobile_temp;
}

void macro::set_user_PA1(int _selected_user, int _covered_pico)
{
	selected_user_PA1	= _selected_user;
	covered_pico_PA1	= _covered_pico;
}
