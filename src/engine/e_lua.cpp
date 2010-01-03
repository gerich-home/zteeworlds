#include "e_lua.h"
                     
#include "e_console.h"
#include <base/system.h>
                          
static const int LuaDebug = 0;

static lua_State * _LuaState = NULL;

static int last_result = 0;

static int _lua_console_print(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0)
	{
		int i;
		for (i = 1; i <= count; i++)
		{
			if (lua_isstring(L, i))
			{
				console_print(lua_tostring(L, i));
			}
		}
	}
	return 0;
}

static int _lua_console_exec(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0)
	{
		int i;
		for (i = 1; i <= count; i++)
		{
			if (lua_isstring(L, i))
			{
				console_execute_line(lua_tostring(L, i));
			}
		}
	}
	return 0;
}

static int _lua_dbg_msg(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0)
	{
		int i;
		for (i = 1; i <= count; i++)
		{
			if (lua_isstring(L, i))
			{
				dbg_msg("lua script", lua_tostring(L, i));
			}
		}
	}
	return 0;
}

void LuaRegisterFuncs(lua_State * State)
{
	LUA_REGISTER_FUNC(console_print)
	LUA_REGISTER_FUNC(console_exec)
	LUA_REGISTER_FUNC(dbg_msg)
}

int InitLua()
{
	IOHANDLE MainScript;

	const char * MainScriptPath = "data/scripts/main.lua";

	_LuaState = lua_open();
	if (!_LuaState) return 0;

	luaL_openlibs(_LuaState);
	LuaRegisterFuncs(_LuaState);

	dbg_msg("lua", "Lua initialized");

	if ((MainScript = io_open(MainScriptPath, IOFLAG_READ)))
	{
		io_close(MainScript);
	        LuaExecFile(MainScriptPath);
	}

	return 1;
}

void CloseLua()
{
	if (_LuaState) lua_close(_LuaState);
}

int LuaCall()
{
	if (!_LuaState) InitLua();
	int status;
	int base = lua_gettop(_LuaState);  /* function index */
	lua_pushliteral(_LuaState, "_TRACEBACK");
	lua_rawget(_LuaState, LUA_GLOBALSINDEX);  /* get traceback function */
	lua_insert(_LuaState, base);  /* put it under chunk and args */
	status = lua_pcall(_LuaState, 0, 0, base);
	lua_remove(_LuaState, base);  /* remove traceback function */
	return status;

}

int LuaExecFile(const char * filename)
{
	int result;

	if (!_LuaState) InitLua();

	if (LuaDebug) dbg_msg("lua/debug", "Loading script %s", filename);

	result = luaL_loadfile(_LuaState, filename);

	if (result)
	{
		dbg_msg("lua/debug", "Cannot load script %s (errorcode %d)", filename, result);
		return result == 0 ? -999 : result;
	}

	result = LuaCall();                          

	lua_gc(_LuaState, LUA_GCCOLLECT, 0);

	last_result = result;

	if (LuaDebug) dbg_msg("lua/debug", "Script %s finished with errorcode %d", filename, result);

	return result;
}

int LuaExecString(const char * Line)
{
	int result;

	if (!_LuaState) InitLua();

	if (LuaDebug) dbg_msg("lua/debug", "Loading line %s", Line);

	result = luaL_loadstring(_LuaState, Line);

	if (result)
	{
		dbg_msg("lua/debug", "Cannot load line %s (errorcode %d)", Line, result);
		return result == 0 ? -999 : result;
	}

	result = LuaCall();                        

	lua_gc(_LuaState, LUA_GCCOLLECT, 0);

	last_result = result;

	if (LuaDebug) dbg_msg("lua/debug", "Line %s finished with errorcode %s", Line, result);

	return result;
}

const char * LuaError()
{
	const char * err = lua_tostring(_LuaState, -1);
	if (!err) err = "(no error message)";
	return err;

	if (last_result)
	{
		return lua_tostring(_LuaState, -1);
	}
	return "";
}

lua_State * GetLuaState()
{
	return _LuaState;
}
