/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <base/system.h>
#include <base/math.hpp>
#include <base/vmath.hpp>

#include "menus.hpp"
#include "skins.hpp"

#include <engine/e_client_interface.h>

extern "C" {
	#include <engine/client/ec_font.h>
}

#include <game/version.hpp>
#include <game/generated/g_protocol.hpp>

#include <game/generated/gc_data.hpp>
#include <game/client/gameclient.hpp>
#include <mastersrv/mastersrv.h>

vec4 MENUS::gui_color;
vec4 MENUS::color_tabbar_inactive_outgame;
vec4 MENUS::color_tabbar_active_outgame;
vec4 MENUS::color_tabbar_inactive;
vec4 MENUS::color_tabbar_active;
vec4 MENUS::color_tabbar_inactive_ingame;
vec4 MENUS::color_tabbar_active_ingame;


float MENUS::button_height = 25.0f;
float MENUS::listheader_height = 17.0f;
float MENUS::fontmod_height = 0.8f;

INPUT_EVENT MENUS::inputevents[MAX_INPUTEVENTS];
int MENUS::num_inputevents;

inline float hue_to_rgb(float v1, float v2, float h)
{
   if(h < 0) h += 1;
   if(h > 1) h -= 1;
   if((6 * h) < 1) return v1 + ( v2 - v1 ) * 6 * h;
   if((2 * h) < 1) return v2;
   if((3 * h) < 2) return v1 + ( v2 - v1 ) * ((2.0f/3.0f) - h) * 6;
   return v1;
}

inline vec3 hsl_to_rgb(vec3 in)
{
	float v1, v2;
	vec3 out;

	if(in.s == 0)
	{
		out.r = in.l;
		out.g = in.l;
		out.b = in.l;
	}
	else
	{
		if(in.l < 0.5f) 
			v2 = in.l * (1 + in.s);
		else           
			v2 = (in.l+in.s) - (in.s*in.l);

		v1 = 2 * in.l - v2;

		out.r = hue_to_rgb(v1, v2, in.h + (1.0f/3.0f));
		out.g = hue_to_rgb(v1, v2, in.h);
		out.b = hue_to_rgb(v1, v2, in.h - (1.0f/3.0f));
	} 

	return out;
}


MENUS::MENUS()
{
	popup = POPUP_NONE;
	active_page = PAGE_INTERNET;
	game_page = PAGE_GAME;
	
	need_restart = false;
	need_sendinfo = false;
	menu_active = true;
	
	escape_pressed = false;
	enter_pressed = false;
	num_inputevents = 0;
	
	demos = 0;
	num_demos = 0;
	
	last_input = time_get();
}

vec4 MENUS::button_color_mul(const void *id)
{
	if(ui_active_item() == id)
		return vec4(1,1,1,0.5f);
	else if(ui_hot_item() == id)
		return vec4(1,1,1,1.5f);
	return vec4(1,1,1,1);
}

void MENUS::ui_draw_browse_icon(int what, const RECT *r)
{
	gfx_texture_set(data->images[IMAGE_BROWSEICONS].id);
	gfx_quads_begin();
	select_sprite(what);
	gfx_quads_drawTL(r->x,r->y,r->w,r->h);
	gfx_quads_end();
}


void MENUS::ui_draw_menu_button(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	ui_draw_rect(r, vec4(1,1,1,0.5f)*button_color_mul(id), CORNER_ALL, 5.0f);
	ui_do_label(r, text, r->h*fontmod_height, 0);
}

void MENUS::ui_draw_keyselect_button(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	ui_draw_rect(r, vec4(1,1,1,0.5f)*button_color_mul(id), CORNER_ALL, 5.0f);
	ui_do_label(r, text, r->h*fontmod_height, 0);
}

void MENUS::ui_draw_menu_tab_button(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	int corners = CORNER_T;
	vec4 colormod(1,1,1,1);
	if(extra)
		corners = *(int *)extra;
	
	if(checked)
		ui_draw_rect(r, color_tabbar_active, corners, 10.0f);
	else
		ui_draw_rect(r, color_tabbar_inactive, corners, 10.0f);
	ui_do_label(r, text, r->h*fontmod_height, 0);
}


void MENUS::ui_draw_settings_tab_button(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	if(checked)
		ui_draw_rect(r, color_tabbar_active, CORNER_R, 10.0f);
	else
		ui_draw_rect(r, color_tabbar_inactive, CORNER_R, 10.0f);
	ui_do_label(r, text, r->h*fontmod_height, 0);
}

void MENUS::ui_draw_grid_header(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	if(checked)
		ui_draw_rect(r, vec4(1,1,1,0.5f), CORNER_T, 5.0f);
	RECT t;
	ui_vsplit_l(r, 5.0f, 0, &t);
	ui_do_label(&t, text, r->h*fontmod_height, -1);
}

void MENUS::ui_draw_list_row(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	if(checked)
	{
		RECT sr = *r;
		ui_margin(&sr, 1.5f, &sr);
		ui_draw_rect(&sr, vec4(1,1,1,0.5f), CORNER_ALL, 4.0f);
	}
	ui_do_label(r, text, r->h*fontmod_height, -1);
}

void MENUS::ui_draw_checkbox_common(const void *id, const char *text, const char *boxtext, const RECT *r)
{
	RECT c = *r;
	RECT t = *r;
	c.w = c.h;
	t.x += c.w;
	t.w -= c.w;
	ui_vsplit_l(&t, 5.0f, 0, &t);
	
	ui_margin(&c, 2.0f, &c);
	ui_draw_rect(&c, vec4(1,1,1,0.25f)*button_color_mul(id), CORNER_ALL, 3.0f);
	c.y += 2;
	ui_do_label(&c, boxtext, r->h*fontmod_height*0.75f, 0);
	ui_do_label(&t, text, r->h*fontmod_height*0.8f, -1);
}

void MENUS::ui_draw_checkbox(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	ui_draw_checkbox_common(id, text, checked?"X":"", r);
}


void MENUS::ui_draw_checkbox_number(const void *id, const char *text, int checked, const RECT *r, const void *extra)
{
	char buf[16];
	str_format(buf, sizeof(buf), "%d", checked);
	ui_draw_checkbox_common(id, text, buf, r);
}

int MENUS::ui_do_edit_box(void *id, const RECT *rect, char *str, int str_size, float font_size, bool hidden)
{
    int inside = ui_mouse_inside(rect);
	int r = 0;
	static int at_index = 0;

	if(ui_last_active_item() == id)
	{
		int len = strlen(str);
			
		if (inside && ui_mouse_button(0))
		{
			int mx_rel = (int)(ui_mouse_x() - rect->x);

			for (int i = 1; i <= len; i++)
			{
				if (gfx_text_width(0, font_size, str, i) + 10 > mx_rel)
				{
					at_index = i - 1;
					break;
				}

				if (i == len)
					at_index = len;
			}
		}

		for(int i = 0; i < num_inputevents; i++)
		{
			len = strlen(str);
			INPUT_EVENT e = inputevents[i];
			char c = e.ch;
			int k = e.key;

			if (at_index > len)
				at_index = len;
			
			if (!(c >= 0 && c < 32) && c != 127)
			{
				if (len < str_size - 1 && at_index < str_size - 1)
				{
					memmove(str + at_index + 1, str + at_index, len - at_index + 1);
					str[at_index] = c;
					at_index++;
					r = 1;
				}
			}
			
			if(e.flags&INPFLAG_PRESS)
			{
				if (k == KEY_BACKSPACE && at_index > 0)
				{
					memmove(str + at_index - 1, str + at_index, len - at_index + 1);
					at_index--;
					r = 1;
				}
				else if (k == KEY_DELETE && at_index < len)
				{
					memmove(str + at_index, str + at_index + 1, len - at_index);
					r = 1;
				}
				else if (k == KEY_RETURN)
					ui_clear_last_active_item();
				else if (k == KEY_LEFT && at_index > 0)
					at_index--;
				else if (k == KEY_RIGHT && at_index < len)
					at_index++;
				else if (k == KEY_HOME)
					at_index = 0;
				else if (k == KEY_END)
					at_index = len;
			}
		}
	}

	bool just_got_active = false;
	
	if(ui_active_item() == id)
	{
		if(!ui_mouse_button(0))
			ui_set_active_item(0);
	}
	else if(ui_hot_item() == id)
	{
		if(ui_mouse_button(0))
		{
			if (ui_last_active_item() != id)
				just_got_active = true;
			ui_set_active_item(id);
		}
	}
	
	if(inside)
		ui_set_hot_item(id);

	RECT textbox = *rect;
	ui_draw_rect(&textbox, vec4(1,1,1,0.5f), CORNER_ALL, 5.0f);
	ui_vmargin(&textbox, 5.0f, &textbox);
	
	const char *display_str = str;
	char stars[128];
	
	if(hidden)
	{
		unsigned s = strlen(str);
		if(s >= sizeof(stars))
			s = sizeof(stars)-1;
		memset(stars, '*', s);
		stars[s] = 0;
		display_str = stars;
	}

	ui_do_label(&textbox, display_str, font_size, -1);
	
	if (ui_last_active_item() == id && !just_got_active)
	{
		float w = gfx_text_width(0, font_size, display_str, at_index);
		textbox.x += w*ui_scale();
		ui_do_label(&textbox, "_", font_size, -1);
	}

	return r;
}

float MENUS::ui_do_scrollbar_v(const void *id, const RECT *rect, float current)
{
	RECT handle;
	static float offset_y;
	ui_hsplit_t(rect, 33, &handle, 0);

	handle.y += (rect->h-handle.h)*current;

	/* logic */
    float ret = current;
    int inside = ui_mouse_inside(&handle);

	if(ui_active_item() == id)
	{
		if(!ui_mouse_button(0))
			ui_set_active_item(0);
		
		float min = rect->y;
		float max = rect->h-handle.h;
		float cur = ui_mouse_y()-offset_y;
		ret = (cur-min)/max;
		if(ret < 0.0f) ret = 0.0f;
		if(ret > 1.0f) ret = 1.0f;
	}
	else if(ui_hot_item() == id)
	{
		if(ui_mouse_button(0))
		{
			ui_set_active_item(id);
			offset_y = ui_mouse_y()-handle.y;
		}
	}
	
	if(inside)
		ui_set_hot_item(id);

	// render
	RECT rail;
	ui_vmargin(rect, 5.0f, &rail);
	ui_draw_rect(&rail, vec4(1,1,1,0.25f), 0, 0.0f);

	RECT slider = handle;
	slider.w = rail.x-slider.x;
	ui_draw_rect(&slider, vec4(1,1,1,0.25f), CORNER_L, 2.5f);
	slider.x = rail.x+rail.w;
	ui_draw_rect(&slider, vec4(1,1,1,0.25f), CORNER_R, 2.5f);

	slider = handle;
	ui_margin(&slider, 5.0f, &slider);
	ui_draw_rect(&slider, vec4(1,1,1,0.25f)*button_color_mul(id), CORNER_ALL, 2.5f);
	
    return ret;
}



float MENUS::ui_do_scrollbar_h(const void *id, const RECT *rect, float current)
{
	RECT handle;
	static float offset_x;
	ui_vsplit_l(rect, 33, &handle, 0);

	handle.x += (rect->w-handle.w)*current;

	/* logic */
    float ret = current;
    int inside = ui_mouse_inside(&handle);

	if(ui_active_item() == id)
	{
		if(!ui_mouse_button(0))
			ui_set_active_item(0);
		
		float min = rect->x;
		float max = rect->w-handle.w;
		float cur = ui_mouse_x()-offset_x;
		ret = (cur-min)/max;
		if(ret < 0.0f) ret = 0.0f;
		if(ret > 1.0f) ret = 1.0f;
	}
	else if(ui_hot_item() == id)
	{
		if(ui_mouse_button(0))
		{
			ui_set_active_item(id);
			offset_x = ui_mouse_x()-handle.x;
		}
	}
	
	if(inside)
		ui_set_hot_item(id);

	// render
	RECT rail;
	ui_hmargin(rect, 5.0f, &rail);
	ui_draw_rect(&rail, vec4(1,1,1,0.25f), 0, 0.0f);

	RECT slider = handle;
	slider.h = rail.y-slider.y;
	ui_draw_rect(&slider, vec4(1,1,1,0.25f), CORNER_T, 2.5f);
	slider.y = rail.y+rail.h;
	ui_draw_rect(&slider, vec4(1,1,1,0.25f), CORNER_B, 2.5f);

	slider = handle;
	ui_margin(&slider, 5.0f, &slider);
	ui_draw_rect(&slider, vec4(1,1,1,0.25f)*button_color_mul(id), CORNER_ALL, 2.5f);
	
    return ret;
}

int MENUS::ui_do_key_reader(void *id, const RECT *rect, int key)
{
	// process
	static void *grabbed_id = 0;
	static bool mouse_released = true;
	int inside = ui_mouse_inside(rect);
	int new_key = key;
	
	if(!ui_mouse_button(0) && grabbed_id == id)
		mouse_released = true;

	if(ui_active_item() == id)
	{
		if(binder.got_key)
		{
			new_key = binder.key.key;
			binder.got_key = false;
			ui_set_active_item(0);
			mouse_released = false;
			grabbed_id = id;
		}
	}
	else if(ui_hot_item() == id)
	{
		if(ui_mouse_button(0) && mouse_released)
		{
			binder.take_key = true;
			binder.got_key = false;
			ui_set_active_item(id);
		}
	}
	
	if(inside)
		ui_set_hot_item(id);

	// draw
	if (ui_active_item() == id)
		ui_draw_keyselect_button(id, "???", 0, rect, 0);
	else
	{
		if(key == 0)
			ui_draw_keyselect_button(id, "", 0, rect, 0);
		else
			ui_draw_keyselect_button(id, inp_key_name(key), 0, rect, 0);
	}
	return new_key;
}


int MENUS::render_menubar(RECT r)
{
	RECT box = r;
	RECT button;
	
	int active_page = config.ui_page;
	int new_page = -1;
	
	if(client_state() != CLIENTSTATE_OFFLINE)
		active_page = game_page;
	
	if(client_state() == CLIENTSTATE_OFFLINE)
	{
		/* offline menus */
		if(0) // this is not done yet
		{
			ui_vsplit_l(&box, 90.0f, &button, &box);
			static int news_button=0;
			if (ui_do_button(&news_button, "News", active_page==PAGE_NEWS, &button, ui_draw_menu_tab_button, 0))
				new_page = PAGE_NEWS;
			ui_vsplit_l(&box, 30.0f, 0, &box); 
		}

		ui_vsplit_l(&box, 100.0f, &button, &box);
		static int internet_button=0;
		int corners = CORNER_TL;
		if (ui_do_button(&internet_button, "Internet", active_page==PAGE_INTERNET, &button, ui_draw_menu_tab_button, &corners))
		{
			client_serverbrowse_refresh(BROWSETYPE_INTERNET);
			new_page = PAGE_INTERNET;
		}

		//ui_vsplit_l(&box, 4.0f, 0, &box);
		ui_vsplit_l(&box, 80.0f, &button, &box);
		static int lan_button=0;
		corners = 0;
		if (ui_do_button(&lan_button, "LAN", active_page==PAGE_LAN, &button, ui_draw_menu_tab_button, &corners))
		{
			client_serverbrowse_refresh(BROWSETYPE_LAN);
			new_page = PAGE_LAN;
		}

		//ui_vsplit_l(&box, 4.0f, 0, &box);
		ui_vsplit_l(&box, 110.0f, &button, &box);
		static int favorites_button=0;
		corners = CORNER_TR;
		if (ui_do_button(&favorites_button, "Favorites", active_page==PAGE_FAVORITES, &button, ui_draw_menu_tab_button, &corners))
		{
			client_serverbrowse_refresh(BROWSETYPE_FAVORITES);
			new_page  = PAGE_FAVORITES;
		}
		
		ui_vsplit_l(&box, 4.0f*5, 0, &box);
		ui_vsplit_l(&box, 100.0f, &button, &box);
		static int demos_button=0;
		if (ui_do_button(&demos_button, "Demos", active_page==PAGE_DEMOS, &button, ui_draw_menu_tab_button, 0))
		{
			demolist_populate();
			new_page  = PAGE_DEMOS;
		}		
	}
	else
	{
		/* online menus */
		ui_vsplit_l(&box, 90.0f, &button, &box);
		static int game_button=0;
		if (ui_do_button(&game_button, "Game", active_page==PAGE_GAME, &button, ui_draw_menu_tab_button, 0))
			new_page = PAGE_GAME;

		ui_vsplit_l(&box, 4.0f, 0, &box);
		ui_vsplit_l(&box, 140.0f, &button, &box);
		static int server_info_button=0;
		if (ui_do_button(&server_info_button, "Server Info", active_page==PAGE_SERVER_INFO, &button, ui_draw_menu_tab_button, 0))
			new_page = PAGE_SERVER_INFO;

		ui_vsplit_l(&box, 4.0f, 0, &box);
		ui_vsplit_l(&box, 140.0f, &button, &box);
		static int callvote_button=0;
		if (ui_do_button(&callvote_button, "Call Vote", active_page==PAGE_CALLVOTE, &button, ui_draw_menu_tab_button, 0))
			new_page = PAGE_CALLVOTE;
			
		ui_vsplit_l(&box, 30.0f, 0, &box);
	}
		
	/*
	ui_vsplit_r(&box, 110.0f, &box, &button);
	static int system_button=0;
	if (ui_do_button(&system_button, "System", config.ui_page==PAGE_SYSTEM, &button, ui_draw_menu_tab_button, 0))
		config.ui_page = PAGE_SYSTEM;
		
	ui_vsplit_r(&box, 30.0f, &box, 0);
	*/
	
	ui_vsplit_r(&box, 90.0f, &box, &button);
	static int quit_button=0;
	if (ui_do_button(&quit_button, "Quit", 0, &button, ui_draw_menu_tab_button, 0))
		popup = POPUP_QUIT;

	ui_vsplit_r(&box, 10.0f, &box, &button);
	ui_vsplit_r(&box, 120.0f, &box, &button);
	static int settings_button=0;
	if (ui_do_button(&settings_button, "Settings", active_page==PAGE_SETTINGS, &button, ui_draw_menu_tab_button, 0))
		new_page = PAGE_SETTINGS;
	
	if(new_page != -1)
	{
		if(client_state() == CLIENTSTATE_OFFLINE)
			config.ui_page = new_page;
		else
			game_page = new_page;
	}
		
	return 0;
}

void MENUS::render_loading(float percent)
{
	// need up date this here to get correct
	vec3 rgb = hsl_to_rgb(vec3(config.ui_color_hue/255.0f, config.ui_color_sat/255.0f, config.ui_color_lht/255.0f));
	gui_color = vec4(rgb.r, rgb.g, rgb.b, config.ui_color_alpha/255.0f);
	
    RECT screen = *ui_screen();
	gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);
	
	render_background();

	float tw;

	float w = 700;
	float h = 200;
	float x = screen.w/2-w/2;
	float y = screen.h/2-h/2;

	gfx_blend_normal();

	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(0,0,0,0.50f);
	draw_round_rect(x, y, w, h, 40.0f);
	gfx_quads_end();


	const char *caption = "Loading";

	tw = gfx_text_width(0, 48.0f, caption, -1);
	RECT r;
	r.x = x;
	r.y = y+20;
	r.w = w;
	r.h = h;
	ui_do_label(&r, caption, 48.0f, 0, -1);

	gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_setcolor(1,1,1,0.75f);
	draw_round_rect(x+40, y+h-75, (w-80)*percent, 25, 5.0f);
	gfx_quads_end();

	gfx_swap();
}

void MENUS::render_news(RECT main_view)
{
	ui_draw_rect(&main_view, color_tabbar_active, CORNER_ALL, 10.0f);
}

void MENUS::init()
{
	if(config.cl_show_welcome)
		popup = POPUP_FIRST_LAUNCH;
	config.cl_show_welcome = 0;
}

int MENUS::render()
{
    RECT screen = *ui_screen();
	gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);

	static bool first = true;
	if(first)
	{
		if(config.ui_page == PAGE_INTERNET)
			client_serverbrowse_refresh(0);
		else if(config.ui_page == PAGE_LAN)
			client_serverbrowse_refresh(1);
		first = false;
	}
	
	if(client_state() == CLIENTSTATE_ONLINE)
	{
		color_tabbar_inactive = color_tabbar_inactive_ingame;
		color_tabbar_active = color_tabbar_active_ingame;
	}
	else
	{
		render_background();
		color_tabbar_inactive = color_tabbar_inactive_outgame;
		color_tabbar_active = color_tabbar_active_outgame;
	}
	
	RECT tab_bar;
	RECT main_view;

	// some margin around the screen
	ui_margin(&screen, 10.0f, &screen);
	
	if(popup == POPUP_NONE)
	{
		// do tab bar
		ui_hsplit_t(&screen, 24.0f, &tab_bar, &main_view);
		ui_vmargin(&tab_bar, 20.0f, &tab_bar);
		render_menubar(tab_bar);
		
		// news is not implemented yet
		if(config.ui_page <= PAGE_NEWS || config.ui_page > PAGE_SETTINGS || (client_state() == CLIENTSTATE_OFFLINE && config.ui_page >= PAGE_GAME && config.ui_page <= PAGE_CALLVOTE))
		{
			client_serverbrowse_refresh(BROWSETYPE_INTERNET);
			config.ui_page = PAGE_INTERNET;
		}
		
		// render current page
		if(client_state() != CLIENTSTATE_OFFLINE)
		{
			if(game_page == PAGE_GAME)
				render_game(main_view);
			else if(game_page == PAGE_SERVER_INFO)
				render_serverinfo(main_view);
			else if(game_page == PAGE_CALLVOTE)
				render_servercontrol(main_view);
			else if(game_page == PAGE_SETTINGS)
				render_settings(main_view);
		}
		else if(config.ui_page == PAGE_NEWS)
			render_news(main_view);
		else if(config.ui_page == PAGE_INTERNET)
			render_serverbrowser(main_view);
		else if(config.ui_page == PAGE_LAN)
			render_serverbrowser(main_view);
		else if(config.ui_page == PAGE_DEMOS)
			render_demolist(main_view);
		else if(config.ui_page == PAGE_FAVORITES)
			render_serverbrowser(main_view);
		else if(config.ui_page == PAGE_SETTINGS)
			render_settings(main_view);
	}
	else
	{
		// make sure that other windows doesn't do anything funnay!
		//ui_set_hot_item(0);
		//ui_set_active_item(0);
		char buf[128];
		const char *title = "";
		const char *extra_text = "";
		const char *button_text = "";
		int extra_align = 0;
		
		if(popup == POPUP_CONNECTING)
		{
			title = "Connecting to";
			extra_text = config.ui_server_address;  // TODO: query the client about the address
			button_text = "Abort";
			if(client_mapdownload_totalsize() > 0)
			{
				title = "Downloading map";
				str_format(buf, sizeof(buf), "%d/%d KiB", client_mapdownload_amount()/1024, client_mapdownload_totalsize()/1024);
				extra_text = buf;
			}
		}
		else if(popup == POPUP_DISCONNECTED)
		{
			title = "Disconnected";
			extra_text = client_error_string();
			button_text = "Ok";
			extra_align = -1;
		}
		else if(popup == POPUP_PURE)
		{
			title = "Disconnected";
			extra_text = "The server is running a non-standard tuning on a pure game mode.";
			button_text = "Ok";
			extra_align = -1;
		}
		else if(popup == POPUP_PASSWORD)
		{
			title = "Password Error";
			extra_text = client_error_string();
			button_text = "Try Again";
		}
		else if(popup == POPUP_QUIT)
		{
			title = "Quit";
			extra_text = "Are you sure that you want to quit?";
		}
		else if(popup == POPUP_FIRST_LAUNCH)
		{
			title = "Welcome to Teeworlds";
			extra_text =
			"As this is the first time you launch the game, please enter your nick name below. "
			"It's recommended that you check the settings to adjust them to your liking "
			"before joining a server.";
			button_text = "Ok";
			extra_align = -1;
		}
		
		RECT box, part;
		box = screen;
		ui_vmargin(&box, 150.0f, &box);
		ui_hmargin(&box, 150.0f, &box);
		
		// render the box
		ui_draw_rect(&box, vec4(0,0,0,0.5f), CORNER_ALL, 15.0f);
		 
		ui_hsplit_t(&box, 20.f, &part, &box);
		ui_hsplit_t(&box, 24.f, &part, &box);
		ui_do_label(&part, title, 24.f, 0);
		ui_hsplit_t(&box, 20.f, &part, &box);
		ui_hsplit_t(&box, 24.f, &part, &box);
		ui_vmargin(&part, 20.f, &part);
		
		if(extra_align == -1)
			ui_do_label(&part, extra_text, 20.f, -1, (int)part.w);
		else
			ui_do_label(&part, extra_text, 20.f, 0, -1);

		if(popup == POPUP_QUIT)
		{
			RECT yes, no;
			ui_hsplit_b(&box, 20.f, &box, &part);
			ui_hsplit_b(&box, 24.f, &box, &part);
			ui_vmargin(&part, 80.0f, &part);
			
			ui_vsplit_mid(&part, &no, &yes);
			
			ui_vmargin(&yes, 20.0f, &yes);
			ui_vmargin(&no, 20.0f, &no);

			static int button_abort = 0;
			if(ui_do_button(&button_abort, "No", 0, &no, ui_draw_menu_button, 0) || escape_pressed)
				popup = POPUP_NONE;

			static int button_tryagain = 0;
			if(ui_do_button(&button_tryagain, "Yes", 0, &yes, ui_draw_menu_button, 0) || enter_pressed)
				client_quit();
		}
		else if(popup == POPUP_PASSWORD)
		{
			RECT label, textbox, tryagain, abort;
			
			ui_hsplit_b(&box, 20.f, &box, &part);
			ui_hsplit_b(&box, 24.f, &box, &part);
			ui_vmargin(&part, 80.0f, &part);
			
			ui_vsplit_mid(&part, &abort, &tryagain);
			
			ui_vmargin(&tryagain, 20.0f, &tryagain);
			ui_vmargin(&abort, 20.0f, &abort);
			
			static int button_abort = 0;
			if(ui_do_button(&button_abort, "Abort", 0, &abort, ui_draw_menu_button, 0) || escape_pressed)
				popup = POPUP_NONE;

			static int button_tryagain = 0;
			if(ui_do_button(&button_tryagain, "Try again", 0, &tryagain, ui_draw_menu_button, 0) || enter_pressed)
			{
				client_connect(config.ui_server_address);
			}
			
			ui_hsplit_b(&box, 60.f, &box, &part);
			ui_hsplit_b(&box, 24.f, &box, &part);
			
			ui_vsplit_l(&part, 60.0f, 0, &label);
			ui_vsplit_l(&label, 100.0f, 0, &textbox);
			ui_vsplit_l(&textbox, 20.0f, 0, &textbox);
			ui_vsplit_r(&textbox, 60.0f, &textbox, 0);
			ui_do_label(&label, "Password:", 20, -1);
			ui_do_edit_box(&config.password, &textbox, config.password, sizeof(config.password), 14.0f, true);
		}
		else if(popup == POPUP_FIRST_LAUNCH)
		{
			RECT label, textbox;
			
			ui_hsplit_b(&box, 20.f, &box, &part);
			ui_hsplit_b(&box, 24.f, &box, &part);
			ui_vmargin(&part, 80.0f, &part);
			
			static int enter_button = 0;
			if(ui_do_button(&enter_button, "Enter", 0, &part, ui_draw_menu_button, 0) || enter_pressed)
				popup = POPUP_NONE;
			
			ui_hsplit_b(&box, 40.f, &box, &part);
			ui_hsplit_b(&box, 24.f, &box, &part);
			
			ui_vsplit_l(&part, 60.0f, 0, &label);
			ui_vsplit_l(&label, 100.0f, 0, &textbox);
			ui_vsplit_l(&textbox, 20.0f, 0, &textbox);
			ui_vsplit_r(&textbox, 60.0f, &textbox, 0);
			ui_do_label(&label, "Nickname:", 20, -1);
			ui_do_edit_box(&config.player_name, &textbox, config.player_name, sizeof(config.player_name), 14.0f);
		}
		else
		{
			ui_hsplit_b(&box, 20.f, &box, &part);
			ui_hsplit_b(&box, 24.f, &box, &part);
			ui_vmargin(&part, 120.0f, &part);

			static int button = 0;
			if(ui_do_button(&button, button_text, 0, &part, ui_draw_menu_button, 0) || escape_pressed || enter_pressed)
			{
				if(popup == POPUP_CONNECTING)
					client_disconnect();
				popup = POPUP_NONE;
			}
		}
	}
	
	return 0;
}


void MENUS::set_active(bool active)
{
	menu_active = active;
	if(!menu_active && need_sendinfo)
	{
		gameclient.send_info(false);
		need_sendinfo = false;
	}
}

void MENUS::on_reset()
{
}

bool MENUS::on_mousemove(float x, float y)
{
	last_input = time_get();
	
	if(!menu_active)
		return false;
		
	mouse_pos.x += x;
	mouse_pos.y += y;
	if(mouse_pos.x < 0) mouse_pos.x = 0;
	if(mouse_pos.y < 0) mouse_pos.y = 0;
	if(mouse_pos.x > gfx_screenwidth()) mouse_pos.x = gfx_screenwidth();
	if(mouse_pos.y > gfx_screenheight()) mouse_pos.y = gfx_screenheight();
	
	return true;
}

bool MENUS::on_input(INPUT_EVENT e)
{
	last_input = time_get();
	
	// special handle esc and enter for popup purposes
	if(e.flags&INPFLAG_PRESS)
	{
		if(e.key == KEY_ESCAPE)
		{
			escape_pressed = true;
			set_active(!is_active());
			return true;
		}
	}
		
	if(is_active())
	{
		// special for popups
		if(e.flags&INPFLAG_PRESS && e.key == KEY_RETURN)
			enter_pressed = true;
		
		if(num_inputevents < MAX_INPUTEVENTS)
			inputevents[num_inputevents++] = e;
		return true;
	}
	return false;
}

void MENUS::on_statechange(int new_state, int old_state)
{
	if(new_state == CLIENTSTATE_OFFLINE)
	{
		popup = POPUP_NONE;
		if(client_error_string() && client_error_string()[0] != 0)
		{
			if(strstr(client_error_string(), "password"))
			{
				popup = POPUP_PASSWORD;
				ui_set_hot_item(&config.password);
				ui_set_active_item(&config.password);
			}
			else
				popup = POPUP_DISCONNECTED;
		}	}
	else if(new_state == CLIENTSTATE_LOADING)
	{
		popup = POPUP_CONNECTING;
		client_serverinfo_request();
	}
	else if(new_state == CLIENTSTATE_CONNECTING)
		popup = POPUP_CONNECTING;
	else if (new_state == CLIENTSTATE_ONLINE || new_state == CLIENTSTATE_DEMOPLAYBACK)
	{
		popup = POPUP_NONE;
		set_active(false);
	}
}

void MENUS::on_render()
{
	if(client_state() != CLIENTSTATE_ONLINE && client_state() != CLIENTSTATE_DEMOPLAYBACK)
		set_active(true);

	if(client_state() == CLIENTSTATE_DEMOPLAYBACK)
	{
		RECT screen = *ui_screen();
		gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);
		render_demoplayer(screen);
	}
	
	if(client_state() == CLIENTSTATE_ONLINE && gameclient.servermode == gameclient.SERVERMODE_PUREMOD)
	{
		client_disconnect();
		set_active(true);
		popup = POPUP_PURE;
	}
	
	if(!is_active())
	{
		escape_pressed = false;
		enter_pressed = false;
		num_inputevents = 0;
		return;
	}
	
	// update colors
	vec3 rgb = hsl_to_rgb(vec3(config.ui_color_hue/255.0f, config.ui_color_sat/255.0f, config.ui_color_lht/255.0f));
	gui_color = vec4(rgb.r, rgb.g, rgb.b, config.ui_color_alpha/255.0f);

	color_tabbar_inactive_outgame = vec4(0,0,0,0.25f);
	color_tabbar_active_outgame = vec4(0,0,0,0.5f);

	float color_ingame_scale_i = 0.5f;
	float color_ingame_scale_a = 0.2f;
	color_tabbar_inactive_ingame = vec4(
		gui_color.r*color_ingame_scale_i,
		gui_color.g*color_ingame_scale_i,
		gui_color.b*color_ingame_scale_i,
		gui_color.a*0.8f);
	
	color_tabbar_active_ingame = vec4(
		gui_color.r*color_ingame_scale_a,
		gui_color.g*color_ingame_scale_a,
		gui_color.b*color_ingame_scale_a,
		gui_color.a);
    
	// update the ui
	RECT *screen = ui_screen();
	float mx = (mouse_pos.x/(float)gfx_screenwidth())*screen->w;
	float my = (mouse_pos.y/(float)gfx_screenheight())*screen->h;
		
	int buttons = 0;
	if(inp_key_pressed(KEY_MOUSE_1)) buttons |= 1;
	if(inp_key_pressed(KEY_MOUSE_2)) buttons |= 2;
	if(inp_key_pressed(KEY_MOUSE_3)) buttons |= 4;
		
	ui_update(mx,my,mx*3.0f,my*3.0f,buttons);
    
	// render
	if(client_state() != CLIENTSTATE_DEMOPLAYBACK)
		render();

	// render cursor
	gfx_texture_set(data->images[IMAGE_CURSOR].id);
	gfx_quads_begin();
	gfx_setcolor(1,1,1,1);
	gfx_quads_drawTL(mx,my,24,24);
	gfx_quads_end();

	// render debug information
	if(config.debug)
	{
		RECT screen = *ui_screen();
		gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);

		char buf[512];
		str_format(buf, sizeof(buf), "%p %p %p", ui_hot_item(), ui_active_item(), ui_last_active_item());
		TEXT_CURSOR cursor;
		gfx_text_set_cursor(&cursor, 10, 10, 10, TEXTFLAG_RENDER);
		gfx_text_ex(&cursor, buf, -1);
	}

	escape_pressed = false;
	enter_pressed = false;
	num_inputevents = 0;
}

static int texture_blob = -1;

void MENUS::render_background()
{
	//gfx_clear(1,1,1);
	//render_sunrays(0,0);
	if(texture_blob == -1)
		texture_blob = gfx_load_texture("blob.png", IMG_AUTO, 0);


	float sw = 300*gfx_screenaspect();
	float sh = 300;
	gfx_mapscreen(0, 0, sw, sh);

	RECT s = *ui_screen();

	// render background color
	gfx_texture_set(-1);
	gfx_quads_begin();
		//vec4 bottom(gui_color.r*0.3f, gui_color.g*0.3f, gui_color.b*0.3f, 1.0f);
		//vec4 bottom(0, 0, 0, 1.0f);
		vec4 bottom(gui_color.r, gui_color.g, gui_color.b, 1.0f);
		vec4 top(gui_color.r, gui_color.g, gui_color.b, 1.0f);
		gfx_setcolorvertex(0, top.r, top.g, top.b, top.a);
		gfx_setcolorvertex(1, top.r, top.g, top.b, top.a);
		gfx_setcolorvertex(2, bottom.r, bottom.g, bottom.b, bottom.a);
		gfx_setcolorvertex(3, bottom.r, bottom.g, bottom.b, bottom.a);
		gfx_quads_drawTL(0, 0, sw, sh);
	gfx_quads_end();
	
	// render the tiles
	gfx_texture_set(-1);
	gfx_quads_begin();
		float size = 15.0f;
		float offset_time = fmod(client_localtime()*0.15f, 2.0f);
		for(int y = -2; y < (int)(sw/size); y++)
			for(int x = -2; x < (int)(sh/size); x++)
			{
				gfx_setcolor(0,0,0,0.045f);
				gfx_quads_drawTL((x-offset_time)*size*2+(y&1)*size, (y+offset_time)*size, size, size);
			}
	gfx_quads_end();

	// render border fade
	gfx_texture_set(texture_blob);
	gfx_quads_begin();
		gfx_setcolor(0,0,0,0.5f);
		gfx_quads_drawTL(-100, -100, sw+200, sh+200);
	gfx_quads_end();

	// restore screen	
    {RECT screen = *ui_screen();
	gfx_mapscreen(screen.x, screen.y, screen.w, screen.h);}	
}
