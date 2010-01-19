#include <engine/e_common_interface.h>
#include "layers.hpp"
#include <engine/e_lua.h>

static MAPITEM_LAYER_TILEMAP *game_layer = 0;
static MAPITEM_GROUP *game_group = 0;

static int groups_start = 0;
static int groups_num = 0;
static int layers_start = 0;
static int layers_num = 0;

void _lua_map_package_register();

void layers_init()
{
	map_get_type(MAPITEMTYPE_GROUP, &groups_start, &groups_num);
	map_get_type(MAPITEMTYPE_LAYER, &layers_start, &layers_num);
	
	for(int g = 0; g < layers_num_groups(); g++)
	{
		MAPITEM_GROUP *group = layers_get_group(g);
		for(int l = 0; l < group->num_layers; l++)
		{
			MAPITEM_LAYER *layer = layers_get_layer(group->start_layer+l);
			
			if(layer->type == LAYERTYPE_TILES)
			{
				MAPITEM_LAYER_TILEMAP *tilemap = (MAPITEM_LAYER_TILEMAP *)layer;
				if(tilemap->flags&1)
				{
					game_layer = tilemap;
					game_group = group;
				}
			}			
		}
	}
	
	_lua_map_package_register();
}

int layers_num_groups() { return groups_num; }
MAPITEM_GROUP *layers_get_group(int index)
{
	return (MAPITEM_GROUP *)map_get_item(groups_start+index, 0, 0);
}

MAPITEM_LAYER *layers_get_layer(int index)
{
	return (MAPITEM_LAYER *)map_get_item(layers_start+index, 0, 0);
}

MAPITEM_LAYER_TILEMAP *layers_game_layer()
{
	return game_layer;
}

MAPITEM_GROUP *layers_game_group()
{
	return game_group;
}

#ifndef CONF_TRUNC

static int _lua_map_groups_start(lua_State * L)
{
	lua_pushinteger(L, groups_start);
	return 1;
}

static int _lua_map_groups_num(lua_State * L)
{
	lua_pushinteger(L, groups_num);
	return 1;
}

static int _lua_map_layers_start(lua_State * L)
{
	lua_pushinteger(L, layers_start);
	return 1;
}

static int _lua_map_layers_num(lua_State * L)
{
	lua_pushinteger(L, layers_num);
	return 1;
}

static int _lua_map_group_info(lua_State * L)
{
	int count = lua_gettop(L);
	if (count < 2 || !lua_isnumber(L, 1) || !lua_isstring(L, 2)) return 0;

	int index = lua_tointeger(L, 1);
	if (index < -1 || index >= layers_num) return 0;

	MAPITEM_GROUP * g;
	if (index >= 0) g = layers_get_group(index);
	else g = game_group;

	if (!g) return 0;

	const char * action = lua_tostring(L, 2);

#ifdef __GNUC__
#define GROUP_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, g->name); \
		return 1; \
	}
#else
#define GROUP_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, g->##name); \
		return 1; \
	}
#endif

GROUP_VAR(version)
GROUP_VAR(offset_x)
GROUP_VAR(offset_y)
GROUP_VAR(parallax_x)
GROUP_VAR(parallax_y)
GROUP_VAR(start_layer)
GROUP_VAR(num_layers)
GROUP_VAR(use_clipping)
GROUP_VAR(clip_x)
GROUP_VAR(clip_y)
GROUP_VAR(clip_w)
GROUP_VAR(clip_h)

#undef GROUP_VAR

	return 0;
}

static int _lua_map_layer_info(lua_State * L)
{
	int count = lua_gettop(L);
	if (count < 2 || !lua_isnumber(L, 1) || !lua_isstring(L, 2)) return 0;

	int index = lua_tointeger(L, 1);
	if (index < -1 || index >= layers_num) return 0;

	MAPITEM_LAYER * g;
	if (index >= 0) g = layers_get_layer(index);
	else g = (MAPITEM_LAYER *)game_layer;

	if (!g) return 0;

	const char * action = lua_tostring(L, 2);
#ifdef __GNUC__
#define LAYER_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, g->name); \
		return 1; \
	}
#else
#define LAYER_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, g->##name); \
		return 1; \
	}
#endif

LAYER_VAR(version)
LAYER_VAR(type)
LAYER_VAR(flags)

#undef LAYER_VAR

	if (g->type == LAYERTYPE_TILES)
	{
	  
#ifdef __GNUC__
#define TILELAYER_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, ((MAPITEM_LAYER_TILEMAP *)g)->name); \
		return 1; \
	}
#else
#define TILELAYER_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, ((MAPITEM_LAYER_TILEMAP *)g)->##name); \
		return 1; \
	}
#endif

TILELAYER_VAR(width)
TILELAYER_VAR(height)
TILELAYER_VAR(flags)
TILELAYER_VAR(color.r)
TILELAYER_VAR(color.g)
TILELAYER_VAR(color.b)
TILELAYER_VAR(color.a)
TILELAYER_VAR(color_env)
TILELAYER_VAR(color_env_offset)
TILELAYER_VAR(image)
TILELAYER_VAR(data)

#undef TILELAYER_VAR

	}

	if (g->type == LAYERTYPE_QUADS)
	{
	  
#ifdef __GNUC__
#define QUADSLAYER_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, ((MAPITEM_LAYER_QUADS *)g)->name); \
		return 1; \
	}
#else
#define QUADSLAYER_VAR(name) if (str_comp_nocase(action, #name ) == 0) \
	{ \
		lua_pushinteger(L, ((MAPITEM_LAYER_QUADS *)g)->##name); \
		return 1; \
	}
#endif

QUADSLAYER_VAR(num_quads)
QUADSLAYER_VAR(data)
QUADSLAYER_VAR(image)

#undef QUADSLAYER_VAR

	}

	return 0;
}

static int _lua_map_get_tile(lua_State * L)
{
	int count = lua_gettop(L);
	if (count < 3 || !lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3)) return 0;

	int index = lua_tointeger(L, 1);
	if (index < -1 || index >= layers_num) return 0;

	MAPITEM_LAYER * g;
	if (index >= 0) g = layers_get_layer(index);
	else g = (MAPITEM_LAYER *)game_layer;

	if (!g || g->type != LAYERTYPE_TILES) return 0;

	MAPITEM_LAYER_TILEMAP * tl = (MAPITEM_LAYER_TILEMAP *)g;

	TILE * tiles = (TILE *)map_get_data(tl->data);

	int x = lua_tointeger(L, 2);
	int y = lua_tointeger(L, 3);

	if (x < 0 || x >= tl->width || y < 0 || y >= tl->height) return 0;

	return tiles[tl->height * y + x].index;
}

static const luaL_reg map_lib[] = {
#define LUA_LIB_FUNC(name) { #name , _lua_map_##name },

LUA_LIB_FUNC(groups_start)
LUA_LIB_FUNC(groups_num)
LUA_LIB_FUNC(layers_start)
LUA_LIB_FUNC(layers_num)
LUA_LIB_FUNC(group_info)
LUA_LIB_FUNC(layer_info)
LUA_LIB_FUNC(get_tile)

#undef LUA_LIB_FUNC
	{NULL, NULL}
};

void _lua_map_package_register()
{
	luaL_register(GetLuaState(), "map", map_lib);
}

#else

void _lua_map_package_register()
{
}

#endif