#include "macro.h"

Macro::Macro(point location, double t_pow)
{
	this->location   = location;
	this->tx_power   = t_pow;
	this->num_mobile = 0;
}

void Macro::register_mobile_to_service(int mob)
{
	mobile_service[this->num_mobile++] = mob;
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

int Macro::getMobileCount()
{
	return this->num_mobile;
}
