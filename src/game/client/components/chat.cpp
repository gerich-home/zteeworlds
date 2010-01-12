#include <string.h> // strcmp
#include <ctype.h>

#include <engine/e_client_interface.h>
#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>

#include <game/client/gameclient.hpp>

#include <game/client/components/sounds.hpp>

#include <engine/e_lua.h>

#include "chat.hpp"

static const int smileys_count = 16;
static const int smileys_texts_per_smile = 6;
static const int smileys_texts_count = smileys_count * smileys_texts_per_smile;

static const char * smileys_texts[16][6] = {
	{":)",			":-)",			"=)",		"",			"",			""},
	{";)",			";-)",			"",			"",			"",			""},
	{":D",			"=D",			":-D",		"",			"",			""},
	{"8)",			"8-)",			":cool:",	"*cool*",	"",			""},
	{":/",			":-/",			":\\",		":-\\",		"",			""},
	{":|",			":-|",			"=|",		"",			"",			""},
	{":(",			":-(",			"=(",		"",			"",			""},
	{":O",			":-O",			"=O",		"",			"",			""},
	{":P",			":-P",			"=P",		"",			"",			""},
	{":lol:",		"*lol*",		":rofl:",	"*rofl*",	"",			""},
	{":[",			":-[",			"=[",		":]",		":-]",		"=]"},
	{":evil:",		"*evil*",		":E",		":-E",		"=E",		""},
	{":!",			":-!",			"=!",		":sick:",	"*sick*",	""},
	{":dead:",		"*dead*",		"X(",		"X-(",		"",			""},
	{":mad:",		"*mad*",		":crazy:",	"*crazy*",	"",			""},
	{"?)",			"?-)",			"?(",		"?-(",		"",			""}
};

void CHAT::on_statechange(int new_state, int old_state)
{
	if(old_state <= CLIENTSTATE_CONNECTING)
	{
		mode = MODE_NONE;
		for(int i = 0; i < MAX_LINES; i++)
			lines[i].time = 0;
		current_line = 0;
	}
}

void CHAT::con_say(void *result, void *user_data)
{
	((CHAT*)user_data)->say(0, console_arg_string(result, 0));
}

void CHAT::con_sayteam(void *result, void *user_data)
{
	((CHAT*)user_data)->say(1, console_arg_string(result, 0));
}

void CHAT::con_chat(void *result, void *user_data)
{
	const char *mode = console_arg_string(result, 0);
	if(strcmp(mode, "all") == 0)
		((CHAT*)user_data)->enable_mode(0);
	else if(strcmp(mode, "team") == 0)
		((CHAT*)user_data)->enable_mode(1);
	else
		dbg_msg("console", _t("expected all or team as mode"));
}

static int _lua_say(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0 && lua_isstring(L, 1))
	{
		const char * line = lua_tostring(L, 1);
		gameclient.chat->say(0, line);		
	}
	return 0;
}

static int _lua_say_team(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0 && lua_isstring(L, 1))
	{
		const char * line = lua_tostring(L, 1);
		gameclient.chat->say(1, line);		
	}
	return 0;
}

static int _lua_chat(lua_State * L)
{
	int count = lua_gettop(L);
	if (count > 0 && lua_isstring(L, 1))
	{
		const char * line = lua_tostring(L, 1);
		const char *mode = line;
		if(strcmp(mode, "all") == 0)
			gameclient.chat->enable_mode(0);
		else if(strcmp(mode, "team") == 0)
			gameclient.chat->enable_mode(1);
		else
			dbg_msg("console", _t("expected all or team as mode"));		
	}
	return 0;
}

void CHAT::on_console_init()
{
	MACRO_REGISTER_COMMAND("say", "r", CFGFLAG_CLIENT, con_say, this, "Say in chat");
	MACRO_REGISTER_COMMAND("say_team", "r", CFGFLAG_CLIENT, con_sayteam, this, "Say in team chat");
	MACRO_REGISTER_COMMAND("chat", "s", CFGFLAG_CLIENT, con_chat, this, "Enable chat with all/team mode");
	
	LUA_REGISTER_FUNC(say)
	LUA_REGISTER_FUNC(say_team)
	LUA_REGISTER_FUNC(chat)
	
	for (int i = 0; i < MAX_HISTORY_LINES; i++)
		memset(history[i].text, 0, sizeof(history[0].text));
}

bool CHAT::on_input(INPUT_EVENT e)
{
	if(mode == MODE_NONE)
	{
		curr_history_line = -1;
		return false;
	}

	if(e.flags&INPFLAG_PRESS && e.key == KEY_ESCAPE)
		mode = MODE_NONE;
	else if(e.flags&INPFLAG_PRESS && (e.key == KEY_RETURN || e.key == KEY_KP_ENTER))
	{
		if(input.get_string()[0])
		{
			gameclient.chat->say(mode == MODE_ALL ? 0 : 1, input.get_string());
			if (str_comp_nocase(input.get_string(), history[0].text) != 0)
			{
				for (int i = MAX_HISTORY_LINES - 2; i >= 0; i--)
					str_copy(history[i + 1].text, history[i].text, sizeof(history[0].text));
				str_copy(history[0].text, input.get_string(), sizeof(history[0].text));
			}
		}
		mode = MODE_NONE;
	}
	else if (e.flags&INPFLAG_PRESS && e.key == KEY_UP)
	{
		curr_history_line = curr_history_line + 1;
		if (curr_history_line < 0) curr_history_line = -1;
		while ((curr_history_line >= MAX_HISTORY_LINES || history[curr_history_line].text[0] == 0) && (curr_history_line >= 0)) curr_history_line--;
		if (curr_history_line < 0) curr_history_line = -1;
		if (curr_history_line < 0)
			input.set("");
		else
			input.set(history[curr_history_line].text);
	}
	else if (e.flags&INPFLAG_PRESS && e.key == KEY_DOWN)
	{
		curr_history_line = curr_history_line - 1;
		if (curr_history_line < 0) curr_history_line = -1;
		while ((curr_history_line >= MAX_HISTORY_LINES || history[curr_history_line].text[0] == 0) && (curr_history_line >= 0)) curr_history_line--;
		if (curr_history_line < 0) curr_history_line = -1;
		if (curr_history_line < 0)
			input.set("");
		else
			input.set(history[curr_history_line].text);
	}
	else
		input.process_input(e);
	
	return true;
}


void CHAT::enable_mode(int team)
{
	if(mode == MODE_NONE)
	{
		if(team)
			mode = MODE_TEAM;
		else
			mode = MODE_ALL;
		
		input.clear();
		inp_clear_events();
	}
}

void CHAT::on_message(int msgtype, void *rawmsg)
{
	if(msgtype == NETMSGTYPE_SV_CHAT)
	{
		NETMSG_SV_CHAT *msg = (NETMSG_SV_CHAT *)rawmsg;
		add_line(msg->cid, msg->team, msg->message);

		if(msg->cid >= 0)
		{
			if(config.cl_chatsound)
				gameclient.sounds->play(SOUNDS::CHN_GUI, SOUND_CHAT_CLIENT, 0, vec2(0,0));
		}
		else
		{
			if(config.cl_servermsgsound)
				gameclient.sounds->play(SOUNDS::CHN_GUI, SOUND_CHAT_SERVER, 0, vec2(0,0));
		}
	}
}

void CHAT::add_line(int client_id, int team, const char *line)
{
	current_line = (current_line+1)%MAX_LINES;
	lines[current_line].time = time_get();
	lines[current_line].client_id = client_id;
	lines[current_line].team = team;
	lines[current_line].name_color = -2;

	if(client_id == -1) // server message
	{
		str_copy(lines[current_line].name, "*** ", sizeof(lines[current_line].name));
		str_format(lines[current_line].text, sizeof(lines[current_line].text), "%s", line);
	}
	else
	{
		if(gameclient.clients[client_id].team == -1)
			lines[current_line].name_color = -1;

		if(gameclient.snap.gameobj && gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS)
		{
			if(gameclient.clients[client_id].team == 0)
				lines[current_line].name_color = 0;
			else if(gameclient.clients[client_id].team == 1)
				lines[current_line].name_color = 1;
		}
		
		str_copy(lines[current_line].name, gameclient.clients[client_id].name, sizeof(lines[current_line].name));
		str_format(lines[current_line].text, sizeof(lines[current_line].text), ": %s", line);
	}
	
	dbg_msg("chat", "%s%s", lines[current_line].name, lines[current_line].text);
}

void CHAT::on_render()
{
	gfx_mapscreen(0,0,300*gfx_screenaspect(),300);
	float x = 10.0f;
	float y = 300.0f-28.0f;
	if(mode != MODE_NONE)
	{
		// render chat input
		TEXT_CURSOR cursor;
		gfx_text_set_cursor(&cursor, x, y, 7.5f, TEXTFLAG_RENDER);
		cursor.line_width = 200.0f;
		
		if(mode == MODE_ALL)
			gfx_text_ex(&cursor, "All: ", -1);
		else if(mode == MODE_TEAM)
			gfx_text_ex(&cursor, "Team: ", -1);
		else
			gfx_text_ex(&cursor, "Chat: ", -1);
			
		gfx_text_ex(&cursor, input.get_string(), input.cursor_offset());
		TEXT_CURSOR marker = cursor;
		gfx_text_ex(&marker, "|", -1);
		gfx_text_ex(&cursor, input.get_string()+input.cursor_offset(), -1);
	}

	y -= 8;

	int i;
	for(i = 0; i < MAX_LINES; i++)
	{
		int r = ((current_line-i)+MAX_LINES)%MAX_LINES;
		if(time_get() > lines[r].time+15*time_freq())
			break;

		float begin = x;
		float fontsize = 7.0f;
		
		// get the y offset
		TEXT_CURSOR cursor;
		gfx_text_set_cursor(&cursor, begin, 0, fontsize, 0);
		cursor.line_width = 200.0f;
		gfx_text_ex(&cursor, lines[r].name, -1);
		gfx_text_ex(&cursor, lines[r].text, -1);
		if(config.cl_render_chat && !config.cl_render_servermsg && !(lines[r].client_id == -1))
			y -= cursor.y + cursor.font_size;
		else if(!config.cl_render_chat && config.cl_render_servermsg && (lines[r].client_id == -1))
			y -= cursor.y + cursor.font_size;
		else if(config.cl_render_chat && config.cl_render_servermsg)
			y -= cursor.y + cursor.font_size;

		// cut off if msgs waste too much space
		if(y < 208.0f)
			break;
		
		// reset the cursor
		gfx_text_set_cursor(&cursor, begin, y, fontsize, TEXTFLAG_RENDER);
		cursor.line_width = 200.0f;

		// render name
		if(config.cl_clear_all)
			return;
		
		gfx_text_color(0.8f,0.8f,0.8f,1);
		if(lines[r].client_id == -1)
			gfx_text_color(1,1,0.5f,1); // system
		else if(lines[r].team)
			gfx_text_color(0.45f,0.9f,0.45f,1); // team message
		else if(lines[r].name_color == 0)
			gfx_text_color(1.0f,0.5f,0.5f,1); // red
		else if(lines[r].name_color == 1)
			gfx_text_color(0.7f,0.7f,1.0f,1); // blue
		else if(lines[r].name_color == -1)
			gfx_text_color(0.75f,0.5f,0.75f, 1); // spectator
			
		// render name
		if(config.cl_render_chat && !config.cl_render_servermsg && !(lines[r].client_id == -1))
			gfx_text_ex(&cursor, lines[r].name, -1);
		else if(!config.cl_render_chat && config.cl_render_servermsg && (lines[r].client_id == -1))
			gfx_text_ex(&cursor, lines[r].name, -1);
		else if(config.cl_render_chat && config.cl_render_servermsg)
			gfx_text_ex(&cursor, lines[r].name, -1);

		// render line
		gfx_text_color(1,1,1,1);
		if(lines[r].client_id == -1)
			gfx_text_color(1,1,0.5f,1); // system
		else if(lines[r].team)
			gfx_text_color(0.65f,1,0.65f,1); // team message

		if (config.gfx_smileys && lines[r].client_id != -1)
		{
			char buf[1024];
			memset(buf, 0, sizeof(buf));
			
			int tlen = str_length(lines[r].text);
			
			char * c = lines[r].text;
			char * end = c + tlen;
			char * d = buf;
			bool prevSmile = false;
			while (*c)
			{
				if (str_utf8_isstart(*c) && (c == lines[r].text || *(c - 1) < 'A' || prevSmile))
				{
					char buf0[64];
					char buf1[64];
					bool SmileFound = false;
					for (int i = 0; i < smileys_count; i++)
					{
						for (int j = 0; j < smileys_texts_per_smile; j++)
						{
							int len = str_length(smileys_texts[i][j]);
							if (len == 0) continue;
							if (end - c < len || *(c + len) >= 'A') continue;
							
							memset(buf0, 0, sizeof(buf0));
							memset(buf1, 0, sizeof(buf1));
							mem_copy(buf0, smileys_texts[i][j], len + 1);
							mem_copy(buf1, c, len);
							
							if (str_comp_nocase(buf0, buf1) == 0)
							{
								int SmileCode = 0xFFF00 + i;
								
								str_utf8_encode(d, SmileCode);
								c += len;
								d += str_utf8_char_length(SmileCode);
								
								SmileFound = true;
								break;
							}
						}
						if (SmileFound) break;
					}
					if (!SmileFound)
					{
						*d = *c;
						c++;
						d++;
						prevSmile = false;
					} else prevSmile = true;
				} else {
					*d = *c;
					c++;
					d++;
					prevSmile = false;
				}
			}
			
			cursor.flags |= TEXTFLAG_SMILEYS;
			gfx_text_ex(&cursor, buf, -1);
		} else
		{
			if(config.cl_render_chat && !config.cl_render_servermsg && !(lines[r].client_id == -1))
				gfx_text_ex(&cursor, lines[r].text, -1);
			else if(!config.cl_render_chat && config.cl_render_servermsg && (lines[r].client_id == -1))
				gfx_text_ex(&cursor, lines[r].text, -1);
			else if(config.cl_render_chat && config.cl_render_servermsg)
				gfx_text_ex(&cursor, lines[r].text, -1);
		}
	}

	gfx_text_color(1,1,1,1);
}

void CHAT::say(int team, const char *line)
{
	// send chat message
	NETMSG_CL_SAY msg;
	msg.team = team;
	msg.message = line;
	msg.pack(MSGFLAG_VITAL);
	client_send_msg();
}
