#pragma once

#include <math.h>

#include "parameters.h"
#include "point.h"
#include "macro.h"
#include "pico.h"

class Mobile
{

private:

	double distance_macro[MACRO_NUM];
	int macro_neighbor[MACRO_NUM];
	double channel_gain_service_macro;

	int allocated_macro_count;
	int allocated_ABS_count;
	int allocated_nonABS_count;

public:

	point location;

	double QoS;

	int service_BS;					// 0: macro 1:pico

	int num_interferer_pico;

	int pico_neighbor[PICO_NUM];

	int macro_service;
	int pico_service;

	int associated_BS_static;		// 0: macro 1: pico

	double channel_gain_service_pico;

	double macro_interference;
	double pico_interference;

	double distance_pico[PICO_NUM];

	double channel_gain_macro[MACRO_NUM];
	double channel_gain_pico[PICO_NUM];

	Mobile(point location, double qos);

	void locate_on_macro_of(int mac, Macro *macro);
	void locate_on_pico_of(int pic, Pico *pico);

	void set_dist_pico(int cell_num, double loc_x, double loc_y, double tx_pow, double no);
	void set_dist_pico_1( int cell_num, double dist_temp, double tx_pow, double no);

	void set_pico_interference(int pico_num);
	void set_macro_interference(int macro_num);

	void set_serviceBS(int _serviceBS );

	void increase_allocated_macro_count();
	void increase_allocated_ABS_count();
	void increase_allocated_nonABS_count();

	int get_allocated_macro_count();
	int get_allocated_ABS_count();
	int get_allocated_nonABS_count();

};

