#include <engine/e_client_interface.h>
#include <string.h> // strlen
#include "lineinput.hpp"

LINEINPUT::LINEINPUT()
{
	clear();
}

void LINEINPUT::clear()
{
	mem_zero(str, sizeof(str));
	len = 0;
	cursor_pos = 0;
}

void LINEINPUT::set(const char *string)
{
	str_copy(str, string, sizeof(str));
	len = strlen(str);
	cursor_pos = len;
}

void LINEINPUT::process_input(INPUT_EVENT e)
{
	if(cursor_pos > len)
		cursor_pos = len;
	
	unsigned int c = e.ch;
	int k = e.key;
	
	// 127 is produced on Mac OS X and corresponds to the delete key
	if (!(c >= 0 && c < 32) && c != 127)
	{
		char temp[5];
		int char_len = str_utf8_encode(temp, c);
		if (len < sizeof(str) - char_len && cursor_pos < sizeof(str) - char_len)
		{
			memmove(str + cursor_pos + char_len, str + cursor_pos, len - cursor_pos + 1);
			str_utf8_encode(&str[cursor_pos], c);
			cursor_pos += char_len;
			len += char_len;
		}
	}
	
	if(e.flags&INPFLAG_PRESS)
	{
		if (k == KEY_BACKSPACE && cursor_pos > 0)
		{
			while (1)
			{
				bool is_start = str_utf8_isstart(str[cursor_pos - 1]);
				memmove(str + cursor_pos - 1, str + cursor_pos, len - cursor_pos + 1);
				cursor_pos--;
				len--;
				if (cursor_pos == 0 || is_start) break;
			}
		}
		else if (k == KEY_DELETE && cursor_pos < len)
		{
			do
			{
				memmove(str + cursor_pos, str + cursor_pos + 1, len - cursor_pos);
				len--;
			}
			while (len > 0 && !str_utf8_isstart(str[cursor_pos]));
		}
		else if (k == KEY_LEFT && cursor_pos > 0)
		{
			do
			{
				cursor_pos--;
			}
			while (cursor_pos > 0 && !str_utf8_isstart(str[cursor_pos]));
		}
		else if (k == KEY_RIGHT && cursor_pos < len)
		{
			do
			{
				cursor_pos++;
			}
			while (cursor_pos < len && !str_utf8_isstart(str[cursor_pos]));
		}
		else if (k == KEY_HOME)
			cursor_pos = 0;
		else if (k == KEY_END)
			cursor_pos = len;
	}
}
