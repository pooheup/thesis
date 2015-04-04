#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include <math.h>

#define PI					3.141592653589793238462643383279

#define LOC_SETUP			0			/* �� ����� ��ġ ������ ���� ����, 0: random 1: ���� ����, 1�ϰ�� eICIC.cpp���� ��ġ�� �������־�� �� */

#define SIMULATION_TIME		2000001  /* ��ü simulation ���� Ƚ�� */
#define PRINTF_TIME			10000	/* ��� �� ��� �ֱ� */

#define MOBILE_NUM          50		/* Number of User */
#define MACRO_NUM			7		/* Number of macro BS */
#define PICO_NUM			15	/* Number of pico BS */

#define AREA_DIST			2000	/* �ùķ��̼� ��ü ���� ũ��, AREA_DIST*ARE_DIST �� ����, random�ϰ� ��ġ ���Ҷ� ���, ��ġ ������ ��� �ǹ� ���� */

#define AREA_RADIUS			1500	/* ��ü ���� �ݰ� r, theata �� �����Ҷ� �̿�. 7 �� ���� 1500 */

#define NEIGHBOR_DIST_M		1000		// ����ϰ� ��ũ��, ���� ���� ����, �Ÿ� ����
#define NEIGHBOR_DIST_P		1000		// ����ϰ� ����
#define MP_INT_DIST			1000		// ��ũ�ο� ����

#define QOS					-10.0		// �� ������ ���� utility�� QoS ��
#define STEPSIZE2			0.0005
#define STEPSIZE3			0.0001
#define STEPSIZE4			0.00005

#define BW					20000000		// ��ü Bandwidth, Hz ����
#define	BW_PER_RB			15000		// �Ҵ��ϴ� ���� �ڿ��� Bandwidth, Hz ����, noise ���� ���

#define NOISE_FACTOR		-174	/* dB */

#define NOISE				(BW_PER_RB * powl(10, (NOISE_FACTOR / 10)))

#define	MACRO_TX_POWER		40		/* watt */
#define PICO_TX_POWER		1		/* watt */

#define LN_SHAD				8		/* log-normal shadowing (dB)*/
#define S_AWGN				-30		/* Standard Derivation for AWGN (dB) */
#define CORR_DIST			50		/* correlation distance for log-normal shadowing (meter) */

#define RATE_MAX			100		/* lambda = 0 �� ��� rate Ru �� ���ԵǴ� �� */

#define PATH_LOSS_EXPO		4		/* Path Loss Exponent */

#define ABS_STATIC			40		// static �񱳿�, abs ��, % 0~100
#define CRE_STATIC			12		// static �񱳿�, cre ��, dB

#endif /* _PARAMETER_H_ */

