#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

#include "parameters.h"
#include "rndfunctions.h"
#include "macro.h"
#include "pico.h"
#include "mobile.h"
#include "eICIC.h"
#include "point.h"

int main()
{
	time_t starttime;
	time(&starttime);
	
	srand(starttime);

	Macro **macros = (Macro **) malloc(sizeof(Macro *) * MACRO_NUM);
	Pico **picos = (Pico **) malloc(sizeof(Pico *) * PICO_NUM);
	Mobile **mobiles = (Mobile **) malloc(sizeof(Mobile *) * MOBILE_NUM);
	initialize(macros, picos, mobiles);

	// mobile--macro
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		int service_macro = -1;
		double service_distance = DBL_MAX;

		for (int j = 0; j < MACRO_NUM; j++)
		{
			double distance = POINT_DISTANCE(mobiles[i]->location, macros[j]->getLocation());
			int is_neighbor = distance < NEIGHBOR_DIST_M;

			// 더 가까운 macro를 찾았을 때
			if (distance < service_distance)
			{
				service_macro = j;
				service_distance = distance;
			}

			mobiles[i]->set_dist_macro(j, distance, macros[j]->getTxPower(), NOISE);
			mobiles[i]->macro_neighbor[j] = is_neighbor;
			macros[j]->mobile[i] = is_neighbor;

		}

		mobiles[i]->set_serviceBS_macro(service_macro);
		macros[mobiles[i]->macro_service]->mobile_service_01[i] = 1;

	}

	// pico-mobile 간 data를 취합하기 위한 pico temp data 초기화
	int pico_num_servicemobile_temp[PICO_NUM];

	int pico_servicemobile_01_temp[MOBILE_NUM][PICO_NUM];

	// pico!
	// pico--mobile
	for (int i = 0; i < PICO_NUM; i++)
	{
		pico_num_servicemobile_temp[i] = 0;

		for (int j = 0; j < MOBILE_NUM; j++) pico_servicemobile_01_temp[j][i] = 0;
	}

	// 모바일 pico 거리, 이웃노드 수, service 설정		
	// pico class에 저장하기 위해 pico 관련 데이터도 위의 temp data를 이용하여 동시에 처리
	// 모두에 대해 0,1로 간섭유무 표현 1이면 간섭. 다 더해야 한다.
	int mobile_num_neighborBS_temp_pico[MOBILE_NUM];
	int mobile_service_pico_temp[MOBILE_NUM];

	// 위치관계에 따른 채널 정립. 이웃 찾기. 거리, 파워 기반 신호 세기.(간섭으로 써도 됨)
	// 채널 계산
	// mobile--pico
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		int neighbor_count = 0;
		int service_pico = -1;
		double service_distance = DBL_MAX;

		for (int j = 0; j < PICO_NUM; j++)
		{
			double distance = POINT_DISTANCE(mobiles[i]->location, picos[j]->location);
			int is_neighbor = distance < NEIGHBOR_DIST_P;

			if (is_neighbor)
				neighbor_count++;

			if (distance < service_distance)
			{
				service_distance = distance;
				service_pico = j;
			}

			mobiles[i]->set_dist_pico_1(j, distance, picos[j]->tx_power, NOISE);
			mobiles[i]->pico_neighbor[j] = is_neighbor;

		}

		pico_num_servicemobile_temp[service_pico]++;
		pico_servicemobile_01_temp[i][service_pico] = 1;

		mobile_num_neighborBS_temp_pico[i]	= neighbor_count;
		mobile_service_pico_temp[i]			= service_pico;
	}

	// macro--pico
	for (int i = 0; i < MACRO_NUM; i++)
	{
		for (int j = 0; j < PICO_NUM; j++)
		{
			picos[j]->distance_macro[i] = POINT_DISTANCE(macros[i]->getLocation(), picos[j]->location);
			picos[j]->macro_neighbor[i] = is_neighbor;
				= picos[j]->distance_macro[i] < MP_INT_DIST;
		}
	}

	// initial setting 각 클래스 초기화, 
	// 각 클래스에 모바일 간섭 기지국 수, 서비스 기지국 저장

	// mobile!
	for (int i = 0; i < MOBILE_NUM; i++)
	{

		mobiles[i]->set_num_int_pico(mobile_num_neighborBS_temp_pico[i]);
		mobiles[i]->set_serviceBS_pico(mobile_service_pico_temp[i]);

	}

	// static 을 위해 cre bias를 통한 cell association
	double cre_bias = pow(10.0, CRE_STATIC / 10.0);
	for (int i = 0; i < MOBILE_NUM; i++) mobiles[i]->cell_association_static(cre_bias);

	// pico 정보 넣기.
	// pico--mobile
	for (int i = 0; i < PICO_NUM; i++)
	{
		// mobile 이웃 정보
		picos[i]->num_service_mobile = pico_num_servicemobile_temp[i];

		for (int j = 0; j < MOBILE_NUM; j++)
		{
			picos[i]->service_mobile_01[j] = pico_servicemobile_01_temp[j][i];
		}
	}

	// pico의 이웃 정보.
	for (int i = 0; i < PICO_NUM; i++) picos[i]->set_neighbor();
	for (int i = 0; i < MACRO_NUM; i++) macros[i]->macro_set_neighbor();

	// 각 모바일이 겪는 interference calculation
	// 모든 기지국에 대한 간섭을 계산. 실제 이용시 자신이 할당받는 기지국의 신호는 제해야 함.
	// mobile!
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		mobiles[i]->set_pico_interference(PICO_NUM);
		mobiles[i]->set_macro_interference(MACRO_NUM);
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

	int abs_count_macro[MACRO_NUM];
	int abs_count_pico[PICO_NUM];

	for (int i=0; i < MACRO_NUM; i++) abs_count_macro[i] = 0;
	for (int i=0; i < PICO_NUM; i++) abs_count_pico[i] = 0;

	// mobile!
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		lambda[i]		= 0.1;
		thrp_result[i]	= 0.0;
		rate_user[i]	= 0.0;

		mu[i]			= 0.0;

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
	for (int i = 0; i < MOBILE_NUM; i++)
	{
		thrp_result_PA1[i]	= 0.0;
		rate_user_PA1[i]	=	0.0;
	}

	////////////////////////////////////////////////////////////////////////////// 알고리즘 따라 연산 //////////////////////////////////////////////////////////////////////

	// 출력 파일들
	std::ofstream Savefile("eICIC.txt");
	std::ofstream savel("lambda.txt");
	std::ofstream savem("mu.txt");
	std::ofstream results("results.txt");
	savel.precision(5);
	savem.precision(5);
	results.precision(5);

	for (int t = 0; t < SIMULATION_TIME; t++)
	{
		double step_size2;
		double step_size = 1.0 / ((double)(t + 1));
		//step_size2 = STEPSIZE2;
		step_size2 = (t < 10000) ? STEPSIZE2 : STEPSIZE3;
		if (t > 100000) step_size2 = STEPSIZE4;

		//step_size2 = step_size;

		double signal_temp;
		double interference_temp;
		for (int i = 0; i < MOBILE_NUM; i++)
		{
			// macro 평균 thrpt 계산
			signal_temp			= mobiles[i]->channel_gain_macro[mobiles[i]->macro_service] *rayleigh() * log_normal();
			interference_temp	= (mobiles[i]->macro_interference + mobiles[i]->pico_interference - mobiles[i]->channel_gain_macro[mobiles[i]->macro_service]) *rayleigh() * log_normal();
			thrpt_macro[i]		= cal_thrpt_i(signal_temp, interference_temp, NOISE) / 1000000.0;
			thrpt_macro[i] = thrpt_macro[i] / 10.0;

			// pico ABS 평균 thrpt 계산
			signal_temp			= mobiles[i]->channel_gain_pico[mobiles[i]->pico_service] *rayleigh() * log_normal();
			interference_temp	= (mobiles[i]->pico_interference - mobiles[i]->channel_gain_pico[mobiles[i]->pico_service]) *rayleigh() * log_normal();
			thrpt_ABS[i]		= cal_thrpt_i(signal_temp, interference_temp, NOISE) / 1000000.0;
			thrpt_ABS[i] = thrpt_ABS[i] / 10.0;

			// pico non-ABS 평균 thrpt 계산
			//signal_temp			= mobiles[i]->channel_gain_pico[mobiles[i]->pico_service] *rayleigh() * log_normal();
			interference_temp	= interference_temp + (mobiles[i]->macro_interference ) *rayleigh() * log_normal();
			thrpt_nonABS[i]		= cal_thrpt_i(signal_temp, interference_temp, NOISE) / 1000000.0;
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

		double object_value_es = 0.0;
		double sum_rate_es		= 0.0;

		///////////////////////////////////////////////// 제안하는 알고리즘을 바탕으로 솔루션 찾기.
		// 변수 선언
		int macro_user_PA[MACRO_NUM];
		int macro_cover_pico_PA[MACRO_NUM];
		int pico_ABS_user_PA[PICO_NUM];
		int pico_ABS_user_PA2[PICO_NUM];
		int pico_nA_user1_PA[PICO_NUM];
		int pico_nA_user2_PA[PICO_NUM];
		int pico_nA_01_PA[PICO_NUM]; // non-ABS 상황에서 몇번째 유저 할당되는지 indicator. 0 first, 1 second

		for (int i = 0; i < PICO_NUM; i++) pico_nA_user2_PA[i] = -1;

		// 각 pico 에서 ABS best user, non-ABS first.second user 선택
		for (int i = 0; i < PICO_NUM; i++)
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

			for (int j = 0; j < picos[i]->num_service_mobile; j++)
			{
				// ABS best user 찾기
				if (lambda[picos[i]->service_mobile[j]] * thrpt_ABS[picos[i]->service_mobile[j]] > temp_pico_ABS_PA)
				{
					temp_pico_ABS_PA2 = temp_pico_ABS_PA;
					temp_pico_ABS_PA_user2 = temp_pico_ABS_PA_user;

					temp_pico_ABS_PA		= lambda[picos[i]->service_mobile[j]] * thrpt_ABS[picos[i]->service_mobile[j]];
					temp_pico_ABS_PA_user	= picos[i]->service_mobile[j];
				} 
				else // non-ABS second 찾기
				{
					if (lambda[picos[i]->service_mobile[j]] * thrpt_ABS[picos[i]->service_mobile[j]] > temp_pico_ABS_PA2)
					{
						temp_pico_ABS_PA2 = lambda[picos[i]->service_mobile[j]] * thrpt_ABS[picos[i]->service_mobile[j]];
						temp_pico_ABS_PA_user2 = picos[i]->service_mobile[j];
					}
				}

				// non-ABS first 찾기
				if (lambda[picos[i]->service_mobile[j]] * thrpt_nonABS[picos[i]->service_mobile[j]] > temp_pico_nA_PA1)
				{
					//if (temp_pico_nA_PA1 >= temp_pico_nA_PA2)
					//{
					temp_pico_nA_PA2		= temp_pico_nA_PA1;
					temp_pico_nA_PA2_user	= temp_pico_nA_PA1_user; 
					//}
						
					temp_pico_nA_PA1		= lambda[picos[i]->service_mobile[j]] * thrpt_nonABS[picos[i]->service_mobile[j]];
					temp_pico_nA_PA1_user	= picos[i]->service_mobile[j];
				}
				else // non-ABS second 찾기
				{
					if (lambda[picos[i]->service_mobile[j]] * thrpt_nonABS[picos[i]->service_mobile[j]] > temp_pico_nA_PA2)
					{
						temp_pico_nA_PA2		= lambda[picos[i]->service_mobile[j]] * thrpt_nonABS[picos[i]->service_mobile[j]];
						temp_pico_nA_PA2_user	= picos[i]->service_mobile[j];
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
		for (int i = 0; i < MACRO_NUM; i++)
		{
			int temp_macro_PA_user = -1;
			double temp_macro_PA = -1000.0;
			for (int j = 0; j < macros[i]->num_mobile; j++)
			{
				if (macros[i]->mobile_service[j] == pico_nA_user1_PA[mobiles[macros[i]->mobile_service[j]]->pico_service])
				{
					int user_temp_temp = pico_nA_user2_PA[mobiles[macros[i]->mobile_service[j]]->pico_service]; // second user num
					double temp_temp;
					if (user_temp_temp != -1) temp_temp = (lambda[macros[i]->mobile_service[j]] * thrpt_macro[macros[i]->mobile_service[j]] - lambda[macros[i]->mobile_service[j]] * thrpt_nonABS[macros[i]->mobile_service[j]] + lambda[user_temp_temp] * thrpt_nonABS[user_temp_temp]);
					else temp_temp = (lambda[macros[i]->mobile_service[j]] * thrpt_macro[macros[i]->mobile_service[j]] - lambda[macros[i]->mobile_service[j]] * thrpt_nonABS[macros[i]->mobile_service[j]]);

					if (temp_temp > temp_macro_PA)
					{
						temp_macro_PA		= temp_temp;
						temp_macro_PA_user	= macros[i]->mobile_service[j];
					}
				}
				else
				{
					if (lambda[macros[i]->mobile_service[j]] * thrpt_macro[macros[i]->mobile_service[j]] > temp_macro_PA)
					{
						temp_macro_PA		= lambda[macros[i]->mobile_service[j]] * thrpt_macro[macros[i]->mobile_service[j]];
						temp_macro_PA_user	= macros[i]->mobile_service[j];
					}
				}
			}
			macro_user_PA[i]		= temp_macro_PA_user;
			macro_cover_pico_PA[i]	= mobiles[temp_macro_PA_user]->pico_service;
			pico_nA_01_PA[mobiles[temp_macro_PA_user]->pico_service] = 1;
		}

		for (int i = 0; i < MACRO_NUM; i++) macros[i]->set_user_PA1(macro_user_PA[i], macro_cover_pico_PA[i]);
		for (int i = 0; i < PICO_NUM; i++) picos[i]->set_user_PA1(pico_ABS_user_PA[i], pico_ABS_user_PA2[i], pico_nA_user1_PA[i], pico_nA_user2_PA[i], pico_nA_01_PA[i]);

		int _macro_num = 0;
		double objective_value_best_PA1 = -1.0;
		int state_temp_PA1[MACRO_NUM];
		int state_best_PA1[MACRO_NUM];
		int user_state_best_PA1[MOBILE_NUM];
		
		for (int i = 0; i < MACRO_NUM; i++)
		{
			state_temp_PA1[i] = 0;
			state_best_PA1[i] = 0;
		}
		for (int i = 0; i < MOBILE_NUM; i++) user_state_best_PA1[i] = 0;

		// macro 들의 모든 조합에 대하여 최적의 값 선택
		PA1_call_next_pico(_macro_num, &objective_value_best_PA1, state_temp_PA1, state_best_PA1, user_state_best_PA1, mobiles, picos, macros, lambda, thrpt_macro, thrpt_ABS, thrpt_nonABS);

		// resource 정보 입력
		// best state 즉, 구한 자원 할당 값 입력
		for (int i = 0; i < MOBILE_NUM; i++)
		{
			if (user_state_best_PA1[i] == 1)	resource_macro_PA1[i]		= 1;
			else if (user_state_best_PA1[i] == 2) resource_ABS_PA1[i]	= 1;
			else if ( (user_state_best_PA1[i] == 3) || (user_state_best_PA1[i] == 4)) resource_nonABS_PA1[i]	= 1;
		}

		// 현재까지 얻은 throughput 입력
		for (int i = 0; i < MOBILE_NUM; i++) thrp_result_PA1[i] = thrp_result_PA1[i] + thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i];

		// 평균 rate Ru, rate_user[i], 업데이트
		for (int i = 0; i < MOBILE_NUM; i++)
		{
			if (lambda[i] == 0.0) rate_user_PA1[i] = RATE_MAX;
						//else rate_user[i] = 0.8* rate_user[i] + 0.2 * (1.0 + mu[i]) / lambda[i];
			else rate_user_PA1[i] = 0.8 * rate_user_PA1[i] + 0.2 * (1.0 + mu[i]) / lambda[i];
		}

		// 각 유저 어느 기지국 사용했는지 count
		for (int i = 0; i < MOBILE_NUM; i++)
		{
			if (user_state_best_PA1[i] == 1)	num_allocated_macro[i]			= num_allocated_macro[i] + 1;
			else if (user_state_best_PA1[i] == 2) num_allocated_ABS[i]			= num_allocated_ABS[i] + 1;
			else if ((user_state_best_PA1[i] == 3) || (user_state_best_PA1[i] == 4)) num_allocated_nonABS[i]	= num_allocated_nonABS[i] + 1;
		}

		// 각 기지국별 자원 사용했는지 여부 count // 사용한 유저가 없을경우 해당 기지국은 ABS. abs_count 증가
		for (int i=0; i < MACRO_NUM; i++)
		{
			int resource_used_temp = 0; // 자원 할당 여부. 1이면 사용, 0이면 사용 안함
			for (int j = 0; j < macros[i]->num_mobile; j++)
			{
				if (resource_macro_PA1[macros[i]->mobile_service[j]] == 1) resource_used_temp = 1;
			}
			if (resource_used_temp == 0) abs_count_macro[i]++;
		}

		

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// dual update
		// proposed algorithm update
		// dual variable, lambda, mu 업데이트
		for (int i = 0; i < MOBILE_NUM; i++)
		{
			double lambda_temp, mu_temp;
			//if ((thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i] >= 0.0)	//feasilbity
				//&& (abs(thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i]) * lambda[i] < 0.05))
			if ( (abs(thrp_result_PA1[i] / (1 + t) - rate_user_PA1[i]) * lambda[i] < 0.05))
				lambda_temp = lambda[i] - step_size * (thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i] - rate_user_PA1[i]);
			else
				lambda_temp = lambda[i] - step_size2 * (thrpt_macro[i] * resource_macro_PA1[i] + thrpt_ABS[i] * resource_ABS_PA1[i] + thrpt_nonABS[i] * resource_nonABS_PA1[i] - rate_user_PA1[i]);
			lambda[i] = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

			//if ((log(rate_user_PA1[i]) >= mobiles[i]->QoS)
				//&& (abs(log(rate_user_PA1[i]) - mobiles[i]->QoS) * mu[i] < 0.01))
			if ( (abs(log(rate_user_PA1[i]) - mobiles[i]->QoS) * mu[i] < 0.01))
				mu_temp = mu[i] - step_size * (log(rate_user_PA1[i]) - mobiles[i]->QoS);
			else
				mu_temp = mu[i] - step_size2 * (log(rate_user_PA1[i]) - mobiles[i]->QoS);
			mu[i] = (0.0 > mu_temp) ? 0.0 : mu_temp;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////// 값들 출력
		if (t % 1000 == 0) printf("%s", ">");
		
		// PIINTF_TIME 주기로 출력. // 화면, eICIC.txt, lambda.txt, mu.txt 출력
		/*
		if (t%PRINTF_TIME == 0)
		{
			printf("\n\n");
			printf("\n%s\t%d\n", "simulation time\t", t);
			Savefile << "simulation time\t" << t << std::endl;
			printf("%s\t\t%s\n", "rate Ru", "throughput");

			Savefile << "rate RU\t\t thoroughput" << std::endl;

			double function_result = 0.0;
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t%f\t%f\t%f\n", rate_user[i], log(rate_user[i]), (thrp_result[i] / (1 + t)), log(thrp_result[i] / (1 + t)));
				Savefile << rate_user[i] << "\t" << log(rate_user[i]) << "\t" << (thrp_result[i] / (1 + t)) << "\t" << log(thrp_result[i] / (1 + t)) << std::endl;
			}
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t", lambda[i]);
				Savefile << lambda[i] << "\t";
				savel << lambda[i] << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";

			for (int i = 0; i < MOBILE_NUM; i++)
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

			for (int i = 0; i < MOBILE_NUM; i++)
			{
				printf("%d\t%d\t%d\n", resource_macro[i], resource_ABS[i], resource_nonABS[i]);
				Savefile << resource_macro[i] << "\t" << resource_ABS[i] << "\t" << resource_nonABS[i] << std::endl;

			}
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				printf("%d\t%d\t%d\n", num_allocated_macro[i], num_allocated_ABS[i], num_allocated_nonABS[i]);
				Savefile << num_allocated_macro[i] << "\t" << num_allocated_ABS[i] << "\t" << num_allocated_nonABS[i] << std::endl;
			}
			printf("\n");
			for (int i = 0; i < MACRO_NUM; i++)
			{
				printf("%d\t", abs_count_macro[i]);
			}

			printf("\n\n");
			Savefile << std::endl << std::endl;

		}*/

		if ((t >=10000) && (t%PRINTF_TIME == 0))
		{
			time_t checktime;
			time(&checktime);
			int timeep = checktime - starttime;
			printf("\n\n");
			printf("\n%s\t%d\n", "simulation time\t", t);
			printf("%d seconds elapsed \n", timeep);
			//Savefile << "simulation time\t" << t << std::endl;
			printf("%s\t\t%s\n", "rate Ru", "throughput");

			//Savefile << "rate RU\t\t thoroughput" << std::endl;

			double function_result = 0.0;
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				//printf("%f\t%f\t%f\t%f\n", rate_user_PA1[i], log(rate_user_PA1[i]), (thrp_result_PA1[i] / (1 + t)), log(thrp_result_PA1[i] / (1 + t)));
				printf("%f\t%f\t%f\t%f\t%f\t%f\n", rate_user_PA1[i], (thrp_result_PA1[i] / (1 + t)), log(rate_user_PA1[i]), log(thrp_result_PA1[i] / (1 + t)), lambda[i], mu[i] );
				//Savefile << rate_user[i] << "\t" << log(rate_user[i]) << "\t" << (thrp_result[i] / (1 + t)) << "\t" << log(thrp_result[i] / (1 + t)) << std::endl;
			}
			/*
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				printf("%f\t", lambda[i]);
				Savefile << lambda[i] << "\t";
				savel << lambda[i] << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";

			for (int i = 0; i < MOBILE_NUM; i++)
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

			/*
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				printf("%d\t%d\t%d\n", resource_macro_PA1[i], resource_ABS_PA1[i], resource_nonABS_PA1[i]);
				//Savefile << resource_macro[i] << "\t" << resource_ABS[i] << "\t" << resource_nonABS[i] << std::endl;

			}
			*/
			//for (int i = 0; i < MOBILE_NUM; i++)
			//{
				//printf("%d\t%d\t%d\n", num_allocated_macro[i], num_allocated_ABS[i], num_allocated_nonABS[i]);
				//Savefile << num_allocated_macro[i] << "\t" << num_allocated_ABS[i] << "\t" << num_allocated_nonABS[i] << std::endl;
			//}

			printf("\n");
			for (int i = 0; i < MACRO_NUM; i++)
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
			for (int i = 0; i < MOBILE_NUM; i++)
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
			for (int i = 0; i < MOBILE_NUM; i++)
			{
				results << "USER " << i + 1 << "\t\t" << rate_user[i] << "\t\t" << (thrp_result[i] / (1 + t)) << "\t\t" << log(rate_user[i]) << "\t\t" << log(thrp_result[i] / (1 + t)) << "\t\t" << QOS << std::endl;
			}
		}
	}
	system("pause");

	for (int i = 0; i < MACRO_NUM; i++)
		delete macros[i];
	free(macros);

	for (int i = 0; i < PICO_NUM; i++)
		delete picos[i];
	free(picos);

	for (int i = 0; i < MOBILE_NUM; i++)
		delete mobiles[i];
	free(mobiles);

	return 0;
}

void initialize(Macro **macros, Pico **picos, Mobile **mobiles)
{

	// 각 노드의 위치 직접 지정, 위치를 지정해줄 경우 아래에서 직접 입력, 직접 입력할 경우 parameter.h의 LOC_SETUP = 1 로 설정
	if (LOC_SETUP == 1)
	{
		//double bias_x = 1000;
		//double bias_y = 500;
		macros[0] = new Macro({ 0.0, 0.0 }, MACRO_TX_POWER);
		macros[1] = new Macro({ 1000.0, 0.0 }, MACRO_TX_POWER);
		macros[2] = new Macro({ -1000.0, 0.0 }, MACRO_TX_POWER);
		macros[3] = new Macro({ 500.0, 866.0 }, MACRO_TX_POWER);
		macros[4] = new Macro({ 500.0, -866.0 }, MACRO_TX_POWER);
		macros[5] = new Macro({ -500.0, 866.0 }, MACRO_TX_POWER);
		macros[6] = new Macro({ -500.0, -866.0 }, MACRO_TX_POWER);

		std::ifstream pico_loc("pico.txt");
		for (int i = 0; i < PICO_NUM; i++)
		{
			point location;
			pico_loc >> location.x >> location.y;
			picos[i] = new Pico(location, PICO_TX_POWER);
		}

		//std::ifstream node("node.txt");
		std::ifstream node("node_1.txt");
		for (int i = 0; i < MOBILE_NUM; i++)
		{
			point location;
			node >> location.x >> location.y;
			mobiles[i] = new Mobile(location, QOS);
		}

	}
	else
	{

		for (int i = 0; i < MACRO_NUM; i++)
		{
			double radius	= uniform() * AREA_RADIUS;
			double angle	= uniform() * 2 * PI;
			macros[i] = new Macro({
				radius * cos(angle),
				radius * sin(angle)
			}, MACRO_TX_POWER);
		}

		for (int i = 0; i < PICO_NUM; i++)
		{
			double radius	= uniform() * AREA_RADIUS;
			double angle	= uniform() * 2 * PI;
			picos[i] = new Pico({
				radius * cos(angle),
				radius * sin(angle)
			}, PICO_TX_POWER);
		}

		for (int i = 0; i < MOBILE_NUM; i++)
		{
			double radius	= uniform() * AREA_RADIUS;
			double angle	= uniform() * 2 * PI;
			mobiles[i] = new Mobile({
				radius * cos(angle),
				radius * sin(angle)
			}, QOS);
		}

	}

}


double cal_thrpt_s(double _sinr, double _BW)
{
	double throughput = _BW * log(1 + _sinr);
	return throughput;
}

double cal_thrpt_i(double _channel_gain, double _interference, double _no )
{
	double throughput = BW * log(1 + (_channel_gain / (_interference + _no )));
	return throughput;
}

void PA1_calculation(int _macro_num, double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, Mobile **mobiles, Pico **picos, Macro **macros, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS)
{
	int i;
	double _objective_temp = 0.0;
	int _user_state_temp[MOBILE_NUM];
	for (i = 0; i < MOBILE_NUM; i++) _user_state_temp[i] = 0;

	// macro 값 계산
	for (i = 0; i < MACRO_NUM; i++)
	{
		if (_state_temp[i] == 1)
		{
			_objective_temp = _objective_temp + _lambda[macros[i]->selected_user_PA1] * _thrpt_macro[macros[i]->selected_user_PA1];
			_user_state_temp[macros[i]->selected_user_PA1] = 1;
		}
	}
	// pico 값 계산
	for (i = 0; i < PICO_NUM; i++)
	{
		int ABS_indicator = 0; // 0 ABS, 1 non-ABS, 2 non-ABS & second user

		for (int j = 0; j < MACRO_NUM; j++)
		{
			if ((picos[i]->macro_neighbor[j] == 1) && (_state_temp[j] == 1))
			{
				ABS_indicator = 1;
			}
		}

		if (ABS_indicator == 1 && picos[i]->nA_user1_PA1 != -1 && macros[mobiles[picos[i]->nA_user1_PA1]->macro_service]->selected_user_PA1 == picos[i]->nA_user1_PA1) ABS_indicator = 2;

		if (ABS_indicator == 0)
		{
			if (picos[i]->ABS_user_PA1 != -1)
			{
				if (_state_temp[mobiles[picos[i]->ABS_user_PA1]->macro_service] == 0 || (_state_temp[mobiles[picos[i]->ABS_user_PA1]->macro_service] == 1 && macros[mobiles[picos[i]->ABS_user_PA1]->macro_service]->selected_user_PA1 != picos[i]->ABS_user_PA1))
				{
					_objective_temp = _objective_temp + _lambda[picos[i]->ABS_user_PA1] * _thrpt_ABS[picos[i]->ABS_user_PA1];
					_user_state_temp[picos[i]->ABS_user_PA1] = 2;
				}
				else if (picos[i]->ABS_user2_PA1 != -1)
				{
					if (_state_temp[mobiles[picos[i]->ABS_user2_PA1]->macro_service] == 0 || (_state_temp[mobiles[picos[i]->ABS_user2_PA1]->macro_service] == 1 && macros[mobiles[picos[i]->ABS_user2_PA1]->macro_service]->selected_user_PA1 != picos[i]->ABS_user2_PA1))
					{
						_objective_temp = _objective_temp + _lambda[picos[i]->ABS_user2_PA1] * _thrpt_ABS[picos[i]->ABS_user2_PA1];
						_user_state_temp[picos[i]->ABS_user2_PA1] = 2;
					}
				}

			}


		}
		else if (ABS_indicator == 1)
		{
			if (picos[i]->nA_user1_PA1 != -1)
			{
				_objective_temp = _objective_temp + _lambda[picos[i]->nA_user1_PA1] * _thrpt_nonABS[picos[i]->nA_user1_PA1];
				_user_state_temp[picos[i]->nA_user1_PA1] = 3;
			}
		}
		else
		{
			if ((picos[i]->nA_user2_PA1 != -1) && (_user_state_temp[picos[i]->nA_user2_PA1] != 1))
			{
				_objective_temp = _objective_temp + _lambda[picos[i]->nA_user2_PA1] * _thrpt_nonABS[picos[i]->nA_user2_PA1];
				_user_state_temp[picos[i]->nA_user2_PA1] = 4;
			}
		}
	}
	if (_objective_temp > *_best_value)
	{
		*_best_value = _objective_temp;
		for (i = 0; i < MACRO_NUM; i++) _state_best[i] = _state_temp[i];
		for (i = 0; i < MOBILE_NUM; i++) _user_state_best[i] = _user_state_temp[i];
	}

	//_state_temp 초기화
	// for (i = 0; i < MACRO_NUM; i++) _state_temp[i] = 0;
}

void PA1_call_next_pico(int _macro_num, double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, Mobile **mobiles, Pico **picos, Macro **macros, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS)
{
	for (int i = 0; i < 2; i++)
	{
		_state_temp[_macro_num] = i;
		if ((_macro_num + 1) < MACRO_NUM) PA1_call_next_pico((_macro_num + 1), _best_value, _state_temp, _state_best, _user_state_best, mobiles, picos, macros, _lambda, _thrpt_macro, _thrpt_ABS, _thrpt_nonABS);
		else PA1_calculation((_macro_num + 1), _best_value, _state_temp, _state_best, _user_state_best, mobiles, picos, macros, _lambda, _thrpt_macro, _thrpt_ABS, _thrpt_nonABS);
	}
}
