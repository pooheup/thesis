#pragma once

#include <math.h>

#include "parameters.h"

#include "mobile.h"
#include "macro.h"
#include "pico.h"

void initialize(Macro **macros, Pico **picos, Mobile **mobiles);

void PA1_calculation(
	//int _macro_num, 
	double *_best_value, 
	int *macro_state_PA1,
	int *_state_best,
	int *_user_state_best,
	Mobile **mobiles,
	Pico **picos,
	Macro **macros
);

void PA1_call_next_pico(
	int _macro_num,
	double *_best_value,
	int *macro_state_PA1,
	int *_state_best,
	int *_user_state_best,
	Mobile **mobiles,
	Pico **picos,
	Macro **macros
);
