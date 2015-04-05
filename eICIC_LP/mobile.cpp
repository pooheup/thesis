#include "rndfunctions.h"
#include "mobile.h"

#define cal_thrpt_i(channel_gain, interference, noise) \
	(BW * log(1 + ((channel_gain) / ((interference) + (noise)))))

Mobile::Mobile(point loc, double qos)
: location(loc), QoS(qos)
{

	this->macro_service = -1;
	this->pico_service  = -1;

	this->num_interferer_pico = 0;

	this->lambda = 0.1;
	this->mu = 0.0;

	this->thrp_result_PA1 = 0.0;
	this->rate_user_PA1 = 0.0;

	this->resource_macro_PA1 = 0;
	this->resource_ABS_PA1 = 0;
	this->resource_nonABS_PA1 = 0;

	this->allocated_macro_count = 0;
	this->allocated_ABS_count = 0;
	this->allocated_nonABS_count = 0;

	for (int mac = 0; mac < MACRO_NUM; mac++)
	{
		macro_neighbor[mac] = -1;
		distance_macro[mac] = -1.0;
	}

	for (int pic = 0; pic < PICO_NUM; pic++)
	{
		pico_neighbor[pic] = -1;
		distance_pico[pic] = -1.0;
	}
}

void Mobile::locate_on_macro_of(int mac, Macro *macro)
{
	double distance = POINT_DISTANCE(macro->getLocation(), this->location);
	int is_neighbor = distance < NEIGHBOR_DIST_M;

	if (this->macro_service < 0)
	{
		this->macro_service = mac;
	}
	else
	{
		// 이번 macro가 이전 service macro보다 더 가까울 때
		if (distance < this->distance_macro[this->macro_service])
		{
			this->macro_service = mac;
		}
	}

	this->distance_macro[mac]     = distance;
	this->macro_neighbor[mac]     = is_neighbor;
	this->channel_gain_macro[mac] = macro->getTxPower() * pow( (1/distance), PATH_LOSS_EXPO );
}

void Mobile::locate_on_pico_of(int pic, Pico *pico)
{
	double distance = POINT_DISTANCE(pico->location, this->location);
	int is_neighbor = distance < NEIGHBOR_DIST_P;

	if (is_neighbor)
		this->num_interferer_pico++;

	if (this->pico_service < 0)
	{
		this->pico_service = pic;
	}
	else
	{
		if (distance < this->distance_pico[this->pico_service])
		{
			this->pico_service = pic;
		}
	}

	this->distance_pico[pic]     = distance;
	this->channel_gain_pico[pic] = pico->tx_power * pow((1/distance), PATH_LOSS_EXPO);
	this->pico_neighbor[pic]     = is_neighbor;
}

void Mobile::generate_channel_gain()
{
	double signal, interference;

	// 각 매크로에 대한 채널
	double macro_channel[MACRO_NUM];
	double macro_channel_sum = 0.0;
	for (int mac = 0; mac < MACRO_NUM; mac++)
	{
		macro_channel[mac]
			= this->channel_gain_macro[mac]
			* RAYLEIGH
			* LOG_NORMAL
		;
		macro_channel_sum += macro_channel[mac];
	}

	// 각 피코에 대한 채널
	double pico_channel[PICO_NUM];
	double pico_channel_sum = 0.0;
	for (int pic = 0; pic < PICO_NUM; pic++)
	{
		pico_channel[pic]
			= this->channel_gain_pico[pic]
			* RAYLEIGH
			* LOG_NORMAL
		;
		pico_channel_sum += pico_channel[pic];
	}

	this->thrpt_macro = cal_thrpt_i(
		macro_channel[this->macro_service],
		macro_channel_sum + pico_channel_sum - macro_channel[this->macro_service],
		NOISE
	) / MEGA;

	this->thrpt_ABS = cal_thrpt_i(
		pico_channel[this->pico_service],
		pico_channel_sum - pico_channel[this->pico_service],
		NOISE
	) / MEGA;

	this->thrpt_nonABS = cal_thrpt_i(
		pico_channel[this->pico_service],
		macro_channel_sum + pico_channel_sum - pico_channel[this->pico_service],
		NOISE
	) / MEGA;

}

void Mobile::allocate_resource(int user_state_best_PA1)
{
	switch (user_state_best_PA1)
	{
		case 0:
			this->resource_macro_PA1  = 0;
			this->resource_ABS_PA1    = 0;
			this->resource_nonABS_PA1 = 0;
			break;
		case 1:
			this->resource_macro_PA1  = 1;
			this->resource_ABS_PA1    = 0;
			this->resource_nonABS_PA1 = 0;
			break;
		case 2:
			this->resource_macro_PA1  = 0;
			this->resource_ABS_PA1    = 1;
			this->resource_nonABS_PA1 = 0;
			break;
		case 3:
		case 4:
			this->resource_macro_PA1  = 0;
			this->resource_ABS_PA1    = 0;
			this->resource_nonABS_PA1 = 1;
			break;
	}
}

void Mobile::calculate_throughput()
{
	this->thrp_result_PA1
		= this->thrp_result_PA1
		+ this->thrpt_macro  * this->resource_macro_PA1
		+ this->thrpt_ABS    * this->resource_ABS_PA1
		+ this->thrpt_nonABS * this->resource_nonABS_PA1
	;
}

void Mobile::count_cell_association(int user_state_best_PA1)
{
	switch (user_state_best_PA1)
	{
		case 1:
			this->allocated_macro_count++;
			break;
		case 2:
			this->allocated_ABS_count++;
			break;
		case 3:
		case 4:
			this->allocated_nonABS_count++;
			break;
	}
}

void Mobile::calculate_rate_user()
{
	if (this->lambda == 0.0)
		this->rate_user_PA1 = RATE_MAX;
	else
		//this->rate_user_PA1 = 0.8* this->rate_user_PA1 + 0.2 * (1.0 + this->mu) / this->lambda;
		this->rate_user_PA1
			= 0.8 * this->rate_user_PA1
			+ 0.2 * (1.0 + this->mu) / this->lambda
		;
}

void Mobile::calculate_dual_variable(const int t, const double STEP_SIZE, const double STEP_SIZE2)
{
	double instant_rate
		= this->thrpt_macro  * this->resource_macro_PA1
		+ this->thrpt_ABS    * this->resource_ABS_PA1
		+ this->thrpt_nonABS * this->resource_nonABS_PA1
	;

	double lambda_temp, mu_temp;
	//if (     (this->thrp_result_PA1 / (1 + t) - this->rate_user_PA1 >= 0.0)  //feasilbity
	//	&& (abs(this->thrp_result_PA1 / (1 + t) - this->rate_user_PA1) * this->lambda < 0.05))
	if ( (abs(this->thrp_result_PA1 / (1 + t) - this->rate_user_PA1) * this->lambda < 0.05))
		lambda_temp = this->lambda - STEP_SIZE * (instant_rate - this->rate_user_PA1);
	else
		lambda_temp = this->lambda - STEP_SIZE2 * (instant_rate - this->rate_user_PA1);
	this->lambda = (0.0 > lambda_temp) ? 0.0 : lambda_temp;

	//if ((log(this->rate_user_PA1) >= this->QoS)
		//&& (abs(log(this->rate_user_PA1) - this->QoS) * this->mu < 0.01))
	if ( (abs(log(this->rate_user_PA1) - this->QoS) * this->mu < 0.01))
		mu_temp = this->mu - STEP_SIZE * (log(this->rate_user_PA1) - this->QoS);
	else
		mu_temp = this->mu - STEP_SIZE2 * (log(this->rate_user_PA1) - this->QoS);
	this->mu = (0.0 > mu_temp) ? 0.0 : mu_temp;
}

int Mobile::get_allocated_macro_count()  { return this->allocated_macro_count; }
int Mobile::get_allocated_ABS_count()    { return this->allocated_ABS_count; }
int Mobile::get_allocated_nonABS_count() { return this->allocated_nonABS_count; }
