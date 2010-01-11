#include <engine/e_client_interface.h>
#include <engine/e_config.h>
//#include <gc_client.hpp>
#include <game/generated/gc_data.hpp>

#include <game/client/components/particles.hpp>
#include <game/client/components/skins.hpp>
#include <game/client/components/flow.hpp>
#include <game/client/components/damageind.hpp>
#include <game/client/components/sounds.hpp>
#include <game/client/gameclient.hpp>

#include "effects.hpp"

inline vec2 random_dir() { return normalize(vec2(frandom()-0.5f, frandom()-0.5f)); }

EFFECTS::EFFECTS()
{
	add_50hz = false;
	add_100hz = false;
}

void EFFECTS::air_jump(vec2 pos)
{
     if(config.gfx_eyecandy)
    {
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_AIRJUMP;
	p.pos = pos + vec2(-6.0f, 16.0f);
	p.vel = vec2(0, -200);
	p.life_span = 0.9f;
	p.start_size = 48.0f;
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	p.rotspeed = pi*2;
	p.gravity = 500;
	p.friction = 0.7f;
	p.flow_affected = 0.0f;
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);

	p.pos = pos + vec2(6.0f, 16.0f);
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
    }
	gameclient.sounds->play(SOUNDS::CHN_WORLD, SOUND_PLAYER_AIRJUMP, 1.0f, pos);
}

void EFFECTS::damage_indicator(vec2 pos, vec2 dir)
{
	gameclient.damageind->create(pos, dir);
}

void EFFECTS::powerupshine(vec2 pos, vec2 size)
{
	if(!add_50hz)
		return;
		
   if(config.gfx_eyecandy)
    {
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_SLICE;
	p.pos = pos + vec2((frandom()-0.5f)*size.x, (frandom()-0.5f)*size.y);
	p.vel = vec2(0, 0);
	if(config.gfx_eyecandy)
	{
	p.gravity = -100;
	p.life_span = 1.3f;
	//p.color = vec4(1.0f,0.15f,0.15f,1.0f);
	p.color = vec4(frandom()*0.5f + 0.5f,frandom()*0.5f + 0.5f,frandom()*0.5f + 0.5f,0.8f);
	}
	else
	{
	p.gravity = -500;
	p.life_span = 0.5f;
	}
	p.start_size = 16.0f;
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	p.rotspeed = pi*2;
	p.friction = 0.9f;
	p.flow_affected = 0.0f;
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
  }
}

void EFFECTS::weaponshine(vec2 pos, vec2 size)
{
	if(!add_50hz)
		return;
		
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_SLICE;
	p.pos = pos + vec2((frandom()-0.5f)*size.x, (frandom()-0.5f)*size.y);
	p.vel = vec2(0, 0);
	p.gravity = -100;
	p.life_span = 7.0f / 10.0f;
	//p.color = vec4(0.15f,0.5f,0.7f,0.5f);
	p.color = vec4(frandom()*0.5f + 0.5f,frandom()*0.5f + 0.5f,frandom()*0.5f + 0.5f,0.5f);
	p.start_size = 18.0f;
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	p.rotspeed = pi*2;
	p.friction = 0.9f;
	p.flow_affected = 0.0f;
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
}
void EFFECTS::redflagshine(vec2 pos, vec2 size)
{
	if(!add_50hz)
		return;
		
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_SLICE;
	p.pos = pos + vec2((frandom()-0.5f)*size.x / 2.0f, (frandom()-0.5f)*size.y);
	p.vel = vec2(0, 0);
	p.gravity = frandom() -170.0;
	p.life_span = 7.0f / 10.0f;
	//p.color = vec4(0.85f,0.2f,0.2f,1.0f);
	p.color = vec4(frandom()*0.5f + 0.5f,frandom()*0.5f,frandom()*0.5f,0.8f);
	p.start_size = 15.0f;
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	p.rotspeed = pi*2;
	p.friction = 0.9f;
	p.flow_affected = 0.0f;
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
}

void EFFECTS::blueflagshine(vec2 pos, vec2 size)
{
	if(!add_50hz)
		return;
		
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_SLICE;
	p.pos = pos + vec2((frandom()-0.5f)*size.x / 2.0f, (frandom()-0.5f)*size.y);
	p.vel = vec2(0, 0);
	p.gravity = frandom() -170;
	p.life_span = 7.0f / 10.0f;
	//p.color = vec4(0.2f,0.2f,0.85f,1.0f);
	p.color = vec4(frandom()*0.5f,frandom()*0.5f,frandom()*0.5f + 0.5f,0.8f);
	p.start_size = 15.0f;
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	p.rotspeed = pi*2;
	p.friction = 0.9f;
	p.flow_affected = 0.0f;
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
}

void EFFECTS::smoketrail(vec2 pos, vec2 vel)
{
	if(!add_50hz)
		return;
		
   if(config.gfx_eyecandy)
  {
	PARTICLE p;
	p.set_default();
	int effect = rand()%5;
	if(effect == 1)
	p.spr = SPRITE_PART_SLICE;	
	else if(effect == 2)
	p.spr = SPRITE_PART_AIRJUMP;
	else if(effect == 3)
	p.spr = SPRITE_PART_BALL;
	else if(effect == 4)
	p.spr = SPRITE_PART_SHELL;
	else if(effect == 0)
	{
	p.spr = SPRITE_PART_SMOKE;
	}	
	p.pos = pos;
	p.vel = vel + random_dir()*50.0f;
	if(config.gfx_eyecandy)
	{
	p.life_span = 0.8f + frandom()*0.8f;
	p.start_size = 12.0f + frandom()*6;
	p.end_size = 0;
	p.friction = 0.80005;
	p.gravity = frandom()*-1000.0f;
	}
	else
	{
	p.life_span = 0.5f + frandom()*0.5f;
	p.start_size = 12.0f + frandom()*8;
	p.end_size = 0;
	p.friction = 0.7;
	p.gravity = frandom()*-500.0f;
	}

	p.color = vec4(frandom()*0.5f+0.5f,frandom()*0.5f+0.5f,frandom()*0.5f+0.5f,frandom()*0.5f+0.5f);
	gameclient.particles->add(PARTICLES::GROUP_PROJECTILE_TRAIL, &p);
  }
}


void EFFECTS::skidtrail(vec2 pos, vec2 vel)
{
	if(!add_100hz)
		return;
	
   if(config.gfx_eyecandy)
    {
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_SMOKE;
	p.pos = pos;
	p.vel = vel + random_dir()*50.0f;
	p.life_span = 0.5f + frandom()*0.5f;
	p.start_size = 24.0f + frandom()*12;
	p.end_size = 0;
	p.friction = 0.7f;
	p.gravity = frandom()*-500.0f;
	p.color = vec4(0.75f,0.75f,0.75f,1.0f);
	gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);	
  }
}

void EFFECTS::bullettrail(vec2 pos)
{
	if(!add_100hz)
		return;
		
       if(config.gfx_eyecandy)
	{
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_BALL;
	p.color = vec4(frandom() * 0.5f + 0.5f, frandom() * 0.5f + 0.5f, frandom() * 0.5f + 0.5f ,1.0f);
	p.pos = pos;
	if(config.gfx_eyecandy)
	{
	p.life_span = 0.25f + frandom()*0.75f;
	p.friction = 0.7f;
	p.gravity = -40 + frandom()*3.0f ;
	}
	else {
	p.life_span = 0.25f + frandom()*0.25f;
	p.friction = 0.7f;
	     }
	p.start_size = 8.0f;
	p.end_size = 0;
	gameclient.particles->add(PARTICLES::GROUP_PROJECTILE_TRAIL, &p);
      }
}

void EFFECTS::sgbullettrail(vec2 pos)
{
	if(!add_100hz)
		return;
		
       if(config.gfx_eyecandy)
	{
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_BALL;
	//p.color = vec4(frandom(), frandom(), frandom() ,1.0f);
	p.color = vec4(frandom()*0.5f+0.5f,frandom()*0.5f+0.5f,frandom()*0.5f+0.5f,1.0f);
	p.pos = pos;
	if(config.gfx_eyecandy)
	{
	p.life_span = 0.35f + frandom()*0.75f;
	p.friction = 0.5f;
	p.gravity = -70 + frandom()*4.0f ;
	}
	else {
	p.life_span = 0.25f + frandom()*0.25f;
	p.friction = 0.7f;
	     }
	p.start_size = 9.0f;
	p.end_size = 0;
	gameclient.particles->add(PARTICLES::GROUP_PROJECTILE_TRAIL, &p);
      }
}

void EFFECTS::playerspawn(vec2 pos)
{
    if(config.gfx_eyecandy)
    {
	for(int i = 0; i < 32; i++)
	{
		PARTICLE p;
		p.set_default();
		p.spr = SPRITE_PART_SHELL;
		p.pos = pos;
		p.vel = random_dir() * (pow(frandom(), 3)*600.0f);
		p.life_span = 0.3f + frandom()*0.3f;
		p.start_size = 64.0f + frandom()*32;
		p.end_size = 0;
		p.rot = frandom()*pi*2;
		p.rotspeed = frandom();
		p.gravity = frandom()*-800.0f;
		p.friction = 0.7f;
		p.color = vec4(0xb5/255.0f, 0x50/255.0f, 0xcb/255.0f, 1.0f);
		gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
		
	}
      }
	gameclient.sounds->play(SOUNDS::CHN_WORLD, SOUND_PLAYER_SPAWN, 1.0f, pos);
}

void EFFECTS::playerdeath(vec2 pos, int cid)
{
	vec3 blood_color(1.0f,1.0f,1.0f);

	if(cid >= 0)	
	{
		const SKINS::SKIN *s = gameclient.skins->get(gameclient.clients[cid].skin_id);
		if(s)
			blood_color = s->blood_color;
	}
	
   if(config.gfx_eyecandy)
      {
	for(int i = 0; i < 64; i++)
	{
		PARTICLE p;
		p.set_default();
		p.spr = SPRITE_PART_SPLAT01 + (rand()%3);
		p.pos = pos;
		p.vel = random_dir() * ((frandom()+0.1f)*900.0f);
		p.life_span = 0.3f + frandom()*0.3f;
		p.start_size = 24.0f + frandom()*16;
		p.end_size = 0;
		p.rot = frandom()*pi*2;
		p.rotspeed = (frandom()-0.5f) * pi;
		p.gravity = 800.0f;
		p.friction = 0.8f;
		vec3 c = blood_color * (0.75f + frandom()*0.25f);
		int rcolor = rand()%6;
		if(rcolor == 1)
		{
		p.color = vec4(1.0f, 0.0f, 0.0f, 0.75f);
		}
		if(rcolor == 2)
		{
		p.color = vec4(0.0f, 1.0f, 0.0f, 0.75f);
		}
		if(rcolor == 3)
		{
		p.color = vec4(0.0f, 0.0f, 1.0f, 0.75f);
		}
		if(rcolor == 4)
		{
		p.color = vec4(0.0f, 0.0f, 0.0f, 0.75f);
		}
		if(rcolor == 5)
		{
		p.color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		if(rcolor == 0)
		{
		p.color = vec4(c.r, c.g, c.b, 0.75f);
		}
		
		gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
	}
    }
}


void EFFECTS::explosion(vec2 pos)
{
	// add to flow
	for(int y = -8; y <= 8; y++)
		for(int x = -8; x <= 8; x++)
		{
			if(x == 0 && y == 0)
				continue;
			
			float a = 1 - (length(vec2(x,y)) / length(vec2(8,8)));
			gameclient.flow->add(pos+vec2(x,y)*16, normalize(vec2(x,y))*5000.0f*a, 10.0f);
		}
		
	// add the explosion
   if(config.gfx_eyecandy)
      {
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_EXPL01;
	p.pos = pos;
	if(config.gfx_eyecandy)
	{
	p.life_span = 0.6f;
	p.start_size = 200.0f;
	}	
	else 
	{
	p.life_span = 0.4f;
	p.start_size = 150.0f;
	}
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	gameclient.particles->add(PARTICLES::GROUP_EXPLOSIONS, &p);
      }
	
	// add the smoke
	for(int i = 0; i < 24; i++)
	{
		PARTICLE p;
		p.set_default();
		p.spr = SPRITE_PART_SMOKE;
		p.pos = pos;
		p.vel = random_dir() * ((1.0f + frandom()*0.2f) * 1000.0f);
		p.life_span = 0.5f + frandom()*0.4f;
		p.start_size = 32.0f + frandom()*8;
		p.end_size = 0;
		p.gravity = frandom()*-800.0f;
		p.friction = 0.4f;
		p.color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), frandom());
		gameclient.particles->add(PARTICLES::GROUP_GENERAL, &p);
	}
}


void EFFECTS::hammerhit(vec2 pos)
{
	// add the explosion
   if(config.gfx_eyecandy)
    {
	PARTICLE p;
	p.set_default();
	p.spr = SPRITE_PART_EXPL01;
	p.pos = pos;
	p.life_span = 0.4f;
	p.start_size = 150.0f;
	p.end_size = 0;
	p.rot = frandom()*pi*2;
	gameclient.particles->add(PARTICLES::GROUP_EXPLOSIONS, &p);	
     }
	gameclient.sounds->play(SOUNDS::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, pos);
}

void EFFECTS::on_render()
{
	static int64 last_update_100hz = 0;
	static int64 last_update_50hz = 0;

	if(time_get()-last_update_100hz > time_freq()/100)
	{
		add_100hz = true;
		last_update_100hz = time_get();
	}
	else
		add_100hz = false;

	if(time_get()-last_update_50hz > time_freq()/100)
	{
		add_50hz = true;
		last_update_50hz = time_get();
	}
	else
		add_50hz = false;
		
	if(add_50hz)
		gameclient.flow->update();
}