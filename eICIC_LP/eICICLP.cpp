#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "parameters.h"
#include "rndfunctions.h"
#include "macro.h"
#include "pico.h"
#include "mobile.h"
#include "eICIC.h"

void main()
{
	time_t starttime;
	time_t checktime;
	time(&starttime);
	
	srand(starttime);

// 초기화	
	int    i, j, t;
	double step_size = 1.0;

	// 출력 파일들
	std::ofstream Savefile("eICIC.txt");
	std::ofstream savel("lambda.txt");
	std::ofstream savem("mu.txt");
	std::ofstream results("results.txt");
//	std::ifstream node("node.txt");
	std::ifstream node("node_1.txt");
	std::ifstream pico_loc("pico.txt");
	savel.precision(5);
	savem.precision(5);
	results.precision(5);
	
	// noise
	double no = BW_PER_RB * powl(10, (NOISE / 10));

	// 위치 설정
	double macro_loc_temp[MACRO_NUM][2];
	double pico_loc_temp[PICO_NUM][2];
	double mobile_loc_temp[MOBILE_NUM][2];
/*
	for (i = 0; i < MACRO_NUM; i++)
	{
		for (j = 0; j<2; j++)
		{
			macro_loc_temp[i][j] = uniform() * AREA_DIST;
		}
	}

	for (i = 0; i < PICO_NUM; i++)
	{
		for (j = 0; j<2; j++)
		{
			pico_loc_temp[i][j] = uniform() * AREA_DIST;
		}
	}

	for (i = 0; i < MOBILE_NUM; i++)
	{
		for (j = 0; j<2; j++)
		{
			mobile_loc_temp[i][j] = uniform() * AREA_DIST;
		}
	}
*/
	for (i = 0; i < MACRO_NUM; i++)
	{
		double radius	= uniform() * AREA_RADIUS;
		double angle	= uniform() * 2 * PI;

		macro_loc_temp[i][0] = radius * cos(angle);
		macro_loc_temp[i][1] = radius * sin(angle);
	}

	for (i = 0; i < PICO_NUM; i++)
	{
		double radius	= uniform() * AREA_RADIUS;
		double angle	= uniform() * 2 * PI;

		pico_loc_temp[i][0] = radius * cos(angle);
		pico_loc_temp[i][1] = radius * sin(angle);
	}

	for (i = 0; i < MOBILE_NUM; i++)
	{
		double radius	= uniform() * AREA_RADIUS;
		double angle	= uniform() * 2 * PI;

		mobile_loc_temp[i][0] = radius * cos(angle);
		mobile_loc_temp[i][1] = radius * sin(angle);
	}

	// 각 노드의 위치 직접 지정, 위치를 지정해줄 경우 아래에서 직접 입력, 직접 입력할 경우 parameter.h의 LOC_SETUP = 1 로 설정
	if (LOC_SETUP == 1)
	{
	//	double bias_x = 1000;
//		double bias_y = 500;
		macro_loc_temp[0][0] = 0.0;
		macro_loc_temp[0][1] = 0.0;

		macro_loc_temp[1][0] = 1000.0;
		macro_loc_temp[1][1] = 0.0;

		macro_loc_temp[2][0] = -1000.0;
		macro_loc_temp[2][1] = 0.0;

		macro_loc_temp[3][0] = 500.0;
		macro_loc_temp[3][1] = 866.0;

		macro_loc_temp[4][0] = 500.0;
		macro_loc_temp[4][1] = -866.0;

		macro_loc_temp[5][0] = -500.0;
		macro_loc_temp[5][1] = 866.0;

		macro_loc_temp[6][0] = -500.0;
		macro_loc_temp[6][1] = -866.0;

////
		for (i = 0; i < PICO_NUM; i++)
		{
			pico_loc >> pico_loc_temp[i][0] >> pico_loc_temp[i][1];
		}

		for (i = 0; i < MOBILE_NUM; i++)
		{
			node >> mobile_loc_temp[i][0] >> mobile_loc_temp[i][1];
		}
		/*
		pico_loc_temp[0][0] = 1429;
		pico_loc_temp[0][1] = 1059;

		pico_loc_temp[1][0] = 1086;
		pico_loc_temp[1][1] = 694;

		pico_loc_temp[2][0] = 880;
		pico_loc_temp[2][1] = 1770;

		pico_loc_temp[3][0] = 1600;
		pico_loc_temp[3][1] = 1600;

		pico_loc_temp[4][0] = 606;
		pico_loc_temp[4][1] = 1194;



		mobile_loc_temp[0][0] = 1938;
		mobile_loc_temp[0][1] = 1012;

		mobile_loc_temp[1][0] = 265;
		mobile_loc_temp[1][1] = 1356;

		mobile_loc_temp[2][0] = 933;
		mobile_loc_temp[2][1] = 453;

		mobile_loc_temp[3][0] = 1418;
		mobile_loc_temp[3][1] = 1255;

		mobile_loc_temp[4][0] = 1119;
		mobile_loc_temp[4][1] = 1097;

		mobile_loc_temp[5][0] = 108;
		mobile_loc_temp[5][1] = 748;

		mobile_loc_temp[6][0] = 477;
		mobile_loc_temp[6][1] = 1713;
		//
		mobile_loc_temp[7][0] = 1796;
		mobile_loc_temp[7][1] = 1248;
		//
		mobile_loc_temp[8][0] = 1792;
		mobile_loc_temp[8][1] = 542;

		mobile_loc_temp[9][0] = 1245;
		mobile_loc_temp[9][1] = 1527;

		mobile_loc_temp[10][0] = 1738;
		mobile_loc_temp[10][1] = 1827;

		mobile_loc_temp[11][0] = 1384;
		mobile_loc_temp[11][1] = 809;
		*/

		
	}

	for (i = 0; i < PICO_NUM; i++)
	{
		printf("%f\t%f\n", pico_loc_temp[i][0], pico_loc_temp[i][1]);
	}
	printf("\n");
	for (i = 0; i < MOBILE_NUM; i++)
	{
		printf("%f\t%f\n", mobile_loc_temp[i][0], mobile_loc_temp[i][1]);
	}
	printf("\n");

	// 모바일 매크로 거리, 이웃노드 수, service BS 설정
	int mobile_num_neighborBS_temp[MOBILE_NUM];
	int mobile_service_macro_temp[MOBILE_NUM];
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
	double mobile_macro_dist_temp[MOBILE_NUM][MACRO_NUM];
	double mobile_service_macro_dist_temp[MOBILE_NUM];

	int mobile_macro_neighbor_temp[MOBILE_NUM][MACRO_NUM];

	for (i = 0; i < MOBILE_NUM; i++)
	{
		int neighbor_temp = 0;
		int service_macro_temp;

		double service_dist_temp = AREA_DIST * 2;

		for (j = 0; j < MACRO_NUM; j++)
		{
			mobile_macro_dist_temp[i][j] = mobile_set_dist_BS_temp(mobile_loc_temp[i][0], mobile_loc_temp[i][1], macro_loc_temp[j][0], macro_loc_temp[j][1]);

			if (mobile_macro_dist_temp[i][j] < NEIGHBOR_DIST_M)
			{
				neighbor_temp++;
				mobile_macro_neighbor_temp[i][j] = 1;
			}
			else
			{
				mobile_macro_neighbor_temp[i][j] = 0;
			}

			if (mobile_macro_dist_temp[i][j] < service_dist_temp)
			{
				service_dist_temp = mobile_macro_dist_temp[i][j];
				service_macro_temp = j;
			}
		}
		mobile_num_neighborBS_temp[i] = neighbor_temp;
		mobile_service_macro_temp[i] = service_macro_temp;
		mobile_service_macro_dist_temp[i] = service_dist_temp;
	}

	// pico-mobile 간 data를 취합하기 위한 pico temp data 초기화
	int pico_num_neighbormobile_temp[PICO_NUM];
	int pico_num_neighborMacro_temp[PICO_NUM];

	int pico_num_servicemobile_temp[PICO_NUM];

	int pico_servicemobile_01_temp[MOBILE_NUM][PICO_NUM];

	for (i = 0; i < PICO_NUM; i++)
	{
		pico_num_neighbormobile_temp[i] = 0;
		pico_num_neighborMacro_temp[i] = 0;

		pico_num_servicemobile_temp[i] = 0;

		for (j = 0; j < MOBILE_NUM; j++) pico_servicemobile_01_temp[j][i] = 0;
	}

	// 모바일 pico 거리, 이웃노드 수, service 설정		
	// pico class에 저장하기 위해 pico 관련 데이터도 위의 temp data를 이용하여 동시에 처리
	// 모두에 대해 0,1로 간섭유무 표현 1이면 간섭. 다 더해야 한다.
	int mobile_num_neighborBS_temp_pico[MOBILE_NUM];
	int mobile_service_pico_temp[MOBILE_NUM];

	double mobile_pico_dist_temp[MOBILE_NUM][PICO_NUM];

	int mobile_pico_neighbor_temp[MOBILE_NUM][PICO_NUM];

	for (i = 0; i < MOBILE_NUM; i++)
	{
		int neighbor_temp = 0;
		int service_pico_temp;

		double service_dist_temp = AREA_DIST * 2;

		for (j = 0; j < PICO_NUM; j++)
		{
			mobile_pico_dist_temp[i][j] = mobile_set_dist_BS_temp(mobile_loc_temp[i][0], mobile_loc_temp[i][1], pico_loc_temp[j][0], pico_loc_temp[j][1]);

			if (mobile_pico_dist_temp[i][j] < NEIGHBOR_DIST_P)
			{
				neighbor_temp++;
				mobile_pico_neighbor_temp[i][j] = 1;

				pico_num_neighbormobile_temp[j]++;
			}
			else
			{
				mobile_pico_neighbor_temp[i][j] = 0;
			}

			if (mobile_pico_dist_temp[i][j] < service_dist_temp)
			{
				service_dist_temp = mobile_pico_dist_temp[i][j];
				service_pico_temp = j;
			}
		}
		pico_num_servicemobile_temp[service_pico_temp]++;

		pico_servicemobile_01_temp[i][service_pico_temp] = 1;

		mobile_num_neighborBS_temp_pico[i]	= neighbor_temp;
		mobile_service_pico_temp[i]			= service_pico_temp;
	}

	// 매크로 피코 거리, 이웃노드 수					
	int macro_num_neighborBS_temp_pico[MOBILE_NUM];

	double macro_pico_dist_temp[MACRO_NUM][PICO_NUM];

	int macro_pico_neighbor_temp[MACRO_NUM][PICO_NUM];

	int macro_pico_neighbor_01_temp[MACRO_NUM][PICO_NUM];

	for (i = 0; i < MACRO_NUM; i++)
	{
		int neighbor_temp = 0;

		for (j = 0; j < PICO_NUM; j++)
		{
			macro_pico_dist_temp[i][j] = mobile_set_dist_BS_temp(macro_loc_temp[i][0], macro_loc_temp[i][1], pico_loc_temp[j][0], pico_loc_temp[j][1]);

			if (macro_pico_dist_temp[i][j] < MP_INT_DIST)
			{
				macro_pico_neighbor_temp[i][neighbor_temp] = j;
				neighbor_temp++;
				pico_num_neighborMacro_temp[j]++;

				macro_pico_neighbor_01_temp[i][j] = 1;
			}
			else macro_pico_neighbor_01_temp[i][j] = 0;
		}
		macro_num_neighborBS_temp_pico[i] = neighbor_temp;
	}

// class 생성: macro, pico, mobile
	macro macro[MACRO_NUM];
	pico pico[PICO_NUM];
	mobile mobile[MOBILE_NUM];

	// initial setting 각 클래스 초기화, 
	// 각 클래스에 모바일 간섭 기지국 수, 서비스 기지국 저장
	for (i = 0; i < MACRO_NUM; i++)
	{
		macro[i].macro_set_initial(macro_loc_temp[i][0], macro_loc_temp[i][1], MACRO_TX_POWER);
	}

	for (i = 0; i < PICO_NUM; i++)
	{
		pico[i].pico_set_initial(pico_loc_temp[i][0], pico_loc_temp[i][1], PICO_TX_POWER);
	}

	for (i = 0; i < MOBILE_NUM; i++)
	{
		mobile[i].mobile_set_initial(mobile_loc_temp[i][0], mobile_loc_temp[i][1], QOS);

		mobile[i].mobile_set_num_int_macro(mobile_num_neighborBS_temp[i]);
		mobile[i].mobile_set_serviceBS_macro(mobile_service_macro_temp[i]);

		mobile[i].mobile_set_num_int_pico(mobile_num_neighborBS_temp_pico[i]);
		mobile[i].mobile_set_serviceBS_pico(mobile_service_pico_temp[i]);

		macro[mobile_service_macro_temp[i]].mobile_service_01[i] = 1;
	}

// 위치관계에 따른 채널 정립. 이웃 찾기.  거리, 파워 기반 신호 세기.(간섭으로 써도 됨)
	// 채널 계산
	for (i = 0; i < MOBILE_NUM; i++)
	{
		for (j = 0; j < MACRO_NUM; j++)
		{
			// mobile[i].mobile_set_dist_macro( j, macro[j].location_x, macro[j].location_y, macro[j].tx_power, no );
			mobile[i].mobile_set_dist_macro_1(j, mobile_macro_dist_temp[i][j], macro[j].tx_power, no);
			mobile[i].macro_neighbor[j] = mobile_macro_neighbor_temp[i][j];
			macro[j].mobile[i] = mobile_macro_neighbor_temp[i][j];
		}

		for (j = 0; j < PICO_NUM; j++)
		{
			mobile[i].mobile_set_dist_pico_1(j, mobile_pico_dist_temp[i][j], pico[j].tx_power, no);
			mobile[i].pico_neighbor[j] = mobile_pico_neighbor_temp[i][j];
		}
	}

	// static 을 위해 cre bias를 통한 cell association
	double cre_bias = pow(10.0, CRE_STATIC / 10.0);
	for (i = 0; i < MOBILE_NUM; i++) mobile[i].mobile_cell_association_static(cre_bias);

	// macro 정보 넣기. 이웃 수, 이웃 명단
	for (i = 0; i < MACRO_NUM; i++)
	{
		macro[i].num_pico = macro_num_neighborBS_temp_pico[i];

		for (j = 0; j < PICO_NUM; j++)
		{
			macro[i].pico_neighbor[j] = macro_pico_neighbor_01_temp[i][j];
		}
	}

	// pico 정보 넣기.
	for (i = 0; i < PICO_NUM; i++)
	{
		pico[i].num_macro = pico_num_neighborMacro_temp[i];
		pico[i].num_mobile = pico_num_neighbormobile_temp[i];

		// mobile 이웃 정보
		pico[i].num_service_mobile = pico_num_servicemobile_temp[i];

		for (j = 0; j < MOBILE_NUM; j++)
		{
			pico[i].service_mobile_01[j] = pico_servicemobile_01_temp[j][i];
			pico[i].distance_mobile[j] = mobile_pico_dist_temp[j][i];
		}
		// 매크로 이웃 정보
		for (j = 0; j < MACRO_NUM; j++)
		{
			pico[i].macro_neighbor[j] = macro_pico_neighbor_01_temp[j][i];
			pico[i].distance_macro[j] = macro_pico_dist_temp[j][i];
		}
	}

	// pico의 이웃 정보.
//	for (i = 0; i < PICO_NUM; i++) pico[i].set_neighbor();
	for (i = 0; i < PICO_NUM; i++) pico[i].set_neighbor_1();
	for (i = 0; i < MACRO_NUM; i++) macro[i].macro_set_neighbor();

	// 각 모바일이 겪는 interference calculation
	// 모든 기지국에 대한 간섭을 계산. 실제 이용시 자신이 할당받는 기지국의 신호는 제해야 함.
	for (i = 0; i < MOBILE_NUM; i++)
	{
		mobile[i].mobile_set_pico_interference(PICO_NUM);
		mobile[i].mobile_set_macro_interference(MACRO_NUM);
	}
	
// 시뮬레이션에서 필요한 각 변수들 선언
	// 매 timeslot에서의 평균 throughput
	double thrpt_macro[MOBILE_NUM];
	double thrpt_ABS[MOBILE_NUM];
	double thrpt_nonABS[MOBILE_NUM];

	int resource_macro[MOBILE_NUM];
	int resource_ABS[MOBILE_NUM];
	int resource_nonABS[MOBILE_NUM];

	int num_allocated_macro[MOBILE_NUM];
	int num_allocated_ABS[MOBILE_NUM];
	int num_allocated_nonABS[MOBILE_NUM];

	// dual variable
	double lambda[MOBILE_NUM];
	double mu[MOBILE_NUM];

	// 현재까지 얻은 평균 throughput
	double thrp_result[MOBILE_NUM];

	// Ru 값
	double rate_user[MOBILE_NUM];

	// weight
	double weight[MOBILE_NUM];

	int abs_count_macro[MACRO_NUM];
	int abs_count_pico[PICO_NUM];

	for (i=0; i < MACRO_NUM; i++) abs_count_macro[i] = 0;
	for (i=0; i < PICO_NUM; i++) abs_count_pico[i] = 0;

	for (i = 0; i < MOBILE_NUM; i++)
	{
		lambda[i]		= 0.1;
		mu[i]			= 0.0;
		thrp_result[i]	= 0.0;
		rate_user[i]	= 0.0;
		weight[i]		= 1.0;

		num_allocated_macro[i]	= 0;
		num_allocated_ABS[i]	= 0;
		num_allocated_nonABS[i]	= 0;

	}

	// PA1
	int resource_macro_PA1[MOBILE_NUM];
	int resource_ABS_PA1[MOBILE_NUM];
	int resource_nonABS_PA1[MOBILE_NUM];

	double thrp_result_PA1[MOBILE_NUM];
	double rate_user_PA1[MOBILE_NUM];
	for (i = 0; i < MOBILE_NUM; i++)
	{
		thrp_result_PA1[i]	= 0.0;
		rate_user_PA1[i]	=	0.0;
	}

	// static
	double thrp_result_static[MOBILE_NUM];
	double lambda_static[MOBILE_NUM];
	double rate_user_static[MOBILE_NUM];

	for (i = 0; i < MOBILE_NUM; i++)
	{
		thrp_result_static[i]	= 0.0;
		lambda_static[i]		= 1.0;
		rate_user_static[i]		= 0.0;
	}

	// static_1
	double thrp_result_static_1[MOBILE_NUM];
	double lambda_static_1[MOBILE_NUM];
	double rate_user_static_1[MOBILE_NUM];

	for (i = 0; i < MOBILE_NUM; i++)
	{
		thrp_result_static_1[i]	= 0.0;
		lambda_static_1[i]		= 1.0;
		rate_user_static_1[i]		= 0.0;
	}

	// static_2
	double thrp_result_static_2[MOBILE_NUM];
	double lambda_static_2[MOBILE_NUM];
	double rate_user_static_2[MOBILE_NUM];

	for (i = 0; i < MOBILE_NUM; i++)
	{
		thrp_result_static_2[i]	= 0.0;
		lambda_static_2[i]		= 1.0;
		rate_user_static_2[i]		= 0.0;
	}
	
	// static_3
	double thrp_result_static_3[MOBILE_NUM];
	double lambda_static_3[MOBILE_NUM];
	double rate_user_static_3[MOBILE_NUM];

	for (i = 0; i < MOBILE_NUM; i++)
	{
		thrp_result_static_3[i]	= 0.0;
		lambda_static_3[i]		= 1.0;
		rate_user_static_3[i]		= 0.0;
	}

//////////////////////////////////////////////////////////////////////////////  알고리즘 따라 연산  //////////////////////////////////////////////////////////////////////
	for (t = 0; t < SIMULATION_TIME; t++)
	{
		double step_size2;
		step_size = 1.0 / ((double)(t + 1));
//		step_size2 = STEPSIZE2;
		step_size2 = (t < 10000) ? STEPSIZE2 : STEPSIZE3;
		if (t > 100000) step_size2 = STEPSIZE4;

//		step_size2 = step_size;

		double signal_temp;
		double interference_temp;
		for (i = 0; i < MOBILE_NUM; i++)
		{
			// macro 평균 thrpt 계산
			signal_temp			= mobile[i].channel_gain_macro[mobile[i].macro_service] *rayleigh() * log_normal();
			interference_temp	= (mobile[i].macro_interference + mobile[i].pico_interference - mobile[i].channel_gain_macro[mobile[i].macro_service]) *rayleigh() * log_normal();
			thrpt_macro[i]		= cal_thrpt_i(signal_temp, interference_temp, no) / 1000000.0;
			thrpt_macro[i] = thrpt_macro[i] / 10.0;

			// pico ABS 평균 thrpt 계산
			signal_temp			= mobile[i].channel_gain_pico[mobile[i].pico_service] *rayleigh() * log_normal();
			interference_temp	= (mobile[i].pico_interference - mobile[i].channel_gain_pico[mobile[i].pico_service]) *rayleigh() * log_normal();
			thrpt_ABS[i]		= cal_thrpt_i(signal_temp, interference_temp, no) / 1000000.0;
			thrpt_ABS[i] = thrpt_ABS[i] / 10.0;

			// pico non-ABS 평균 thrpt 계산
//			signal_temp			= mobile[i].channel_gain_pico[mobile[i].pico_service] *rayleigh() * log_normal();
			interference_temp	= interference_temp + (mobile[i].macro_interference ) *rayleigh() * log_normal();
			thrpt_nonABS[i]		= cal_thrpt_i(signal_temp, interference_temp, no) / 1000000.0;
			thrpt_nonABS[i] = thrpt_nonABS[i] / 10.0;

			// 할당 값 초기화
			resource_macro[i]	= 0;
			resource_ABS[i]		= 0;
			resource_nonABS[i]	= 0;

			// 할당 값 초기화 PA1
			resource_macro_PA1[i]	= 0;
			resource_ABS_PA1[i]		= 0;
			resource_nonABS_PA1[i]	= 0;
		}

		double object_value_es  = 0.0;
		double sum_rate_es		= 0.0;
		/*
		/////////////////////////////////////////////////////  exhaustive search
		// allocation을 찾기 위한 변수들
		int macro_allocation[MACRO_NUM];
		int pico_ABS_allocation[PICO_NUM];
		int pico_nonABS_allocation[PICO_NUM];
		int user_allocated[MOBILE_NUM];

		for (i = 0; i < MACRO_NUM; i++)	macro_allocation[i] = -1;
		for (i = 0; i < PICO_NUM; i++)
		{
			pico_ABS_allocation[i] = -1;
			pico_nonABS_allocation[i] = -1;
		}
		for (i = 0; i < MOBILE_NUM; i++) user_allocated[i] = 0;

		int user_num = 0;
		double objective_value_temp = 0.0;
		double objective_value_best = -1.0;
		int state_temp[MOBILE_NUM];
		int state_best[MOBILE_NUM];
		for (i = 0; i < MOBILE_NUM; i++)
		{
			state_temp[i] = 0;
			state_best[i] = 0;
		}

		// 최적의 할당을 exhaustive 방식으로 찾으며, 다음의 함수를 호출
		exhaustive_search_call_next_user(user_num, objective_value_temp, &objective_value_best, state_temp, state_best, mobile, macro, lambda, thrpt_macro, thrpt_ABS, thrpt_nonABS);
		
		// best state 즉, 구한 자원 할당 값 입력
		for (i = 0; i < MOBILE_NUM; i++)
		{
			if (state_best[i] == 1)	resource_macro[i]		= 1;
			else if (state_best[i] == 2) resource_ABS[i]	= 1;
			else if (state_best[i] == 3) resource_nonABS[i]	= 1;
		}

		for (i = 0; i < MOBILE_NUM; i++)
		{
			if (state_best[i] == 1)	num_allocated_macro[i]			= num_allocated_macro[i] + 1;
			else if (state_best[i] == 2) num_allocated_ABS[i]		= num_allocated_ABS[i] + 1;
			else if (state_best[i] == 3) num_allocated_nonABS[i]	= num_allocated_nonABS[i]+ 1;
		}

		// 각 기지국별 자원 사용했는지 여부 count // 사용한 유저가 없을경우 해당 기지국은 ABS. abs_count 증가
		for (i=0; i < MACRO_NUM; i++)
		{
			int resource_used_temp = 0; // 자원 할당 여부. 1이면 사용, 0이면 사용 안함
			for (j = 0; j < macro[i].num_mobile; j++)
			{
				if (resource_macro[macro[i].mobile_service[j]] == 1) resource_used_temp = 1;
			}
			if (resource_used_temp == 0) abs_count_macro[i]++;
		}

		// 현재까지 얻은 throughput 입력
		for (i = 0; i < MOBILE_NUM; i++) thrp_result[i] = thrp_result[i] + thrpt_macro[i] * resource_macro[i] + thrpt_ABS[i] * resource_ABS[i] + thrpt_nonABS[i] * resource_nonABS[i];

		// 평균 rate Ru, rate_user[i], 업데이트
		for (i = 0; i < MOBILE_NUM; i++)
		{
			if (lambda[i] == 0.0) rate_user[i] = RATE_MAX;
//			else rate_user[i] = 0.8* rate_user[i] + 0.2 * (weight[i] + mu[i]) / lambda[i];
			else rate_user[i] = (weight[i] + mu[i]) / lambda[i];
		}

		// object 값 계산
//		double object_value_es = 0.0;
//		for (i = 0; i < MOBILE_NUM; i++) 		object_value_es = object_value_es + log(thrp_result[i] / (1 + t));
//		for (i = 0; i < MOBILE_NUM; i++)		sum_rate_es		= sum_rate_es + thrp_result[i] / (1 + t);


//		for (i = 0; i < MOBILE_NUM; i++) 		object_value_es = object_value_es + log(thrp_result[i] / 1000000 / (1 + t));
//		for (i = 0; i < MOBILE_NUM; i++)		sum_rate_es		= sum_rate_es + thrp_result[i] /1000000 / (1 + t);
		*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////  제안하는 알고리즘을 바탕으로 솔루션 찾기.
		// 변수 선언
		int macro_user_PA[MACRO_NUM];
		int macro_cover_pico_PA[MACRO_NUM];
		int pico_ABS_user_PA[PICO_NUM];
		int pico_ABS_user_PA2[PICO_NUM];
		int pico_nA_user1_PA[PICO_NUM];
		int pico_nA_user2_PA[PICO_NUM];
		int pico_nA_01_PA[PICO_NUM];  // non-ABS 상황에서 몇번째 유저 할당되는지 indicator. 0 first, 1 second

		for (i = 0; i < PICO_NUM; i++) pico_nA_user2_PA[i] = -1;

		// 각 pico 에서 ABS best user, non-ABS first.second user 선택
		for (i = 0; i < PICO_NUM; i++)
		{
			int temp_pico_ABS_PA_user;
			int temp_pico_nA_PA1_user;
			int temp_pico_nA_PA2_user;
			int temp_pico_ABS_PA_user2;
			double temp_pico_ABS_PA = -10.0; // ABS best
			double temp_pico_ABS_PA2 = -11.0; // ABS best
			double temp_pico_nA_PA1 = -10.0; // non-ABS first
			double temp_pico_nA_PA2 = -11.0; // non-ABS second

			temp_pico_nA_PA1_user = -1;
			temp_pico_nA_PA2_user = -1;
			temp_pico_ABS_PA_user = -1;
			temp_pico_ABS_PA_user2 = -1;

			for (j = 0; j < pico[i].num_service_mobile; j++)
			{
				// ABS best user 찾기
				if (lambda[pico[i].service_mobile[j]] * thrpt_ABS[pico[i].service_mobile[j]] > temp_pico_ABS_PA)
				{
					temp_pico_ABS_PA2 = temp_pico_ABS_PA;
					temp_pico_ABS_PA_user2 = temp_pico_ABS_PA_user;

					temp_pico_ABS_PA		= lambda[pico[i].service_mobile[j]] * thrpt_ABS[pico[i].service_mobile[j]];
					temp_pico_ABS_PA_user	= pico[i].service_mobile[j];
				} 
				else // non-ABS second 찾기
				{
					if (lambda[pico[i].service_mobile[j]] * thrpt_ABS[pico[i].service_mobile[j]] > temp_pico_ABS_PA2)
					{
						temp_pico_ABS_PA2 = lambda[pico[i].service_mobile[j]] * thrpt_ABS[pico[i].service_mobile[j]];
						temp_pico_ABS_PA_user2 = pico[i].service_mobile[j];
					}
				}

				// non-ABS first 찾기
				if (lambda[pico[i].service_mobile[j]] * thrpt_nonABS[pico[i].service_mobile[j]] > temp_pico_nA_PA1)
				{
//					if (temp_pico_nA_PA1 >= temp_pico_nA_PA2)
//					{
					temp_pico_nA_PA2		= temp_pico_nA_PA1;
					temp_pico_nA_PA2_user	= temp_pico_nA_PA1_user; 
//					}
						
					temp_pico_nA_PA1		= lambda[pico[i].service_mobile[j]] * thrpt_nonABS[pico[i].service_mobile[j]];
					temp_pico_nA_PA1_user	= pico[i].service_mobile[j];
				}
				else // non-ABS second 찾기
				{
					if (lambda[pico[i].service_mobile[j]] * thrpt_nonABS[pico[i].service_mobile[j]] > temp_pico_nA_PA2)
					{
						temp_pico_nA_PA2		= lambda[pico[i].service_mobile[j]] * thrpt_nonABS[pico[i].service_mobile[j]];
						temp_pico_nA_PA2_user	= pico[i].service_mobile[j];
					}
				}
			}
			pico_ABS_user_PA2[i] = temp_pico_ABS_PA_user2;
			pico_ABS_user_PA[i] = temp_pico_ABS_PA_user;
			pico_nA_user1_PA[i] = temp_pico_nA_PA1_user;
			pico_nA_user2_PA[i] = temp_pico_nA_PA2_user;
			pico_nA_01_PA[i]	= 0; // 초기화
		}

		// 각 macro 유저 찾기 // mobile 번호: macro[i].mobile_service[j]
		for (i = 0; i < MACRO_NUM; i++)
		{
			int temp_macro_PA_user = -1;
			double temp_macro_PA = -1000.0;
			for (j = 0; j < macro[i].num_mobile; j++)
			{
				if (macro[i].mobile_service[j] == pico_nA_user1_PA[mobile[macro[i].mobile_service[j]].pico_service])
				{
					int user_temp_temp = pico_nA_user2_PA[mobile[macro[i].mobile_service[j]].pico_service]; // second user num
					double temp_temp;
					if (user_temp_temp != -1)  temp_temp = (lambda[macro[i].mobile_service[j]] * thrpt_macro[macro[i].mobile_service[j]] - lambda[macro[i].mobile_service[j]] * thrpt_nonABS[macro[i].mobile_service[j]] + lambda[user_temp_temp] * thrpt_nonABS[user_temp_temp]);
					else temp_temp = (lambda[macro[i].mobile_service[j]] * thrpt_macro[macro[i].mobile_service[j]] - lambda[macro[i].mobile_service[j]] * thrpt_nonABS[macro[i].mobile_service[j]]);

					if (temp_temp > temp_macro_PA)
					{
						temp_macro_PA		= temp_temp;
						temp_macro_PA_user	= macro[i].mobile_service[j];
					}
				}
				else
				{
					if (lambda[macro[i].mobile_service[j]] * thrpt_macro[macro[i].mobile_service[j]] > temp_macro_PA)
					{
						temp_macro_PA		= lambda[macro[i].mobile_service[j]] * thrpt_macro[macro[i].mobile_service[j]];
						temp_macro_PA_user	= macro[i].mobile_service[j];
					}
				}
			}
			macro_user_PA[i]		= temp_macro_PA_user;
			macro_cover_pico_PA[i]	= mobile[temp_macro_PA_user].pico_service;
			pico_nA_01_PA[mobile[temp_macro_PA_user].pico_service] = 1;
		}

		for (i = 0; i < MACRO_NUM; i++) macro[i].set_user_PA1(macro_user_PA[i], macro_cover_pico_PA[i]);
		for (i = 0; i < PICO_NUM; i++) pico[i].set_user_PA1(pico_ABS_user_PA[i], pico_ABS_user_PA2[i], pico_nA_user1_PA[i], pico_nA_user2_PA[i], pico_nA_01_PA[i]);

		int _macro_num = 0;
		double objective_value_best_PA1 = -1.0;
		int state_temp_PA1[MACRO_NUM];
		int state_best_PA1[MACRO_NUM];
		int user_state_best_PA1[MOBILE_NUM];
		
		for (i = 0; i < MACRO_NUM; i++)
		{
			state_temp_PA1[i] = 0;
			state_best_PA1[i] = 0;
		}
		for (i = 0; i < MOBILE_NUM; i++) user_state_best_PA1[i] = 0;

		// macro 들의 모든 조합에 대하여 최적의 값 선택
		PA1_call_next_pico(_macro_num, &objective_value_best_PA1, state_temp_PA1, state_best_PA1, user_state_best_PA1, mobile, pico, macro, lambda, thrpt_macro, thrpt_ABS, thrpt_nonABS);
		/*
		// exhaustive 값과 비교
		for (i = 0; i < MOBILE_NUM; i++)
		{
			
			std::cout << t << "\t" << i << "\t" << user_state_best_PA1[i] << "\t" << state_best[i] << std::endl;
		}
		std::cout << objective_value_best_PA1 << "\t" << objective_value_best << std::endl;
		Savefile << objective_value_best_PA1 << "\t" << objective_value_best << std::endl;
		if (abs(objective_value_best - objective_value_best_PA1) > 0.0001)
		{
			std::cout << "Proposed Algorithm error occurred" << std::endl;
			//Savefile << "Proposed Algorithm error occurred" << std::endl;

		}
		// 구한 값 update
		*/
		// resource 정보 입력
		// best state 즉, 구한 자원 할당 값 입력
		for (i = 0; i < MOBILE_NUM; i++)
		{
			if (user_state_best_PA1[i] == 1)	resource_macro_PA1[i]		= 1;
			else if (user_state_best_PA1[i] == 2) resource_ABS_PA1[i]	= 1;
			else if ( (user_state_best_PA1[i] == 3) || (user_state_best_PA1[i] == 4)) resource_nonABS_PA1[i]	= 1;
		}

		// 현재까지 얻은 throughput 입력
		for (i = 0; i < MOBILE_NUM; i++) thrp_result_PA1[i] = thrp_result_PA1[i] + thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i];


/*		// proposed algorithm update
		// dual variable, lambda, mu 업데이트
		for (i = 0; i < MOBILE_NUM; i++)
		{
			double lambda_temp, mu_temp;
			//			if ((thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i] >= 0.0)	//feasilbity
			//				&& (abs(thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i]) * lambda[i] < 0.05))
			if ((abs(thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i]) * lambda[i] < 0.05))
				lambda_temp = lambda[i] - step_size  * (thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i] - rate_user_PA1[i]);
			else
				lambda_temp = lambda[i] - step_size2 * (thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i] - rate_user_PA1[i]);
			lambda[i] = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

			//	if ((log(rate_user_PA1[i]) >= mobile[i].QoS)
			//		&& (abs(log(rate_user_PA1[i]) - mobile[i].QoS) * mu[i] < 0.01))
			if ((abs(log(rate_user_PA1[i]) - mobile[i].QoS) * mu[i] < 0.01))
				mu_temp = mu[i] - step_size  * (log(rate_user_PA1[i]) - mobile[i].QoS);
			else
				mu_temp = mu[i] - step_size2 * (log(rate_user_PA1[i]) - mobile[i].QoS);
			mu[i] = (0.0 > mu_temp) ? 0.0 : mu_temp;
		}
*/

		// 평균 rate Ru, rate_user[i], 업데이트
		for (i = 0; i < MOBILE_NUM; i++)
		{
			if (lambda[i] == 0.0) rate_user_PA1[i] = RATE_MAX;
			//			else rate_user[i] = 0.8* rate_user[i] + 0.2 * (weight[i] + mu[i]) / lambda[i];
			else rate_user_PA1[i] = 0.8 * rate_user_PA1[i] + 0.2 * (1.0 + mu[i]) / lambda[i];
		}


		// 각 유저 어느 기지국 사용했는지 count
		for (i = 0; i < MOBILE_NUM; i++)
		{
			if (user_state_best_PA1[i] == 1)	num_allocated_macro[i]			= num_allocated_macro[i] + 1;
			else if (user_state_best_PA1[i] == 2) num_allocated_ABS[i]			= num_allocated_ABS[i] + 1;
			else if ((user_state_best_PA1[i] == 3) || (user_state_best_PA1[i] == 4)) num_allocated_nonABS[i]	= num_allocated_nonABS[i] + 1;
		}

		// 각 기지국별 자원 사용했는지 여부 count // 사용한 유저가 없을경우 해당 기지국은 ABS. abs_count 증가
		for (i=0; i < MACRO_NUM; i++)
		{
			int resource_used_temp = 0; // 자원 할당 여부. 1이면 사용, 0이면 사용 안함
			for (j = 0; j < macro[i].num_mobile; j++)
			{
				if (resource_macro_PA1[macro[i].mobile_service[j]] == 1) resource_used_temp = 1;
			}
			if (resource_used_temp == 0) abs_count_macro[i]++;
		}

		

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// dual update
/*	// exhaustive search update
		// dual variable, lambda, mu 업데이트
		for (i = 0; i < MOBILE_NUM; i++)
		{
			double lambda_temp, mu_temp;
			if ((thrp_result[i] / (1 + t) - rate_user[i] >= 0)	//feasilbity
				&& ((thrp_result[i] / (1 + t) - rate_user[i]) * lambda[i] < 0.01))
				lambda_temp = lambda[i] - step_size* (thrpt_macro[i] * resource_macro[i] + thrpt_ABS[i] * resource_ABS[i] + thrpt_nonABS[i] * resource_nonABS[i] - rate_user[i]);
			else
				lambda_temp = lambda[i] - step_size2 * (thrpt_macro[i] * resource_macro[i] + thrpt_ABS[i] * resource_ABS[i] + thrpt_nonABS[i] * resource_nonABS[i] - rate_user[i]);
			lambda[i] = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

			if ((log(rate_user[i]) >= mobile[i].QoS) 
				&& ((log(rate_user[i]) - mobile[i].QoS) * mu[i] < 0.001))
				mu_temp = mu[i] - step_size* (log(rate_user[i]) - mobile[i].QoS);
			else
				mu_temp = mu[i] - step_size2 * (log(rate_user[i]) - mobile[i].QoS);
			mu[i] = (0.0 > mu_temp) ? 0.0 : mu_temp;
		}
*/
	// proposed algorithm update
		// dual variable, lambda, mu 업데이트
		for (i = 0; i < MOBILE_NUM; i++)
		{
			double lambda_temp, mu_temp;
//			if ((thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i] >= 0.0)	//feasilbity
//				&& (abs(thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i]) * lambda[i] < 0.05))
			if ( (abs(thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i]) * lambda[i] < 0.05))
				lambda_temp = lambda[i] - step_size  * (thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i] - rate_user_PA1[i]);
			else
				lambda_temp = lambda[i] - step_size2 * (thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i] - rate_user_PA1[i]);
			lambda[i] = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

		//	if ((log(rate_user_PA1[i]) >= mobile[i].QoS)
		//		&& (abs(log(rate_user_PA1[i]) - mobile[i].QoS) * mu[i] < 0.01))
			if ( (abs(log(rate_user_PA1[i]) - mobile[i].QoS) * mu[i] < 0.01))
				mu_temp = mu[i] - step_size  * (log(rate_user_PA1[i]) - mobile[i].QoS);
			else
				mu_temp = mu[i] - step_size2 * (log(rate_user_PA1[i]) - mobile[i].QoS);
			mu[i] = (0.0 > mu_temp) ? 0.0 : mu_temp;
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////			
		/////////////////////////////////////////////////  값들 출력
		if (t % 1000 == 0) printf("%s", ">");
		
		// PIINTF_TIME 주기로 출력. // 화면, eICIC.txt, lambda.txt, mu.txt 출력
/*		if (t%PRINTF_TIME == 0)
		{
			printf("\n\n");
			printf("\n%s\t%d\n", "simulation time\t", t);
			Savefile << "simulation time\t" << t << std::endl;
			printf("%s\t\t%s\n", "rate Ru", "throughput");

			Savefile << "rate RU\t\t thoroughput" << std::endl;

			double function_result = 0.0;
			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t%f\t%f\t%f\n", rate_user[i], log(rate_user[i]), (thrp_result[i] / (1 + t)), log(thrp_result[i] / (1 + t)));
				Savefile << rate_user[i] << "\t" << log(rate_user[i]) << "\t" << (thrp_result[i] / (1 + t)) << "\t" << log(thrp_result[i] / (1 + t)) << std::endl;
			}
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t", lambda[i]);
				Savefile << lambda[i] << "\t";
				savel << lambda[i] << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";


			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t", mu[i]);
				Savefile << mu[i] << "\t";
				savem << mu[i] << "\t";

			}
			printf("\n");
			Savefile << std::endl;
			savel << std::endl;
			savem << std::endl;

			printf("\n%s\t%s\t%s\n", "macro", "ABS", "non-ABS");
			Savefile << "macro\t" << "ABS\t" << "non-ABS" << std::endl;

			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%d\t%d\t%d\n", resource_macro[i], resource_ABS[i], resource_nonABS[i]);
				Savefile << resource_macro[i] << "\t" << resource_ABS[i] << "\t" << resource_nonABS[i] << std::endl;

			}
			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%d\t%d\t%d\n", num_allocated_macro[i], num_allocated_ABS[i], num_allocated_nonABS[i]);
				Savefile << num_allocated_macro[i] << "\t" << num_allocated_ABS[i] << "\t" << num_allocated_nonABS[i] << std::endl;
			}
			printf("\n");
			for (i = 0; i < MACRO_NUM; i++)
			{
				printf("%d\t", abs_count_macro[i]);
			}

			printf("\n\n");
			Savefile << std::endl << std::endl;


		}*/

		if ((t >=10000) && (t%PRINTF_TIME == 0))
		{
			time(&checktime);
			int timeep = checktime - starttime;
			printf("\n\n");
			printf("\n%s\t%d\n", "simulation time\t", t);
			printf("%d seconds elapsed \n", timeep);
//			Savefile << "simulation time\t" << t << std::endl;
			printf("%s\t\t%s\n", "rate Ru", "throughput");

//			Savefile << "rate RU\t\t thoroughput" << std::endl;

			double function_result = 0.0;
			for (i = 0; i < MOBILE_NUM; i++)
			{
//				printf("%f\t%f\t%f\t%f\n", rate_user_PA1[i], log(rate_user_PA1[i]), (thrp_result_PA1[i] / (1 + t)), log(thrp_result_PA1[i] / (1 + t)));
				printf("%f\t%f\t%f\t%f\t%f\t%f\n", rate_user_PA1[i], (thrp_result_PA1[i] / (1 + t)), log(rate_user_PA1[i]), log(thrp_result_PA1[i] / (1 + t)), lambda[i], mu[i] );
//				Savefile << rate_user[i] << "\t" << log(rate_user[i]) << "\t" << (thrp_result[i] / (1 + t)) << "\t" << log(thrp_result[i] / (1 + t)) << std::endl;
			}
/*			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t", lambda[i]);
				Savefile << lambda[i] << "\t";
				savel << lambda[i] << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";


			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t", mu[i]);
				Savefile << mu[i] << "\t";
				savem << mu[i] << "\t";

			}
			printf("\n");
			Savefile << std::endl;
			savel << std::endl;
			savem << std::endl;

			printf("\n%s\t%s\t%s\n", "macro", "ABS", "non-ABS");
			Savefile << "macro\t" << "ABS\t" << "non-ABS" << std::endl;

/*			for (i = 0; i < MOBILE_NUM; i++)
			{
				printf("%d\t%d\t%d\n", resource_macro_PA1[i], resource_ABS_PA1[i], resource_nonABS_PA1[i]);
//				Savefile << resource_macro[i] << "\t" << resource_ABS[i] << "\t" << resource_nonABS[i] << std::endl;

			}
*/
//			for (i = 0; i < MOBILE_NUM; i++)
//			{
//				printf("%d\t%d\t%d\n", num_allocated_macro[i], num_allocated_ABS[i], num_allocated_nonABS[i]);
//				Savefile << num_allocated_macro[i] << "\t" << num_allocated_ABS[i] << "\t" << num_allocated_nonABS[i] << std::endl;
//			}

			printf("\n");
			for (i = 0; i < MACRO_NUM; i++)
			{
				printf("%d\t", abs_count_macro[i]);
				printf("%f\n", (double)abs_count_macro[i] / ((double)t + 1));
			}

			printf("\n\n");
			Savefile << std::endl << std::endl;

			printf("\n%s\t%d\n", "simulation time\t", t);
			printf("%d seconds elapsed \n", timeep);
			printf("%f\n", QOS);
			printf("\n\n");

			double sum_utility_temp= 0.0;
			for (i = 0; i < MOBILE_NUM; i++)
			{
				sum_utility_temp = sum_utility_temp + log(thrp_result_PA1[i] / (1 + t));
			}
			printf("%s\t%f\n", "sum utility", sum_utility_temp);

		}
		
		// 최종 결과 출력 results.txt 출력
		if (t == (SIMULATION_TIME-1)){
			results << "R E S U L T S" << std::endl;
			results << "Rate, Throughput, Utility of Rate, Utility of Throughput, QoS" << std::endl << std::endl;
			results << "USER\t\t" << "Rate\t\t" << "Thrpt\t\t" << "Util Rate\t" << "Util Thrpt\t" << "QoS" << std::endl;
			for (i = 0; i < MOBILE_NUM; i++)
			{
				results << "USER " << i + 1 << "\t\t" << rate_user[i] << "\t\t" << (thrp_result[i] / (1 + t)) << "\t\t" << log(rate_user[i]) << "\t\t" << log(thrp_result[i] / (1 + t)) << "\t\t" << QOS << std::endl;
			}
		}
	}
	system("pause");
}