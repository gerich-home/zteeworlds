
#include <base/math.hpp>

#include <string.h> // strcmp, strlen, strncpy
#include <stdlib.h> // atoi

#include <engine/e_client_interface.h>

#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>

#include <game/client/ui.hpp>
#include <game/client/render.hpp>
#include <game/client/gameclient.hpp>
#include <game/client/animstate.hpp>

#include "binds.hpp"
#include "menus.hpp"
#include "skins.hpp"
#include "languages.hpp"

MENUS_KEYBINDER MENUS::binder;

MENUS_KEYBINDER::MENUS_KEYBINDER()
{
	take_key = false;
	got_key = false;
}

bool MENUS_KEYBINDER::on_input(INPUT_EVENT e)
{
	if(take_key)
	{
		if(e.flags&INPFLAG_PRESS && e.key != KEY_ESCAPE)
		{
			key = e;
			got_key = true;
			take_key = false;
		}
		return true;
	}
	
	return false;
}

void MENUS::render_settings_player(RECT main_view)
{
	RECT button;
	RECT skinselection;
	ui_vsplit_l(&main_view, 300.0f, &main_view, &skinselection);


	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);

	// render settings
	{	
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		ui_do_label(&button, _t("Name:"), 14.0, -1);
		ui_vsplit_l(&button, 80.0f, 0, &button);
		ui_vsplit_l(&button, 180.0f, &button, 0);
		if(ui_do_edit_box(config.player_name, &button, config.player_name, sizeof(config.player_name), 14.0f))
			need_sendinfo = true;

		static int dynamic_camera_button = 0;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if(ui_do_button(&dynamic_camera_button, _t("Dynamic Camera"), config.cl_mouse_deadzone != 0, &button, ui_draw_checkbox, 0))
		{
			
			if(config.cl_mouse_deadzone)
			{
				config.cl_mouse_followfactor = 0;
				config.cl_mouse_max_distance = 400;
				config.cl_mouse_deadzone = 0;
			}
			else
			{
				config.cl_mouse_followfactor = 60;
				config.cl_mouse_max_distance = 1000;
				config.cl_mouse_deadzone = 300;
			}
		}

		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_autoswitch_weapons, _t("Switch weapon on pickup"), config.cl_autoswitch_weapons, &button, ui_draw_checkbox, 0))
			config.cl_autoswitch_weapons ^= 1;
			
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_show_ghost, _t("Show ghost"), config.cl_show_ghost, &button, ui_draw_checkbox, 0))
			config.cl_show_ghost ^= 1;
		
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_nameplates, _t("Show name plates"), config.cl_nameplates, &button, ui_draw_checkbox, 0))
			config.cl_nameplates ^= 1;

		//if(config.cl_nameplates)
		{
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			ui_vsplit_l(&button, 15.0f, 0, &button);
			if (ui_do_button(&config.cl_nameplates_always, _t("Always show name plates"), config.cl_nameplates_always, &button, ui_draw_checkbox, 0))
				config.cl_nameplates_always ^= 1;
		}
			
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.player_color_body, _t("Custom colors"), config.player_use_custom_color, &button, ui_draw_checkbox, 0))
		{
			config.player_use_custom_color = config.player_use_custom_color?0:1;
			need_sendinfo = true;
		}
		
		if(config.player_use_custom_color)
		{
			int *colors[2];
			colors[0] = &config.player_color_body;
			colors[1] = &config.player_color_feet;
			
			const char *parts[] = {"Body", "Feet"};
			parts[0] = _t("Body");
			parts[1] = _t("Feet");
			
			const char *labels[] = {"Hue", "Sat.", "Lht."};
			labels[0] = _t("Hue");
			labels[1] = _t("Sat.");
			labels[2] = _t("Lht.");
			static int color_slider[2][3] = {{0}};
			//static float v[2][3] = {{0, 0.5f, 0.25f}, {0, 0.5f, 0.25f}};
				
			for(int i = 0; i < 2; i++)
			{
				RECT text;
				ui_hsplit_t(&main_view, 20.0f, &text, &main_view);
				ui_vsplit_l(&text, 15.0f, 0, &text);
				ui_do_label(&text, parts[i], 14.0f, -1);
				
				int prevcolor = *colors[i];
				int color = 0;
				for(int s = 0; s < 3; s++)
				{
					RECT text;
					ui_hsplit_t(&main_view, 19.0f, &button, &main_view);
					ui_vsplit_l(&button, 30.0f, 0, &button);
					ui_vsplit_l(&button, 30.0f, &text, &button);
					ui_vsplit_r(&button, 5.0f, &button, 0);
					ui_hsplit_t(&button, 4.0f, 0, &button);
					
					float k = ((prevcolor>>((2-s)*8))&0xff)  / 255.0f;
					k = ui_do_scrollbar_h(&color_slider[i][s], &button, k);
					color <<= 8;
					color += clamp((int)(k*255), 0, 255);
					ui_do_label(&text, labels[s], 15.0f, -1);
					 
				}
		
				if(*colors[i] != color)
					need_sendinfo = true;
					
				*colors[i] = color;
				ui_hsplit_t(&main_view, 5.0f, 0, &main_view);
			}
		}
		
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_default_skin_only, _t("Default skin only"), config.cl_default_skin_only, &button, ui_draw_checkbox, 0))
		{
			config.cl_default_skin_only = config.cl_default_skin_only?0:1;
			if (!gameclient.skins->all_skins_loaded)
				gameclient.skins->load_all();
		}
		
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.zpack2_compatible_cyrillic, _t("Z-Team Pack 2 compatible cyrillic"), config.zpack2_compatible_cyrillic, &button, ui_draw_checkbox, 0))
			config.zpack2_compatible_cyrillic ^= 1;
			
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_autorecord, _t("Auto demo recording"), config.cl_autorecord, &button, ui_draw_checkbox, 0))
		{
			config.cl_autorecord = config.cl_autorecord?0:1;
		}

		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_gameover_screenshot, _t("Make screenshot after game over"), config.cl_gameover_screenshot, &button, ui_draw_checkbox, 0))
		{
			config.cl_gameover_screenshot = config.cl_gameover_screenshot?0:1;
		}
		
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_new_scoreboard, _t("New detailed scoreboard"), config.cl_new_scoreboard, &button, ui_draw_checkbox, 0))
		{
			config.cl_new_scoreboard = config.cl_new_scoreboard?0:1;
		}
		
		{
			RECT langlist = main_view;
			ui_vmargin(&langlist, 5.0f, &langlist);
			
			RECT header, footer;
			ui_hsplit_t(&langlist, 20, &button, &langlist);
			
			// draw header
			ui_hsplit_t(&langlist, 20, &header, &langlist);
			ui_draw_rect(&header, vec4(1,1,1,0.25f), CORNER_T, 5.0f); 
			ui_do_label(&header, _t("Language"), 14.0f, 0);
			
			char buf[1024];

			// draw footers	
			ui_hsplit_b(&langlist, 20, &langlist, &footer);
			buf[0] = 0;
			for (int i = 0; i < gameclient.languages->languages.size(); i++)
				if (str_comp_nocase(config.language, (const char *)gameclient.languages->languages[i]->filename) == 0)
				{
					str_format(buf, sizeof(buf), _t("Current: %s"), gameclient.languages->languages[i]->name);
					break;
				}
			if (!buf[0])
				str_format(buf, sizeof(buf), _t("Current: %s"), _t("unknown"));
			ui_draw_rect(&footer, vec4(1,1,1,0.25f), CORNER_B, 5.0f); 
			ui_vsplit_l(&footer, 10.0f, 0, &footer);
			ui_do_label(&footer, buf, 14.0f, -1);
						
			ui_draw_rect(&langlist, vec4(0,0,0,0.15f), 0, 0);

			RECT scroll;
			ui_vsplit_r(&langlist, 15, &langlist, &scroll);

			RECT list = langlist;
			ui_hsplit_t(&list, 20, &button, &list);
			
			int num = (int)(langlist.h/button.h);
			static float scrollvalue = 0;
			static int scrollbar = 0;
			ui_hmargin(&scroll, 5.0f, &scroll);
			scrollvalue = ui_do_scrollbar_v(&scrollbar, &scroll, scrollvalue);

			int num_langs = gameclient.languages->languages.size();
			int start = (int)((num_langs-num)*scrollvalue);
			if(start < 0)
				start = 0;
				
			for(int i = start; i < start+num && i < num_langs; i++)
			{					
				int selected = 0;
				selected = str_comp_nocase(config.language, (const char *)gameclient.languages->languages[i]->filename) == 0;
				
				ui_do_label(&button, (const char *)gameclient.languages->languages[i]->name, 14.0f, 0);
				
				if(ui_do_button(gameclient.languages->languages[i], "", selected, &button, ui_draw_list_row, 0))
				{
					mem_copy(config.language, (const char *)gameclient.languages->languages[i]->filename, sizeof(config.language));
				}
					
				ui_hsplit_t(&list, 20, &button, &list);
			}
		}
	}
		
	// draw header
	RECT header, footer;
	ui_hsplit_t(&skinselection, 20, &header, &skinselection);
	ui_draw_rect(&header, vec4(1,1,1,0.25f), CORNER_T, 5.0f); 
	ui_do_label(&header, _t("Skins"), 18.0f, 0);

	// draw footers	
	ui_hsplit_b(&skinselection, 20, &skinselection, &footer);
	ui_draw_rect(&footer, vec4(1,1,1,0.25f), CORNER_B, 5.0f); 
	ui_vsplit_l(&footer, 10.0f, 0, &footer);

	// modes
	ui_draw_rect(&skinselection, vec4(0,0,0,0.15f), 0, 0);

	RECT scroll;
	ui_vsplit_r(&skinselection, 15, &skinselection, &scroll);

	RECT list = skinselection;
	ui_hsplit_t(&list, 50, &button, &list);
	
	int num = (int)(skinselection.h/button.h);
	static float scrollvalue = 0;
	static int scrollbar = 0;
	ui_hmargin(&scroll, 5.0f, &scroll);
	scrollvalue = ui_do_scrollbar_v(&scrollbar, &scroll, scrollvalue);

	int start = (int)((gameclient.skins->num()-num)*scrollvalue);
	if(start < 0)
		start = 0;
		
	for(int i = start; i < start+num && i < gameclient.skins->num(); i++)
	{
		const SKINS::SKIN *s = gameclient.skins->get(i);
		
		// no special skins
		if(s->name[0] == 'x' && s->name[1] == '_')
		{
			num++;
			continue;
		}
		if(config.cl_default_skin_only && strcmp(s->name, "default") != 0)
		{
			num++;
			continue;
		}
		
		char buf[128];
		str_format(buf, sizeof(buf), "%s", s->name);
		int selected = 0;
		if(strcmp(s->name, config.player_skin) == 0)
			selected = 1;
		
		TEE_RENDER_INFO info;
		info.texture = s->org_texture;
		info.color_body = vec4(1,1,1,1);
		info.color_feet = vec4(1,1,1,1);
		if(config.player_use_custom_color)
		{
			info.color_body = gameclient.skins->get_color(config.player_color_body);
			info.color_feet = gameclient.skins->get_color(config.player_color_feet);
			info.texture = s->color_texture;
		}
			
		info.size = ui_scale()*50.0f;
		
		RECT icon;
		RECT text;
		ui_vsplit_l(&button, 50.0f, &icon, &text);
		
		if(ui_do_button(s, "", selected, &button, ui_draw_list_row, 0))
		{
			config_set_player_skin(&config, s->name);
			need_sendinfo = true;
		}

		ui_hsplit_t(&text, 12.0f, 0, &text); // some margin from the top
		ui_do_label(&text, buf, 18.0f, 0);
		
		ui_hsplit_t(&icon, 5.0f, 0, &icon); // some margin from the top
		render_tee(ANIMSTATE::get_idle(), &info, 0, vec2(1, 0), vec2(icon.x+icon.w/2, icon.y+icon.h/2));
		
		if(config.debug)
		{
			gfx_texture_set(-1);
			gfx_quads_begin();
			gfx_setcolor(s->blood_color.r, s->blood_color.g, s->blood_color.b, 1.0f);
			gfx_quads_drawTL(icon.x, icon.y, 12, 12);
			gfx_quads_end();
		}
		
		ui_hsplit_t(&list, 50, &button, &list);
	}
}

typedef void (*assign_func_callback)(CONFIGURATION *config, int value);

typedef struct 
{
	const char *name;
	const char *command;
	int keyid;
} KEYINFO;

KEYINFO keys[] = 
{
	{ "Move Left:", "+left", 0},
	{ "Move Right:", "+right", 0 },
	{ "Jump:", "+jump", 0 },
	{ "Fire:", "+fire", 0 },
	{ "Hook:", "+hook", 0 },
	{ "Hammer:", "+weapon1", 0 },
	{ "Pistol:", "+weapon2", 0 },
	{ "Shotgun:", "+weapon3", 0 },
	{ "Grenade:", "+weapon4", 0 },
	{ "Rifle:", "+weapon5", 0 },
	{ "Next Weapon:", "+nextweapon", 0 },
	{ "Prev. Weapon:", "+prevweapon", 0 },
	{ "Vote Yes:", "vote yes", 0 },
	{ "Vote No:", "vote no", 0 },
	{ "Chat:", "chat all", 0 },
	{ "Team Chat:", "chat team", 0 },
	{ "Emoticon:", "+emote", 0 },
	{ "Console:", "toggle_local_console", 0 },
	{ "Remote Console:", "toggle_remote_console", 0 },
	{ "Screenshot:", "screenshot", 0 },
	{ "Scoreboard:", "+scoreboard", 0 },
	{ "Fast menu:", "+fastmenu", 0 },
};

const int key_count = sizeof(keys) / sizeof(KEYINFO);

void MENUS::ui_do_getbuttons(int start, int stop, RECT view)
{
	for (int i = start; i < stop; i++)
	{
		KEYINFO key = keys[i];
		RECT button, label;
		ui_hsplit_t(&view, 20.0f, &button, &view);
		ui_vsplit_l(&button, 130.0f, &label, &button);
	
		ui_do_label(&label, _t(key.name), 14.0f, -1);
		int oldid = key.keyid;
		int newid = ui_do_key_reader((void *)keys[i].name, &button, oldid);
		if(newid != oldid)
		{
			gameclient.binds->bind(oldid, "");
			gameclient.binds->bind(newid, keys[i].command);
		}
		ui_hsplit_t(&view, 5.0f, 0, &view);
	}
}

void MENUS::render_settings_controls(RECT main_view)
{
	// this is kinda slow, but whatever
	for(int i = 0; i < key_count; i++)
		keys[i].keyid = 0;
	
	for(int keyid = 0; keyid < KEY_LAST; keyid++)
	{
		const char *bind = gameclient.binds->get(keyid);
		if(!bind[0])
			continue;
		
		for(int i = 0; i < key_count; i++)
			if(strcmp(bind, keys[i].command) == 0)
			{
				keys[i].keyid = keyid;
				break;
			}
	}

	RECT movement_settings, weapon_settings, voting_settings, chat_settings, misc_settings, reset_button;
	ui_vsplit_l(&main_view, main_view.w/2-5.0f, &movement_settings, &voting_settings);
	
	/* movement settings */
	{
		ui_hsplit_t(&movement_settings, main_view.h/2-5.0f, &movement_settings, &weapon_settings);
		ui_draw_rect(&movement_settings, vec4(1,1,1,0.25f), CORNER_ALL, 10.0f);
		ui_margin(&movement_settings, 10.0f, &movement_settings);
		
		gfx_text(0, movement_settings.x, movement_settings.y, 14, _t("Movement"), -1);
		
		ui_hsplit_t(&movement_settings, 14.0f+5.0f+10.0f, 0, &movement_settings);
		
		{
			RECT button, label;
			ui_hsplit_t(&movement_settings, 20.0f, &button, &movement_settings);
			ui_vsplit_l(&button, 130.0f, &label, &button);
			ui_do_label(&label, _t("Mouse sens."), 14.0f, -1);
			ui_hmargin(&button, 2.0f, &button);
			config.inp_mousesens = (int)(ui_do_scrollbar_h(&config.inp_mousesens, &button, (config.inp_mousesens-5)/500.0f)*500.0f)+5;
			//*key.key = ui_do_key_reader(key.key, &button, *key.key);
			ui_hsplit_t(&movement_settings, 20.0f, 0, &movement_settings);
		}
		
		ui_do_getbuttons(0, 5, movement_settings);

	}
	
	/* weapon settings */
	{
		ui_hsplit_t(&weapon_settings, 10.0f, 0, &weapon_settings);
		ui_hsplit_t(&weapon_settings, main_view.h/2-5.0f-45.0f, &weapon_settings, &reset_button);
		ui_draw_rect(&weapon_settings, vec4(1,1,1,0.25f), CORNER_ALL, 10.0f);
		ui_margin(&weapon_settings, 10.0f, &weapon_settings);

		gfx_text(0, weapon_settings.x, weapon_settings.y, 14, _t("Weapon"), -1);
		
		ui_hsplit_t(&weapon_settings, 14.0f+5.0f+10.0f, 0, &weapon_settings);
		ui_do_getbuttons(5, 12, weapon_settings);
	}
	
	/* voting settings */
	{
		ui_vsplit_l(&voting_settings, 10.0f, 0, &voting_settings);
		ui_hsplit_t(&voting_settings, main_view.h/4-5.0f, &voting_settings, &chat_settings);
		ui_draw_rect(&voting_settings, vec4(1,1,1,0.25f), CORNER_ALL, 10.0f);
		ui_margin(&voting_settings, 10.0f, &voting_settings);
	
		gfx_text(0, voting_settings.x, voting_settings.y, 14, _t("Voting"), -1);
		
		ui_hsplit_t(&voting_settings, 14.0f+5.0f+10.0f, 0, &voting_settings);
		ui_do_getbuttons(12, 14, voting_settings);
	}
	
	/* chat settings */
	{
		ui_hsplit_t(&chat_settings, 10.0f, 0, &chat_settings);
		ui_hsplit_t(&chat_settings, main_view.h/4-10.0f, &chat_settings, &misc_settings);
		ui_draw_rect(&chat_settings, vec4(1,1,1,0.25f), CORNER_ALL, 10.0f);
		ui_margin(&chat_settings, 10.0f, &chat_settings);
	
		gfx_text(0, chat_settings.x, chat_settings.y, 14, _t("Chat"), -1);
		
		ui_hsplit_t(&chat_settings, 14.0f+5.0f+10.0f, 0, &chat_settings);
		ui_do_getbuttons(14, 16, chat_settings);
	}
	
	/* misc settings */
	{
		ui_hsplit_t(&misc_settings, 10.0f, 0, &misc_settings);
		//ui_hsplit_t(&misc_settings, main_view.h/2-5.0f-45.0f, &misc_settings, &reset_button);
		ui_draw_rect(&misc_settings, vec4(1,1,1,0.25f), CORNER_ALL, 10.0f);
		ui_margin(&misc_settings, 10.0f, &misc_settings);
	
		gfx_text(0, misc_settings.x, misc_settings.y, 14, _t("Miscellaneous"), -1);
		
		ui_hsplit_t(&misc_settings, 14.0f+5.0f+10.0f, 0, &misc_settings);
		ui_do_getbuttons(16, 22, misc_settings);
	}
	
	// defaults
	ui_hsplit_t(&reset_button, 10.0f, 0, &reset_button);
	static int default_button = 0;
	if (ui_do_button((void*)&default_button, _t("Reset to defaults"), 0, &reset_button, ui_draw_menu_button, 0))
		gameclient.binds->set_defaults();
}

void MENUS::render_settings_graphics(RECT main_view)
{
	RECT button;
	char buf[128];
	
	static const int MAX_RESOLUTIONS = 256;
	static VIDEO_MODE modes[MAX_RESOLUTIONS];
	static int num_modes = -1;
	
	if(num_modes == -1)
		num_modes = gfx_get_video_modes(modes, MAX_RESOLUTIONS);
	
	RECT modelist;
	ui_vsplit_l(&main_view, 300.0f, &main_view, &modelist);
	
	// draw allmodes switch
	RECT header, footer;
	ui_hsplit_t(&modelist, 20, &button, &modelist);
	if (ui_do_button(&config.gfx_display_all_modes, _t("Show only supported"), config.gfx_display_all_modes^1, &button, ui_draw_checkbox, 0))
	{
		config.gfx_display_all_modes ^= 1;
		num_modes = gfx_get_video_modes(modes, MAX_RESOLUTIONS);
	}
	
	// draw header
	ui_hsplit_t(&modelist, 20, &header, &modelist);
	ui_draw_rect(&header, vec4(1,1,1,0.25f), CORNER_T, 5.0f); 
	ui_do_label(&header, _t("Display Modes"), 14.0f, 0);

	// draw footers	
	ui_hsplit_b(&modelist, 20, &modelist, &footer);
	str_format(buf, sizeof(buf), _t("Current: %dx%d %d bit"), config.gfx_screen_width, config.gfx_screen_height, config.gfx_color_depth);
	ui_draw_rect(&footer, vec4(1,1,1,0.25f), CORNER_B, 5.0f); 
	ui_vsplit_l(&footer, 10.0f, 0, &footer);
	ui_do_label(&footer, buf, 14.0f, -1);

	// modes
	ui_draw_rect(&modelist, vec4(0,0,0,0.15f), 0, 0);

	RECT scroll;
	ui_vsplit_r(&modelist, 15, &modelist, &scroll);

	RECT list = modelist;
	ui_hsplit_t(&list, 20, &button, &list);
	
	int num = (int)(modelist.h/button.h);
	static float scrollvalue = 0;
	static int scrollbar = 0;
	ui_hmargin(&scroll, 5.0f, &scroll);
	scrollvalue = ui_do_scrollbar_v(&scrollbar, &scroll, scrollvalue);

	int start = (int)((num_modes-num)*scrollvalue);
	if(start < 0)
		start = 0;
		
	for(int i = start; i < start+num && i < num_modes; i++)
	{
		int depth = modes[i].red+modes[i].green+modes[i].blue;
		if(depth < 16)
			depth = 16;
		else if(depth > 16)
			depth = 24;
			
		int selected = 0;
		if(config.gfx_color_depth == depth &&
			config.gfx_screen_width == modes[i].width &&
			config.gfx_screen_height == modes[i].height)
		{
			selected = 1;
		}
		
		str_format(buf, sizeof(buf), _t("  %dx%d %d bit"), modes[i].width, modes[i].height, depth);
		if(ui_do_button(&modes[i], buf, selected, &button, ui_draw_list_row, 0))
		{
			config.gfx_color_depth = depth;
			config.gfx_screen_width = modes[i].width;
			config.gfx_screen_height = modes[i].height;
			if(!selected)
				need_restart = true;
		}
		
		ui_hsplit_t(&list, 20, &button, &list);
	}
	
	
	// switches
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_fullscreen, _t("Fullscreen"), config.gfx_fullscreen, &button, ui_draw_checkbox, 0))
	{
		config.gfx_fullscreen ^= 1;
		need_restart = true;
	}

	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_vsync, _t("V-Sync"), config.gfx_vsync, &button, ui_draw_checkbox, 0))
	{
		config.gfx_vsync ^= 1;
		need_restart = true;
	}

	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_fsaa_samples, _t("FSAA samples"), config.gfx_fsaa_samples, &button, ui_draw_checkbox_number, 0))
	{
		config.gfx_fsaa_samples = (config.gfx_fsaa_samples+1)%17;
		need_restart = true;
	}
		
	ui_hsplit_t(&main_view, 40.0f, &button, &main_view);
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_texture_quality, _t("Quality Textures"), config.gfx_texture_quality, &button, ui_draw_checkbox, 0))
	{
		config.gfx_texture_quality ^= 1;
		need_restart = true;
	}

	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_texture_compression, _t("Texture Compression"), config.gfx_texture_compression, &button, ui_draw_checkbox, 0))
	{
		config.gfx_texture_compression ^= 1;
		need_restart = true;
	}

	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_high_detail, _t("High Detail"), config.gfx_high_detail, &button, ui_draw_checkbox, 0))
		config.gfx_high_detail ^= 1;

	//
	
	RECT text;
	ui_hsplit_t(&main_view, 20.0f, 0, &main_view);
	ui_hsplit_t(&main_view, 20.0f, &text, &main_view);
	//ui_vsplit_l(&text, 15.0f, 0, &text);
	ui_do_label(&text, _t("UI Color"), 14.0f, -1);
	
	const char *labels[] = {"Hue", "Sat.", "Lht.", "Alpha"};
	labels[0] = _t("Hue");
	labels[1] = _t("Sat.");
	labels[2] = _t("Lht.");
	labels[3] = _t("Alpha");
	int *color_slider[4] = {&config.ui_color_hue, &config.ui_color_sat, &config.ui_color_lht, &config.ui_color_alpha};
	for(int s = 0; s < 4; s++)
	{
		RECT text;
		ui_hsplit_t(&main_view, 19.0f, &button, &main_view);
		ui_vmargin(&button, 15.0f, &button);
		ui_vsplit_l(&button, 50.0f, &text, &button);
		ui_vsplit_r(&button, 5.0f, &button, 0);
		ui_hsplit_t(&button, 4.0f, 0, &button);
		
		float k = (*color_slider[s]) / 255.0f;
		k = ui_do_scrollbar_h(color_slider[s], &button, k);
		*color_slider[s] = (int)(k*255.0f);
		ui_do_label(&text, labels[s], 15.0f, -1);
	}		
	
	ui_hsplit_t(&main_view, 40.0f, &button, &main_view);
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.ui_new_background, _t("New background"), config.ui_new_background, &button, ui_draw_checkbox, 0))
		config.ui_new_background ^= 1;
	if (config.ui_new_background)
	{
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.ui_new_background_type, _t("New background type"), config.ui_new_background_type + 1, &button, ui_draw_checkbox_number, 0))
		{
			config.ui_new_background_type = (config.ui_new_background_type + 1)%4;
		}
	}
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_freetype_font, _t("Use FreeType for displaying text"), config.gfx_freetype_font, &button, ui_draw_checkbox, 0))
		config.gfx_freetype_font ^= 1;
		
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_smileys, _t("Smileys"), config.gfx_smileys, &button, ui_draw_checkbox, 0))
		config.gfx_smileys ^= 1;
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_text_shadows, _t("Shadows for text"), config.gfx_text_shadows, &button, ui_draw_checkbox, 0))
		config.gfx_text_shadows ^= 1;
		
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_shadows, _t("Shadows"), config.gfx_shadows, &button, ui_draw_checkbox, 0))
		config.gfx_shadows ^= 1;
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_outlines, _t("Outlines"), config.gfx_outlines, &button, ui_draw_checkbox, 0))
		config.gfx_outlines ^= 1;
		
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.gfx_eyecandy, _t("Eye candy"), config.gfx_eyecandy, &button, ui_draw_checkbox, 0))
		config.gfx_eyecandy ^= 1;
}

void MENUS::render_settings_sound(RECT main_view)
{
	RECT button;
	ui_vsplit_l(&main_view, 300.0f, &main_view, 0);
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.snd_enable, _t("Use Sounds"), config.snd_enable, &button, ui_draw_checkbox, 0))
	{
		config.snd_enable ^= 1;
		need_restart = true;
	}
	
	if(!config.snd_enable)
		return;
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.snd_nonactive_mute, _t("Mute when not active"), config.snd_nonactive_mute, &button, ui_draw_checkbox, 0))
		config.snd_nonactive_mute ^= 1;
		
	// sample rate box
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%d", config.snd_rate);
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		ui_do_label(&button, _t("Sample Rate"), 14.0f, -1);
		ui_vsplit_l(&button, 110.0f, 0, &button);
		ui_vsplit_l(&button, 180.0f, &button, 0);
		ui_do_edit_box(&config.snd_rate, &button, buf, sizeof(buf), 14.0f);
		int before = config.snd_rate;
		config.snd_rate = atoi(buf);
		
		if(config.snd_rate != before)
			need_restart = true;

		if(config.snd_rate < 1)
			config.snd_rate = 1;
	}
	
	// volume slider
	{
		RECT button, label;
		ui_hsplit_t(&main_view, 5.0f, &button, &main_view);
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		ui_vsplit_l(&button, 170.0f, &label, &button);
		ui_hmargin(&button, 2.0f, &button);
		ui_do_label(&label, _t("Sound Volume"), 14.0f, -1);
		config.snd_volume = (int)(ui_do_scrollbar_h(&config.snd_volume, &button, config.snd_volume/100.0f)*100.0f);
		ui_hsplit_t(&main_view, 20.0f, 0, &main_view);
	}
	
	{
		RECT button, label;
		ui_hsplit_t(&main_view, 5.0f, &button, &main_view);
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		ui_vsplit_l(&button, 170.0f, &label, &button);
		ui_hmargin(&button, 2.0f, &button);
		ui_do_label(&label, _t("Music Volume"), 14.0f, -1);
		config.music_volume = (int)(ui_do_scrollbar_h(&config.music_volume, &button, config.music_volume/100.0f)*100.0f);
		ui_hsplit_t(&main_view, 20.0f, 0, &main_view);
	}
}

void MENUS::render_settings_beep(RECT main_view)
{
	RECT button;
	
	ui_vsplit_l(&main_view, 300.0f, &main_view, 0);
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.cl_change_sound, _t("Change chat sound"), config.cl_change_sound, &button, ui_draw_checkbox, 0))
		config.cl_change_sound ^= 1;
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.cl_change_color, _t("Change color of chat messages"), config.cl_change_color, &button, ui_draw_checkbox, 0))
		config.cl_change_color ^= 1;
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.cl_block_spammer, _t("Block all messages from spaming people"), config.cl_block_spammer, &button, ui_draw_checkbox, 0))
		config.cl_block_spammer ^= 1;
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.cl_anti_spam, _t("Block spam"), config.cl_anti_spam, &button, ui_draw_checkbox, 0))
		config.cl_anti_spam ^= 1;
		
	ui_hsplit_t(&main_view, 10.0f, &button, &main_view);
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	ui_do_label(&button, _t("Search for names:"), 14.0, -1);
	ui_vsplit_l(&button, 140.0f, 0, &button);
	ui_vsplit_l(&button, 380.0f, &button, 0);
	ui_do_edit_box(config.cl_search_name, &button, config.cl_search_name, sizeof(config.cl_search_name), 14.0f);
	
	ui_hsplit_t(&main_view, 5.0f, &button, &main_view);
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	ui_do_label(&button, _t("Spammer names:"), 14.0, -1);
	ui_vsplit_l(&button, 140.0f, 0, &button);
	ui_vsplit_l(&button, 380.0f, &button, 0);
	ui_do_edit_box(config.cl_spammer_name, &button, config.cl_spammer_name, sizeof(config.cl_spammer_name), 14.0f);
 
	// information text
	ui_hsplit_b(&main_view, 25.0f, &main_view, &button);
	ui_do_label(&button, _t("Enter the names u want to look for.\nSeperate them with a simple space."), 14.0f, -1);

	ui_hsplit_b(&main_view, -10.0f, &main_view, &button);
	ui_vsplit_l(&button, main_view.w*4.1, 0, &button);
	ui_do_label(&button, _t("by Sushi :)"), 10.0f, 0);
}

void MENUS::render_settings_hudmod(RECT main_view)
{
	RECT button, text, right_view;
	ui_vsplit_l(&main_view, 300.0f, &main_view, &right_view);
	
	// general settings
	ui_hsplit_t(&main_view, 15.0f, &text, &main_view);
	ui_do_label(&text, _t("General settings"), 14.0f, -1);
		
	ui_hsplit_t(&main_view, 10.0f, &button, &main_view);
	
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.cl_clear_hud, _t("Clear hud"), config.cl_clear_hud, &button, ui_draw_checkbox, 0))
		config.cl_clear_hud ^= 1;
	ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
	if (ui_do_button(&config.cl_clear_all, _t("Clear all"), config.cl_clear_all, &button, ui_draw_checkbox, 0))
		config.cl_clear_all ^= 1;
		
	ui_hsplit_t(&main_view, 40.0f, &button, &main_view);
 
	// special settings
	if(!config.cl_clear_all)
	{
		ui_hsplit_t(&main_view, 15.0f, &text, &main_view);
		ui_do_label(&text, _t("Special settings"), 14.0f, -1);
		
		ui_hsplit_t(&main_view, 10.0f, &button, &main_view);
		
		if(!config.cl_clear_hud)
		{
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_render_time, _t("Server time"), config.cl_render_time, &button, ui_draw_checkbox, 0))
				config.cl_render_time ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);	
			if (ui_do_button(&config.cl_render_hp, _t("Health"), config.cl_render_hp, &button, ui_draw_checkbox, 0))
				config.cl_render_hp ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_render_ammo, _t("Ammunition"), config.cl_render_ammo, &button, ui_draw_checkbox, 0))
				config.cl_render_ammo ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_render_crosshair, _t("Crosshair"), config.cl_render_crosshair, &button, ui_draw_checkbox, 0))
				config.cl_render_crosshair ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_render_score, _t("Team score"), config.cl_render_score, &button, ui_draw_checkbox, 0))
				config.cl_render_score ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_render_viewmode, _t("Viewmode"), config.cl_render_viewmode, &button, ui_draw_checkbox, 0))
				config.cl_render_viewmode ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_render_infomsg, _t("Info messages"), config.cl_render_infomsg, &button, ui_draw_checkbox, 0))
				config.cl_render_infomsg ^= 1;
			ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
			if (ui_do_button(&config.cl_showfps, _t("FPS"), config.cl_showfps, &button, ui_draw_checkbox, 0))
				config.cl_showfps ^= 1;
		}
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_scoreboard, _t("Scoreboard"), config.cl_render_scoreboard, &button, ui_draw_checkbox, 0))
			config.cl_render_scoreboard ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_warmup, _t("Warmup"), config.cl_render_warmup, &button, ui_draw_checkbox, 0))
			config.cl_render_warmup ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_broadcast, _t("Broadcast"), config.cl_render_broadcast, &button, ui_draw_checkbox, 0))
			config.cl_render_broadcast ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_servermsg, _t("Server messages"), config.cl_render_servermsg, &button, ui_draw_checkbox, 0))
			config.cl_render_servermsg ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_chat, _t("Chat"), config.cl_render_chat, &button, ui_draw_checkbox, 0))
			config.cl_render_chat ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_kill, _t("Kill messages"), config.cl_render_kill, &button, ui_draw_checkbox, 0))
			config.cl_render_kill ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_render_vote, _t("Votes"), config.cl_render_vote, &button, ui_draw_checkbox, 0))
			config.cl_render_vote ^= 1;
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		if (ui_do_button(&config.cl_warning_teambalance, _t("Team balance warning"), config.cl_warning_teambalance, &button, ui_draw_checkbox, 0))
			config.cl_warning_teambalance ^= 1;
	}
	
	// sound settings
	ui_hsplit_t(&right_view, 15.0f, &text, &right_view);
	ui_do_label(&text, _t("Sound settings"), 14.0f, -1);
	
	ui_hsplit_t(&right_view, 10.0f, &button, &right_view);

	ui_hsplit_t(&right_view, 20.0f, &button, &right_view);
	if (ui_do_button(&config.cl_servermsgsound, _t("Activate server message sound"), config.cl_servermsgsound, &button, ui_draw_checkbox, 0))
		config.cl_servermsgsound ^= 1;
	ui_hsplit_t(&right_view, 20.0f, &button, &right_view);
	if (ui_do_button(&config.cl_chatsound, _t("Activate chat message sound"), config.cl_chatsound, &button, ui_draw_checkbox, 0))
		config.cl_chatsound ^= 1;
	
	// default button
	ui_hsplit_b(&main_view, 20.0f, 0, &button);
	static int default_button = 0;
	if(ui_do_button((void*)&default_button, _t("Reset to defaults"), 0, &button, ui_draw_menu_button, 0))
	{
		config.cl_render_time = 1;
		config.cl_render_warmup = 1;
		config.cl_render_broadcast = 1;
		config.cl_render_hp = 1;
		config.cl_render_ammo = 1;
		config.cl_render_crosshair = 1;
		config.cl_render_score = 1;
		config.cl_showfps = 0;
		config.cl_render_viewmode = 1;
		config.cl_render_infomsg = 1;
		config.cl_render_scoreboard = 1;
		config.cl_render_servermsg = 1;
		config.cl_render_chat = 1;
		config.cl_render_kill = 1;
		config.cl_render_vote = 1;
		config.cl_clear_hud = 0;
		config.cl_clear_all = 0;
		config.cl_servermsgsound = 1;
		config.cl_chatsound = 1;
		config.cl_warning_teambalance = 1;
	}
	
	// information text
	ui_hsplit_b(&main_view, 15.0f, &main_view, &button);
	ui_vsplit_l(&button, main_view.w*4.1, 0, &button);
	ui_do_label(&button, "by Sushi :)", 10.0f, 0);
}

	/*
static void menu2_render_settings_network(RECT main_view)
{
	RECT button;
	ui_vsplit_l(&main_view, 300.0f, &main_view, 0);
	
	{
		ui_hsplit_t(&main_view, 20.0f, &button, &main_view);
		ui_do_label(&button, _t("Rcon Password"), 14.0, -1);
		ui_vsplit_l(&button, 110.0f, 0, &button);
		ui_vsplit_l(&button, 180.0f, &button, 0);
		ui_do_edit_box(&config.rcon_password, &button, config.rcon_password, sizeof(config.rcon_password), true);
	}
}*/

void MENUS::render_settings(RECT main_view)
{
	static int settings_page = 0;
	
	// render background
	RECT temp, tabbar;
	ui_vsplit_r(&main_view, 120.0f, &main_view, &tabbar);
	ui_draw_rect(&main_view, color_tabbar_active, CORNER_B|CORNER_TL, 10.0f);
	ui_hsplit_t(&tabbar, 50.0f, &temp, &tabbar);
	ui_draw_rect(&temp, color_tabbar_active, CORNER_R, 10.0f);
	
	ui_hsplit_t(&main_view, 10.0f, 0, &main_view);
	
	RECT button;
	
	const char *tabs[] = {"Player", "Controls", "Graphics", "Sound", "Beep", "Hud-Mod"};
	tabs[0] = _t("Player");
	tabs[1] = _t("Controls");
	tabs[2] = _t("Graphics");
	tabs[3] = _t("Sound");
	int num_tabs = (int)(sizeof(tabs)/sizeof(*tabs));

	for(int i = 0; i < num_tabs; i++)
	{
		ui_hsplit_t(&tabbar, 10, &button, &tabbar);
		ui_hsplit_t(&tabbar, 26, &button, &tabbar);
		if(ui_do_button(tabs[i], tabs[i], settings_page == i, &button, ui_draw_settings_tab_button, 0))
			settings_page = i;
	}
	
	ui_margin(&main_view, 10.0f, &main_view);
	
	if(settings_page == 0)
		render_settings_player(main_view);
	else if(settings_page == 1)
		render_settings_controls(main_view);
	else if(settings_page == 2)
		render_settings_graphics(main_view);
	else if(settings_page == 3)
		render_settings_sound(main_view);
	else if(settings_page == 4)
		render_settings_beep(main_view);
	else if(settings_page == 5)
		render_settings_hudmod(main_view);

	if(need_restart)
	{
		RECT restart_warning;
		ui_hsplit_b(&main_view, 40, &main_view, &restart_warning);
		ui_do_label(&restart_warning, _t("You must restart the game for all settings to take effect."), 15.0f, -1, 220);
	}
}
