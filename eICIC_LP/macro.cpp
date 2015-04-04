#include "macro.h"

Macro::Macro(point location, double t_pow)
{
	this->location = location;
	tx_power	= t_pow;
}

void Macro::set_neighbor()
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

void Macro::set_user_PA1(int _selected_user, int _covered_pico)
{
	selected_user_PA1	= _selected_user;
	covered_pico_PA1	= _covered_pico;
}

point Macro::getLocation()
{
	return location;
}

double Macro::getTxPower()
{
	return tx_power;
}
