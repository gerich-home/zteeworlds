#include <stdio.h>
#include "ed_editor.hpp"


// popup menu handling
static struct
{
	Rect rect;
	void *id;
	int (*func)(Rect rect);
	int is_menu;
	void *extra;
} ui_popups[8];

static int ui_num_popups = 0;

void ui_invoke_popup_menu(void *id, int flags, float x, float y, float w, float h, int (*func)(Rect rect), void *extra)
{
	dbg_msg("", "invoked");
	ui_popups[ui_num_popups].id = id;
	ui_popups[ui_num_popups].is_menu = flags;
	ui_popups[ui_num_popups].rect.x = x;
	ui_popups[ui_num_popups].rect.y = y;
	ui_popups[ui_num_popups].rect.w = w;
	ui_popups[ui_num_popups].rect.h = h;
	ui_popups[ui_num_popups].func = func;
	ui_popups[ui_num_popups].extra = extra;
	ui_num_popups++;
}

void ui_do_popup_menu()
{
	for(int i = 0; i < ui_num_popups; i++)
	{
		bool inside = ui_mouse_inside(&ui_popups[i].rect);
		ui_set_hot_item(&ui_popups[i].id);
		
		if(ui_active_item() == &ui_popups[i].id)
		{
			if(!ui_mouse_button(0))
			{
				if(!inside)
					ui_num_popups--;
				ui_set_active_item(0);
			}
		}
		else if(ui_hot_item() == &ui_popups[i].id)
		{
			if(ui_mouse_button(0))
				ui_set_active_item(&ui_popups[i].id);
		}
		
		int corners = CORNER_ALL;
		if(ui_popups[i].is_menu)
			corners = CORNER_R|CORNER_B;
		
		Rect r = ui_popups[i].rect;
		ui_draw_rect(&r, vec4(0.5f,0.5f,0.5f,0.75f), corners, 3.0f);
		ui_margin(&r, 1.0f, &r);
		ui_draw_rect(&r, vec4(0,0,0,0.75f), corners, 3.0f);
		ui_margin(&r, 4.0f, &r);
		
		if(ui_popups[i].func(r))
			ui_num_popups--;
			
		if(inp_key_down(KEY_ESCAPE))
			ui_num_popups--;
	}
}


int popup_group(Rect view)
{
	// remove group button
	Rect button;
	ui_hsplit_b(&view, 12.0f, &view, &button);
	static int delete_button = 0;
	
	// don't allow deletion of game group
	if(editor.map.game_group != editor.get_selected_group() &&
		do_editor_button(&delete_button, _t("Delete Group"), 0, &button, draw_editor_button, 0, _t("Delete group")))
	{
		editor.map.delete_group(editor.selected_group);
		return 1;
	}

	// new tile layer
	ui_hsplit_b(&view, 10.0f, &view, &button);
	ui_hsplit_b(&view, 12.0f, &view, &button);
	static int new_quad_layer_button = 0;
	if(do_editor_button(&new_quad_layer_button, _t("Add Quads Layer"), 0, &button, draw_editor_button, 0, _t("Creates a new quad layer")))
	{
		LAYER *l = new LAYER_QUADS;
		editor.map.groups[editor.selected_group]->add_layer(l);
		editor.selected_layer = editor.map.groups[editor.selected_group]->layers.len()-1;
		return 1;
	}

	// new quad layer
	ui_hsplit_b(&view, 5.0f, &view, &button);
	ui_hsplit_b(&view, 12.0f, &view, &button);
	static int new_tile_layer_button = 0;
	if(do_editor_button(&new_tile_layer_button, _t("Add Tile Layer"), 0, &button, draw_editor_button, 0, _t("Creates a new tile layer")))
	{
		LAYER *l = new LAYER_TILES(50, 50);
		editor.map.groups[editor.selected_group]->add_layer(l);
		editor.selected_layer = editor.map.groups[editor.selected_group]->layers.len()-1;
		return 1;
	}
	
	enum
	{
		PROP_ORDER=0,
		PROP_POS_X,
		PROP_POS_Y,
		PROP_PARA_X,
		PROP_PARA_Y,
		PROP_USE_CLIPPING,
		PROP_CLIP_X,
		PROP_CLIP_Y,
		PROP_CLIP_W,
		PROP_CLIP_H,
		NUM_PROPS,
	};
	
	PROPERTY props[] = {
		{"Order", editor.selected_group, PROPTYPE_INT_STEP, 0, editor.map.groups.len()-1},
		{"Pos X", -editor.map.groups[editor.selected_group]->offset_x, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Pos Y", -editor.map.groups[editor.selected_group]->offset_y, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Para X", editor.map.groups[editor.selected_group]->parallax_x, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Para Y", editor.map.groups[editor.selected_group]->parallax_y, PROPTYPE_INT_SCROLL, -1000000, 1000000},

		{"Use Clipping", editor.map.groups[editor.selected_group]->use_clipping, PROPTYPE_BOOL, 0, 1},
		{"Clip X", editor.map.groups[editor.selected_group]->clip_x, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Clip Y", editor.map.groups[editor.selected_group]->clip_y, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Clip W", editor.map.groups[editor.selected_group]->clip_w, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Clip H", editor.map.groups[editor.selected_group]->clip_h, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{0},
	};
	props[0].name = _t("Order");
	props[1].name = _t("Pos X");
	props[2].name = _t("Pos Y");
	props[3].name = _t("Para X");
	props[4].name = _t("Para Y");
	props[5].name = _t("Use Clipping");
	props[6].name = _t("Clip X");
	props[7].name = _t("Clip Y");
	props[8].name = _t("Clip W");
	props[9].name = _t("Clip H");
	
	static int ids[NUM_PROPS] = {0};
	int new_val = 0;
	
	// cut the properties that isn't needed
	if(editor.get_selected_group()->game_group)
		props[PROP_POS_X].name = 0;
		
	int prop = editor.do_properties(&view, props, ids, &new_val);
	if(prop == PROP_ORDER)
		editor.selected_group = editor.map.swap_groups(editor.selected_group, new_val);
		
	// these can not be changed on the game group
	if(!editor.get_selected_group()->game_group)
	{
		if(prop == PROP_PARA_X) editor.map.groups[editor.selected_group]->parallax_x = new_val;
		else if(prop == PROP_PARA_Y) editor.map.groups[editor.selected_group]->parallax_y = new_val;
		else if(prop == PROP_POS_X) editor.map.groups[editor.selected_group]->offset_x = -new_val;
		else if(prop == PROP_POS_Y) editor.map.groups[editor.selected_group]->offset_y = -new_val;
		else if(prop == PROP_USE_CLIPPING) editor.map.groups[editor.selected_group]->use_clipping = new_val;
		else if(prop == PROP_CLIP_X) editor.map.groups[editor.selected_group]->clip_x = new_val;
		else if(prop == PROP_CLIP_Y) editor.map.groups[editor.selected_group]->clip_y = new_val;
		else if(prop == PROP_CLIP_W) editor.map.groups[editor.selected_group]->clip_w = new_val;
		else if(prop == PROP_CLIP_H) editor.map.groups[editor.selected_group]->clip_h = new_val;
	}
	
	return 0;
}

int popup_layer(Rect view)
{
	// remove layer button
	Rect button;
	ui_hsplit_b(&view, 12.0f, &view, &button);
	static int delete_button = 0;
	
	// don't allow deletion of game layer
	if(editor.map.game_layer != editor.get_selected_layer(0) &&
		do_editor_button(&delete_button, "Delete Layer", 0, &button, draw_editor_button, 0, "Deletes the layer"))
	{
		editor.map.groups[editor.selected_group]->delete_layer(editor.selected_layer);
		return 1;
	}

	ui_hsplit_b(&view, 10.0f, &view, 0);
	
	LAYERGROUP *current_group = editor.map.groups[editor.selected_group];
	LAYER *current_layer = editor.get_selected_layer(0);
	
	enum
	{
		PROP_GROUP=0,
		PROP_ORDER,
		PROP_HQ,
		NUM_PROPS,
	};
	
	PROPERTY props[] = {
		{"Group", editor.selected_group, PROPTYPE_INT_STEP, 0, editor.map.groups.len()-1},
		{"Order", editor.selected_layer, PROPTYPE_INT_STEP, 0, current_group->layers.len()},
		{"Detail", current_layer->flags&LAYERFLAG_DETAIL, PROPTYPE_BOOL, 0, 1},
		{0},
	};
	
	static int ids[NUM_PROPS] = {0};
	int new_val = 0;
	int prop = editor.do_properties(&view, props, ids, &new_val);		
	
	if(prop == PROP_ORDER)
		editor.selected_layer = current_group->swap_layers(editor.selected_layer, new_val);
	else if(prop == PROP_GROUP && current_layer->type != LAYERTYPE_GAME)
	{
		if(new_val >= 0 && new_val < editor.map.groups.len())
		{
			current_group->layers.remove(current_layer);
			editor.map.groups[new_val]->layers.add(current_layer);
			editor.selected_group = new_val;
			editor.selected_layer = editor.map.groups[new_val]->layers.len()-1;
		}
	}
	else if(prop == PROP_HQ)
	{
		current_layer->flags &= ~LAYERFLAG_DETAIL;
		if(new_val)
			current_layer->flags |= LAYERFLAG_DETAIL;
	}
		
	return current_layer->render_properties(&view);
}

int popup_quad(Rect view)
{
	QUAD *quad = editor.get_selected_quad();

	Rect button;
	
	// delete button
	ui_hsplit_b(&view, 12.0f, &view, &button);
	static int delete_button = 0;
	if(do_editor_button(&delete_button, _t("Delete"), 0, &button, draw_editor_button, 0, _t("Deletes the current quad")))
	{
		LAYER_QUADS *layer = (LAYER_QUADS *)editor.get_selected_layer_type(0, LAYERTYPE_QUADS);
		if(layer)
		{
			layer->quads.removebyindex(editor.selected_quad);
			editor.selected_quad--;
		}
		return 1;
	}

	// square button
	ui_hsplit_b(&view, 10.0f, &view, &button);
	ui_hsplit_b(&view, 12.0f, &view, &button);
	static int sq_button = 0;
	if(do_editor_button(&sq_button, _t("Square"), 0, &button, draw_editor_button, 0, _t("Squares the current quad")))
	{
		int top = quad->points[0].y;
		int left = quad->points[0].x;
		int bottom = quad->points[0].y;
		int right = quad->points[0].x;
		
		for(int k = 1; k < 4; k++)
		{
			if(quad->points[k].y < top) top = quad->points[k].y;
			if(quad->points[k].x < left) left = quad->points[k].x;
			if(quad->points[k].y > bottom) bottom = quad->points[k].y;
			if(quad->points[k].x > right) right = quad->points[k].x;
		}
		
		quad->points[0].x = left; quad->points[0].y = top;
		quad->points[1].x = right; quad->points[1].y = top;
		quad->points[2].x = left; quad->points[2].y = bottom;
		quad->points[3].x = right; quad->points[3].y = bottom;
		return 1;
	}

	// fix coords
	ui_hsplit_b(&view, 10.0f, &view, &button);
	ui_hsplit_b(&view, 12.0f, &view, &button);

	static int fc_button = 0;
	if(do_editor_button(&fc_button, _t("Fix Coordinates"), 0, &button, draw_editor_button, 0, _t("Fixes coordinates of the quads points")))
	{
		for ( int p = 0; p < 4; p++ ) {
			int x = quad->points[p].x / 1000;
			int y = quad->points[p].y / 1000;

			quad->points[p].x = 1000*x;
			quad->points[p].y = 1000*y;
		}

		return 1;
	}


	// move quad
	enum
	{
		POS_X=0,
		POS_Y,
		NUM_POS,
	};

	int pivot_pos[NUM_POS] = {0};
	int old_pos[NUM_POS] = {0};

	pivot_pos[POS_X]	= quad->points[4].x/1000;
	pivot_pos[POS_Y]	= quad->points[4].y/1000;
	old_pos[POS_X]		= pivot_pos[POS_X];
	old_pos[POS_Y]		= pivot_pos[POS_Y];


	PROPERTY pivot_props[] = {
		{_t("X"), pivot_pos[POS_X], PROPTYPE_INT_STEP, 0, 255},
		{_t("Y"), pivot_pos[POS_Y], PROPTYPE_INT_STEP, 0, 255},
		{0},
	};
	
	static int pivot_ids[NUM_POS] = {0};

	int new_val = 0;
	int pivot_prop = editor.do_properties(&view, pivot_props, pivot_ids, &new_val);

	if(pivot_prop == POS_X)
	{
		quad->points[4].x = new_val*1000;
		for ( int v = 0; v < 4; v++ ) {
			int x = quad->points[v].x / 1000;
			quad->points[v].x = (x+new_val-old_pos[POS_X])*1000;
		}
	}
	if(pivot_prop == POS_Y)
	{
		quad->points[4].y = new_val*1000;
		for ( int v = 0; v < 4; v++ ) {
			int y = quad->points[v].y / 1000;
			quad->points[v].y = (y+new_val-old_pos[POS_Y])*1000;
		}
	}

	enum
	{
		PROP_POS_ENV=0,
		PROP_POS_ENV_OFFSET,
		PROP_COLOR_ENV,
		PROP_COLOR_ENV_OFFSET,
		NUM_PROPS,
	};
	
	PROPERTY props[] = {
		{"Pos. Env", quad->pos_env, PROPTYPE_INT_STEP, -1, editor.map.envelopes.len()},
		{"Pos. TO", quad->pos_env_offset, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		{"Color Env", quad->color_env, PROPTYPE_INT_STEP, -1, editor.map.envelopes.len()},
		{"Color TO", quad->color_env_offset, PROPTYPE_INT_SCROLL, -1000000, 1000000},
		
		{0},
	};
	props[0].name = _t("Pos. Env");
	props[1].name = _t("Pos. TO");
	props[2].name = _t("Color Env");
	props[3].name = _t("Color TO");
	
	static int ids[NUM_PROPS] = {0};
	new_val = 0;
	int prop = editor.do_properties(&view, props, ids, &new_val);		
	
	if(prop == PROP_POS_ENV) quad->pos_env = clamp(new_val, -1, editor.map.envelopes.len()-1);
	if(prop == PROP_POS_ENV_OFFSET) quad->pos_env_offset = new_val;
	if(prop == PROP_COLOR_ENV) quad->color_env = clamp(new_val, -1, editor.map.envelopes.len()-1);
	if(prop == PROP_COLOR_ENV_OFFSET) quad->color_env_offset = new_val;
	
	return 0;
}

int popup_point(Rect view)
{
	QUAD *quad = editor.get_selected_quad();
	
	enum
	{
		PROP_R=0,
		PROP_G,
		PROP_B,
		PROP_A,
		NUM_PROPS,
	};
	enum
	{
		POS_X=0,
		POS_Y,
		NUM_POS,
	};
	
	int color[NUM_PROPS]	= {0};
	int ppos[NUM_POS]		= {0};

	int v = 0;
	for(int n = 0; n < 4; n++)
	{
		if(editor.selected_points&(1<<n))
			v = n;
	}
	
	color[PROP_R] = quad->colors[v].r;
	color[PROP_G] = quad->colors[v].g;
	color[PROP_B] = quad->colors[v].b;
	color[PROP_A] = quad->colors[v].a;
	ppos[POS_X]	  = quad->points[v].x/1000;
	ppos[POS_Y]	  = quad->points[v].y/1000;	

	PROPERTY props[] = {
		{_t("R"), color[PROP_R], PROPTYPE_INT_STEP, 0, 255},
		{_t("G"), color[PROP_G], PROPTYPE_INT_STEP, 0, 255},
		{_t("B"), color[PROP_B], PROPTYPE_INT_STEP, 0, 255},
		{_t("A"), color[PROP_A], PROPTYPE_INT_STEP, 0, 255},
		{0},
	};
	
	PROPERTY pos_props[] = {
		{_t("X"), ppos[POS_X], PROPTYPE_INT_STEP, 0, 255},
		{_t("Y"), ppos[POS_Y], PROPTYPE_INT_STEP, 0, 255},
		{0},
	};
	
	static int ids[NUM_PROPS] = {0};
	static int pos_ids[NUM_POS] = {0};
	
	int new_val = 0;
	int prop = editor.do_properties(&view, props, ids, &new_val);		

	int pos_prop = editor.do_properties(&view, pos_props, pos_ids, &new_val);
	
	if(prop == PROP_R && new_val > 0 && new_val < 256)
	{
		color[PROP_R] = 0;
		quad->colors[v].r = new_val;
	}
	if(prop == PROP_G && new_val > 0 && new_val < 256)
	{
		color[PROP_G] = 0;
		quad->colors[v].g = new_val;
	}
	if(prop == PROP_B && new_val > 0 && new_val < 256)
	{
		color[PROP_B] = 0;
		quad->colors[v].b = new_val;
	}
	if(prop == PROP_A && new_val > 0 && new_val < 256)
	{
		color[PROP_A] = 0;
		quad->colors[v].a = new_val;
	}

	if(pos_prop == POS_X)
		quad->points[v].x = new_val*1000;
	if(pos_prop == POS_Y)
		quad->points[v].y = new_val*1000;

	return 0;
}



static int select_image_selected = -100;
static int select_image_current = -100;

int popup_select_image(Rect view)
{
	Rect buttonbar, imageview;
	ui_vsplit_l(&view, 80.0f, &buttonbar, &view);
	ui_margin(&view, 10.0f, &imageview);
	
	int show_image = select_image_current;
	
	for(int i = -1; i < editor.map.images.len(); i++)
	{
		Rect button;
		ui_hsplit_t(&buttonbar, 12.0f, &button, &buttonbar);
		ui_hsplit_t(&buttonbar, 2.0f, 0, &buttonbar);
		
		if(ui_mouse_inside(&button))
			show_image = i;
			
		if(i == -1)
		{
			if(do_editor_button(&editor.map.images[i], _t("None"), i==select_image_current, &button, draw_editor_button_menuitem, 0, 0))
				select_image_selected = -1;
		}
		else
		{
			if(do_editor_button(&editor.map.images[i], editor.map.images[i]->name, i==select_image_current, &button, draw_editor_button_menuitem, 0, 0))
				select_image_selected = i;
		}
	}
	
	if(show_image >= 0 && show_image < editor.map.images.len())
		gfx_texture_set(editor.map.images[show_image]->tex_id);
	else
		gfx_texture_set(-1);
	gfx_quads_begin();
	gfx_quads_drawTL(imageview.x, imageview.y, imageview.w, imageview.h);
	gfx_quads_end();

	return 0;
}

void popup_select_image_invoke(int current, float x, float y)
{
	static int select_image_popup_id = 0;
	select_image_selected = -100;
	select_image_current = current;
	ui_invoke_popup_menu(&select_image_popup_id, 0, x, y, 400, 300, popup_select_image);
}

int popup_select_image_result()
{
	if(select_image_selected == -100)
		return -100;
		
	select_image_current = select_image_selected;
	select_image_selected = -100;
	return select_image_current;
}





