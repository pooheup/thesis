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
	int service_mobile_01[MOBILE_NUM]; // 1이 서비스
	int service_mobile[MOBILE_NUM]; // 위에서 가져옴

	point location;

	double tx_power;


	// proposed algorithm 1
	int ABS_user_PA1;
	int ABS_user2_PA1;
	int nA_user1_PA1;
	int nA_user2_PA1;
	int nA_mode;		// 0 first user: nA_user1_PA1, 1 second user: nA_user2_PA1

	Pico(point location, double t_pow);

	void locate_on_macro_of(int i, Macro *macro);
	int is_neighbor_macro(int i);

	void set_neighbor();

	void set_user_PA1(int _ABS_user_PA1, int _ABS_user2_PA1, int _nA_user1_PA1, int _nA_user2_PA1, int _nA_mode);
};

