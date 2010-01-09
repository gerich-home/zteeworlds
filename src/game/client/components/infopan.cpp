#include <string.h> // strcmp

#include <engine/e_client_interface.h>
#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>

#include <game/client/gameclient.hpp>

#include <game/client/components/sounds.hpp>

#include "infopan.hpp"

void INFOPAN::con_infomsg(void *result, void *user_data)
{
	((INFOPAN*)user_data)->add_line(console_arg_string(result, 0));
}

static int _lua_infomsg(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0 && lua_isstring(L, 1))
	{
		const char * line = lua_tostring(L, 1);
		gameclient.infopan->add_line(line);		
	}
	return 0;
}

void INFOPAN::on_console_init()
{
	MACRO_REGISTER_COMMAND("infomsg", "r", CFGFLAG_CLIENT, con_infomsg, this, "Info message");
	
	LUA_REGISTER_FUNC(infomsg)
}

void INFOPAN::on_message(int msgtype, void *rawmsg)
{
}

void INFOPAN::add_line(const char *line)
{
	current_line = (current_line+1)%MAX_LINES;
	lines[current_line].time = time_get();
	str_format(lines[current_line].text, sizeof(lines[MAX_LINES - current_line].text), "%s", line);
	dbg_msg("info", "%s", lines[current_line].text);
}

void INFOPAN::on_render()
{
	gfx_mapscreen(0,0,300*gfx_screenaspect(),300);
	float x = 10.0f;
	float y = 80.0f;

	y -= 8;

	int i;
	for(i = MAX_LINES - 1; i >= 0; i--)
	{
		int r = ((current_line-i)+MAX_LINES)%MAX_LINES;
		if(time_get() > lines[r].time+15*time_freq())
			continue;

		float begin = x;
		float fontsize = 7.0f;

		// get the y offset
		TEXT_CURSOR cursor;
		gfx_text_set_cursor(&cursor, begin, 0, fontsize, 0);
		cursor.line_width = 200.0f;
		gfx_text_ex(&cursor, lines[r].text, -1);
		y += cursor.y + cursor.font_size;

		// cut off if msgs waste too much space
		if(y > 200.0f)
			break;

		// reset the cursor
		gfx_text_set_cursor(&cursor, begin, y, fontsize, TEXTFLAG_RENDER);
		cursor.line_width = 200.0f;

		// render line
		gfx_text_color(1.0f, 0.75f, 0.5f, 1.0f);
		gfx_text_ex(&cursor, lines[r].text, -1);
	}

	gfx_text_color(1,1,1,1);
}

