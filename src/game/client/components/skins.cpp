/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <base/system.h>
#include <base/math.hpp>

#include <engine/e_client_interface.h>

#include <engine/e_engine.h>

#include <game/client/gameclient.hpp>

#include "skins.hpp"

SKINS::SKINS()
{
	skins.clear();
	all_skins_loaded = false;
}

SKINS::~SKINS()
{
	if (skins.size() > 0)
	{
		for (int i = 0; i < skins.size(); i++)
		{
			if (skins[i])
			{
				mem_free(skins[i]);
				skins[i] = 0;
			}
		}
	}
	skins.clear();
	all_skins_loaded = false;
}

bool SKINS::load_skin(const char * name)
{
	if (config.cl_default_skin_only && name &&
		!(str_comp_nocase(name, "default.png") == 0 || name[0] == 'x' || name[1] == '_'))
		return false;

	SKIN new_skin;
	mem_zero(&new_skin, sizeof(SKIN));

	int l = strlen(name);
	if(l == 0)
		return false;

	char buf[512];

	if(strcmp(name+l-4, ".png") != 0)
	{
		if (find(name) >= 0)
			return true;

		str_format(buf, sizeof(buf), "skins/%s.png", name);
	}
	else
	{
		str_format(buf, sizeof(buf), "%s", name);
		buf[l - 4] = 0;
		if (find(buf) >= 0)
			return true;

		str_format(buf, sizeof(buf), "skins/%s", name);
	}

	IMAGE_INFO info;
	if(!gfx_load_png(&info, buf))
	{
		return false;
	}

	new_skin.org_texture = gfx_load_texture_raw(info.width, info.height, info.format, info.data, info.format, 0);

	int body_size = 96; // body size
	unsigned char *d = (unsigned char *)info.data;
	int pitch = info.width*4;

	// dig out blood color
	{
		int colors[3] = {0};
		for(int y = 0; y < body_size; y++)
			for(int x = 0; x < body_size; x++)
			{
				if(d[y*pitch+x*4+3] > 128)
				{
					colors[0] += d[y*pitch+x*4+0];
					colors[1] += d[y*pitch+x*4+1];
					colors[2] += d[y*pitch+x*4+2];
				}
			}

		new_skin.blood_color = normalize(vec3(colors[0], colors[1], colors[2]));
	}

	// create colorless version
	int step = info.format == IMG_RGBA ? 4 : 3;

	// make the texture gray scale
	for(int i = 0; i < info.width*info.height; i++)
	{
		int v = (d[i*step]+d[i*step+1]+d[i*step+2])/3;
		d[i*step] = v;
		d[i*step+1] = v;
		d[i*step+2] = v;
	}


	{
		int freq[256] = {0};
		int org_weight = 0;
		int new_weight = 192;

		// find most common frequence
		for(int y = 0; y < body_size; y++)
			for(int x = 0; x < body_size; x++)
			{
				if(d[y*pitch+x*4+3] > 128)
					freq[d[y*pitch+x*4]]++;
			}

		for(int i = 1; i < 256; i++)
		{
			if(freq[org_weight] < freq[i])
				org_weight = i;
		}

		// reorder
		int inv_org_weight = 255-org_weight;
		int inv_new_weight = 255-new_weight;
		for(int y = 0; y < body_size; y++)
			for(int x = 0; x < body_size; x++)
			{
				int v = d[y*pitch+x*4];
				if(v <= org_weight)
					v = (int)(((v/(float)org_weight) * new_weight));
				else
					v = (int)(((v-org_weight)/(float)inv_org_weight)*inv_new_weight + new_weight);
				d[y*pitch+x*4] = v;
				d[y*pitch+x*4+1] = v;
				d[y*pitch+x*4+2] = v;
			}
	}

	new_skin.color_texture = gfx_load_texture_raw(info.width, info.height, info.format, info.data, info.format, 0);
	mem_free(info.data);

	// set skin data
	if(strcmp(name+l-4, ".png") != 0)
	{
		strncpy(new_skin.name, name, min((int)sizeof(new_skin.name),l));
	} else {
		strncpy(new_skin.name, name, min((int)sizeof(new_skin.name),l-4));
	}
	new_skin.term[0] = 0;

	SKIN * pnew_skin = (SKIN *)mem_alloc(sizeof(SKIN), 1);
	if (pnew_skin)
	{
		mem_copy(pnew_skin, &new_skin, sizeof(SKIN));

		skins.push_back(pnew_skin);
	} else return false;

	return true;
}

void SKINS::skinscan(const char *name, int is_dir, void *user)
{
	SKINS *self = (SKINS *)user;

	if (is_dir) return;

	if (self)
	{
		int found = self->find(name);

		if (found < 0)
			self->load_skin(name);
	}
}

void SKINS::load_all()
{
	engine_listdir(LISTDIRTYPE_ALL, "skins", skinscan, this);
	if (!config.cl_default_skin_only)
		all_skins_loaded = true;
}

void SKINS::init()
{
	// load skins
	if (skins.size() > 0)
	{
		for (int i = 0; i < skins.size(); i++)
		{
			if (skins[i])
			{
				mem_free(skins[i]);
				skins[i] = 0;
			}
		}
	}
	skins.clear();
	all_skins_loaded = false;

	load_all();
}

int SKINS::num()
{
	return skins.size();
}

const SKINS::SKIN *SKINS::get(int index)
{
	return skins[index%skins.size()];
}

int SKINS::find(const char *name)
{
	if (!name) return -1;
	
	const char * _name = (config.cl_default_skin_only && !(name[0] == 'x' || name[1] == '_')) ? "default" : name;
	
	for(int i = 0; i < skins.size(); i++)
	{
		if(strcmp(skins[i]->name, _name) == 0)
			return i;
	}
	
	return -1;
}

// these converter functions were nicked from some random internet pages
static float hue_to_rgb(float v1, float v2, float h)
{
   if(h < 0) h += 1;
   if(h > 1) h -= 1;
   if((6 * h) < 1) return v1 + ( v2 - v1 ) * 6 * h;
   if((2 * h) < 1) return v2;
   if((3 * h) < 2) return v1 + ( v2 - v1 ) * ((2.0f/3.0f) - h) * 6;
   return v1;
}

static vec3 hsl_to_rgb(vec3 in)
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

vec4 SKINS::get_color(int v)
{
	vec3 r = hsl_to_rgb(vec3((v>>16)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
	return vec4(r.r, r.g, r.b, 1.0f);
}

void SKINS::on_render()
{
	static bool firstTime = true;
	static int oldDefaultSkinOnly = 0;
	if (firstTime)
	{
		firstTime = false;
	} else {	
		if (config.cl_default_skin_only != oldDefaultSkinOnly)
		{
			if (!config.cl_default_skin_only)
				load_all();
				
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if (!config.cl_default_skin_only)
				{
					gameclient.clients[i].skin_id = find(gameclient.clients[i].skin_name);
					if(gameclient.clients[i].skin_id < 0)
					{
						if (load_skin(gameclient.clients[i].skin_name))
							gameclient.clients[i].skin_id = find(gameclient.clients[i].skin_name);
					}
				}
				else
					gameclient.clients[i].skin_id = -1;
					
				if(gameclient.clients[i].skin_id < 0)
				{
					gameclient.clients[i].skin_id = find("default");
					if(gameclient.clients[i].skin_id < 0)
						gameclient.clients[i].skin_id = 0;
				}
				
				gameclient.clients[i].update_render_info();
			}
		}
	}
	oldDefaultSkinOnly = config.cl_default_skin_only;
}
