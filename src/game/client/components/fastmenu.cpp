#include <engine/e_client_interface.h>
#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>

#include <engine/e_lua.h>

#include <game/gamecore.hpp> // get_angle
#include <game/client/gameclient.hpp>
#include <game/client/ui.hpp>
#include <game/client/render.hpp>
#include "fastmenu.hpp"

FASTMENU::FASTMENU()
{
	on_reset();
}

FASTMENU::~FASTMENU()
{
	clear();
}

void FASTMENU::add(const char * title, const char * command)
{
	fastmenu_command * cmd = new fastmenu_command;
	cmd->title = strdup(title);
	cmd->command = strdup(command);
	commands.push_back(cmd);
}

void FASTMENU::con_key_fastmenu(void *result, void *user_data)
{
	((FASTMENU *)user_data)->active = console_arg_int(result, 0) != 0;
}

void FASTMENU::con_fastmenu(void *result, void *user_data)
{
	((FASTMENU *)user_data)->fastmenu(console_arg_int(result, 0));
}

void FASTMENU::con_fastmenu_add(void *result, void *user_data)
{
	((FASTMENU *)user_data)->add(console_arg_string(result, 0), console_arg_string(result, 1));
}

void FASTMENU::con_fastmenu_clear(void *result, void *user_data)
{
	((FASTMENU *)user_data)->clear();
}

void FASTMENU::con_fastmenu_delete(void *result, void *user_data)
{
	((FASTMENU *)user_data)->erase(console_arg_int(result, 0));
}

void FASTMENU::con_fastmenu_list(void *result, void *user_data)
{
	((FASTMENU *)user_data)->list();
}

static int _lua_fastmenu(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0 && lua_isnumber(L, 1))
	{
		gameclient.fastmenu->fastmenu(lua_tointeger(L, 1));
	} else {
		gameclient.fastmenu->active = 1;
	}
	return 0;
}

static int _lua_select_fastmenu(lua_State * L)
{
	int count = lua_gettop(L);
	int value = 1;
	if (count > 0 && lua_isnumber(L, 1))
	{
		value = lua_tointeger(L, 1);
	}
	gameclient.fastmenu->active = value;
	return 0;
}

static int _lua_fastmenu_add(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 1 && lua_isstring(L, 1) && lua_isstring(L, 2))
	{
		gameclient.fastmenu->add(lua_tostring(L, 1), lua_tostring(L, 2));
	}
	return 0;
}

static int _lua_fastmenu_delete(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0)
	{
		for (int i = 1; i <= count; i++)
		{
			if (lua_isnumber(L, i))
				gameclient.fastmenu->erase(lua_tointeger(L, 1));
		}
	}
	return 0;
}

static int _lua_fastmenu_clear(lua_State * L)
{
	gameclient.fastmenu->clear();
	return 0;
}

static int _lua_fastmenu_list(lua_State * L)
{
	gameclient.fastmenu->list();
	return 0;
}

void FASTMENU::on_console_init()
{
	MACRO_REGISTER_COMMAND("+fastmenu", "", CFGFLAG_CLIENT, con_key_fastmenu, this, "Open fastmenu command selector");
	MACRO_REGISTER_COMMAND("fastmenu", "i", CFGFLAG_CLIENT, con_fastmenu, this, "Use fastmenu command");
	MACRO_REGISTER_COMMAND("fastmenu_add", "sr", CFGFLAG_CLIENT, con_fastmenu_add, this, "Add fastmenu command (title, command)");
	MACRO_REGISTER_COMMAND("fastmenu_clear", "", CFGFLAG_CLIENT, con_fastmenu_clear, this, "Clear fastmenu");
	MACRO_REGISTER_COMMAND("fastmenu_delete", "i", CFGFLAG_CLIENT, con_fastmenu_delete, this, "Delete fastmenu command");
	MACRO_REGISTER_COMMAND("fastmenu_list", "", CFGFLAG_CLIENT, con_fastmenu_list, this, "List fastmenu commands");

	LUA_REGISTER_FUNC(fastmenu)
	LUA_REGISTER_FUNC(select_fastmenu)
	LUA_REGISTER_FUNC(fastmenu_add)
	LUA_REGISTER_FUNC(fastmenu_clear)
	LUA_REGISTER_FUNC(fastmenu_delete)
	LUA_REGISTER_FUNC(fastmenu_list)
}

void FASTMENU::list()
{
	if (commands.size() == 0) return;
	for (int i = 0; i < commands.size(); i++)
	{
		if (!commands[i] || !commands[i]->title || !commands[i]->command)
			dbg_msg("fastmenu", "#%d. %s", i + 1, _t("error"));
		else
			dbg_msg("fastmenu", "#%d. %s => \"%s\"", i + 1, commands[i]->title, commands[i]->command);
	}
}

void FASTMENU::erase(int command)
{
	if (command < 1 || command > commands.size()) return;
	if (commands[command - 1])
	{
		if (commands[command - 1]->title) delete commands[command - 1]->title;
		if (commands[command - 1]->command) delete commands[command - 1]->command;
		commands[command - 1]->title = NULL;
		commands[command - 1]->command = NULL;
		delete commands[command - 1];
		commands[command - 1] = NULL;
	}
	commands.erase(commands.begin() + command - 1);
}

void FASTMENU::clear()
{
	if (commands.size() > 0)
	{
		for (int i = 0; i < commands.size(); i++)
		{
			if (!commands[i]) continue;

			if (commands[i]->title) delete commands[i]->title;
			if (commands[i]->command) delete commands[i]->command;
			commands[i]->title = NULL;
			commands[i]->command = NULL;

			delete commands[i];
			commands[i] = NULL;
		}
		commands.clear();
	}
}

void FASTMENU::on_reset()
{
	was_active = false;
	active = false;
	selected_command = -1;
}

void FASTMENU::on_message(int msgtype, void *rawmsg)
{
}

bool FASTMENU::on_mousemove(float x, float y)
{
	if(!active)
		return false;
	
	selector_mouse += vec2(x,y);
	return true;
}

void FASTMENU::draw_circle(float x, float y, float r, int segments, int sel)
{
	float f_segments = (float)segments;
	int seg_per_cmd = segments / commands.size();
	for(int i = 0; i < segments; i+=1)
	{
		float a1 = i/f_segments * 2*pi;
		float a2 = (i+1)/f_segments * 2*pi;
		float a3 = (i+2)/f_segments * 2*pi;
		float ca1 = cosf(a1);
		float ca2 = cosf(a2);
		float ca3 = cosf(a3);
		float sa1 = sinf(a1);
		float sa2 = sinf(a2);
		float sa3 = sinf(a3);

		if (((i + seg_per_cmd/2)%segments) / seg_per_cmd == sel)
		{
			gfx_setcolor(0.5f,0.5f,0.5f,0.3f);
		} else {
			gfx_setcolor(0,0,0,0.3f);
		}

		gfx_quads_draw_freeform(
			x + ca1 * r / 3, y + sa1 * r / 3,  
			x + ca1 * r, y + sa1 * r,
			x + ca2 * r / 3, y + sa2 * r / 3,
			x + ca2 * r, y + sa2 * r
		);
	}
}

	
void FASTMENU::on_render()
{
	if (commands.size() == 0) return;

	if(!active)
	{
		if(was_active && selected_command != -1)
			fastmenu(selected_command + 1);
		was_active = false;
		return;
	}

	if (!was_active)
	{
		selector_mouse.x = 0;
		selector_mouse.y = 0;
	}
	
	was_active = true;

    RECT screen = *ui_screen();
	
	int x, y;
	inp_mouse_relative(&x, &y);

	selector_mouse.x += x;
	selector_mouse.y += y;

	if (length(selector_mouse) > min(screen.w, screen.h) / 3)
		selector_mouse = normalize(selector_mouse) * (min(screen.w, screen.h) / 3);

	float selected_angle = get_angle(selector_mouse) + 2*pi/(commands.size() * 2);
	if (selected_angle < 0)
		selected_angle += 2*pi;

	if (length(selector_mouse) > min(screen.w, screen.h) / 9)
	{
		if (commands.size() > 1)
			selected_command = (int)(selected_angle / (2*pi) * commands.size());
		else
			selected_command = 0;
	}
	else
		selected_command = -1;


	gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);

	gfx_blend_normal();

	gfx_texture_set(-1);
	gfx_quads_begin();
	int sectors = commands.size() > 64 ? commands.size() * 2 : (64 / commands.size()) * commands.size();
	draw_circle(screen.w/2, screen.h/2, min(screen.w, screen.h) / 3, sectors, selected_command);
	gfx_quads_end();

	for (int i = 0; i < commands.size(); i++)
	{
		if (!commands[i] || !commands[i]->title) continue;

		float angle = 2*pi*i/(commands.size());
		if (angle > pi)
			angle -= 2*pi;

		bool selected = selected_command == i;

		if (selected) continue;
                                                  
		float size = 14.0f;

		float nudge_x = min(screen.w, screen.h) / 5 * cos(angle) - gfx_text_width(0, size, commands[i]->title, -1) / 2.0f;
		float nudge_y = min(screen.w, screen.h) / 5 * sin(angle) - size / 2.0f;

		gfx_text(0, screen.w/2 + nudge_x, screen.h/2 + nudge_y, size, commands[i]->title, -1);
	}

	if (selected_command >= 0 && commands[selected_command] && commands[selected_command]->title)
	{                          
		float angle = 2*pi*selected_command/(commands.size());
		if (angle > pi)
			angle -= 2*pi;

		bool selected = true;
                                                   
		float size = 24.0f;

		float nudge_x = min(screen.w, screen.h) / 5 * cos(angle) - gfx_text_width(0, size, commands[selected_command]->title, -1) / 2.0f;
		float nudge_y = min(screen.w, screen.h) / 5 * sin(angle) - size / 2.0f;

		gfx_text_color(1.0f, 1.0f, 1.0f, 1.0f);
		gfx_text(0, screen.w/2 + nudge_x, screen.h/2 + nudge_y, size, commands[selected_command]->title, -1);
	}

    gfx_text_color(1.0f, 1.0f, 1.0f, 1.0f);

    gfx_texture_set(data->images[IMAGE_CURSOR].id);
    gfx_quads_begin();
    gfx_setcolor(1,1,1,1);
    gfx_quads_drawTL(selector_mouse.x+screen.w/2,selector_mouse.y+screen.h/2,24,24);
    gfx_quads_end();
}

void FASTMENU::fastmenu(int command)
{
	if (command > 0 && command <= commands.size() && commands[command - 1] && commands[command - 1]->command)
	{
		if (commands[command - 1]->command[0] != '!')
			console_execute_line(commands[command - 1]->command);
		else
			LuaExecString(commands[command - 1]->command);
	}
}

void FASTMENU::on_save()
{
	char buffer[1024 * 4];
	char *end = buffer+sizeof(buffer)-8;
	client_save_line("fastmenu_clear");
	if (commands.size() == 0) return;
	for(int i = 0; i < commands.size(); i++)
	{
		if(!commands[i] || !commands[i]->title || !commands[i]->command)
			continue;

		str_format(buffer, sizeof(buffer), "fastmenu_add ");
		
		// process the string. we need to escape some characters
		const char *src = commands[i]->title;
		char *dst = buffer + strlen(buffer);
		*dst++ = '"';
		while(*src && dst < end)
		{
			if(*src == '"' || *src == '\\') // escape \ and "
				*dst++ = '\\';
			*dst++ = *src++;
		}
		*dst++ = '"';
		*dst++ = ' ';
		src = commands[i]->command;
		*dst++ = '"';
		while(*src && dst < end)
		{
			if(*src == '"' || *src == '\\') // escape \ and "
				*dst++ = '\\';
			*dst++ = *src++;
		}
		*dst++ = '"';
		*dst++ = 0;
		
		client_save_line(buffer);
	}
}
