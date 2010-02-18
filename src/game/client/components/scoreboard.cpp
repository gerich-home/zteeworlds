#include <string.h>
#include <engine/e_client_interface.h>
#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>
#include <game/client/gameclient.hpp>
#include <game/client/animstate.hpp>
#include <game/client/render.hpp>
#include <game/client/components/motd.hpp>
#include "scoreboard.hpp"

#include <engine/e_lua.h>


SCOREBOARD::SCOREBOARD()
{
	on_reset();
}

void SCOREBOARD::con_key_scoreboard(void *result, void *user_data)
{
	((SCOREBOARD *)user_data)->active = console_arg_int(result, 0) != 0;
}

void SCOREBOARD::on_reset()
{
	active = false;
}

static int _lua_scoreboard(lua_State * L)
{
	int count = lua_gettop(L);
	int value = 1;
	if (count > 0 && lua_isnumber(L, 1))
	{
		value = lua_tointeger(L, 1);
	}
	gameclient.scoreboard->active = value;
	return 0;
}

void SCOREBOARD::on_console_init()
{
	MACRO_REGISTER_COMMAND("+scoreboard", "", CFGFLAG_CLIENT, con_key_scoreboard, this, "Show scoreboard");
	
	LUA_REGISTER_FUNC(scoreboard)
}

void SCOREBOARD::render_goals(float x, float y, float w)
{
	float h = 50.0f;

	gfx_blend_normal();
	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.5f);
	draw_round_rect(x-10.f, y-10.f, w, h, 10.0f);
	gfx_quads_end();

	// render goals
	//y = ystart+h-54;
	float tw = 0.0f;
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->score_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), _t("Score Limit: %d"), gameclient.snap.gameobj->score_limit);
		gfx_text(0, x+20.0f, y, 22.0f, buf, -1);
		tw += gfx_text_width(0, 22.0f, buf, -1);
	}
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->time_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), _t("Time Limit: %d min"), gameclient.snap.gameobj->time_limit);
		gfx_text(0, x+220.0f, y, 22.0f, buf, -1);
		tw += gfx_text_width(0, 22.0f, buf, -1);
	}
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->round_num && gameclient.snap.gameobj->round_current)
	{
		char buf[64];
		str_format(buf, sizeof(buf), _t("Round %d/%d"), gameclient.snap.gameobj->round_current, gameclient.snap.gameobj->round_num);
		gfx_text(0, x+450.0f, y, 22.0f, buf, -1);
		
	/*[48c3fd4c][game/scoreboard]: timelimit x:219.428558
	[48c3fd4c][game/scoreboard]: round x:453.142822*/
	}
}

void SCOREBOARD::render_spectators(float x, float y, float w)
{
	char buffer[1024*4];
	int count = 0;
	float h = 120.0f;
	
	str_copy(buffer, _t("Spectators: "), sizeof(buffer));

	gfx_blend_normal();
	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.5f);
	draw_round_rect(x-10.f, y-10.f, w, h, 10.0f);
	gfx_quads_end();
	
	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);

		if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->team == -1)
			{
				if(count)
					strcat(buffer, ", ");
				strcat(buffer, gameclient.clients[info->cid].name);
				count++;
			}
		}
	}
	
	gfx_text(0, x+10, y, 32, buffer, (int)w-20);
}

void SCOREBOARD::render_scoreboard(float x, float y, float w, int team, const char *title)
{
	//float ystart = y;
	float h = 750.0f;

	gfx_blend_normal();
	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.5f);
	draw_round_rect(x-10.f, y-10.f, w, h, 17.0f);
	gfx_quads_end();

	// render title
	if(!title)
	{
		if(gameclient.snap.gameobj->game_over)
			title = _t("Game Over");
		else
			title = _t("Score Board");
	}

	float tw = gfx_text_width(0, 48, title, -1);

	if(team == -1)
	{
		gfx_text(0, x+w/2-tw/2, y, 48, title, -1);
	}
	else
	{
		gfx_text(0, x+10, y, 48, title, -1);

		if(gameclient.snap.gameobj)
		{
			char buf[128];
			int score = team ? gameclient.snap.gameobj->teamscore_blue : gameclient.snap.gameobj->teamscore_red;
			str_format(buf, sizeof(buf), "%d", score);
			tw = gfx_text_width(0, 48, buf, -1);
			gfx_text(0, x+w-tw-30, y, 48, buf, -1);
		}
	}

	y += 54.0f;

	// find players
	const NETOBJ_PLAYER_INFO *players[MAX_CLIENTS] = {0};
	int num_players = 0;
	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);

		if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->team == team)
			{
				players[num_players] = info;
				num_players++;
			}
		}
	}

	// sort players
	for(int k = 0; k < num_players; k++) // ffs, bubblesort
	{
		for(int i = 0; i < num_players-k-1; i++)
		{
			if(players[i]->score < players[i+1]->score)
			{
				const NETOBJ_PLAYER_INFO *tmp = players[i];
				players[i] = players[i+1];
				players[i+1] = tmp;
			}
		}
	}

	// render headlines
	gfx_text(0, x+10, y, 24.0f, _t("Score"), -1);
	gfx_text(0, x+125, y, 24.0f, _t("Name"), -1);
	gfx_text(0, x+w-70, y, 24.0f, _t("Ping"), -1);
	y += 29.0f;

	float font_size = 35.0f;
	float line_height = 50.0f;
	float tee_sizemod = 1.0f;
	float tee_offset = 0.0f;
	
	if(num_players > 13)
	{
		font_size = 30.0f;
		line_height = 40.0f;
		tee_sizemod = 0.8f;
		tee_offset = -5.0f;
	}
	
	// render player scores
	for(int i = 0; i < num_players; i++)
	{
		const NETOBJ_PLAYER_INFO *info = players[i];

		// make sure that we render the correct team

		char buf[128];
		if(info->local)
		{
			// background so it's easy to find the local player
			gfx_texture_set(-1);
			gfx_quads_begin();
			gfx_setcolor(1,1,1,0.25f);
			draw_round_rect(x, y, w-20, line_height*0.95f, 17.0f);
			gfx_quads_end();
		}

		str_format(buf, sizeof(buf), "%4d", info->score);
		gfx_text(0, x+60-gfx_text_width(0, font_size,buf,-1), y, font_size, buf, -1);
		
		gfx_text(0, x+128, y, font_size, gameclient.clients[info->cid].name, -1);

		str_format(buf, sizeof(buf), "%4d", info->latency);
		float tw = gfx_text_width(0, font_size, buf, -1);
		gfx_text(0, x+w-tw-35, y, font_size, buf, -1);

		// render avatar
		if((gameclient.snap.flags[0] && gameclient.snap.flags[0]->carried_by == info->cid) ||
			(gameclient.snap.flags[1] && gameclient.snap.flags[1]->carried_by == info->cid))
		{
			gfx_blend_normal();
			gfx_texture_set(data->images[IMAGE_GAME].id);
			gfx_quads_begin();

			if(info->team == 0) select_sprite(SPRITE_FLAG_BLUE, SPRITE_FLAG_FLIP_X);
			else select_sprite(SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);
			
			float size = 64.0f;
			gfx_quads_drawTL(x+55, y-15, size/2, size);
			gfx_quads_end();
		}
		
		TEE_RENDER_INFO teeinfo = gameclient.clients[info->cid].render_info;
		teeinfo.size *= tee_sizemod;
		render_tee(ANIMSTATE::get_idle(), &teeinfo, EMOTE_NORMAL, vec2(1,0), vec2(x+90, y+28+tee_offset));

		
		y += line_height;
	}
}

void SCOREBOARD::render_new()
{
	if (!data || !gameclient.snap.gameobj) return;

	float width = 400*3.0f*gfx_screenaspect();
	float height = 400*3.0f;

	static float w = 1500.0f;
	float h = 900.0f;
	
	float need_w = 1500.0f;
	
	int weapons = 0;
	bool active_weapons[NUM_WEAPONS];
	
	for (int j = 0; j < NUM_WEAPONS; j++) active_weapons[j] = false;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!gameclient.snap.player_infos[i]) continue;
		for (int j = 0; j < NUM_WEAPONS; j++)
		{
			if (gameclient.clients[i].stats.kills[j] != 0 || gameclient.clients[i].stats.killed[j] != 0)
			{
				active_weapons[j] = true;
			}
		}
	}
	
	for (int j = 0; j < NUM_WEAPONS; j++) if (active_weapons[j]) weapons++;
	
	{
		Rect main_view_t;
		main_view_t.x = (width - w) / 2;
		main_view_t.y = (height - h) / 2;
		main_view_t.w = w;
		main_view_t.h = h;
		
		Rect header_t;

		ui_hsplit_t(&main_view_t, 40.0f, &header_t, &main_view_t);

		ui_vsplit_l(&header_t, 15.0f, 0, &header_t);
		ui_vsplit_r(&header_t, 15.0f, &header_t, 0);

		ui_margin(&main_view_t, 10.0f, &main_view_t);

		ui_vsplit_l(&header_t, 15.0f, 0, &main_view_t);
		ui_vsplit_r(&header_t, 15.0f, &main_view_t, 0);
		
		header_t.w -=  50.0f + 330.0f + 125.0f + 75.0f;
		
		float spacing_t = header_t.w / (NUM_WEAPONS + 4);
		float spacing_real_t = header_t.w / (weapons + (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS ? 4 : 2));
		
		need_w -= abs(spacing_t - spacing_real_t);
	}
	
	if (abs(w - need_w) < 7.0f) w = need_w;
	else w = w * 0.9f + need_w * 0.1f;

	float x = (width - w) / 2;
	float y = (height - h) / 2;

	gfx_mapscreen(0, 0, width, height);

	Rect main_view;
	main_view.x = x;
	main_view.y = y;
	main_view.w = w;
	main_view.h = h;

	ui_draw_rect(&main_view, vec4(0.0f, 0.0f, 0.0f, 0.5f), CORNER_ALL, 10.0f);

	Rect header, footer;

	ui_hsplit_t(&main_view, 40.0f, &header, &main_view);
	ui_draw_rect(&header, vec4(1.0f, 1.0f, 1.0f, 0.25f), CORNER_T, 10.0f);

	ui_hsplit_b(&main_view, 35.0f, &main_view, &footer);
	ui_draw_rect(&footer, vec4(1.0f, 1.0f, 1.0f, 0.25f), CORNER_B, 10.0f);

	ui_vsplit_l(&header, 15.0f, 0, &header);
	ui_vsplit_r(&header, 15.0f, &header, 0);

	ui_margin(&main_view, 10.0f, &main_view);

	ui_vsplit_l(&header, 15.0f, 0, &main_view);
	ui_vsplit_r(&header, 15.0f, &main_view, 0);

	ui_vsplit_l(&footer, 25.0f, 0, &footer);
	ui_vsplit_r(&footer, 25.0f, &footer, 0);

	main_view.w += 10.0f;

	if(gameclient.snap.gameobj && gameclient.snap.gameobj->score_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), _t("Score limit: %d"), gameclient.snap.gameobj->score_limit);
		ui_do_label(&footer, buf, footer.h * 0.8f, -1);
	}

	ui_vsplit_l(&footer, 400.0f, 0, &footer);

	if(gameclient.snap.gameobj && gameclient.snap.gameobj->time_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), _t("Time limit: %d"), gameclient.snap.gameobj->time_limit);
		ui_do_label(&footer, buf, footer.h * 0.8f, -1);
	}

	ui_vsplit_l(&footer, 800.0f, 0, &footer);

	if(gameclient.snap.gameobj && gameclient.snap.gameobj->round_num && gameclient.snap.gameobj->round_current)
	{
		char buf[64];
		str_format(buf, sizeof(buf), _t("Round: %d/%d"), gameclient.snap.gameobj->round_current, gameclient.snap.gameobj->round_num);
		ui_do_label(&footer, buf, footer.h * 0.8f, -1);
	}

	float header_width = header.w;

	ui_vsplit_l(&header, 50.0f, 0, &header);
	ui_do_label(&header, _t("Name"), header.h * 0.8f, -1);


	ui_vsplit_l(&header, 330.0f, 0, &header);

	{
		Rect line_t = header;
		line_t.x += abs(125.0f - gfx_text_width(0, header.h * 0.8f, _t("Score"), -1)) / 2.0f;
		ui_do_label(&line_t, _t("Score"), header.h * 0.8f, -1);
	}

	ui_vsplit_l(&header, 125.0f, 0, &header);

	{
		Rect line_t = header;
		line_t.x += abs(75.0f - gfx_text_width(0, header.h * 0.8f, _t("Ping"), -1)) / 2.0f;
		ui_do_label(&line_t, _t("Ping"), header.h * 0.8f, -1);
	}

	ui_vsplit_l(&header, 75.0f, 0, &header);

	float sprite_size = header.h * 0.8f;
	float spacing;
	
	if (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS)
		spacing = (header_width - 50.0f - 330.0f - 125.0f - 75.0f) / (weapons + 4);
	else
		spacing = (header_width - 50.0f - 330.0f - 125.0f - 75.0f) / (weapons + 2);

	gfx_texture_set(data->images[IMAGE_GAME].id);
	gfx_quads_begin();

	gfx_quads_setrotation(0);

	{
		select_sprite(&data->sprites[SPRITE_STAR1]);
		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size * 0.5f + header.h * 0.1f, sprite_size, sprite_size);
		ui_vsplit_l(&header, spacing, 0, &header);
	}
	{
		select_sprite(&data->sprites[SPRITE_RED_MINUS]);
		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size * 0.5f + header.h * 0.1f, sprite_size, sprite_size);
		ui_vsplit_l(&header, spacing, 0, &header);
	}

	for (int i = 0; i < NUM_WEAPONS; i++)
	{
		if (!active_weapons[i]) continue;
		
		select_sprite((i == WEAPON_HAMMER || i == WEAPON_NINJA) ? data->weapons.id[i].sprite_body : data->weapons.id[i].sprite_proj);

		float sw = i != WEAPON_NINJA ? sprite_size : sprite_size * 2.0f;

		gfx_quads_draw(header.x + spacing * 0.5f, header.y + sprite_size * 0.5f + header.h * 0.1f, sw, sprite_size);

		ui_vsplit_l(&header, spacing, 0, &header);
	}

	if (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS)
	{
		select_sprite(&data->sprites[SPRITE_FLAG_RED]);
		gfx_quads_draw(header.x + spacing, header.y + sprite_size * 0.5f + header.h * 0.1f, sprite_size * 0.5f, sprite_size);
		//ui_vsplit_l(&header, ((header_width / 2) * 4.25f / 5) / (NUM_WEAPONS + 3), 0, &header);
	}

	gfx_quads_end();

	main_view.y = y + 35.0f;
	main_view.h = h - 70.0f - 70.0f * 3 - 15.0f;

	float line_height = main_view.h / (float)MAX_CLIENTS;

	for (int team = 0; team <= 2; team++)
	{
		if (!(gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS) && team == 1) continue;

		const NETOBJ_PLAYER_INFO *players[MAX_CLIENTS] = {0};
		int num_players = 0;
		for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
		{
			SNAP_ITEM item;
			const void *data = snap_get_item(SNAP_CURRENT, i, &item);

			if(item.type == NETOBJTYPE_PLAYER_INFO)
			{
				const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
				if(info->team == (team == 2 ? -1 : team))
				{
					players[num_players] = info;
					num_players++;
				}
			}
		}

		if (team == 2 && num_players == 0) continue;

		// sort players
		for(int k = 0; k < num_players; k++) // ffs, bubblesort
		{
			for(int i = 0; i < num_players-k-1; i++)
			{
				if(players[i]->score < players[i+1]->score)
				{
					const NETOBJ_PLAYER_INFO *tmp = players[i];
					players[i] = players[i+1];
					players[i+1] = tmp;
				}
			}
		}

		if (gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS)
		{
			if (team == 0 || team == 1)
			{
				Rect line = main_view;

				ui_vsplit_l(&line, (header_width / 2) / 5, 0, &line);

				if (team == 0)
					ui_do_label(&line, _t("Red"), line_height * 0.8f * 2.0f, -1);
				else if (team == 1)
					ui_do_label(&line, _t("Blue"), line_height * 0.8f * 2.0f, -1);

				line.x = x + w - 25.0f;
				line.w = 500.0f;
				line.x -= line.w;

				Rect line2 = line;

				char buf[128];
				int score = team ? gameclient.snap.gameobj->teamscore_blue : gameclient.snap.gameobj->teamscore_red;
				str_format(buf, sizeof(buf), "%d", score);

				line2.x += abs(400.0f - gfx_text_width(0, line_height * 0.8f * 2.0f, buf, -1)) / 2.0f;
				line2.w = line.w - line2.x + line.x;
				ui_do_label(&line2, buf, line_height * 0.8f * 2.0f, -1);

				line.x -= line.w * 0.5f;

				line2 = line;
				str_format(buf, sizeof(buf), "%d", num_players);
				line2.x += abs(400.0f - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
				line2.w = line.w - line2.x + line.x;
				line2.y += line_height * 0.5f;
				line2.h = line.h - line2.y + line.y;
				ui_do_label(&line2, buf, line_height * 0.8f * 1.00f, -1);

				ui_hsplit_t(&main_view, line_height * 2.0f, 0, &main_view);
			}
		} else {
			if (team == 0)
			{
				Rect line = main_view;
				ui_vsplit_l(&line, (header_width / 2) / 5, 0, &line);
				ui_do_label(&line, _t("Players"), line_height * 0.8f * 2.0f, -1);

				line.x = x + w - 25.0f;
				line.w = 500.0f;
				line.x -= line.w * 1.5f;

				Rect line2 = line;

				line2 = line;
				char buf[128];
				str_format(buf, sizeof(buf), "%d", num_players);
				line2.x += abs(400.0f - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
				line2.w = line.w - line2.x + line.x;
				line2.y += line_height * 0.5f;
				line2.h = line.h - line2.y + line.y;
				ui_do_label(&line2, buf, line_height * 0.8f * 1.00f, -1);

				ui_hsplit_t(&main_view, line_height * 2.0f, 0, &main_view);
			}
		}

		if (team == 2)
		{
			main_view.y = y + h - 35.0f - 15.0f;
			main_view.h = line_height * 2.0f + line_height * num_players;
			main_view.y -= main_view.h;

			Rect line = main_view;
			ui_vsplit_l(&line, (header_width / 2) / 5, 0, &line);
			ui_do_label(&line, _t("Spectators"), line_height * 0.8f * 2.0f, -1);

			line.x = x + w - 25.0f;
			line.w = 500.0f;
			line.x -= line.w * 1.5f;

			Rect line2 = line;

			line2 = line;
			char buf[128];
			str_format(buf, sizeof(buf), "%d", num_players);
			line2.x += abs(400.0f - gfx_text_width(0, line_height * 0.8f * 1.0f, buf, -1)) / 2.0f;
			line2.w = line.w - line2.x + line.x;
			line2.y += line_height * 0.5f;
			line2.h = line.h - line2.y + line.y;
			ui_do_label(&line2, buf, line_height * 0.8f * 1.00f, -1);

			ui_hsplit_t(&main_view, line_height * 2.0f, 0, &main_view);
		}

		for (int i = 0; i < num_players; i++)
		{
			char buf[128];

			const NETOBJ_PLAYER_INFO *info = players[i];

			Rect line = main_view;
			line.h = line_height;

			if (info->local)
				ui_draw_rect(&line, vec4(1.0f, 1.0f, 1.0f, 0.25f), CORNER_ALL, 5.0f);

			TEE_RENDER_INFO teeinfo = gameclient.clients[info->cid].render_info;
			teeinfo.size *= line_height / 64.0f;

			if((gameclient.snap.flags[0] && gameclient.snap.flags[0]->carried_by == info->cid) ||
				(gameclient.snap.flags[1] && gameclient.snap.flags[1]->carried_by == info->cid))
			{
				gfx_blend_normal();
				gfx_texture_set(data->images[IMAGE_GAME].id);
				gfx_quads_begin();

				if(info->team == 0) select_sprite(SPRITE_FLAG_BLUE, SPRITE_FLAG_FLIP_X);
				else select_sprite(SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

				float size = line_height;

				gfx_quads_drawTL(line.x + abs(75.0f - teeinfo.size) / 2.0f - 35.0f * (line_height / 64.0f), line.y + teeinfo.size / 2.0f - 40.0f * (line_height / 64.0f), size/2, size);
				gfx_quads_end();
			}

			render_tee(ANIMSTATE::get_idle(), &teeinfo, EMOTE_NORMAL, vec2(1,0), vec2(line.x + abs(75.0f - teeinfo.size) / 2.0f, line.y + teeinfo.size / 2.0f));

			ui_vsplit_l(&line, 50.0f, 0, &line);

			ui_do_label(&line, gameclient.clients[info->cid].name, line_height * 0.8f, -1);

			ui_vsplit_l(&line, 330.0f, 0, &line);

			if (team != 2)
			{
				Rect line_t = line;
				str_format(buf, sizeof(buf), "%d", info->score);
				line_t.x += abs(125.0f - gfx_text_width(0, line_height * 0.8f, buf, -1)) * 0.5f;
				ui_do_label(&line_t, buf, line_height * 0.8f, -1);
			}

			ui_vsplit_l(&line, 125.0f, 0, &line);

			{
				Rect line_t = line;
				str_format(buf, sizeof(buf), "%d", info->latency);
				line_t.x += abs(75.0f - gfx_text_width(0, line_height * 0.8f, buf, -1)) * 0.5f;
				ui_do_label(&line_t, buf, line_height * 0.8f, -1);
			}

			if (team != 2)
			{
				ui_vsplit_l(&line, 75.0f, 0, &line);

				{
					Rect line_t = line;
					
					if (gameclient.clients[info->cid].stats.total_kills == 0 && gameclient.clients[info->cid].stats.total_killed == 0)
						str_format(buf, sizeof(buf), _t("---"));
					else
						str_format(buf, sizeof(buf), "%d/%.1f", gameclient.clients[info->cid].stats.total_kills - gameclient.clients[info->cid].stats.total_killed, (float)gameclient.clients[info->cid].stats.total_kills / (float)(gameclient.clients[info->cid].stats.total_killed == 0 ? 1 : gameclient.clients[info->cid].stats.total_killed));
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) * 0.5f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);
				}

				ui_vsplit_l(&line, spacing, 0, &line);
				{
					Rect line_t = line;
					if (gameclient.clients[info->cid].stats.total_kills == 0 && gameclient.clients[info->cid].stats.total_killed == 0)
						str_format(buf, sizeof(buf), _t("---"));
					else
						str_format(buf, sizeof(buf), "%d/%d", gameclient.clients[info->cid].stats.total_kills, gameclient.clients[info->cid].stats.total_killed);
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) * 0.5f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);
				}

				ui_vsplit_l(&line, spacing, 0, &line);

				for (int i = 0; i < NUM_WEAPONS; i++)
				{
					if (!active_weapons[i]) continue;
					Rect line_t = line;
					if (gameclient.clients[info->cid].stats.kills[i] == 0 && gameclient.clients[info->cid].stats.killed[i] == 0)
						str_format(buf, sizeof(buf), _t("---"));
					else
						str_format(buf, sizeof(buf), "%d/%d", gameclient.clients[info->cid].stats.kills[i], gameclient.clients[info->cid].stats.killed[i]);
					line_t.x += abs(spacing - gfx_text_width(0, line_height * 0.8f, buf, -1)) * 0.5f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);

					ui_vsplit_l(&line, spacing, 0, &line);
				}

				if (gameclient.snap.gameobj->flags&GAMEFLAG_FLAGS)
				{
					Rect line_t = line;
					if (gameclient.clients[info->cid].stats.flag_carried == 0 && gameclient.clients[info->cid].stats.flag_lost == 0 && gameclient.clients[info->cid].stats.flag_killed == 0)
						str_format(buf, sizeof(buf), _t("---"));
					else
						str_format(buf, sizeof(buf), "%d/%d/%d", gameclient.clients[info->cid].stats.flag_carried, gameclient.clients[info->cid].stats.flag_killed, gameclient.clients[info->cid].stats.flag_lost);
					line_t.x += abs(spacing * 2.0f - gfx_text_width(0, line_height * 0.8f, buf, -1)) * 0.5f;
					ui_do_label(&line_t, buf, line_height * 0.8f, -1);
				}
			}
			ui_hsplit_t(&main_view, line_height, 0, &main_view);
		}
	}
}

void SCOREBOARD::on_render()
{
	bool do_scoreboard = false;

	// if we activly wanna look on the scoreboard	
	if(active)
		do_scoreboard = true;
		
	if(config.cl_render_scoreboard && !config.cl_clear_all && gameclient.snap.local_info && gameclient.snap.local_info->team != -1)
	{
		// we are not a spectator, check if we are ead
		if(!gameclient.snap.local_character || gameclient.snap.local_character->health < 0)
			do_scoreboard = true;
	}

	// if we the game is over
	if(config.cl_render_scoreboard && !config.cl_clear_all && gameclient.snap.gameobj && gameclient.snap.gameobj->game_over)
		do_scoreboard = true;
		
	if(!do_scoreboard)
		return;
		
	// if the score board is active, then we should clear the motd message aswell
	if(active)
		gameclient.motd->clear();
	
	if (config.cl_new_scoreboard)
	{
		render_new();
		return;
	}	

	float width = 400*3.0f*gfx_screenaspect();
	float height = 400*3.0f;
	
	gfx_mapscreen(0, 0, width, height);

	float w = 650.0f;

	if(gameclient.snap.gameobj && !(gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS))
	{
		render_scoreboard(width/2-w/2, 150.0f, w, 0, 0);
		//render_scoreboard(gameobj, 0, 0, -1, 0);
	}
	else
	{
			
		if(gameclient.snap.gameobj && gameclient.snap.gameobj->game_over)
		{
			const char *text = _t("DRAW!");
			if(gameclient.snap.gameobj->teamscore_red > gameclient.snap.gameobj->teamscore_blue)
				text = _t("Red Team Wins!");
			else if(gameclient.snap.gameobj->teamscore_blue > gameclient.snap.gameobj->teamscore_red)
				text = _t("Blue Team Wins!");
				
			float w = gfx_text_width(0, 92.0f, text, -1);
			gfx_text(0, width/2-w/2, 45, 92.0f, text, -1);
		}
		
		render_scoreboard(width/2-w-20, 150.0f, w, 0, _t("Red Team"));
		render_scoreboard(width/2 + 20, 150.0f, w, 1, _t("Blue Team"));
	}

	render_goals(width/2-w/2, 150+750+25, w);
	render_spectators(width/2-w/2, 150+750+25+50+25, w);
}
