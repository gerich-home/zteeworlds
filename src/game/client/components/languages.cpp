#include "languages.hpp"

void LANGUAGES::langscan(const char *name, int is_dir, void *user)
{
	LANGUAGES *self = (LANGUAGES *)user;

	if (is_dir) return;
	
	if (self)
	{
		IOHANDLE file;

		char fn_buf[512];
		str_format(fn_buf, sizeof(fn_buf), "languages/%s", name);
		file = engine_openfile(fn_buf, IOFLAG_READ);
		
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
	mem_copy(item->name, "English", str_length("English") + 1);
	memset(item->filename, 0, sizeof(item->filename));
	
	languages.push_back(item);
	
	engine_listdir(LISTDIRTYPE_ALL, "languages", langscan, this);
	
	mem_copy(prev_lang, config.language, sizeof(prev_lang));
	prev_lang[sizeof(prev_lang) - 1] = 0;
	
	lang_load(config.language);
}

void LANGUAGES::on_render()
{
	if (str_comp_nocase(prev_lang, config.language))
	{
		lang_load(config.language);
		
		mem_copy(prev_lang, config.language, sizeof(prev_lang));
		prev_lang[sizeof(prev_lang) - 1] = 0;
	}
}
