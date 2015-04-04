#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <math.h>

#define PI					3.141592653589793238462643383279

#define LOC_SETUP			0			/* 각 노드의 위치 지정에 대한 설정, 0: random 1: 직접 설정, 1일경우 eICIC.cpp에서 위치를 지정해주어야 함 */

#define SIMULATION_TIME		2000001  /* 전체 simulation 수행 횟수 */
#define PRINTF_TIME			10000	/* 결과 값 출력 주기 */

#define MOBILE_NUM          50		/* Number of User */
#define MACRO_NUM			7		/* Number of macro BS */
#define PICO_NUM			15	/* Number of pico BS */

#define AREA_DIST			2000	/* 시뮬레이션 전체 영역 크기, AREA_DIST*ARE_DIST 로 정의, random하게 위치 정할때 사용, 위치 지정할 경우 의미 없음 */

#define AREA_RADIUS			1500	/* 전체 영역 반경 r, theata 로 생성할때 이용. 7 셀 기준 1500 */

#define NEIGHBOR_DIST_M		1000		// 모바일과 매크로, 간섭 관계 설정, 거리 기준
#define NEIGHBOR_DIST_P		1000		// 모바일과 피코
#define MP_INT_DIST			1000		// 매크로와 피코

#define QOS					-10.0		// 각 유저가 갖는 utility의 QoS 값
#define STEPSIZE2			0.0005
#define STEPSIZE3			0.0001
#define STEPSIZE4			0.00005

#define BW					20000000		// 전체 Bandwidth, Hz 단위
#define	BW_PER_RB			15000		// 할당하는 단위 자원의 Bandwidth, Hz 단위, noise 계산시 사용

#define NOISE_FACTOR		-174	/* dB */

#define NOISE				(BW_PER_RB * powl(10, (NOISE_FACTOR / 10)))

#define	MACRO_TX_POWER		40		/* watt */
#define PICO_TX_POWER		1		/* watt */

#define LN_SHAD				8		/* log-normal shadowing (dB)*/
#define S_AWGN				-30		/* Standard Derivation for AWGN (dB) */
#define CORR_DIST			50		/* correlation distance for log-normal shadowing (meter) */

#define RATE_MAX			100		/* lambda = 0 일 경우 rate Ru 가 갖게되는 값 */

#define PATH_LOSS_EXPO		4		/* Path Loss Exponent */

#define ABS_STATIC			40		// static 비교용, abs 값, % 0~100
#define CRE_STATIC			12		// static 비교용, cre 값, dB

#endif /* _PARAMETER_H_ */

