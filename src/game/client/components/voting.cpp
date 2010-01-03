#include <engine/e_client_interface.h>
#include <game/generated/g_protocol.hpp>
#include <base/vmath.hpp>
#include <game/client/render.hpp>
//#include <game/client/gameclient.hpp>
#include "voting.hpp"

void VOTING::con_callvote(void *result, void *user_data)
{
	VOTING *self = (VOTING*)user_data;
	self->callvote(console_arg_string(result, 0), console_arg_string(result, 1));
}

void VOTING::con_vote(void *result, void *user_data)
{
	VOTING *self = (VOTING *)user_data;
	if(str_comp_nocase(console_arg_string(result, 0), "yes") == 0)
		self->vote(1);
	else if(str_comp_nocase(console_arg_string(result, 0), "no") == 0)
		self->vote(-1);
}

void VOTING::callvote(const char *type, const char *value)
{
	NETMSG_CL_CALLVOTE msg = {0};
	msg.type = type;
	msg.value = value;
	msg.pack(MSGFLAG_VITAL);
	client_send_msg();
}

void VOTING::callvote_kick(int client_id)
{
	char buf[32];
	str_format(buf, sizeof(buf), "%d", client_id);
	callvote("kick", buf);
}

void VOTING::callvote_option(int option_id)
{
	VOTEOPTION *option = this->first;
	while(option && option_id >= 0)
	{
		if(option_id == 0)
		{
			callvote("option", option->command);
			break;
		}
		
		option_id--;
		option = option->next;
	}
}

void VOTING::vote(int v)
{
	NETMSG_CL_VOTE msg = {v};
	msg.pack(MSGFLAG_VITAL);
	client_send_msg();
}

VOTING::VOTING()
{
	heap = 0;
	clearoptions();
	on_reset();
}


void VOTING::clearoptions()
{
	if(heap)
		memheap_destroy(heap);
	heap = memheap_create();
	
	first = 0;
	last = 0;
}

void VOTING::on_reset()
{
	closetime = 0;
	description[0] = 0;
	command[0] = 0;
	yes = no = pass = total = 0;
	voted = 0;
}

void VOTING::on_console_init()
{
	MACRO_REGISTER_COMMAND("callvote", "sr", CFGFLAG_CLIENT, con_callvote, this, "Call vote");
	MACRO_REGISTER_COMMAND("vote", "r", CFGFLAG_CLIENT, con_vote, this, "Vote yes/no");
}

void VOTING::on_message(int msgtype, void *rawmsg)
{
	if(msgtype == NETMSGTYPE_SV_VOTE_SET)
	{
		NETMSG_SV_VOTE_SET *msg = (NETMSG_SV_VOTE_SET *)rawmsg;
		if(msg->timeout)
		{
			on_reset();
			str_copy(description, msg->description, sizeof(description));
			str_copy(command, msg->command, sizeof(description));
			closetime = time_get() + time_freq() * msg->timeout;
		}
		else
			on_reset();
	}
	else if(msgtype == NETMSGTYPE_SV_VOTE_STATUS)
	{
		NETMSG_SV_VOTE_STATUS *msg = (NETMSG_SV_VOTE_STATUS *)rawmsg;
		yes = msg->yes;
		no = msg->no;
		pass = msg->pass;
		total = msg->total;
	}	
	else if(msgtype == NETMSGTYPE_SV_VOTE_CLEAROPTIONS)
	{
		clearoptions();
	}
	else if(msgtype == NETMSGTYPE_SV_VOTE_OPTION)
	{
		NETMSG_SV_VOTE_OPTION *msg = (NETMSG_SV_VOTE_OPTION *)rawmsg;
		int len = str_length(msg->command);
	
		VOTEOPTION *option = (VOTEOPTION *)memheap_allocate(heap, sizeof(VOTEOPTION) + len);
		option->next = 0;
		option->prev = last;
		if(option->prev)
			option->prev->next = option;
		last = option;
		if(!first)
			first = option;
		
		mem_copy(option->command, msg->command, len+1);

	}
}

void VOTING::on_render()
{
}


void VOTING::render_bars(RECT bars, bool text)
{
	ui_draw_rect(&bars, vec4(0.8f,0.8f,0.8f,0.5f), CORNER_ALL, bars.h/3);
	
	RECT splitter = bars;
	splitter.x = splitter.x+splitter.w/2;
	splitter.w = splitter.h/2.0f;
	splitter.x -= splitter.w/2;
	ui_draw_rect(&splitter, vec4(0.4f,0.4f,0.4f,0.5f), CORNER_ALL, splitter.h/4);
			
	if(total)
	{
		RECT pass_area = bars;
		if(yes)
		{
			RECT yes_area = bars;
			yes_area.w *= yes/(float)total;
			ui_draw_rect(&yes_area, vec4(0.2f,0.9f,0.2f,0.85f), CORNER_ALL, bars.h/3);
			
			if(text)
			{
				char buf[256];
				str_format(buf, sizeof(buf), "%d", yes);
				ui_do_label(&yes_area, buf, bars.h*0.75f, 0);
			}
			
			pass_area.x += yes_area.w;
			pass_area.w -= yes_area.w;
		}
		
		if(no)
		{
			RECT no_area = bars;
			no_area.w *= no/(float)total;
			no_area.x = (bars.x + bars.w)-no_area.w;
			ui_draw_rect(&no_area, vec4(0.9f,0.2f,0.2f,0.85f), CORNER_ALL, bars.h/3);
			
			if(text)
			{
				char buf[256];
				str_format(buf, sizeof(buf), "%d", no);
				ui_do_label(&no_area, buf, bars.h*0.75f, 0);
			}

			pass_area.w -= no_area.w;
		}

		if(text && pass)
		{
			char buf[256];
			str_format(buf, sizeof(buf), "%d", pass);
			ui_do_label(&pass_area, buf, bars.h*0.75f, 0);
		}
	}	
}


