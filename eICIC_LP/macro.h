#pragma once

#include "parameters.h"
#include "point.h"

class Macro
{

private:

	point location;
	double tx_power;

	int num_mobile;

public:

	int mobile_service[MOBILE_NUM];
	int mobile_service_01[MOBILE_NUM];

	// proposed algorithm 1
	int selected_user_PA1;
	int covered_pico_PA1;

	Macro(point location, double t_pow);

	void set_neighbor();

	void set_user_PA1(int _selected_user, int _covered_pico);

	point getLocation();
	double getTxPower();

	int getMobileCount();

};

