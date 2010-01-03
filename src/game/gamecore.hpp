/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#ifndef GAME_GAME_H
#define GAME_GAME_H

#include <base/system.h>
#include <base/math.hpp>

#include <engine/e_common_interface.h>
#include <math.h>
#include "collision.hpp"
#include <game/generated/g_protocol.hpp>

struct TUNING_PARAMS
{
	TUNING_PARAMS()
	{
		const float ticks_per_second = 50.0f;
		#define MACRO_TUNING_PARAM(name,value) name.set((int)(value*100.0f));
		#include "tuning.hpp"
		#undef MACRO_TUNING_PARAM
	}

	static const char *names[];
	
	#define MACRO_TUNING_PARAM(name,value) tune_param name;
	#include "tuning.hpp"
	#undef MACRO_TUNING_PARAM
	
	static int num() { return sizeof(TUNING_PARAMS)/sizeof(int); }
	bool set(int index, float value);
	bool set(const char *name, float value);
	bool get(int index, float *value);
	bool get(const char *name, float *value);
};


inline vec2 get_direction(int angle)
{
	float a = angle/256.0f;
	return vec2(cosf(a), sinf(a));
}

inline vec2 get_dir(float a)
{
	return vec2(cosf(a), sinf(a));
}

inline float get_angle(vec2 dir)
{
	float a = atan(dir.y/dir.x);
	if(dir.x < 0)
		a = a+pi;
	return a;
}

inline void str_to_ints(int *ints, int num, const char *str)
{
	int index = 0;
	while(num)
	{
		char buf[4] = {0,0,0,0};
		for(int c = 0; c < 4 && str[index]; c++, index++)
			buf[c] = str[index];
		*ints = ((buf[0]+128)<<24)|((buf[1]+128)<<16)|((buf[2]+128)<<8)|(buf[3]+128);
		ints++;
		num--;
	}
	
	// null terminate
	ints[-1] &= 0xffffff00;
}

inline void ints_to_str(const int *ints, int num, char *str)
{
	while(num)
	{
		str[0] = (((*ints)>>24)&0xff)-128;
		str[1] = (((*ints)>>16)&0xff)-128;
		str[2] = (((*ints)>>8)&0xff)-128;
		str[3] = ((*ints)&0xff)-128;
		str += 4;
		ints++;
		num--;
	}
	
	// null terminate
	str[-1] = 0;
}



inline vec2 calc_pos(vec2 p, vec2 v, float curvature, float speed, float t)
{
	vec2 n;
	t *= speed;
	n.x = p.x + v.x*t;
	n.y = p.y + v.y*t + curvature/10000*(t*t);
	return n;
}


template<typename T>
inline T saturated_add(T min, T max, T current, T modifier)
{
	if(modifier < 0)
	{
		if(current < min)
			return current;
		current += modifier;
		if(current < min)
			current = min;
		return current;
	}
	else
	{
		if(current > max)
			return current;
		current += modifier;
		if(current > max)
			current = max;
		return current;
	}
}

void move_point(vec2 *inout_pos, vec2 *inout_vel, float elasticity, int *bounces);
void move_box(vec2 *inout_pos, vec2 *inout_vel, vec2 size, float elasticity);
bool test_box(vec2 pos, vec2 size);
float velocity_ramp(float value, float start, float range, float curvature);

// hooking stuff
enum
{
	HOOK_RETRACTED=-1,
	HOOK_IDLE=0,
	HOOK_RETRACT_START=1,
	HOOK_RETRACT_END=3,
	HOOK_FLYING,
	HOOK_GRABBED,
	
	COREEVENT_GROUND_JUMP=0x01,
	COREEVENT_AIR_JUMP=0x02,
	COREEVENT_HOOK_LAUNCH=0x04,
	COREEVENT_HOOK_ATTACH_PLAYER=0x08,
	COREEVENT_HOOK_ATTACH_GROUND=0x10,
	COREEVENT_HOOK_HIT_NOHOOK=0x20,
	COREEVENT_HOOK_RETRACT=0x40,
};

class WORLD_CORE
{
public:
	WORLD_CORE()
	{
		mem_zero(characters, sizeof(characters));
	}
	
	TUNING_PARAMS tuning;
	class CHARACTER_CORE *characters[MAX_CLIENTS];
};

class CHARACTER_CORE
{
public:
	WORLD_CORE *world;
	
	vec2 pos;
	vec2 vel;
	
	vec2 hook_pos;
	vec2 hook_dir;
	int hook_tick;
	int hook_state;
	int hooked_player;
	
	int jumped;
	
	int direction;
	int angle;
	NETOBJ_PLAYER_INPUT input;
	
	int triggered_events;
	
	void reset();
	void tick(bool use_input);
	void move();
	
	void read(const NETOBJ_CHARACTER_CORE *obj_core);
	void write(NETOBJ_CHARACTER_CORE *obj_core);
	void quantize();
};


#define LERP(a,b,t) (a + (b-a) * t)
#define min(a, b) ( a > b ? b : a)
#define max(a, b) ( a > b ? a : b)

inline bool col_check_point(float x, float y) { return col_is_solid(round(x), round(y)) != 0; }
inline bool col_check_point(vec2 p) { return col_check_point(p.x, p.y); }

#endif
