#include "pico.h"
#include <stdio.h>
#include <math.h>

Pico::Pico(point location, double t_pow)
{
	this->location = location;
	this->tx_power = t_pow;

	this->num_service_mobile = 0;

	for (int mac = 0; mac < MACRO_NUM; mac++)
		macro_neighbor[mac] = -1;

	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		mobile[mob]         = -1;
		service_mobile[mob] = -1;
	}

}

void Pico::locate_on_macro_of(int mac, Macro *macro)
{
	distance_macro[mac] = POINT_DISTANCE(macro->getLocation(), location);
	macro_neighbor[mac] = distance_macro[mac] < MP_INT_DIST;
}

int Pico::is_neighbor_macro(int i)
{
	return macro_neighbor[i] == 1;
}


void Pico::register_mobile_to_service(int mob)
{
	this->service_mobile[this->num_service_mobile++] = mob;
}

void Pico::set_user_PA1(int _ABS_user_PA1,int _ABS_user2_PA1, int _nA_user1_PA1, int _nA_user2_PA1, int _nA_mode)
{
	ABS_user2_PA1   = _ABS_user2_PA1;
	ABS_user_PA1    = _ABS_user_PA1;
	nA_user1_PA1    = _nA_user1_PA1;
	nA_user2_PA1    = _nA_user2_PA1;
	nA_mode         = _nA_mode;
}

