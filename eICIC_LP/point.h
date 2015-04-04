#pragma once

typedef struct {
	double x;
	double y;
} point;

#define POINT_DISTANCE(src, dst) sqrt(pow(((src).x - (dst).x), 2) + pow(((src).y - (dst).y), 2))
