#include <float.h>

#include "macro.h"
#include "mobile.h"

Macro::Macro(point loc, double tx_pow)
: location(loc), tx_power(tx_pow)
{
	this->num_mobile = 0;
}

void Macro::register_mobile_to_service(int mob)
{
	mobile_service[this->num_mobile++] = mob;
}

void Macro::select_users(Mobile **mobiles, Pico **picos)
{

	this->selected_user_PA1 = -1;
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
				temp_temp = mobile->lambda * mobile->thrpt_macro - mobile->lambda * mobile->thrpt_nonABS
					+ mobiles[user_temp_temp]->lambda * mobiles[user_temp_temp]->thrpt_nonABS;
			else
				temp_temp = mobile->lambda * mobile->thrpt_macro - mobile->lambda * mobile->thrpt_nonABS;

			if (temp_temp > macro_PA)
			{
				macro_PA       = temp_temp;
				this->selected_user_PA1  = svc_mob;
			}
		}
		else
		{
			if (mobile->lambda * mobile->thrpt_macro > macro_PA)
			{
				macro_PA       = mobile->lambda * mobile->thrpt_macro;
				this->selected_user_PA1 = svc_mob;
			}
		}
	}
	this->covered_pico_PA1	= mobiles[this->selected_user_PA1]->pico_service;
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
