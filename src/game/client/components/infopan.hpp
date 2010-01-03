#include <game/client/component.hpp>
#include <game/client/lineinput.hpp>

class INFOPAN : public COMPONENT
{
	enum
	{
		MAX_LINES = 10,
	};

	struct LINE
	{
		int64 time;
		char text[512];
	};

	LINE lines[MAX_LINES];
	int current_line;

	static void con_infomsg(void *result, void *user_data);
public:
	void add_line(const char *line);

	virtual void on_console_init();
	virtual void on_render();
	virtual void on_message(int msgtype, void *rawmsg);
};
