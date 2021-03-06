#include "e_language.h"

#include <stdlib.h>
#include <stdio.h>
#include <base/system.h>
#include <string.h>
#include "e_linereader.h"
#include "e_engine.h"
#include <vector>

void format_replace(char * text)
{
	int i = 0;
	int j = 0;
	int len = str_length(text);
	while (i < len)
	{
		if (text[i] == '\\' && text[i + 1] == '\\')
		{
			text[j] = '\\';
			i += 2;
			j++;
			continue;
		}
		if (text[i] == '\\' && text[i + 1] == 'n')
		{
			text[j] = '\n';
			i += 2;
			j++;
			continue;
		}
		if (text[i] == '\\' && text[i + 1] == 'r')
		{
			text[j] = '\r';
			i += 2;
			j++;
			continue;
		}
		text[j] = text[i];
		i++;
		j++;
	}
	mem_zero(text + j, len - j);
}

typedef struct LANGUAGE_ITEM
{
	int gid;
	const char * str;
	const char * translated;
	bool delete_str;
	bool delete_translated;
} LANGUAGE_ITEM;

std::vector<LANGUAGE_ITEM *> language;

void lang_init()
{
	language.clear();
}

void lang_free()
{
	if (language.size() == 0) return;
	for (int i = 0; i < language.size(); i++)
	{
		if (language[i])
		{
			if (language[i]->delete_str && language[i]->str)
			{
				free((void *)language[i]->str);
				language[i]->str = 0;
			}
			if (language[i]->delete_translated && language[i]->translated)
			{
				free((void *)language[i]->translated);
				language[i]->translated = 0;
			}
		}
		delete language[i];
		language[i] = 0;
	}
	language.clear();
}

int lang_load(const char * filename)
{
	lang_free();
	
	if (str_length(filename) == 0) return 1;
	
	IOHANDLE file;

	char fn_buf[512];
	str_format(fn_buf, sizeof(fn_buf), "languages/%s", filename);
	file = engine_openfile(fn_buf, IOFLAG_READ);

	while(file)
	{
		int gid = -1;
		const char * str = 0;
		char *line = "";
		LINEREADER lr;
		char buf[4096];

		linereader_init(&lr, file);
		if (!(line = linereader_get(&lr))) // skiping language name
		{
			io_close(file);
			break;
		}
		
		while (line = linereader_get(&lr))
		{
			format_replace(line);
			if (str_length(line) == 0 || line[0] == '\r' || line[0] == '\n') continue;
			if (!str)
			{
				sscanf(line, "%d", &gid);
				if (gid != -1)
				{
					int start = 0, len = str_length(line);
					while (line[len - 1] <= ' ' && len > 0) len--;
					while (line[start] >= '0' && line[start] <= '9') start++;
					if (line[start] != 0)
					{
						start++;
						if (line[start] != 0)
						{
							len -= start;
							mem_copy(buf, line + start, len);
							buf[len] = 0;
						} else buf[0] = 0;
					} else buf[0] = 0;
					str = buf;
				}
			} else {
				LANGUAGE_ITEM * langitem = new LANGUAGE_ITEM;
				langitem->gid = gid;
				langitem->str = strdup(str);
				langitem->translated = strdup(line);
				langitem->delete_str = true;
				langitem->delete_translated = true;
				
				language.push_back(langitem);
				
				gid = -1;
				str = 0;
			}
		}

		io_close(file);
		file = 0;
		
		dbg_msg("lang", "Language %s loaded", filename);

		return 0;
	}
	
	dbg_msg("lang", "Cannot load language %s", filename);

	return 1;
}

const char * _T(int gid, const char * str)
{
	if (!str) return str;
	char buf[4096];
	int start = 0, end = str_length(str);
	while (str[start] && (str[start] == ' ' || str[start] == '\n') && start < end) start++;
	mem_copy(buf, str + start, end - start + 1);
	end = str_length(buf) - 1;
	while (buf[end] && (buf[end] == ' ' || buf[end] == '\n') && end > 0) { buf[end] = 0; end--; };
	if (language.size() > 0)
	{
		for (int q = 0; q < 2; q++)
		{
			for (int i = 0; i < language.size(); i++)
			{
				if (!language[i] || !language[i]->str) continue;
				if ((language[i]->gid == gid || q == 1) && strcmp(language[i]->str, buf) == 0)
				{
					if (language[i]->translated)
						return language[i]->translated;
					else
						return language[i]->str;
				}
			}
		}
	}
	
	LANGUAGE_ITEM * langitem = new LANGUAGE_ITEM;
	langitem->gid = gid;
	langitem->str = strdup(buf);
	langitem->translated = 0;
	langitem->delete_str = true;
	langitem->delete_translated = false;
	
	language.push_back(langitem);
	
	#ifdef CONF_DEBUG
	dbg_msg("lang", "%d %s", gid, str);
	#endif
	
	return str;
}

int get_gid(const char * str)
{
	int q = 0;
	for (int i = 0; i < strlen(str); i++)
	{
		q = (q + str[i] * i)%65536;
	}
	return q;
}