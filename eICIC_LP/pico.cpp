#include "pico.h"
#include <stdio.h>
#include <math.h>

Pico::Pico(point location, double t_pow)
{
	this->location = location;
	tx_power	= t_pow;

	num_service_mobile = 0;

	for (int i = 0; i < MACRO_NUM; i++)
		macro_neighbor[i]	= -1;

	for (int i = 0; i < MOBILE_NUM; i++)
	{
		mobile[i]			= -1;
		service_mobile[i]	= -1;
		service_mobile_01[i] = 0;
	}

}

void Pico::locate_on_macro_of(int i, Macro *macro)
{
	distance_macro[i] = POINT_DISTANCE(macro->getLocation(), location);
	macro_neighbor[i] = distance_macro[i] < MP_INT_DIST;
}

int Pico::is_neighbor_macro(int i)
{
	return macro_neighbor[i] == 1;
}


void Pico::set_neighbor()
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

