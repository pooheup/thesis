#include <float.h>

#include "macro.h"
#include "mobile.h"

Macro::Macro(point location, double t_pow)
{
	this->location   = location;
	this->tx_power   = t_pow;
	this->num_mobile = 0;
}

void Macro::register_mobile_to_service(int mob)
{
	mobile_service[this->num_mobile++] = mob;
}

void Macro::select_users(Mobile **mobiles, Pico **picos, double *thrpt_macro, double *thrpt_nonABS)
{

	int macro_PA_user = -1;
	double macro_PA = DBL_MIN;

	for (int j = 0; j < this->getMobileCount(); j++)
	{
		const int svc_mob = this->mobile_service[j];
		Mobile *mobile = mobiles[svc_mob];

		if (svc_mob == picos[mobile->pico_service]->nA_user1_PA1)
		{
			int user_temp_temp = picos[mobile->pico_service]->nA_user2_PA1; // second user num
			double temp_temp;
			if (user_temp_temp != -1)
				temp_temp = mobile->lambda * thrpt_macro[svc_mob] - mobile->lambda * thrpt_nonABS[svc_mob]
					+ mobiles[user_temp_temp]->lambda * thrpt_nonABS[user_temp_temp];
			else
				temp_temp = mobile->lambda * thrpt_macro[svc_mob] - mobile->lambda * thrpt_nonABS[svc_mob];

			if (temp_temp > macro_PA)
			{
				macro_PA       = temp_temp;
				macro_PA_user  = svc_mob;
			}
		}
		else
		{
			if (mobile->lambda * thrpt_macro[svc_mob] > macro_PA)
			{
				macro_PA       = mobile->lambda * thrpt_macro[svc_mob];
				macro_PA_user  = svc_mob;
			}
		}
	}
	this->set_user_PA1(macro_PA_user, mobiles[macro_PA_user]->pico_service);
}

void Macro::set_user_PA1(int _selected_user, int _covered_pico)
{
	selected_user_PA1	= _selected_user;
	covered_pico_PA1	= _covered_pico;
}

point Macro::getLocation()
{
	return location;
}

double Macro::getTxPower()
{
	return tx_power;
}

int Macro::getMobileCount()
{
	return this->num_mobile;
}
