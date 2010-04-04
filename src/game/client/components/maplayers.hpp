#include <game/client/component.hpp>

class MAPLAYERS : public COMPONENT
{	
	int type;
	void render_game_layer();
public:
	enum
	{
		TYPE_BACKGROUND=0,
		TYPE_FOREGROUND,
	};

	MAPLAYERS(int type);
	virtual void on_render();
};

