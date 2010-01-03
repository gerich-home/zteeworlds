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

int LuaGetIntConfigValue(const char * name);
const char * LuaGetStrConfigValue(const char * name);

#define LUA_REGISTER_FUNC(func) lua_register(GetLuaState(), #func , _lua_ ## func ); \
	__pragma(message("Lua function: " # func))
#endif