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

	// /////////////////////////////////////////////////////////////////////////
	// 초기화

	Macro **macros = (Macro **) malloc(sizeof(Macro *) * MACRO_NUM);
	Pico **picos = (Pico **) malloc(sizeof(Pico *) * PICO_NUM);
	Mobile **mobiles = (Mobile **) malloc(sizeof(Mobile *) * MOBILE_NUM);
	initialize(macros, picos, mobiles);

	// /////////////////////////////////////////////////////////////////////////

	// TODO 시간 반복문 안으로 이동해야 함
	// 각 모바일이 겪는 interference calculation
	// 모든 기지국에 대한 간섭을 계산. 실제 이용시 자신이 할당받는 기지국의 신호는 제해야 함.
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		mobiles[mob]->set_pico_interference(PICO_NUM);
		mobiles[mob]->set_macro_interference(MACRO_NUM);
	}

	// /////////////////////////////////////////////////////////////////////////
	// 시뮬레이션에서 필요한 각 변수들 선언

	int abs_count_macro[MACRO_NUM];
	for (int mac = 0; mac < MACRO_NUM; mac++)
		abs_count_macro[mac] = 0;

	// /////////////////////////////////
	// 출력 파일
	std::ofstream Savefile("eICIC.txt");

	std::ofstream savel("lambda.txt");
	savel.precision(5);

	std::ofstream savem("mu.txt");
	savem.precision(5);

	std::ofstream results("results.txt");
	results.precision(5);

	// /////////////////////////////////////////////////////////////////////////
	// 알고리즘 따라 연산

	for (int t = 0; t < SIMULATION_TIME; t++)
	{
		const double STEP_SIZE = 1.0 / ((double)(t + 1));
		const double STEP_SIZE2
			= (t > 100000)
			? STEPSIZE4
			:	( (t < 10000)
				? STEPSIZE2
				: STEPSIZE3
				)
		;
		//STEP_SIZE2 = STEP_SIZE;

		// TODO 컨텍스트로 분리

		// /////////////////////////////////////////////////////////////////////
		// 채널 값 계산
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			mobiles[mob]->generate_channel_gain();
		}

		// /////////////////////////////////////////////////////////////////////
		// 제안하는 알고리즘을 바탕으로 솔루션 찾기.

		// /////////////////////////////////////////////////
		// 각 pico 에서 ABS best user, non-ABS first.second user 선택
		for (int pic = 0; pic < PICO_NUM; pic++)
		{
			picos[pic]->select_users(mobiles);
		}

		// /////////////////////////////////////////////////
		// 각 macro 유저 찾기
		// mobile 번호: macro[mac].mobile_service[j]
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			macros[mac]->select_users(mobiles, picos);
		}

		// /////////////////////////////////////////////////////////////////////

		int user_state_best_PA1[MOBILE_NUM];
		calculate(user_state_best_PA1, macros, picos, mobiles);

		// TODO 컨텍스트로 분리
		// PA1
		int resource_macro_PA1[MOBILE_NUM];
		int resource_ABS_PA1[MOBILE_NUM];
		int resource_nonABS_PA1[MOBILE_NUM];

		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			Mobile *mobile = mobiles[mob];

			// /////////////////////////////////////////////
			// resource 정보 입력
			// best state 즉, 구한 자원 할당 값 입력
			switch (user_state_best_PA1[mob])
			{
				case 0:
					resource_macro_PA1[mob]  = 0;
					resource_ABS_PA1[mob]    = 0;
					resource_nonABS_PA1[mob] = 0;
					break;
				case 1:
					resource_macro_PA1[mob]  = 1;
					resource_ABS_PA1[mob]    = 0;
					resource_nonABS_PA1[mob] = 0;
					break;
				case 2:
					resource_macro_PA1[mob]  = 0;
					resource_ABS_PA1[mob]    = 1;
					resource_nonABS_PA1[mob] = 0;
					break;
				case 3:
				case 4:
					resource_macro_PA1[mob]  = 0;
					resource_ABS_PA1[mob]    = 0;
					resource_nonABS_PA1[mob] = 1;
					break;
			}

			// /////////////////////////////////////////////
			// 각 유저 어느 기지국 사용했는지 count
			switch (user_state_best_PA1[mob])
			{
				case 1:
					mobile->increase_allocated_macro_count();
					break;
				case 2:
					mobile->increase_allocated_ABS_count();
					break;
				case 3:
				case 4:
					mobile->increase_allocated_nonABS_count();
					break;
			}

			// /////////////////////////////////////////////
			// 현재까지 얻은 throughput 입력
			mobile->thrp_result_PA1
				= mobile->thrp_result_PA1
				+ mobile->thrpt_macro  * resource_macro_PA1[mob]
				+ mobile->thrpt_ABS    * resource_ABS_PA1[mob]
				+ mobile->thrpt_nonABS * resource_nonABS_PA1[mob]
			;

			// /////////////////////////////////////////////

		}

		// /////////////////////////////////////////////////////////////////////

		// 평균 rate Ru, mobiles[mob]->rate_user_PA1, 업데이트
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			Mobile *mobile = mobiles[mob];

			if (mobile->lambda == 0.0)
				mobile->rate_user_PA1 = RATE_MAX;
			else
				//mobile->rate_user_PA1 = 0.8* mobile->rate_user_PA1 + 0.2 * (1.0 + mobile->mu) / mobile->lambda;
				mobile->rate_user_PA1
					= 0.8 * mobile->rate_user_PA1
					+ 0.2 * (1.0 + mobile->mu) / mobile->lambda
				;
		}

		// 각 기지국별 자원 사용했는지 여부 count // 사용한 유저가 없을경우 해당 기지국은 ABS. abs_count 증가
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			int is_used_resource = 0; // 자원 할당 여부. 1이면 사용, 0이면 사용 안함
			for (int j = 0; j < macros[mac]->getMobileCount(); j++)
			{
				if (resource_macro_PA1[macros[mac]->mobile_service[j]] == 1) is_used_resource = 1;
			}
			if (is_used_resource == 0) abs_count_macro[mac]++;
		}

		// /////////////////////////////////////////////////////////////////////
		// dual update

		// proposed algorithm update
		// dual variable, lambda, mu 업데이트
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			Mobile *mobile = mobiles[mob];

			double lambda_temp, mu_temp;
			//if (     (mobile->thrp_result_PA1 / (1 + t) - mobile->rate_user_PA1 >= 0.0)  //feasilbity
			//	&& (abs(mobile->thrp_result_PA1 / (1 + t) - mobile->rate_user_PA1) * mobile->lambda < 0.05))
			if ( (abs(mobile->thrp_result_PA1 / (1 + t) - mobile->rate_user_PA1) * mobile->lambda < 0.05))
				lambda_temp = mobile->lambda - STEP_SIZE * (mobile->thrpt_macro * resource_macro_PA1[mob] + mobile->thrpt_ABS * resource_ABS_PA1[mob] + mobile->thrpt_nonABS * resource_nonABS_PA1[mob] - mobile->rate_user_PA1);
			else
				lambda_temp = mobile->lambda - STEP_SIZE2 * (mobile->thrpt_macro * resource_macro_PA1[mob] + mobile->thrpt_ABS * resource_ABS_PA1[mob] + mobile->thrpt_nonABS * resource_nonABS_PA1[mob] - mobile->rate_user_PA1);
			mobile->lambda = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

			//if ((log(mobile->rate_user_PA1) >= mobile->QoS)
				//&& (abs(log(mobile->rate_user_PA1) - mobile->QoS) * mobile->mu < 0.01))
			if ( (abs(log(mobile->rate_user_PA1) - mobile->QoS) * mobile->mu < 0.01))
				mu_temp = mobile->mu - STEP_SIZE * (log(mobile->rate_user_PA1) - mobile->QoS);
			else
				mu_temp = mobile->mu - STEP_SIZE2 * (log(mobile->rate_user_PA1) - mobile->QoS);
			mobile->mu = (0.0 > mu_temp) ? 0.0 : mu_temp;
		}

		// /////////////////////////////////////////////////////////////////////
		// 값들 출력

		if (t % 1000 == 0)
			printf("%s", ">");

		// PIINTF_TIME 주기로 출력. // 화면, eICIC.txt, lambda.txt, mu.txt 출력
		/*
		if (t % PRINTF_TIME == 0)
		{
			printf("\n\n");
			printf("\n%s\t%d\n", "simulation time\t", t);
			Savefile << "simulation time\t" << t << std::endl;
			printf("%s\t\t%s\n", "rate Ru", "throughput");

			Savefile << "rate RU\t\t thoroughput" << std::endl;

			double function_result = 0.0;
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t%f\t%f\t%f\n", mobiles[mob]->rate_user_PA1, log(mobiles[mob]->rate_user_PA1), (mobiles[mob]->thrp_result_PA1 / (1 + t)), log(mobiles[mob]->thrp_result_PA1 / (1 + t)));
				Savefile << mobiles[mob]->rate_user_PA1 << "\t" << log(mobiles[mob]->rate_user_PA1) << "\t" << (mobiles[mob]->thrp_result_PA1 / (1 + t)) << "\t" << log(mobiles[mob]->thrp_result_PA1 / (1 + t)) << std::endl;
			}
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", mobiles[mob]->lambda);
				Savefile << mobile[mob]->lambda << "\t";
				savel << mobiles[mob]->lambda << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";

			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", mobiles[mob]->mu);
				Savefile << mobiles[mob]->mu << "\t";
				savem << mobiles[mob]->mu << "\t";

			}
			printf("\n");
			Savefile << std::endl;
			savel << std::endl;
			savem << std::endl;

			printf("\n%s\t%s\t%s\n", "macro", "ABS", "non-ABS");
			Savefile << "macro\t" << "ABS\t" << "non-ABS" << std::endl;

			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%d\t%d\t%d\n", resource_macro[mob], resource_ABS[mob], resource_nonABS[mob]);
				Savefile << resource_macro[mob] << "\t" << resource_ABS[mob] << "\t" << resource_nonABS[mob] << std::endl;

			}
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%d\t%d\t%d\n", mobiles[mob]->get_allocated_macro_count(), mobiles[mob]->get_allocated_ABS_count(), mobiles[mob]->get_allocated_nonABS_count());
				Savefile << mobiles[mob]->get_allocated_macro_count() << "\t" << mobiles[mob]->get_allocated_ABS_count() << "\t" << mobiles[mob]->get_allocated_nonABS_count() << std::endl;
			}
			printf("\n");
			for (int mac = 0; mac < MACRO_NUM; mac++)
			{
				printf("%d\t", abs_count_macro[mac]);
			}

			printf("\n\n");
			Savefile << std::endl << std::endl;

		}
		//*/

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
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				//printf("%f\t%f\t%f\t%f\n", mobiles[mob]->rate_user_PA1, log(mobiles[mob]->rate_user_PA1), (mobiles[mob]->thrp_result_PA1 / (1 + t)), log(mobiles[mob]->thrp_result_PA1 / (1 + t)));
				printf("%f\t%f\t%f\t%f\t%f\t%f\n",
					mobiles[mob]->rate_user_PA1,
					(mobiles[mob]->thrp_result_PA1 / (1 + t)),
					log(mobiles[mob]->rate_user_PA1),
					log(mobiles[mob]->thrp_result_PA1 / (1 + t)),
					mobiles[mob]->lambda,
					mobiles[mob]->mu
				);
				//Savefile << mobiles[mob]->rate_user_PA1 << "\t" << log(mobiles[mob]->rate_user_PA1) << "\t" << (mobiles[mob]->thrp_result_PA1 / (1 + t)) << "\t" << log(mobiles[mob]->thrp_result_PA1 / (1 + t)) << std::endl;
			}
			/*
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", mobiles[mob]->lambda);
				Savefile << mobiles[mob]->lambda_temp << "\t";
				savel << mobiles[mob]->lambda << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";

			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", mobiles[mob]->mu);
				Savefile << mobiles[mob]->mu << "\t";
				savem << mobiles[mob]->mu << "\t";

			}
			printf("\n");
			Savefile << std::endl;
			savel << std::endl;
			savem << std::endl;

			printf("\n%s\t%s\t%s\n", "macro", "ABS", "non-ABS");
			Savefile << "macro\t" << "ABS\t" << "non-ABS" << std::endl;

			/*
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%d\t%d\t%d\n", resource_macro_PA1[mob], resource_ABS_PA1[mob], resource_nonABS_PA1[mob]);
				//Savefile << resource_macro[mob] << "\t" << resource_ABS[mob] << "\t" << resource_nonABS[mob] << std::endl;

			}
			*/
			//for (int mob = 0; mob < MOBILE_NUM; mob++)
			//{
				//printf("%d\t%d\t%d\n", mobiles[mob]->get_allocated_macro_count(), mobiles[mob]->get_allocated_ABS_count(), mobiles[mob]->get_allocated_nonABS_count());
				//Savefile << mobiles[mob]->get_allocated_macro_count() << "\t" << mobiles[mob]->get_allocated_ABS_count() << "\t" << mobiles[mob]->get_allocated_nonABS_count() << std::endl;
			//}

			printf("\n");
			for (int mac = 0; mac < MACRO_NUM; mac++)
			{
				printf("%d\t", abs_count_macro[mac]);
				printf("%f\n", (double)abs_count_macro[mac] / ((double) t + 1));
			}

			printf("\n\n");
			Savefile << std::endl << std::endl;

			printf("\n%s\t%d\n", "simulation time\t", t);
			printf("%d seconds elapsed \n", timeep);
			printf("%f\n", QOS);
			printf("\n\n");

			double sum_utility = 0.0;
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				sum_utility = sum_utility + log(mobiles[mob]->thrp_result_PA1 / (1 + t));
			}
			printf("%s\t%f\n", "sum utility", sum_utility);

		}

		// 최종 결과 출력 results.txt 출력
		if (t == (SIMULATION_TIME-1)){
			results << "R E S U L T S" << std::endl;
			results << "Rate, Throughput, Utility of Rate, Utility of Throughput, QoS" << std::endl << std::endl;
			results << "USER\t\t" << "Rate\t\t" << "Thrpt\t\t" << "Util Rate\t" << "Util Thrpt\t" << "QoS" << std::endl;
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				results << "USER " << mob + 1 << "\t\t" << mobiles[mob]->rate_user_PA1 << "\t\t" << (mobiles[mob]->thrp_result_PA1 / (1 + t)) << "\t\t" << log(mobiles[mob]->rate_user_PA1) << "\t\t" << log(mobiles[mob]->thrp_result_PA1 / (1 + t)) << "\t\t" << QOS << std::endl;
			}
		}

	}

	for (int mac = 0; mac < MACRO_NUM; mac++)
		delete macros[mac];
	free(macros);

	for (int pic = 0; pic < PICO_NUM; pic++)
		delete picos[pic];
	free(picos);

	for (int mob = 0; mob < MOBILE_NUM; mob++)
		delete mobiles[mob];
	free(mobiles);

	system("pause");
	return 0;

}

void initialize(Macro **macros, Pico **picos, Mobile **mobiles)
{

	// /////////////////////////////////////////////////////////////////////////
	// 인스턴스 생성
	if (LOC_SETUP == 1)
	{

		//double bias_x = 1000;
		//double bias_y = 500;

		macros[0] = new Macro({     0.0,    0.0 }, MACRO_TX_POWER);
		macros[1] = new Macro({  1000.0,    0.0 }, MACRO_TX_POWER);
		macros[2] = new Macro({ -1000.0,    0.0 }, MACRO_TX_POWER);
		macros[3] = new Macro({   500.0,  866.0 }, MACRO_TX_POWER);
		macros[4] = new Macro({   500.0, -866.0 }, MACRO_TX_POWER);
		macros[5] = new Macro({  -500.0,  866.0 }, MACRO_TX_POWER);
		macros[6] = new Macro({  -500.0, -866.0 }, MACRO_TX_POWER);

		std::ifstream pico_loc("pico.txt");
		for (int pic = 0; pic < PICO_NUM; pic++)
		{
			point location;
			pico_loc >> location.x >> location.y;
			picos[pic] = new Pico(location, PICO_TX_POWER);
		}

		//std::ifstream node("node.txt");
		std::ifstream node("node_1.txt");
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			point location;
			node >> location.x >> location.y;
			mobiles[mob] = new Mobile(location, QOS);
		}

	}
	else
	{

		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			double radius   = uniform() * AREA_RADIUS;
			double angle    = uniform() * 2 * PI;
			macros[mac] = new Macro({
				radius * cos(angle),
				radius * sin(angle)
			}, MACRO_TX_POWER);
		}

		for (int pic = 0; pic < PICO_NUM; pic++)
		{
			double radius   = uniform() * AREA_RADIUS;
			double angle    = uniform() * 2 * PI;
			picos[pic] = new Pico({
				radius * cos(angle),
				radius * sin(angle)
			}, PICO_TX_POWER);
		}

		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			double radius   = uniform() * AREA_RADIUS;
			double angle    = uniform() * 2 * PI;
			mobiles[mob] = new Mobile({
				radius * cos(angle),
				radius * sin(angle)
			}, QOS);
		}

	}

	// /////////////////////////////////////////////////////////////////////////
	// Pico와 Macro 사이의 거리 등에 따른 값을 연산
	for (int pic = 0; pic < PICO_NUM; pic++)
	{
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			picos[pic]->locate_on_macro_of(mac, macros[mac]);
		}
	}

	// /////////////////////////////////////////////////////////////////////////
	// Mobile과 Macro 사이의 거리 등에 따른 값을 연산
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			mobiles[mob]->locate_on_macro_of(mac, macros[mac]);
		}
		macros[mobiles[mob]->macro_service]->register_mobile_to_service(mob);
	}

	// /////////////////////////////////////////////////////////////////////////
	// Mobile과 Pico 사이의 거리 등에 따른 값을 연산
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		for (int pic = 0; pic < PICO_NUM; pic++)
		{
			mobiles[mob]->locate_on_pico_of(pic, picos[pic]);
		}
		picos[mobiles[mob]->pico_service]->register_mobile_to_service(mob);
	}

}

void calculate(int *user_state_best_PA1, Macro **macros, Pico **picos, Mobile **mobiles)
{

	double objective_value_best_PA1 = -1.0;
	int macro_state_PA1[MACRO_NUM];
	int state_best_PA1[MACRO_NUM];

	for (int mac = 0; mac < MACRO_NUM; mac++)
	{
		macro_state_PA1[mac] = 0;
		state_best_PA1[mac] = 0;
	}

	for (int mob = 0; mob < MOBILE_NUM; mob++)
		user_state_best_PA1[mob] = 0;

	// macro 들의 모든 조합에 대하여 최적의 값 선택
	PA1_call_next_pico(
		0,
		&objective_value_best_PA1,
		macro_state_PA1,
		state_best_PA1,
		user_state_best_PA1,
		mobiles,
		picos,
		macros
	);

}

void PA1_calculation(
	//int mac,
	double *_best_value,
	int *macro_state_PA1,
	int *_state_best,
	int *_user_state_best,
	Mobile **mobiles,
	Pico **picos,
	Macro **macros
)
{
	double _objective_temp = 0.0;
	int _user_state_temp[MOBILE_NUM];
	for (int mob = 0; mob < MOBILE_NUM; mob++)
		_user_state_temp[mob] = 0;

	// macro 값 계산
	for (int mac = 0; mac < MACRO_NUM; mac++)
	{
		if (macro_state_PA1[mac] == 1)
		{
			int selected_user = macros[mac]->selected_user_PA1;
			_objective_temp
				= _objective_temp
				+ mobiles[selected_user]->lambda * mobiles[selected_user]->thrpt_macro
			;
			_user_state_temp[selected_user] = 1;
		}
	}

	// pico 값 계산
	for (int pic = 0; pic < PICO_NUM; pic++)
	{

		Pico *pico = picos[pic];

		// 0: ABS
		// 1: non-ABS
		// 2: non-ABS & second user
		int ABS_indicator = 0;

		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			if (pico->is_neighbor_macro(mac) == 1 && (macro_state_PA1[mac] == 1))
			{
				if (pico->nA_user1_PA1 != -1
					&&
					macro_state_PA1[mobiles[pico->nA_user1_PA1]->macro_service] == 1
					&&
					pico->nA_user1_PA1 == macros[mobiles[pico->nA_user1_PA1]->macro_service]->selected_user_PA1)
					ABS_indicator = 2;
				else
					ABS_indicator = 1;
				break;
			}
		}

		if (ABS_indicator == 0)
		{
			if (pico->ABS_user1_PA1 != -1)
			{
				if (macro_state_PA1[mobiles[pico->ABS_user1_PA1]->macro_service] == 0 || (macro_state_PA1[mobiles[pico->ABS_user1_PA1]->macro_service] == 1 && macros[mobiles[pico->ABS_user1_PA1]->macro_service]->selected_user_PA1 != pico->ABS_user1_PA1))
				{
					_objective_temp = _objective_temp + mobiles[pico->ABS_user1_PA1]->lambda * mobiles[pico->ABS_user1_PA1]->thrpt_ABS;
					_user_state_temp[pico->ABS_user1_PA1] = 2;
				}
				else if (pico->ABS_user2_PA1 != -1)
				{
					if (macro_state_PA1[mobiles[pico->ABS_user2_PA1]->macro_service] == 0 || (macro_state_PA1[mobiles[pico->ABS_user2_PA1]->macro_service] == 1 && macros[mobiles[pico->ABS_user2_PA1]->macro_service]->selected_user_PA1 != pico->ABS_user2_PA1))
					{
						_objective_temp = _objective_temp + mobiles[pico->ABS_user2_PA1]->lambda * mobiles[pico->ABS_user2_PA1]->thrpt_ABS;
						_user_state_temp[pico->ABS_user2_PA1] = 2;
					}
				}

			}

		}
		else if (ABS_indicator == 1)
		{
			if (pico->nA_user1_PA1 != -1)
			{
				_objective_temp = _objective_temp + mobiles[pico->nA_user1_PA1]->lambda * mobiles[pico->nA_user1_PA1]->thrpt_nonABS;
				_user_state_temp[pico->nA_user1_PA1] = 3;
			}
		}
		else
		{
			if ((pico->nA_user2_PA1 != -1) && (_user_state_temp[pico->nA_user2_PA1] != 1))
			{
				_objective_temp = _objective_temp + mobiles[pico->nA_user2_PA1]->lambda * mobiles[pico->nA_user2_PA1]->thrpt_nonABS;
				_user_state_temp[picos[pic]->nA_user2_PA1] = 4;
			}
		}
	}
	if (_objective_temp > *_best_value)
	{
		*_best_value = _objective_temp;
		for (int mac = 0; mac < MACRO_NUM; mac++)
			_state_best[mac] = macro_state_PA1[mac];
		for (int mob = 0; mob < MOBILE_NUM; mob++)
			_user_state_best[mob] = _user_state_temp[mob];
	}

	//macro_state_PA1 초기화
	// for (mac = 0; mac < MACRO_NUM; mac++) macro_state_PA1[mac] = 0;
}

void PA1_call_next_pico(
	int mac,
	double *_best_value,
	int *macro_state_PA1,
	int *_state_best,
	int *_user_state_best,
	Mobile **mobiles,
	Pico **picos,
	Macro **macros
)
{
	for (int i = 0; i < 2; i++)
	{
		macro_state_PA1[mac] = i;
		if ((mac + 1) < MACRO_NUM)
			PA1_call_next_pico(
				mac + 1,
				_best_value,
				macro_state_PA1,
				_state_best,
				_user_state_best,
				mobiles,
				picos,
				macros
			);
		else
			PA1_calculation(
				//mac + 1,
				_best_value,
				macro_state_PA1,
				_state_best,
				_user_state_best,
				mobiles,
				picos,
				macros
			);
	}
}
