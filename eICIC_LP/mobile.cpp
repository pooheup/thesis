#include "mobile.h"

mobile::mobile()
{
	location_x	= 0;
	location_y	= 0;

	QoS			= 0.0;

	pico_interference	= 0.0;
	macro_interference	= 0.0;

	service_BS	= 0;					// 0: macro 1:pico

	num_interferer_macro	= 0;

	macro_service			= -1;
	pico_service			= -1;

	int i;

	for ( i = 0; i < MACRO_NUM; i ++ )
	{
		macro_neighbor[i]	= -1;
	}

	for ( i = 0; i < PICO_NUM; i ++ )
	{
		pico_neighbor[i]	= -1;
	}

	for ( i = 0; i < MACRO_NUM; i ++ )
	{
		distance_macro[i]	= -1.0;
	}

	for ( i = 0; i < PICO_NUM; i ++ )
	{
		distance_pico[i]	= -1.0;
	}
}

mobile::mobile(double loc_x, double loc_y, double qos)
{
	location_x	= loc_x;
	location_y	= loc_y;

	QoS			= qos;

	service_BS	= 0;					// 0: macro 1:pico

	num_interferer_macro	= 0;

	macro_service			= -1;
	pico_service			= -1;

	int i;

	for ( i = 0; i < MACRO_NUM; i ++ )
	{
		macro_neighbor[i]	= -1;
	}

	for ( i = 0; i < PICO_NUM; i ++ )
	{
		pico_neighbor[i]	= -1;
	}

	for ( i = 0; i < MACRO_NUM; i ++ )
	{
		distance_macro[i]	= -1.0;
	}

	for ( i = 0; i < PICO_NUM; i ++ )
	{
		distance_pico[i]	= -1.0;
	}
}

void mobile::mobile_set_initial(double loc_x, double loc_y, double qos)
{
	location_x	= loc_x;
	location_y	= loc_y;

	QoS			= qos;
}

void mobile::mobile_set_dist_macro_1(int cell_num, double dist_temp, double tx_pow, double no)
{
	distance_macro[cell_num]		= dist_temp;
	channel_gain_macro[cell_num]	= tx_pow * pow( (1/dist_temp), PATH_LOSS_EXPO );
}

void mobile::mobile_set_dist_pico_1( int cell_num, double dist_temp, double tx_pow, double no)
{
	distance_pico[cell_num]			= dist_temp;
	channel_gain_pico[cell_num]		= tx_pow * pow( (1/dist_temp), PATH_LOSS_EXPO );
}

void mobile::mobile_set_num_int_macro(int num_macro_temp)
{
	num_interferer_macro = num_macro_temp;
}

void mobile::mobile_set_num_int_pico(int num_pico_temp)
{
	num_interferer_pico  = num_pico_temp;
}

void mobile::mobile_set_serviceBS (int _serviceBS )
{
	service_BS = _serviceBS;
}

void mobile::mobile_set_serviceBS_macro (int macro_temp)
{
	macro_service				= macro_temp;
}

void mobile::mobile_set_serviceBS_pico (int pico_temp)
{
	pico_service = pico_temp;
}

void mobile::mobile_set_pico_interference(int pico_num)
{
	pico_interference = 0.0;

	for (int i = 0; i < pico_num; i++)
	{
		pico_interference = pico_interference + channel_gain_pico[i];
	}
}

void mobile::mobile_set_macro_interference(int macro_num)
{
	macro_interference = 0.0;

	for (int i = 0; i < macro_num; i++)
	{
		macro_interference = macro_interference + channel_gain_macro[i];
	}
}

void mobile::mobile_cell_association_static(double _cre_bias)
{
	if (channel_gain_macro[macro_service] >= (_cre_bias * channel_gain_pico[pico_service])) associated_BS_static = 0;
	else associated_BS_static = 1;
}
