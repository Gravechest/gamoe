#pragma once

#include "small_types.h"

typedef struct{
	union{
		struct{
			i4 x;
			i4 y;
		};
		i4 a[2];
	};
}IVEC2;