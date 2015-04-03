#pragma once

#include "parameters.h"

#include "mobile.h"
#include "macro.h"
#include "pico.h"

double mobile_set_dist_BS_temp ( double location_x, double location_y, double loc_x, double loc_y )
{
	double dist_temp = sqrt( pow( (location_x - loc_x), 2) + pow( (location_y - loc_y), 2) );

	return dist_temp;
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

void exhaustive_search_calculation(int _user_num, double _objective_value, double *_best_value, int *_state_temp, int *_state_best, mobile *_mobile, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_trhpt_nonABS)
{
	double _objective_value_temp = 0.0;
	for (int j = 0; j < MOBILE_NUM; j++)
	{
		if (_state_temp[j] == 1) _objective_value_temp = _objective_value_temp + _lambda[j] * _thrpt_macro[j];
		else if (_state_temp[j] == 2) _objective_value_temp = _objective_value_temp + _lambda[j] * _thrpt_ABS[j];
		else if (_state_temp[j] == 3) _objective_value_temp = _objective_value_temp + _lambda[j] * _trhpt_nonABS[j];
	}
//	if (_state_temp[0] == 0 && _state_temp[1] == 0 && _state_temp[2] == 2){//
//		std::cout << "0 0 2 case : " << _objective_value_temp << std::endl;//
//	}
//	std::cout << _state_temp[0] << _state_temp[1] << _state_temp[2] << "case : " << _objective_value_temp << std::endl;

	if (*_best_value < _objective_value_temp)
	{
		*_best_value = _objective_value_temp;
		for (int i = 0; i < MOBILE_NUM; i++) _state_best[i] = _state_temp[i];
	}
}

void exhaustive_search_call_next_user(int _user_num, double _objective_value, double *_best_value, int *_state_temp, int *_state_best, mobile *_mobile, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_trhpt_nonABS)
{
	// state
	// 0: not use
	// 1: macro
	// 2: pico ABS
	// 3: pico non_ABS
	for (int user_state = 0; user_state < 4; user_state++)
	{

//		for (int i = 0; i < MOBILE_NUM; i++)printf("%d\t\t%d\n", _state_best[i], _state_temp[i]);

		int feasibility = 0; //되!

		if (user_state == 1) // macro 로부터 할당 받았을 때
		{
			for (int i = 0; i < (_user_num); i++)
			{
				if (((_state_temp[i] == 1) ) && (_mobile[_user_num].macro_service == _mobile[i].macro_service))  feasibility = 1; // 동일 매크로 내 다른 유저들
				else if ((_state_temp[i] == 2) && (_macro[_mobile[_user_num].macro_service].pico_neighbor[_mobile[i].pico_service] == 1)) feasibility = 1; // 이 매크로와 연관 있는 모든 피코들. 음...
			}
		}
		else if (user_state == 2) // ABS 할당
		{
			for (int i = 0; i < (_user_num); i++)
			{
				if (((_state_temp[i] == 2) || (_state_temp[i] == 3)) && (_mobile[_user_num].pico_service == _mobile[i].pico_service))  feasibility = 1; // 동일 피코 내 다른 유저들
				else if ((_state_temp[i] == 1) && (_macro[_mobile[i].macro_service].pico_neighbor[_mobile[_user_num].pico_service] == 1)) feasibility = 1; // 이 모바일의 피코와 연관있는 매크로들.
			}
		}
		else if (user_state == 3) // non-ABS 할당
		{
			for (int i = 0; i < (_user_num); i++)
			{
				if (((_state_temp[i] == 2) || (_state_temp[i] == 3)) && (_mobile[_user_num].pico_service == _mobile[i].pico_service))  feasibility = 1;
			}
		}

		if (feasibility == 0)
		{
			_state_temp[_user_num] = user_state;
			if (user_state == 1) _objective_value = _objective_value + _lambda[_user_num] * _thrpt_macro[_user_num];
			else if (user_state == 2) _objective_value = _objective_value + _lambda[_user_num] * _thrpt_ABS[_user_num];
			else if (user_state == 3) _objective_value = _objective_value + _lambda[_user_num] * _trhpt_nonABS[_user_num];

			if ((_user_num + 1) < MOBILE_NUM) exhaustive_search_call_next_user((_user_num + 1), _objective_value, _best_value, _state_temp, _state_best, _mobile, _macro, _lambda, _thrpt_macro, _thrpt_ABS, _trhpt_nonABS);
			else exhaustive_search_calculation((_user_num + 1), _objective_value, _best_value, _state_temp, _state_best, _mobile, _macro, _lambda, _thrpt_macro, _thrpt_ABS, _trhpt_nonABS);
		}
	}

}

void PA1_calculation(int _macro_num, double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, mobile *_mobile, pico *_pico, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS)
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
			_objective_temp = _objective_temp + _lambda[_macro[i].selected_user_PA1] * _thrpt_macro[_macro[i].selected_user_PA1];
			_user_state_temp[_macro[i].selected_user_PA1] = 1;
		}
	}
	// pico 값 계산
	for (i = 0; i < PICO_NUM; i++)
	{
		int ABS_indicator = 0; // 0 ABS, 1 non-ABS, 2 non-ABS & second user

		for (int j = 0; j < MACRO_NUM; j++)
		{
			if ((_pico[i].macro_neighbor[j] == 1) && (_state_temp[j] == 1))
			{
				ABS_indicator = 1;
			}
		}

		if (ABS_indicator == 1 && _pico[i].nA_user1_PA1 != -1 && _macro[_mobile[_pico[i].nA_user1_PA1].macro_service].selected_user_PA1 == _pico[i].nA_user1_PA1) ABS_indicator = 2;

		if (ABS_indicator == 0)
		{
			if (_pico[i].ABS_user_PA1 != -1)
			{
				if (_state_temp[_mobile[_pico[i].ABS_user_PA1].macro_service] == 0 || (_state_temp[_mobile[_pico[i].ABS_user_PA1].macro_service] == 1 && _macro[_mobile[_pico[i].ABS_user_PA1].macro_service].selected_user_PA1 != _pico[i].ABS_user_PA1))
				{
					_objective_temp = _objective_temp + _lambda[_pico[i].ABS_user_PA1] * _thrpt_ABS[_pico[i].ABS_user_PA1];
					_user_state_temp[_pico[i].ABS_user_PA1] = 2;
				}
				else if (_pico[i].ABS_user2_PA1 != -1)
				{
					if (_state_temp[_mobile[_pico[i].ABS_user2_PA1].macro_service] == 0 || (_state_temp[_mobile[_pico[i].ABS_user2_PA1].macro_service] == 1 && _macro[_mobile[_pico[i].ABS_user2_PA1].macro_service].selected_user_PA1 != _pico[i].ABS_user2_PA1))
					{
						_objective_temp = _objective_temp + _lambda[_pico[i].ABS_user2_PA1] * _thrpt_ABS[_pico[i].ABS_user2_PA1];
						_user_state_temp[_pico[i].ABS_user2_PA1] = 2;
					}
				}

			}


		}
		else if (ABS_indicator == 1)
		{
			if (_pico[i].nA_user1_PA1 != -1)
			{
				_objective_temp = _objective_temp + _lambda[_pico[i].nA_user1_PA1] * _thrpt_nonABS[_pico[i].nA_user1_PA1];
				_user_state_temp[_pico[i].nA_user1_PA1] = 3;
			}
		}
		else
		{
			if ((_pico[i].nA_user2_PA1 != -1) && (_user_state_temp[_pico[i].nA_user2_PA1] != 1))
			{
				_objective_temp = _objective_temp + _lambda[_pico[i].nA_user2_PA1] * _thrpt_nonABS[_pico[i].nA_user2_PA1];
				_user_state_temp[_pico[i].nA_user2_PA1] = 4;
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

void PA1_call_next_pico(int _macro_num,  double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, mobile *_mobile, pico *_pico, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS)
{
	for (int i = 0; i < 2; i++)
	{
		_state_temp[_macro_num] = i;
		if ((_macro_num + 1) < MACRO_NUM) PA1_call_next_pico((_macro_num + 1), _best_value, _state_temp, _state_best, _user_state_best, _mobile, _pico, _macro, _lambda, _thrpt_macro, _thrpt_ABS, _thrpt_nonABS);
		else PA1_calculation((_macro_num + 1), _best_value, _state_temp, _state_best, _user_state_best, _mobile, _pico, _macro, _lambda, _thrpt_macro, _thrpt_ABS, _thrpt_nonABS);
	}
}
