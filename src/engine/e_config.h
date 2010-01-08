/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#ifndef _CONFIG_H
#define _CONFIG_H

#include <base/system.h>
#include "e_lua.h"

typedef struct
{ 
    #define MACRO_CONFIG_INT(name,def,min,max,save,desc) int name;
    #define MACRO_CONFIG_STR(name,len,def,save,desc) char name[len]; /* Flawfinder: ignore */
    #include "e_config_variables.h" 
    #undef MACRO_CONFIG_INT 
    #undef MACRO_CONFIG_STR 
} CONFIGURATION;

extern CONFIGURATION config;

void config_init();
void config_reset();
void config_save();

enum
{
	CFGFLAG_SAVE=1,
	CFGFLAG_CLIENT=2,
	CFGFLAG_SERVER=4
};

typedef int (*CONFIG_INT_GETTER)(CONFIGURATION *c);
typedef const char *(*CONFIG_STR_GETTER)(CONFIGURATION *c);
typedef void (*CONFIG_INT_SETTER)(CONFIGURATION *c, int val);
typedef void (*CONFIG_STR_SETTER)(CONFIGURATION *c, const char *str);

#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) int config_get_ ## name (CONFIGURATION *c);
#define MACRO_CONFIG_STR(name,len,def,flags,desc) const char *config_get_ ## name (CONFIGURATION *c);
#include "e_config_variables.h"
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_STR

#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) void config_set_ ## name (CONFIGURATION *c, int val);
#define MACRO_CONFIG_STR(name,len,def,flags,desc) void config_set_ ## name (CONFIGURATION *c, const char *str);
#include "e_config_variables.h"
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_STR

#ifndef CONF_TRUNC
static int _lua_config(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0 && lua_isstring(L, 1))
	{
		const char * param_name = lua_tostring(L, 1);
		if (count > 1)
		{
#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) if (str_comp_nocase(param_name, #name ) == 0 && lua_isnumber(L, 2)) { config.##name = lua_tointeger(L, 2); return 0; }
#define MACRO_CONFIG_STR(name,len,def,flags,desc) if (str_comp_nocase(param_name, #name ) == 0 && lua_isstring(L, 2)) { mem_copy(config.##name, lua_tostring(L, 2), len ); return 0; }
#include "e_config_variables.h"
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_STR
			dbg_msg("lua/debug", "Cannot find config variable %s", param_name);
		} else {
#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) if (str_comp_nocase(param_name, #name ) == 0) { lua_pushinteger(L, config.##name ); return 1; }
#define MACRO_CONFIG_STR(name,len,def,flags,desc) if (str_comp_nocase(param_name, #name ) == 0) { lua_pushstring(L, config.##name ); return 1; }
#include "e_config_variables.h"
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_STR
			dbg_msg("lua/debug", "Cannot find config variable %s", param_name);
		}
	}
	return 0;
}
#endif

#endif
