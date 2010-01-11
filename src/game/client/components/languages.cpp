#include "languages.hpp"

void LANGUAGES::langscan(const char *name, int is_dir, void *user)
{
	LANGUAGES *self = (LANGUAGES *)user;

	if (is_dir) return;
	
	if (self)
	{
		IOHANDLE file;

		file = engine_openfile(name, IOFLAG_READ);
		
		if (file)
		{
			char *line = "";
			LINEREADER lr;

			linereader_init(&lr, file);
			if (!(line = linereader_get(&lr))) 
			{
				io_close(file);
				return; 
			}
			
			LANGUAGE_LIST_ITEM * item = new LANGUAGE_LIST_ITEM;
			mem_copy(item->name, line, str_length(line) + 1);
			mem_copy(item->filename, name, str_length(name) + 1);
			
			self->languages.push_back(item);
		}
	}
}

void LANGUAGES::on_init()
{
	languages.clear();
	LANGUAGE_LIST_ITEM * item = new LANGUAGE_LIST_ITEM;
	mem_copy(item->name, "---", 4);
	memset(item->filename, 0, sizeof(item->filename));
	
	languages.push_back(item);
	
	engine_listdir(LISTDIRTYPE_ALL, "languages", langscan, this);
	
	lang_load(config.language);
}
