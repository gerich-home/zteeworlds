#ifndef _LUA
#define _LUA

extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

int InitLua();
void CloseLua();                            

int LuaExecFile(const char * filename);
int LuaExecString(const char * Line);

const char * LuaError();

lua_State * GetLuaState();

#ifndef CONF_TRUNC

#ifdef __GNUC__
#define LUA_REGISTER_FUNC(func) lua_register(GetLuaState(), #func , _lua_ ## func );
#else
#define LUA_REGISTER_FUNC(func) lua_register(GetLuaState(), #func , _lua_ ## func ); \
	__pragma(message("Lua function: " # func))
#endif
#else

#define LUA_REGISTER_FUNC(func) ;

#endif
	
#endif