#pragma once

#include "parameters.h"

class macro
{
public:

	int num_pico;
	int pico_neighbor[PICO_NUM];
	int num_mobile;
	int mobile[MOBILE_NUM];
	int mobile_service[MOBILE_NUM];
	int mobile_service_01[MOBILE_NUM];
	
	double location_x;
	double location_y;

	double tx_power;

	// proposed algorithm 1
	int selected_user_PA1;
	int covered_pico_PA1;
	////
	macro();
	macro(double loc_x, double loc_y, double t_pow);

	void macro_set_initial(double loc_x, double loc_y, double t_pow);
	void macro_set_neighbor();

	void set_user_PA1(int _selected_user, int _covered_pico);
};

