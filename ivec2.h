#pragma once

#include "small_types.h"

#define IVEC2_ZERO (IVEC2){0,0}

typedef struct{
	union{
		struct{
			i4 x;
			i4 y;
		};
		i4 a[2];
	};
}IVEC2;