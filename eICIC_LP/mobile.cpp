#include "rndfunctions.h"
#include "mobile.h"

#define cal_thrpt_i(channel_gain, interference, no) \
	(BW * log(1 + ((channel_gain) / ((interference) + (no)))))

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

	// macro 평균 thrpt 계산
	signal           = this->channel_gain_macro[this->macro_service] *rayleigh() * log_normal();
	interference     = (this->macro_interference + this->pico_interference - this->channel_gain_macro[this->macro_service]) *rayleigh() * log_normal();
	this->thrpt_macro = cal_thrpt_i(signal, interference, NOISE) / 1000000.0;
	this->thrpt_macro = this->thrpt_macro / 10.0;

	// pico ABS 평균 thrpt 계산
	signal         = this->channel_gain_pico[this->pico_service] *rayleigh() * log_normal();
	interference   = (this->pico_interference - this->channel_gain_pico[this->pico_service]) *rayleigh() * log_normal();
	this->thrpt_ABS = cal_thrpt_i(signal, interference, NOISE) / 1000000.0;
	this->thrpt_ABS = this->thrpt_ABS / 10.0;

	// pico non-ABS 평균 thrpt 계산
	//signal            = this->channel_gain_pico[this->pico_service] *rayleigh() * log_normal();
	interference      = interference + (this->macro_interference ) *rayleigh() * log_normal();
	this->thrpt_nonABS = cal_thrpt_i(signal, interference, NOISE) / 1000000.0;
	this->thrpt_nonABS = this->thrpt_nonABS / 10.0;
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
			this->increase_allocated_macro_count();
			break;
		case 2:
			this->increase_allocated_ABS_count();
			break;
		case 3:
		case 4:
			this->increase_allocated_nonABS_count();
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

void Mobile::set_pico_interference(int pico_num)
{
	pico_interference = 0.0;

	for (int i = 0; i < pico_num; i++)
	{
		pico_interference = pico_interference + channel_gain_pico[i];
	}
}

void Mobile::set_macro_interference(int macro_num)
{
	macro_interference = 0.0;

	for (int i = 0; i < macro_num; i++)
	{
		macro_interference = macro_interference + channel_gain_macro[i];
	}
}

void Mobile::increase_allocated_macro_count()  { this->allocated_macro_count++; }
void Mobile::increase_allocated_ABS_count()    { this->allocated_ABS_count++; }
void Mobile::increase_allocated_nonABS_count() { this->allocated_nonABS_count++; }

int Mobile::get_allocated_macro_count()  { return this->allocated_macro_count; }
int Mobile::get_allocated_ABS_count()    { return this->allocated_ABS_count; }
int Mobile::get_allocated_nonABS_count() { return this->allocated_nonABS_count; }
