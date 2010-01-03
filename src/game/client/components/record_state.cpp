///--- Written by Lite

#include <game/client/render.hpp>
#include <game/client/ui.hpp>

#include "record_state.hpp"

void RECORD_STATE::on_render()
{
	if (!demorec_isrecording()) return;
	if (client_tick()%50 < 25) return;

	gfx_texture_set(-1);
	float width = 300.0f * gfx_screenaspect();
	gfx_mapscreen(0.0f, 0.0f, width, 300.0f);
	gfx_quads_begin();
	gfx_setcolor(1.0f, 0.0f, 0.0f, 1.0f);
	draw_round_rect(width - 15.0f, 10.0f, 5.0f, 5.0f, 2.5f);
	gfx_quads_end();
}
