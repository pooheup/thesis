#pragma once

#include "parameters.h"
#include "point.h"
#include "macro.h"

class Pico
{

private:

	double distance_macro[MACRO_NUM];
	int macro_neighbor[MACRO_NUM];

public:

	int mobile[MOBILE_NUM];

	int num_service_mobile; // # of set
	int service_mobile[MOBILE_NUM]; // ������ ������

	point location;

	double tx_power;


	// proposed algorithm 1
	int ABS_user1_PA1;
	int ABS_user2_PA1;
	int nA_user1_PA1;
	int nA_user2_PA1;

	Pico(point location, double t_pow);

	void locate_on_macro_of(int mac, Macro *macro);
	int is_neighbor_macro(int mac);

	void register_mobile_to_service(int mob);

	void set_user_PA1(int _ABS_user1_PA1, int _ABS_user2_PA1, int _nA_user1_PA1, int _nA_user2_PA1);
};

