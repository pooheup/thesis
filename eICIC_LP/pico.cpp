#include <stdio.h>
#include <math.h>

#include "pico.h"
#include "mobile.h"

Pico::Pico(point location, double t_pow)
{
	this->location = location;
	this->tx_power = t_pow;

	this->num_service_mobile = 0;

	for (int mac = 0; mac < MACRO_NUM; mac++)
		macro_neighbor[mac] = -1;

	for (int mob = 0; mob < MOBILE_NUM; mob++)
	{
		mobile[mob]         = -1;
		service_mobile[mob] = -1;
	}

}

void Pico::locate_on_macro_of(int mac, Macro *macro)
{
	distance_macro[mac] = POINT_DISTANCE(macro->getLocation(), location);
	macro_neighbor[mac] = distance_macro[mac] < MP_INT_DIST;
}

int Pico::is_neighbor_macro(int i)
{
	return macro_neighbor[i] == 1;
}


void Pico::register_mobile_to_service(int mob)
{
	this->service_mobile[this->num_service_mobile++] = mob;
}

void Pico::select_users(Mobile **mobiles, double *thrpt_ABS, double *thrpt_nonABS)
{

	int pico_ABS_PA_user  = -1;
	int pico_nA_PA1_user  = -1;
	int pico_nA_PA2_user  = -1;
	int pico_ABS_PA_user2 = -1;

	double pico_ABS_PA = -10.0; // ABS best
	double pico_ABS_PA2 = -11.0; // ABS best
	double pico_nA_PA1 = -10.0; // non-ABS first
	double pico_nA_PA2 = -11.0; // non-ABS second

	for (int j = 0; j < this->num_service_mobile; j++)
	{
		int svc_mob = this->service_mobile[j];
		Mobile *mobile = mobiles[svc_mob];

		double TODO0 = mobile->lambda * thrpt_ABS[svc_mob];

		// ABS best user 찾기
		if (TODO0 > pico_ABS_PA)
		{
			pico_ABS_PA2 = pico_ABS_PA;
			pico_ABS_PA_user2 = pico_ABS_PA_user;

			pico_ABS_PA        = TODO0;
			pico_ABS_PA_user   = svc_mob;
		}
		else // non-ABS second 찾기
		{
			if (TODO0 > pico_ABS_PA2)
			{
				pico_ABS_PA2 = TODO0;
				pico_ABS_PA_user2 = svc_mob;
			}
		}

		double TODO1 = mobile->lambda * thrpt_nonABS[svc_mob]
		;
		if (TODO1 > pico_nA_PA1)
		{
			// non-ABS first 찾기
			//if (pico_nA_PA1 >= pico_nA_PA2)
			//{
			pico_nA_PA2        = pico_nA_PA1;
			pico_nA_PA2_user   = pico_nA_PA1_user;
			//}

			pico_nA_PA1        = TODO1;
			pico_nA_PA1_user   = svc_mob;
		}
		else if (TODO1 > pico_nA_PA2)
		{
			// non-ABS second 찾기
			pico_nA_PA2        = TODO1;
			pico_nA_PA2_user   = svc_mob;
		}
	}

	this->set_user_PA1(pico_ABS_PA_user, pico_ABS_PA_user2, pico_nA_PA1_user, pico_nA_PA2_user);

}

void Pico::set_user_PA1(int _ABS_user1_PA1,int _ABS_user2_PA1, int _nA_user1_PA1, int _nA_user2_PA1)
{
	ABS_user2_PA1   = _ABS_user2_PA1;
	ABS_user1_PA1   = _ABS_user1_PA1;
	nA_user1_PA1    = _nA_user1_PA1;
	nA_user2_PA1    = _nA_user2_PA1;
}

