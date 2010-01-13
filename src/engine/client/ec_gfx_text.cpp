#include <base/system.h>
#include <string.h>
#include <engine/e_client_interface.h>

#include <SDL_opengl.h>

static int word_length(const char *text)
{
	int s = 1;
	while(1)
	{
		if(*text == 0)
			return s-1;
		if(*text == '\n' || *text == '\t' || *text == ' ')
			return s;
		text++;
		s++;
	}
}

static float text_r=1;
static float text_g=1;
static float text_b=1;
static float text_a=1;

static FONT_SET *default_font_set = 0;
void gfx_text_set_default_font(void *font)
{
	default_font_set = (FONT_SET *)font;
}


void gfx_text_set_cursor(TEXT_CURSOR *cursor, float x, float y, float font_size, int flags)
{
	mem_zero(cursor, sizeof(*cursor));
	cursor->font_size = font_size;
	cursor->start_x = x;
	cursor->start_y = y;
	cursor->x = x;
	cursor->y = y;
	cursor->line_count = 1;
	cursor->line_width = -1;
	cursor->flags = flags;
	cursor->charcount = 0;
}

void gfx_text_ex(TEXT_CURSOR *cursor, const char *text, int length)
{
	FONT_SET *font_set = cursor->font_set;
	float screen_x0, screen_y0, screen_x1, screen_y1;
	float fake_to_screen_x, fake_to_screen_y;
	int actual_x, actual_y;
	CFont *pFont = FreeTypeTextRenderer.GetDefaultFont();
	CFontSizeData *pSizeData = NULL;

	FONT *font;
	int actual_size;
	int i;
	int got_new_line = 0;
	float draw_x, draw_y;
	float cursor_x, cursor_y;
	const char *end;
	
	static int smileys_texture = -1;
	
	if (cursor->flags&TEXTFLAG_SMILEYS && smileys_texture < 0)
		smileys_texture = gfx_load_texture("smileys.png", IMG_AUTO, TEXLOAD_NOMIPMAPS|TEXLOAD_NORESAMPLE);

	float size = cursor->font_size;
	
	/* to correct coords, convert to screen coords, round, and convert back */
	gfx_getscreen(&screen_x0, &screen_y0, &screen_x1, &screen_y1);
	
	fake_to_screen_x = (gfx_screenwidth()/(screen_x1-screen_x0));
	fake_to_screen_y = (gfx_screenheight()/(screen_y1-screen_y0));
	actual_x = cursor->x * fake_to_screen_x;
	actual_y = cursor->y * fake_to_screen_y;

	cursor_x = actual_x / fake_to_screen_x;
	cursor_y = actual_y / fake_to_screen_y;

	/* same with size */
	actual_size = size * fake_to_screen_y;
	size = actual_size / fake_to_screen_y;

	if(!font_set)
		font_set = default_font_set;

	font = font_set_pick(font_set, actual_size);
	pSizeData = FreeTypeTextRenderer.GetSize(pFont, actual_size);
	FreeTypeTextRenderer.RenderSetup(pFont, actual_size);

	if (length < 0)
		length = strlen(text);
		
	end = text + length;

	/* if we don't want to render, we can just skip the first outline pass */
	i = 1;
	if(cursor->flags&TEXTFLAG_RENDER)
		i = 0;

	for(;i < 2; i++)
	{
		const unsigned char *current = (unsigned char *)text;
		int to_render = length;
		draw_x = cursor_x;
		draw_y = cursor_y;
		bool prev_utf8 = false;
		int prev_char = 0;
		bool has_gfx_begin = false;
		
		while(to_render > 0)
		{
			int new_line = 0;
			int this_batch = to_render;
			if(cursor->line_width > 0 && !(cursor->flags&TEXTFLAG_STOP_AT_END))
			{
				int wlen = word_length((char *)current);
				TEXT_CURSOR compare = *cursor;
				compare.x = draw_x;
				compare.y = draw_y;
				compare.flags &= ~TEXTFLAG_RENDER;
				compare.line_width = -1;
				gfx_text_ex(&compare, text, wlen);
				
				if(compare.x-draw_x > cursor->line_width)
				{
					/* word can't be fitted in one line, cut it */
					TEXT_CURSOR cutter = *cursor;
					cutter.charcount = 0;
					cutter.x = draw_x;
					cutter.y = draw_y;
					cutter.flags &= ~TEXTFLAG_RENDER;
					cutter.flags |= TEXTFLAG_STOP_AT_END;
					
					gfx_text_ex(&cutter, (const char *)current, wlen);
					wlen = cutter.charcount;
					new_line = 1;
					
					if(wlen <= 3) /* if we can't place 3 chars of the word on this line, take the next */
						wlen = 0;
				}
				else if(compare.x-cursor->start_x > cursor->line_width)
				{
					new_line = 1;
					wlen = 0;
				}
				
				this_batch = wlen;
			}
			
			if((const char *)current+this_batch > end)
				this_batch = (const char *)end-(const char *)current;
			
			to_render -= this_batch;

			while(this_batch > 0)
			{
				float tex_x0, tex_y0, tex_x1, tex_y1;
				float width, height;
				float x_offset, y_offset, x_advance;
				float advance = 0;
				
				const unsigned char *tmp;
				int character = 0;
				int nextcharacter = 0;
				bool isUtf8Char = false;
				
				const char * prev_ptr = (const char *)current;

				/*character = str_utf8_decode((const char **)(&current));
				if ((void *)prev_ptr == (void *)current || str_utf8_char_length(character) == 0)
				{
					character = *((unsigned char *)current);
					current++;
					isUtf8Char = false;
					this_batch--;
				} else {
					isUtf8Char = config.gfx_freetype_font ? true : str_utf8_char_length(character) > 1;
					this_batch -= str_utf8_char_length(character);
				}
				{
					tmp = current;
					nextcharacter = str_utf8_decode((const char **)(&tmp));
					if (tmp == current || str_utf8_char_length(nextcharacter) == 0)
					{
						nextcharacter = *((unsigned char *)current);
					}
					//if ((void *)nextcharacter > (void *)end) break;
					//this_batch -= str_utf8_char_length(character);
				}*/
				character = str_utf8_decode((const char **)(&current));
				if ((void *)prev_ptr == (void *)current)
				{
					current++;
					this_batch--;
					continue;
				}
				
				this_batch -= str_utf8_char_length(character);
				if (str_utf8_char_length(character) == 0) this_batch -= 1;
				isUtf8Char = config.gfx_freetype_font ? true : str_utf8_char_length(character) > 1;
			
				tmp = current;
				nextcharacter = str_utf8_decode((const char **)(&tmp));
				
				if(character == '\n')
				{
					draw_x = cursor->start_x;
					draw_y += size;
					draw_x = (int)(draw_x * fake_to_screen_x) / fake_to_screen_x; /* realign */
					draw_y = (int)(draw_y * fake_to_screen_y) / fake_to_screen_y;
					continue;
				}

				if (!isUtf8Char)
				{
					if(cursor->flags&TEXTFLAG_RENDER)
					{
						if (has_gfx_begin)
						{
							gfx_quads_end();
							has_gfx_begin = false;
						}
						
						if (i == 0)
							gfx_texture_set(font->outline_texture);
						else
							gfx_texture_set(font->text_texture);

						gfx_quads_begin();
						has_gfx_begin = true;
						
						if (i == 0)
							gfx_setcolor(0.0f, 0.0f, 0.0f, 0.3f*text_a);
						else
							gfx_setcolor(text_r, text_g, text_b, text_a);
					}
					
					font_character_info(font, character, &tex_x0, &tex_y0, &tex_x1, &tex_y1, &width, &height, &x_offset, &y_offset, &x_advance);

					if(cursor->flags&TEXTFLAG_RENDER)
					{
						gfx_quads_setsubset(tex_x0, tex_y0, tex_x1, tex_y1);
						
						if (config.gfx_text_shadows && i == 0)
						{
							gfx_setcolor(0.0f, 0.0f, 0.0f, 0.5f * text_a);
							gfx_quads_drawTL(draw_x+x_offset*size + 1, draw_y+y_offset*size + 1, width*size, height*size);
							gfx_setcolor(0.0f, 0.0f, 0.0f, 0.3f*text_a);
						}
						
						gfx_quads_drawTL(draw_x+x_offset*size, draw_y+y_offset*size, width*size, height*size);
					}

					advance = x_advance + font_kerning(font, *current, *(current+1));
					
					if(cursor->flags&TEXTFLAG_RENDER)
					{
						prev_utf8 = false;
					}
				} else
				if (character >= 0xFFF00 && character <= 0xFFF10 && cursor->flags&TEXTFLAG_SMILEYS)	// Using reserved for private using unicode area for smiles
				{
					int smile_index = character - 0xFFF00;
					
					if (cursor->flags&TEXTFLAG_RENDER)
					{
						if (has_gfx_begin)
						{
							gfx_quads_end();
							has_gfx_begin = false;
						}
						
						// A little hack to displaying smiles at baseline...
						font_character_info(font, 'O', &tex_x0, &tex_y0, &tex_x1, &tex_y1, &width, &height, &x_offset, &y_offset, &x_advance);
						
						float smile_size = width > height ? width : height;
						smile_size *= 1.1f;
						y_offset -= smile_size - (width > height ? width : height);
					
						gfx_texture_set(smileys_texture);
						gfx_quads_begin();
							gfx_quads_setsubset((float)(smile_index%4) * 0.25f, (float)(smile_index/4) * 0.25f, (float)(smile_index%4 + 1) * 0.25f, (float)((smile_index/4) + 1) * 0.25f);
							if (config.gfx_text_shadows && i == 0)
							{
								gfx_setcolor(0.0f, 0.0f, 0.0f, 0.5f * text_a);
								gfx_quads_drawTL(draw_x+0*size + 1, draw_y+y_offset*size + 1, smile_size*size, smile_size*size);
							}
							gfx_setcolor(1.0f, 1.0f, 1.0f, 1.0f);
							gfx_quads_drawTL(draw_x+0*size, draw_y+y_offset*size, smile_size*size, smile_size*size);
						gfx_quads_end();
					}
					
					advance = 1.2f;
				} else
				if (pFont)
				{
					CFontChar * Char = FreeTypeTextRenderer.GetChar(pFont, pSizeData, character);
					if (Char)
					{
						if (cursor->flags&TEXTFLAG_RENDER)
						{
							if (has_gfx_begin)
							{
								gfx_quads_end();
								has_gfx_begin = false;
							}
							gfx_texture_set(pSizeData->m_aTextureIds[1 - i]);
							gfx_quads_begin();
							has_gfx_begin = true;
							
							if (i == 0)
								gfx_setcolor(0.0f, 0.0f, 0.0f, 0.3f*text_a);
							else
								gfx_setcolor(text_r, text_g, text_b, text_a);
						
							gfx_quads_setsubset(Char->m_aUvs[0], Char->m_aUvs[1], Char->m_aUvs[2], Char->m_aUvs[3]);
							
							if (config.gfx_text_shadows && i == 0)
							{
								gfx_setcolor(0.0f, 0.0f, 0.0f, 0.5f * text_a);
								gfx_quads_drawTL(draw_x+Char->m_OffsetX*size+1, draw_y+Char->m_OffsetY*size+1, Char->m_Width*size, Char->m_Height*size);
								gfx_setcolor(0.0f, 0.0f, 0.0f, 0.3f*text_a);
							}
							gfx_quads_drawTL(draw_x+Char->m_OffsetX*size, draw_y+Char->m_OffsetY*size, Char->m_Width*size, Char->m_Height*size);
							
							prev_utf8 = true;
						}

						advance = Char->m_AdvanceX + FreeTypeTextRenderer.Kerning(pFont, character, nextcharacter)/size;
					} else dbg_msg("font", "no char: %c (%d)", character, character);
				}
								
				if(cursor->flags&TEXTFLAG_STOP_AT_END && draw_x+advance*size-cursor->start_x > cursor->line_width)
				{
					/* we hit the end of the line, no more to render or count */
					to_render = 0;
					break;
				}

				draw_x += advance*size;
				cursor->charcount++;
				prev_char = character;
			}
			
			if(new_line)
			{
				draw_x = cursor->start_x;
				draw_y += size;
				got_new_line = 1;
				draw_x = (int)(draw_x * fake_to_screen_x) / fake_to_screen_x; /* realign */
				draw_y = (int)(draw_y * fake_to_screen_y) / fake_to_screen_y;				
			}
		}
		
		if (cursor->flags&TEXTFLAG_RENDER && has_gfx_begin)
			gfx_quads_end();
	}

	cursor->x = draw_x;
	
	if(got_new_line)
		cursor->y = draw_y;
}

void gfx_text(void *font_set_v, float x, float y, float size, const char *text, int max_width)
{
	TEXT_CURSOR cursor;
	gfx_text_set_cursor(&cursor, x, y, size, TEXTFLAG_RENDER);
	cursor.line_width = max_width;
	gfx_text_ex(&cursor, text, -1);
}

float gfx_text_width(void *font_set_v, float size, const char *text, int length)
{
	TEXT_CURSOR cursor;
	gfx_text_set_cursor(&cursor, 0, 0, size, 0);
	gfx_text_ex(&cursor, text, length);
	return cursor.x;
}

void gfx_text_color(float r, float g, float b, float a)
{
	text_r = r;
	text_g = g;
	text_b = b;
	text_a = a;
}
