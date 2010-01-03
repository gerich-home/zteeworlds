#include <base/vmath.hpp>
#include <game/client/component.hpp>
#include <vector>

class SKINS : public COMPONENT
{
public:
	// do this better and nicer
	typedef struct
	{
		int org_texture;
		int color_texture;
		char name[31];
		char term[1];
		vec3 blood_color;
	} SKIN;
	
	bool all_skins_loaded;

	SKINS();
	~SKINS();

	void init();

	vec4 get_color(int v);
	int num();
	const SKIN *get(int index);
	int find(const char *name);

	bool copy_skin(const char * from, const char * to);

	bool load_skin(const char * name);
	void load_all();
	
	virtual void on_render();
private:

	std::vector<SKIN *>skins;

	static void skinscan(const char *name, int is_dir, void *user);
};
