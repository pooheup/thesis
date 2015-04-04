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

	// pico--macro
	for (int pic = 0; pic < PICO_NUM; pic++)
	{
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			picos[pic]->locate_on_macro_of(mac, macros[mac]);
		}
	}

	// mobile--macro
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			double distance = POINT_DISTANCE(mobiles[mob]->location, macros[mac]->getLocation());
			int is_neighbor = distance < NEIGHBOR_DIST_M;

			if (mobiles[mob]->macro_service < 0)
			{
				mobiles[mob]->macro_service = mac;
			}
			else
			{
				// �̹� macro�� ���� service macro���� �� ����� ��
				if (distance < mobiles[mob]->distance_macro[mobiles[mob]->macro_service])
				{
					mobiles[mob]->macro_service = mac;
				}
			}

			mobiles[mob]->set_dist_macro(mac, distance, macros[mac]->getTxPower(), NOISE);
			mobiles[mob]->macro_neighbor[mac] = is_neighbor;
		}

		macros[mobiles[mob]->macro_service]->register_mobile_to_service(mob);

	}

	// ����� pico �Ÿ�, �̿���� ��, service ����      
	// pico class�� �����ϱ� ���� pico ���� �����͵� ���� temp data�� �̿��Ͽ� ���ÿ� ó��
	// ��ο� ���� 0,1�� �������� ǥ�� 1�̸� ����. �� ���ؾ� �Ѵ�.

	// ��ġ���迡 ���� ä�� ����. �̿� ã��. �Ÿ�, �Ŀ� ��� ��ȣ ����.(�������� �ᵵ ��)
	// ä�� ���
	// mobile--pico
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		int neighbor_count = 0;
		int service_pico = -1;
		double service_distance = DBL_MAX;

		for (int pic = 0; pic < PICO_NUM; pic++)
		{
			double distance = POINT_DISTANCE(mobiles[mob]->location, picos[pic]->location);
			int is_neighbor = distance < NEIGHBOR_DIST_P;

			if (is_neighbor)
				neighbor_count++;

			if (distance < service_distance)
			{
				service_distance = distance;
				service_pico = pic;
			}

			mobiles[mob]->set_dist_pico_1(pic, distance, picos[pic]->tx_power, NOISE);
			mobiles[mob]->pico_neighbor[pic] = is_neighbor;

		}

		picos[service_pico]->num_service_mobile++;
		picos[service_pico]->service_mobile_01[mob] = 1;

		mobiles[mob]->set_num_int_pico(neighbor_count);
		mobiles[mob]->set_serviceBS_pico(service_pico);

	}

	// initial setting �� Ŭ���� �ʱ�ȭ, 
	// �� Ŭ������ ����� ���� ������ ��, ���� ������ ����

	// static �� ���� cre bias�� ���� cell association
	double cre_bias = pow(10.0, CRE_STATIC / 10.0);
	for (int mob = 0; mob < MOBILE_NUM; mob++)
		mobiles[mob]->cell_association_static(cre_bias);

	// /////////////////////////////////////////////////////////////////////////
	// �ʱⰪ�� ��� �־��� ����, �� macro/pico/mobile ���� ���¿� ���� ���� ���

	for (int pic = 0; pic < PICO_NUM; pic++)
		picos[pic]->set_neighbor();

	// TODO �ð� �ݺ��� ������ �̵��ؾ� ��
	// �� ������� �޴� interference calculation
	// ��� �������� ���� ������ ���. ���� �̿�� �ڽ��� �Ҵ�޴� �������� ��ȣ�� ���ؾ� ��.
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		mobiles[mob]->set_pico_interference(PICO_NUM);
		mobiles[mob]->set_macro_interference(MACRO_NUM);
	}

	// /////////////////////////////////////////////////////////////////////////

	// �ùķ��̼ǿ��� �ʿ��� �� ������ ����
	// �� timeslot������ ��� throughput
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

	// ������� ���� ��� throughput
	double thrp_result[MOBILE_NUM];

	// Ru ��
	double rate_user[MOBILE_NUM];

	int abs_count_macro[MACRO_NUM];
	int abs_count_pico[PICO_NUM];

	for (int mac = 0; mac < MACRO_NUM; mac++)
		abs_count_macro[mac] = 0;

	for (int pic = 0; pic < PICO_NUM; pic++)
		abs_count_pico[pic] = 0;

	// mobile!
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		lambda[mob]             = 0.1;
		thrp_result[mob]        = 0.0;
		rate_user[mob]          = 0.0;

		mu[mob]                 = 0.0;

		num_allocated_macro[mob]  = 0;
		num_allocated_ABS[mob]    = 0;
		num_allocated_nonABS[mob] = 0;
	}

	// PA1
	int resource_macro_PA1[MOBILE_NUM];
	int resource_ABS_PA1[MOBILE_NUM];
	int resource_nonABS_PA1[MOBILE_NUM];

	double thrp_result_PA1[MOBILE_NUM];
	double rate_user_PA1[MOBILE_NUM];
	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		thrp_result_PA1[mob] = 0.0;
		rate_user_PA1[mob]   = 0.0;
	}

	////////////////////////////////////////////////////////////////////////////// �˰��� ���� ���� //////////////////////////////////////////////////////////////////////

	// ��� ���ϵ�
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
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			// macro ��� thrpt ���
			signal_temp       = mobiles[mob]->channel_gain_macro[mobiles[mob]->macro_service] *rayleigh() * log_normal();
			interference_temp = (mobiles[mob]->macro_interference + mobiles[mob]->pico_interference - mobiles[mob]->channel_gain_macro[mobiles[mob]->macro_service]) *rayleigh() * log_normal();
			thrpt_macro[mob]  = cal_thrpt_i(signal_temp, interference_temp, NOISE) / 1000000.0;
			thrpt_macro[mob]  = thrpt_macro[mob] / 10.0;

			// pico ABS ��� thrpt ���
			signal_temp       = mobiles[mob]->channel_gain_pico[mobiles[mob]->pico_service] *rayleigh() * log_normal();
			interference_temp = (mobiles[mob]->pico_interference - mobiles[mob]->channel_gain_pico[mobiles[mob]->pico_service]) *rayleigh() * log_normal();
			thrpt_ABS[mob]    = cal_thrpt_i(signal_temp, interference_temp, NOISE) / 1000000.0;
			thrpt_ABS[mob]    = thrpt_ABS[mob] / 10.0;

			// pico non-ABS ��� thrpt ���
			//signal_temp       = mobiles[mob]->channel_gain_pico[mobiles[mob]->pico_service] *rayleigh() * log_normal();
			interference_temp = interference_temp + (mobiles[mob]->macro_interference ) *rayleigh() * log_normal();
			thrpt_nonABS[mob] = cal_thrpt_i(signal_temp, interference_temp, NOISE) / 1000000.0;
			thrpt_nonABS[mob] = thrpt_nonABS[mob] / 10.0;

			// �Ҵ� �� �ʱ�ȭ
			resource_macro[mob]  = 0;
			resource_ABS[mob]    = 0;
			resource_nonABS[mob] = 0;

			// �Ҵ� �� �ʱ�ȭ PA1
			resource_macro_PA1[mob]  = 0;
			resource_ABS_PA1[mob]    = 0;
			resource_nonABS_PA1[mob] = 0;
		}

		double object_value_es = 0.0;
		double sum_rate_es      = 0.0;

		///////////////////////////////////////////////// �����ϴ� �˰����� �������� �ַ�� ã��.
		// ���� ����
		int macro_user_PA[MACRO_NUM];
		int macro_cover_pico_PA[MACRO_NUM];
		int pico_ABS_user_PA[PICO_NUM];
		int pico_ABS_user_PA2[PICO_NUM];
		int pico_nA_user1_PA[PICO_NUM];
		int pico_nA_user2_PA[PICO_NUM];
		int pico_nA_01_PA[PICO_NUM]; // non-ABS ��Ȳ���� ���° ���� �Ҵ�Ǵ��� indicator. 0 first, 1 second

		for (int pic = 0; pic < PICO_NUM; pic++)
			pico_nA_user2_PA[pic] = -1;

		// �� pico ���� ABS best user, non-ABS first.second user ����
		for (int pic = 0; pic < PICO_NUM; pic++)
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

			for (int j = 0; j < picos[pic]->num_service_mobile; j++)
			{
				// ABS best user ã��
				if (lambda[picos[pic]->service_mobile[j]] * thrpt_ABS[picos[pic]->service_mobile[j]] > temp_pico_ABS_PA)
				{
					temp_pico_ABS_PA2 = temp_pico_ABS_PA;
					temp_pico_ABS_PA_user2 = temp_pico_ABS_PA_user;

					temp_pico_ABS_PA        = lambda[picos[pic]->service_mobile[j]] * thrpt_ABS[picos[pic]->service_mobile[j]];
					temp_pico_ABS_PA_user   = picos[pic]->service_mobile[j];
				} 
				else // non-ABS second ã��
				{
					if (lambda[picos[pic]->service_mobile[j]] * thrpt_ABS[picos[pic]->service_mobile[j]] > temp_pico_ABS_PA2)
					{
						temp_pico_ABS_PA2 = lambda[picos[pic]->service_mobile[j]] * thrpt_ABS[picos[pic]->service_mobile[j]];
						temp_pico_ABS_PA_user2 = picos[pic]->service_mobile[j];
					}
				}

				// non-ABS first ã��
				if (lambda[picos[pic]->service_mobile[j]] * thrpt_nonABS[picos[pic]->service_mobile[j]] > temp_pico_nA_PA1)
				{
					//if (temp_pico_nA_PA1 >= temp_pico_nA_PA2)
					//{
					temp_pico_nA_PA2        = temp_pico_nA_PA1;
					temp_pico_nA_PA2_user   = temp_pico_nA_PA1_user; 
					//}
						
					temp_pico_nA_PA1        = lambda[picos[pic]->service_mobile[j]] * thrpt_nonABS[picos[pic]->service_mobile[j]];
					temp_pico_nA_PA1_user   = picos[pic]->service_mobile[j];
				}
				else // non-ABS second ã��
				{
					if (lambda[picos[pic]->service_mobile[j]] * thrpt_nonABS[picos[pic]->service_mobile[j]] > temp_pico_nA_PA2)
					{
						temp_pico_nA_PA2        = lambda[picos[pic]->service_mobile[j]] * thrpt_nonABS[picos[pic]->service_mobile[j]];
						temp_pico_nA_PA2_user   = picos[pic]->service_mobile[j];
					}
				}
			}
			pico_ABS_user_PA2[pic] = temp_pico_ABS_PA_user2;
			pico_ABS_user_PA[pic]  = temp_pico_ABS_PA_user;
			pico_nA_user1_PA[pic]  = temp_pico_nA_PA1_user;
			pico_nA_user2_PA[pic]  = temp_pico_nA_PA2_user;
			pico_nA_01_PA[pic]     = 0; // �ʱ�ȭ
		}

		// �� macro ���� ã�� // mobile ��ȣ: macro[mac].mobile_service[j]
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			int temp_macro_PA_user = -1;
			double temp_macro_PA = -1000.0;
			for (int j = 0; j < macros[mac]->getMobileCount(); j++)
			{
				if (macros[mac]->mobile_service[j] == pico_nA_user1_PA[mobiles[macros[mac]->mobile_service[j]]->pico_service])
				{
					int user_temp_temp = pico_nA_user2_PA[mobiles[macros[mac]->mobile_service[j]]->pico_service]; // second user num
					double temp_temp;
					if (user_temp_temp != -1) temp_temp = (lambda[macros[mac]->mobile_service[j]] * thrpt_macro[macros[mac]->mobile_service[j]] - lambda[macros[mac]->mobile_service[j]] * thrpt_nonABS[macros[mac]->mobile_service[j]] + lambda[user_temp_temp] * thrpt_nonABS[user_temp_temp]);
					else temp_temp = (lambda[macros[mac]->mobile_service[j]] * thrpt_macro[macros[mac]->mobile_service[j]] - lambda[macros[mac]->mobile_service[j]] * thrpt_nonABS[macros[mac]->mobile_service[j]]);

					if (temp_temp > temp_macro_PA)
					{
						temp_macro_PA       = temp_temp;
						temp_macro_PA_user  = macros[mac]->mobile_service[j];
					}
				}
				else
				{
					if (lambda[macros[mac]->mobile_service[j]] * thrpt_macro[macros[mac]->mobile_service[j]] > temp_macro_PA)
					{
						temp_macro_PA       = lambda[macros[mac]->mobile_service[j]] * thrpt_macro[macros[mac]->mobile_service[j]];
						temp_macro_PA_user  = macros[mac]->mobile_service[j];
					}
				}
			}
			macro_user_PA[mac]        = temp_macro_PA_user;
			macro_cover_pico_PA[mac]  = mobiles[temp_macro_PA_user]->pico_service;
			pico_nA_01_PA[mobiles[temp_macro_PA_user]->pico_service] = 1;
		}

		for (int mac = 0; mac < MACRO_NUM; mac++)
			macros[mac]->set_user_PA1(macro_user_PA[mac], macro_cover_pico_PA[mac]);

		for (int pic = 0; pic < PICO_NUM; pic++)
			picos[pic]->set_user_PA1(pico_ABS_user_PA[pic], pico_ABS_user_PA2[pic], pico_nA_user1_PA[pic], pico_nA_user2_PA[pic], pico_nA_01_PA[pic]);

		int _macro_num = 0;
		double objective_value_best_PA1 = -1.0;
		int state_temp_PA1[MACRO_NUM];
		int state_best_PA1[MACRO_NUM];
		int user_state_best_PA1[MOBILE_NUM];
		
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			state_temp_PA1[mac] = 0;
			state_best_PA1[mac] = 0;
		}

		for (int mob = 0; mob < MOBILE_NUM; mob++)
			user_state_best_PA1[mob] = 0;

		// macro ���� ��� ���տ� ���Ͽ� ������ �� ����
		PA1_call_next_pico(_macro_num, &objective_value_best_PA1, state_temp_PA1, state_best_PA1, user_state_best_PA1, mobiles, picos, macros, lambda, thrpt_macro, thrpt_ABS, thrpt_nonABS);

		// resource ���� �Է�
		// best state ��, ���� �ڿ� �Ҵ� �� �Է�
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			switch (user_state_best_PA1[mob])
			{
				case 1:
					resource_macro_PA1[mob] = 1;
					break;
				case 2:
					resource_ABS_PA1[mob]   = 1;
					break;
				case 3:
				case 4:
				resource_nonABS_PA1[mob]    = 1;
					break;
			}
		}

		// ������� ���� throughput �Է�
		for (int mob = 0; mob < MOBILE_NUM; mob++)
			thrp_result_PA1[mob]
				= thrp_result_PA1[mob]
				+ thrpt_macro[mob]  * resource_macro_PA1[mob]
				+ thrpt_ABS[mob]    * resource_ABS_PA1[mob]
				+ thrpt_nonABS[mob] * resource_nonABS_PA1[mob]
			;

		// ��� rate Ru, rate_user[mob], ������Ʈ
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			if (lambda[mob] == 0.0)
				rate_user_PA1[mob] = RATE_MAX;
			else
				//rate_user[mob] = 0.8* rate_user[mob] + 0.2 * (1.0 + mu[mob]) / lambda[mob];
				rate_user_PA1[mob]
					= 0.8 * rate_user_PA1[mob]
					+ 0.2 * (1.0 + mu[mob]) / lambda[mob]
				;
		}

		// �� ���� ��� ������ ����ߴ��� count
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			switch (user_state_best_PA1[mob])
			{
				case 1:
					num_allocated_macro[mob]  = num_allocated_macro[mob] + 1;
					break;
				case 2:
					num_allocated_ABS[mob]    = num_allocated_ABS[mob] + 1;
					break;
				case 3:
				case 4:
					num_allocated_nonABS[mob] = num_allocated_nonABS[mob] + 1;
					break;
			}
		}

		// �� �������� �ڿ� ����ߴ��� ���� count // ����� ������ ������� �ش� �������� ABS. abs_count ����
		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			int resource_used_temp = 0; // �ڿ� �Ҵ� ����. 1�̸� ���, 0�̸� ��� ����
			for (int j = 0; j < macros[mac]->getMobileCount(); j++)
			{
				if (resource_macro_PA1[macros[mac]->mobile_service[j]] == 1) resource_used_temp = 1;
			}
			if (resource_used_temp == 0) abs_count_macro[mac]++;
		}

		

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// dual update
		// proposed algorithm update
		// dual variable, lambda, mu ������Ʈ
		for (int mob = 0; mob < MOBILE_NUM; mob++)
		{
			double lambda_temp, mu_temp;
			//if (     (thrp_result_PA1[mob] / (1 + t) - rate_user_PA1[mob] >= 0.0)  //feasilbity
			//	&& (abs(thrp_result_PA1[mob] / (1 + t) - rate_user_PA1[mob]) * lambda[mob] < 0.05))

			double TODO0 = log(rate_user_PA1[mob]) - mobiles[mob]->QoS;
			double TODO1 = abs(TODO0) * mu[mob] < 0.01;
			if (abs(thrp_result_PA1[mob] / (1 + t) - rate_user_PA1[mob]) * lambda[mob] < 0.05)
				lambda_temp = lambda[mob] - step_size  * TODO1;
			else
				lambda_temp = lambda[mob] - step_size2 * TODO1;
			lambda[mob] = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

			//if ((log(rate_user_PA1[mob]) >= mobiles[mob]->QoS)
			//	&& (abs(TODO0) * mu[mob] < 0.01))
			if (abs(TODO0) * mu[mob] < 0.01)
				mu_temp = mu[mob] - step_size  * TODO0;
			else
				mu_temp = mu[mob] - step_size2 * TODO0;
			mu[mob] = (0.0 > mu_temp) ? 0.0 : mu_temp;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////// ���� ���
		if (t % 1000 == 0) printf("%s", ">");
		
		// PIINTF_TIME �ֱ�� ���. // ȭ��, eICIC.txt, lambda.txt, mu.txt ���
		/*
		if (t%PRINTF_TIME == 0)
		{
			printf("\n\n");
			printf("\n%s\t%d\n", "simulation time\t", t);
			Savefile << "simulation time\t" << t << std::endl;
			printf("%s\t\t%s\n", "rate Ru", "throughput");

			Savefile << "rate RU\t\t thoroughput" << std::endl;

			double function_result = 0.0;
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t%f\t%f\t%f\n", rate_user[mob], log(rate_user[mob]), (thrp_result[mob] / (1 + t)), log(thrp_result[mob] / (1 + t)));
				Savefile << rate_user[mob] << "\t" << log(rate_user[mob]) << "\t" << (thrp_result[mob] / (1 + t)) << "\t" << log(thrp_result[mob] / (1 + t)) << std::endl;
			}
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", lambda[mob]);
				Savefile << lambda[mob] << "\t";
				savel << lambda[mob] << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";

			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", mu[mob]);
				Savefile << mu[mob] << "\t";
				savem << mu[mob] << "\t";

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
				printf("%d\t%d\t%d\n", num_allocated_macro[mob], num_allocated_ABS[mob], num_allocated_nonABS[mob]);
				Savefile << num_allocated_macro[mob] << "\t" << num_allocated_ABS[mob] << "\t" << num_allocated_nonABS[mob] << std::endl;
			}
			printf("\n");
			for (int mac = 0; mac < MACRO_NUM; mac++)
			{
				printf("%d\t", abs_count_macro[mac]);
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
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				//printf("%f\t%f\t%f\t%f\n", rate_user_PA1[mob], log(rate_user_PA1[mob]), (thrp_result_PA1[mob] / (1 + t)), log(thrp_result_PA1[mob] / (1 + t)));
				printf("%f\t%f\t%f\t%f\t%f\t%f\n", rate_user_PA1[mob], (thrp_result_PA1[mob] / (1 + t)), log(rate_user_PA1[mob]), log(thrp_result_PA1[mob] / (1 + t)), lambda[mob], mu[mob] );
				//Savefile << rate_user[mob] << "\t" << log(rate_user[mob]) << "\t" << (thrp_result[mob] / (1 + t)) << "\t" << log(thrp_result[mob] / (1 + t)) << std::endl;
			}
			/*
			printf("\n%s\t\t", "lambda");
			Savefile << std::endl << "lambda" << "\t" << "\t";
			savel << t << "\t"; // Number of Simulation
			savem << t << "\t"; // Number of Simulation
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", lambda[mob]);
				Savefile << lambda[mob] << "\t";
				savel << lambda[mob] << "\t";
			}
			printf("\n%s\t\t", "mu");
			Savefile << std::endl << "mu" << "\t" << "\t";

			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				printf("%f\t", mu[mob]);
				Savefile << mu[mob] << "\t";
				savem << mu[mob] << "\t";

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
				//printf("%d\t%d\t%d\n", num_allocated_macro[mob], num_allocated_ABS[mob], num_allocated_nonABS[mob]);
				//Savefile << num_allocated_macro[mob] << "\t" << num_allocated_ABS[mob] << "\t" << num_allocated_nonABS[mob] << std::endl;
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

			double sum_utility_temp= 0.0;
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				sum_utility_temp = sum_utility_temp + log(thrp_result_PA1[mob] / (1 + t));
			}
			printf("%s\t%f\n", "sum utility", sum_utility_temp);

		}
		
		// ���� ��� ��� results.txt ���
		if (t == (SIMULATION_TIME-1)){
			results << "R E S U L T S" << std::endl;
			results << "Rate, Throughput, Utility of Rate, Utility of Throughput, QoS" << std::endl << std::endl;
			results << "USER\t\t" << "Rate\t\t" << "Thrpt\t\t" << "Util Rate\t" << "Util Thrpt\t" << "QoS" << std::endl;
			for (int mob = 0; mob < MOBILE_NUM; mob++)
			{
				results << "USER " << mob + 1 << "\t\t" << rate_user[mob] << "\t\t" << (thrp_result[mob] / (1 + t)) << "\t\t" << log(rate_user[mob]) << "\t\t" << log(thrp_result[mob] / (1 + t)) << "\t\t" << QOS << std::endl;
			}
		}
	}
	system("pause");

	for (int mac = 0; mac < MACRO_NUM; mac++)
		delete macros[mac];
	free(macros);

	for (int pic = 0; pic < PICO_NUM; pic++)
		delete picos[pic];
	free(picos);

	for (int mob = 0; mob < MOBILE_NUM; mob++)
		delete mobiles[mob];
	free(mobiles);

	return 0;
}

void initialize(Macro **macros, Pico **picos, Mobile **mobiles)
{

	// �� ����� ��ġ ���� ����, ��ġ�� �������� ��� �Ʒ����� ���� �Է�, ���� �Է��� ��� parameter.h�� LOC_SETUP = 1 �� ����
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
	double _objective_temp = 0.0;
	int _user_state_temp[MOBILE_NUM];
	for (int mob = 0; mob < MOBILE_NUM; mob++)
		_user_state_temp[mob] = 0;

	// macro �� ���
	for (int mac = 0; mac < MACRO_NUM; mac++)
	{
		if (_state_temp[mac] == 1)
		{
			_objective_temp = _objective_temp + _lambda[macros[mac]->selected_user_PA1] * _thrpt_macro[macros[mac]->selected_user_PA1];
			_user_state_temp[macros[mac]->selected_user_PA1] = 1;
		}
	}
	// pico �� ���
	for (int pic = 0; pic < PICO_NUM; pic++)
	{
		int ABS_indicator = 0; // 0 ABS, 1 non-ABS, 2 non-ABS & second user

		for (int mac = 0; mac < MACRO_NUM; mac++)
		{
			if (picos[pic]->is_neighbor_macro(mac) && (_state_temp[mac] == 1))
			{
				ABS_indicator = 1;
			}
		}

		if (ABS_indicator == 1 && picos[pic]->nA_user1_PA1 != -1 && macros[mobiles[picos[pic]->nA_user1_PA1]->macro_service]->selected_user_PA1 == picos[pic]->nA_user1_PA1) ABS_indicator = 2;

		if (ABS_indicator == 0)
		{
			if (picos[pic]->ABS_user_PA1 != -1)
			{
				if (_state_temp[mobiles[picos[pic]->ABS_user_PA1]->macro_service] == 0 || (_state_temp[mobiles[picos[pic]->ABS_user_PA1]->macro_service] == 1 && macros[mobiles[picos[pic]->ABS_user_PA1]->macro_service]->selected_user_PA1 != picos[pic]->ABS_user_PA1))
				{
					_objective_temp = _objective_temp + _lambda[picos[pic]->ABS_user_PA1] * _thrpt_ABS[picos[pic]->ABS_user_PA1];
					_user_state_temp[picos[pic]->ABS_user_PA1] = 2;
				}
				else if (picos[pic]->ABS_user2_PA1 != -1)
				{
					if (_state_temp[mobiles[picos[pic]->ABS_user2_PA1]->macro_service] == 0 || (_state_temp[mobiles[picos[pic]->ABS_user2_PA1]->macro_service] == 1 && macros[mobiles[picos[pic]->ABS_user2_PA1]->macro_service]->selected_user_PA1 != picos[pic]->ABS_user2_PA1))
					{
						_objective_temp = _objective_temp + _lambda[picos[pic]->ABS_user2_PA1] * _thrpt_ABS[picos[pic]->ABS_user2_PA1];
						_user_state_temp[picos[pic]->ABS_user2_PA1] = 2;
					}
				}

			}


		}
		else if (ABS_indicator == 1)
		{
			if (picos[pic]->nA_user1_PA1 != -1)
			{
				_objective_temp = _objective_temp + _lambda[picos[pic]->nA_user1_PA1] * _thrpt_nonABS[picos[pic]->nA_user1_PA1];
				_user_state_temp[picos[pic]->nA_user1_PA1] = 3;
			}
		}
		else
		{
			if ((picos[pic]->nA_user2_PA1 != -1) && (_user_state_temp[picos[pic]->nA_user2_PA1] != 1))
			{
				_objective_temp = _objective_temp + _lambda[picos[pic]->nA_user2_PA1] * _thrpt_nonABS[picos[pic]->nA_user2_PA1];
				_user_state_temp[picos[pic]->nA_user2_PA1] = 4;
			}
		}
	}
	if (_objective_temp > *_best_value)
	{
		*_best_value = _objective_temp;
		for (int mac = 0; mac < MACRO_NUM; mac++)
			_state_best[mac] = _state_temp[mac];
		for (int mob = 0; mob < MOBILE_NUM; mob++)
			_user_state_best[mob] = _user_state_temp[mob];
	}

	//_state_temp �ʱ�ȭ
	// for (mac = 0; mac < MACRO_NUM; mac++) _state_temp[mac] = 0;
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
