#pragma once

#include "small_types.h"
#include "vec3.h"
#include "vec2.h"

void lightsourceEmit(VEC3 color,VEC2 pos,u4 ammount);
void lightsourcePartEmit(VEC3 color,VEC2 pos,VEC2 angle,u4 ammount,f4 wideness);