#include <game/client/component.hpp>
#include <game/client/lineinput.hpp>

class CHAT : public COMPONENT
{
	LINEINPUT input;
	
	enum 
	{
		MAX_LINES = 10,
		MAX_HISTORY_LINES = 25,
	};

	struct LINE
	{
		int64 time;
		int client_id;
		int team;
		int name_color;
		int contains_name;
		int ignore;
		int spam;
		char name[64];
		char text[512];
	};

	LINE lines[MAX_LINES];
	int current_line;
	
	struct HISTORY_LINE
	{
		char text[512];
	};

	HISTORY_LINE history[MAX_HISTORY_LINES];
	int curr_history_line;

	// chat
	enum
	{
		MODE_NONE=0,
		MODE_ALL,
		MODE_TEAM,
	};

	int mode;
	
	bool contains_name;
	bool ignore_player;
	bool spam;
	
	char last_msg[MAX_CLIENTS][512];
	
	static void con_say(void *result, void *user_data);
	static void con_sayteam(void *result, void *user_data);
	static void con_chat(void *result, void *user_data);
public:
	struct split
	{
		char *pointers[256];
		int count;
	};

	static struct split split (char *in, char delim)
	{
		struct split sp;
		sp.count = 1;
		sp.pointers[0] = in;
 
		while (*++in)
		{
			if (*in == delim)
			{
				*in = 0;
				sp.pointers[sp.count++] = in+1;
			}
		}
		return sp;
	};
	
	bool is_active() const { return mode != MODE_NONE; }
	
	void add_line(int client_id, int team, const char *line);
	
	void enable_mode(int team);
	
	void say(int team, const char *line);
	
	virtual void on_console_init();
	virtual void on_statechange(int new_state, int old_state);
	virtual void on_render();
	virtual void on_message(int msgtype, void *rawmsg);
	virtual bool on_input(INPUT_EVENT e);
};
