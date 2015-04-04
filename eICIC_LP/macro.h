#pragma once

#include "parameters.h"
#include "point.h"

class Macro
{

private:

	point location;

public:

	int num_pico;
	int pico_neighbor[PICO_NUM];
	int num_mobile;
	int mobile[MOBILE_NUM];
	int mobile_service[MOBILE_NUM];
	int mobile_service_01[MOBILE_NUM];

	double tx_power;

	// proposed algorithm 1
	int selected_user_PA1;
	int covered_pico_PA1;

	Macro(point location, double t_pow);

	void macro_set_neighbor();

	void set_user_PA1(int _selected_user, int _covered_pico);

	point getLocation();

};

