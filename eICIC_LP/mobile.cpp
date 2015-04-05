#include "mobile.h"

Mobile::Mobile(point loc, double qos)
: location(loc), QoS(qos)
{

	// 0: macro 1:pico
	this->service_BS    = 0;

	this->macro_service = -1;
	this->pico_service  = -1;

	this->num_interferer_pico = 0;

	this->lambda = 0.1;
	this->mu = 0.0;

	this->thrp_result_PA1 = 0.0;
	this->rate_user_PA1 = 0.0;

	this->allocated_macro_count = 0;
	this->allocated_ABS_count = 0;
	this->allocated_nonABS_count = 0;

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

void Mobile::locate_on_macro_of(int mac, Macro *macro)
{
	double distance = POINT_DISTANCE(macro->getLocation(), this->location);
	int is_neighbor = distance < NEIGHBOR_DIST_M;

	if (this->macro_service < 0)
	{
		this->macro_service = mac;
	}
	else
	{
		// 이번 macro가 이전 service macro보다 더 가까울 때
		if (distance < this->distance_macro[this->macro_service])
		{
			this->macro_service = mac;
		}
	}

	this->distance_macro[mac]     = distance;
	this->macro_neighbor[mac]     = is_neighbor;
	this->channel_gain_macro[mac] = macro->getTxPower() * pow( (1/distance), PATH_LOSS_EXPO );
}

void Mobile::locate_on_pico_of(int pic, Pico *pico)
{
	double distance = POINT_DISTANCE(pico->location, this->location);
	int is_neighbor = distance < NEIGHBOR_DIST_P;

	if (is_neighbor)
		this->num_interferer_pico++;

	if (this->pico_service < 0)
	{
		this->pico_service = pic;
	}
	else
	{
		if (distance < this->distance_pico[this->pico_service])
		{
			this->pico_service = pic;
		}
	}

	this->distance_pico[pic]     = distance;
	this->channel_gain_pico[pic] = pico->tx_power * pow((1/distance), PATH_LOSS_EXPO);
	this->pico_neighbor[pic]     = is_neighbor;
}

void Mobile::set_serviceBS (int _serviceBS )
{
	service_BS = _serviceBS;
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

void Mobile::increase_allocated_macro_count()  { this->allocated_macro_count++; }
void Mobile::increase_allocated_ABS_count()    { this->allocated_ABS_count++; }
void Mobile::increase_allocated_nonABS_count() { this->allocated_nonABS_count++; }

int Mobile::get_allocated_macro_count()  { return this->allocated_macro_count; }
int Mobile::get_allocated_ABS_count()    { return this->allocated_ABS_count; }
int Mobile::get_allocated_nonABS_count() { return this->allocated_nonABS_count; }
