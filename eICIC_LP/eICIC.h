#pragma once

#include <math.h>

#include "parameters.h"

#include "mobile.h"
#include "macro.h"
#include "pico.h"

double cal_thrpt_s(double _sinr, double _BW);

double cal_thrpt_i(double _channel_gain, double _interference, double _no );

void exhaustive_search_calculation(int _user_num, double _objective_value, double *_best_value, int *_state_temp, int *_state_best, Mobile *_mobile, Macro **macros, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_trhpt_nonABS);

void exhaustive_search_call_next_user(int _user_num, double _objective_value, double *_best_value, int *_state_temp, int *_state_best, Mobile *_mobile, Macro **macros, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_trhpt_nonABS);

void PA1_calculation(int _macro_num, double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, Mobile *_mobile, Pico **picos, Macro **macros, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS);

void PA1_call_next_pico(int _macro_num,  double *_best_value, int *_state_temp, int *_state_best, int *_user_state_best, Mobile *_mobile, Pico **picos, Macro **macros, double *_lambda, double *_thrpt_macro, double *_thrpt_ABS, double *_thrpt_nonABS);
