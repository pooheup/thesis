#pragma once

#include "parameters.h"

#include "mobile.h"
#include "macro.h"
#include "pico.h"

double mobile_set_dist_BS_temp ( double location_x, double location_y, double loc_x, double loc_y );

double cal_thrpt_s(double _sinr, double _BW);

double cal_thrpt_i(double _channel_gain, double _interference, double _no );

void exhaustive_search_calculation(int _user_num, double _objective_value, double *_best_value, int *_state_temp, int *_state_best, mobile *_mobile, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_trhpt_nonABS);

void exhaustive_search_call_next_user(int _user_num, double _objective_value, double *_best_value, int *_state_temp, int *_state_best, mobile *_mobile, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_trhpt_nonABS);

void PA1_calculation(int _macro_num, double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, mobile *_mobile, pico *_pico, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS);

void PA1_call_next_pico(int _macro_num,  double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, mobile *_mobile, pico *_pico, macro *_macro, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS);
