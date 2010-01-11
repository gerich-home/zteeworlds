#include <game/client/component.hpp>
#include <engine/e_language.h>
#include <vector>
#include <engine/e_engine.h>
#include <engine/e_linereader.h>

typedef struct LANGUAGE_LIST_ITEM
{
	const char * name[512];
	const char * filename[512];
} LANGUAGE_LIST_ITEM;

class LANGUAGES : public COMPONENT
{
private:
	static void langscan(const char *name, int is_dir, void *user);
public:
	std::vector<LANGUAGE_LIST_ITEM *> languages;
	
	virtual void on_init();
};

