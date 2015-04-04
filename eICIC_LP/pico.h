#pragma once

#include "parameters.h"
#include "point.h"

class Pico
{
public:

	int num_macro;
	int macro_neighbor[MACRO_NUM];
	int num_mobile;
	int mobile[MOBILE_NUM];

	int num_service_mobile;  // # of set
	int service_mobile_01[MOBILE_NUM]; // 1이 서비스
	int service_mobile[MOBILE_NUM]; // 위에서 가져옴

	point location;

	double tx_power;

	double distance_mobile[MOBILE_NUM];

	double channel_gain_ratio[MOBILE_NUM];

	double distance_macro[MACRO_NUM];

	// proposed algorithm 1
	int ABS_user_PA1;
	int ABS_user2_PA1;
	int nA_user1_PA1;
	int nA_user2_PA1;
	int nA_mode;		// 0 first user: nA_user1_PA1, 1 second user: nA_user2_PA1
	////

	Pico();
	Pico(point location, double t_pow);

	void pico_set_initial(point location, double t_pow);

	void set_channel_ratio(int mobile_num, double dist_macro, double macro_pow ,double no);
	void set_neighbor();
	void set_neighbor_1();

	void set_user_PA1(int _ABS_user_PA1, int _ABS_user2_PA1, int _nA_user1_PA1, int _nA_user2_PA1, int _nA_mode);
};

