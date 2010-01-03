#include <base/vmath.hpp>
#include <game/client/component.hpp>
#include <vector>

class FASTMENU : public COMPONENT
{
	struct fastmenu_command
	{
		const char * command;
		const char * title;
	};

	std::vector<fastmenu_command *> commands;

	void draw_circle(float x, float y, float r, int segments, int sel);
	
	bool was_active;
	
	vec2 selector_mouse;
	int selected_command;

	static void con_key_fastmenu(void *result, void *user_data);
	static void con_fastmenu(void *result, void *user_data);
	static void con_fastmenu_add(void *result, void *user_data);
	static void con_fastmenu_clear(void *result, void *user_data);
	static void con_fastmenu_delete(void *result, void *user_data);
	static void con_fastmenu_list(void *result, void *user_data);
	
public:
	FASTMENU();
	~FASTMENU();

	bool active;
	
	virtual void on_reset();
	virtual void on_console_init();
	virtual void on_render();
	virtual void on_message(int msgtype, void *rawmsg);
	virtual bool on_mousemove(float x, float y);
	virtual void on_save();

	void fastmenu(int command);

	void clear();
	void list();
	void add(const char * title, const char * command);
	void erase(int command);
};

