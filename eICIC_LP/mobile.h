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

	double distance_pico[PICO_NUM];
	int pico_neighbor[PICO_NUM];
	double channel_gain_service_pico;
	int num_interferer_pico;

	int allocated_macro_count;
	int allocated_ABS_count;
	int allocated_nonABS_count;

public:

	const point location;

	const double QoS;

	int macro_service;
	int pico_service;

	double macro_interference;
	double pico_interference;

	double channel_gain_macro[MACRO_NUM];
	double channel_gain_pico[PICO_NUM];

	// /////////////////////////////////////////////////////////////////////////
	// 상태 변수

	// dual variable
	double lambda;
	double mu;
	
	// 현재까지 얻은 평균 throughput
	double thrp_result_PA1;
	
	// Ru 값
	double rate_user_PA1;

	// /////////////////////////////////////////////////////////////////////////
	// 컨텍스트 변수

	// 매 timeslot에서의 평균 throughput
	double thrpt_macro;
	double thrpt_ABS;
	double thrpt_nonABS;

	// PA1
	int resource_macro_PA1;
	int resource_ABS_PA1;
	int resource_nonABS_PA1;

	// /////////////////////////////////////////////////////////////////////////
	// 생성자
	Mobile(point location, double qos);

	// /////////////////////////////////////////////////////////////////////////
	// 함수

	void locate_on_macro_of(int mac, Macro *macro);
	void locate_on_pico_of(int pic, Pico *pico);

	void set_pico_interference(int pico_num);
	void set_macro_interference(int macro_num);

	void generate_channel_gain();
	void allocate_resource(int user_state_best_PA1);
	void calculate_throughput();
	void count_cell_association(int user_state_best_PA1);

	void calculate_rate_user();
	void calculate_dual_variable(const int t, const double STEP_SIZE, const double STEP_SIZE2);

	int get_allocated_macro_count();
	int get_allocated_ABS_count();
	int get_allocated_nonABS_count();

};

