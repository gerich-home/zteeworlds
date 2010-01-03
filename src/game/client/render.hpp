/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#ifndef GAME_CLIENT_RENDER_H
#define GAME_CLIENT_RENDER_H

#include <base/vmath.hpp>

#include "../mapitems.hpp"
#include "ui.hpp"

struct TEE_RENDER_INFO
{
	TEE_RENDER_INFO()
	{
		texture = -1;
		color_body = vec4(1,1,1,1);
		color_feet = vec4(1,1,1,1);
		size = 1.0f;
		got_airjump = 1;
	};
	
	int texture;
	vec4 color_body;
	vec4 color_feet;
	float size;
	int got_airjump;
};

// sprite renderings
enum
{
	SPRITE_FLAG_FLIP_Y=1,
	SPRITE_FLAG_FLIP_X=2,
	
	LAYERRENDERFLAG_OPAQUE=1,
	LAYERRENDERFLAG_TRANSPARENT=2,
	
	TILERENDERFLAG_EXTEND=4,
};

struct SPRITE;

void select_sprite(SPRITE *spr, int flags=0, int sx=0, int sy=0);
void select_sprite(int id, int flags=0, int sx=0, int sy=0);

void draw_sprite(float x, float y, float size);

// rects
void draw_round_rect(float x, float y, float w, float h, float r);
void draw_round_rect_ext(float x, float y, float w, float h, float r, int corners);
void ui_draw_rect(const RECT *r, vec4 color, int corners, float rounding);

// larger rendering methods
void render_tilemap_generate_skip();

// object render methods (gc_render_obj.cpp)
void render_tee(class ANIMSTATE *anim, TEE_RENDER_INFO *info, int emote, vec2 dir, vec2 pos);

// map render methods (gc_render_map.cpp)
void render_eval_envelope(ENVPOINT *points, int num_points, int channels, float time, float *result);
void render_quads(QUAD *quads, int num_quads, void (*eval)(float time_offset, int env, float *channels), int flags);
void render_tilemap(TILE *tiles, int w, int h, float scale, vec4 color, int flags);

// helpers
void mapscreen_to_world(float center_x, float center_y, float parallax_x, float parallax_y,
	float offset_x, float offset_y, float aspect, float zoom, float *points);


#endif
