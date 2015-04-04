#include "mobile.h"

Mobile::Mobile(point location, double qos)
{
	this->location      = location;
	this->QoS           = qos;

	// 0: macro 1:pico
	this->service_BS    = 0;

	this->macro_service = -1;
	this->pico_service  = -1;

	for (int mac = 0; mac < MACRO_NUM; mac++)
	{
		macro_neighbor[mac] = -1;
		distance_macro[mac] = -1.0;
	}

	for (int pic = 0; pic < PICO_NUM; pic++)
	{
		pico_neighbor[pic] = -1;
		distance_pico[pic] = -1.0;
	}
}

void Mobile::set_dist_macro(int cell_num, double dist_temp, double tx_pow, double no)
{
	distance_macro[cell_num]        = dist_temp;
	channel_gain_macro[cell_num]    = tx_pow * pow( (1/dist_temp), PATH_LOSS_EXPO );
}

void Mobile::set_dist_pico_1( int cell_num, double dist_temp, double tx_pow, double no)
{
	distance_pico[cell_num]         = dist_temp;
	channel_gain_pico[cell_num]     = tx_pow * pow( (1/dist_temp), PATH_LOSS_EXPO );
}

void Mobile::set_num_int_pico(int num_pico_temp)
{
	num_interferer_pico = num_pico_temp;
}

void Mobile::set_serviceBS (int _serviceBS )
{
	service_BS = _serviceBS;
}

void Mobile::set_serviceBS_pico (int pico_temp)
{
	pico_service = pico_temp;
}

void Mobile::set_pico_interference(int pico_num)
{
	pico_interference = 0.0;

	for (int i = 0; i < pico_num; i++)
	{
		pico_interference = pico_interference + channel_gain_pico[i];
	}
}

void Mobile::set_macro_interference(int macro_num)
{
	macro_interference = 0.0;

	for (int i = 0; i < macro_num; i++)
	{
		macro_interference = macro_interference + channel_gain_macro[i];
	}
}

void Mobile::cell_association_static(double _cre_bias)
{
	if (channel_gain_macro[macro_service] >= (_cre_bias * channel_gain_pico[pico_service])) associated_BS_static = 0;
	else associated_BS_static = 1;
}
