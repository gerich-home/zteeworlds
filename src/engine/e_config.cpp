/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <base/system.h>
#include "e_if_other.h"
#include "e_config.h"
#include "e_linereader.h"
#include "e_engine.h"

CONFIGURATION config;

void config_reset()
{
    #define MACRO_CONFIG_INT(name,def,min,max,flags,desc) config.name = def;
    #define MACRO_CONFIG_STR(name,len,def,flags,desc) str_copy(config.name, def, len);
 
    #include "e_config_variables.h" 
 
    #undef MACRO_CONFIG_INT 
    #undef MACRO_CONFIG_STR 
}

void config_save()
{
	char linebuf[1024*2];
	
	#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) if((flags)&CFGFLAG_SAVE){ str_format(linebuf, sizeof(linebuf), "%s %i", #name, config.name); engine_config_write_line(linebuf); }
	#define MACRO_CONFIG_STR(name,len,def,flags,desc) if((flags)&CFGFLAG_SAVE){ str_format(linebuf, sizeof(linebuf), "%s %s", #name, config.name); engine_config_write_line(linebuf); }

	#include "e_config_variables.h" 

	#undef MACRO_CONFIG_INT 
	#undef MACRO_CONFIG_STR 
}

#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) int config_get_ ## name (CONFIGURATION *c) { return c->name; }
#define MACRO_CONFIG_STR(name,len,def,flags,desc) const char *config_get_ ## name (CONFIGURATION *c) { return c->name; }
#include "e_config_variables.h"
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_STR

#define MACRO_CONFIG_INT(name,def,min,max,flags,desc) void config_set_ ## name (CONFIGURATION *c, int val) { if(min != max) { if (val < min) val = min; if (max != 0 && val > max) val = max; } c->name = val; }
#define MACRO_CONFIG_STR(name,len,def,flags,desc) void config_set_ ## name (CONFIGURATION *c, const char *str) { str_copy(c->name, str, len-1); c->name[sizeof(c->name)-1] = 0; }
#include "e_config_variables.h"
#undef MACRO_CONFIG_INT
#undef MACRO_CONFIG_STR
