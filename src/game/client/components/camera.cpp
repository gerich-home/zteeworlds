#include <engine/e_config.h>
#include <engine/e_client_interface.h>

#include <base/math.hpp>
#include <game/collision.hpp>
#include <game/client/gameclient.hpp>
#include <game/client/component.hpp>

#include "camera.hpp"
#include "controls.hpp"

CAMERA::CAMERA()
{
}

static void con_dynamic_camera(void *result, void *user_data)
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

void CAMERA::on_console_init()
{
	MACRO_REGISTER_COMMAND("+dynamic_camera", "", CFGFLAG_CLIENT, con_dynamic_camera, &config.cl_mouse_deadzone, "Switch dynamic camera mode");
}

void CAMERA::on_render()
{
	//vec2 center;
	zoom = 1.0f;

	// update camera center		
	if(gameclient.snap.spectate)
//		center = gameclient.controls->mouse_pos;
	{
		if(gameclient.freeview)
			center = gameclient.controls->mouse_pos;
		else
			center = gameclient.spectate_pos;
	}
	else
	{

		float l = length(gameclient.controls->mouse_pos);
		float deadzone = config.cl_mouse_deadzone;
		float follow_factor = config.cl_mouse_followfactor/100.0f;
		vec2 camera_offset(0, 0);

		float offset_amount = max(l-deadzone, 0.0f) * follow_factor;
		if(l > 0.0001f) // make sure that this isn't 0
			camera_offset = normalize(gameclient.controls->mouse_pos)*offset_amount;
		
		center = gameclient.local_character_pos + camera_offset;
	}
}
