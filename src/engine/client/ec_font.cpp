/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <base/system.h>
#include <engine/e_client_interface.h>
#include <engine/e_engine.h>
#include "ec_font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <vector>
#include <string.h>

#include <SDL_opengl.h>

static int font_sizes[] = {8,9,10,11,12,13,14,15,16,17,18,19,20,36};
#define NUM_FONT_SIZES (sizeof(font_sizes)/sizeof(int))

static FT_Library ft_library;
static bool ft_FontInitialized = false;

void ft_font_init()
{
	if (ft_FontInitialized) return;
	FT_Init_FreeType(&ft_library);
	ft_FontInitialized = true;
}

ft_FONT *ft_font_load(const char *filename)
{
	if (!ft_FontInitialized) ft_font_init();

	ft_FONT * font = new ft_FONT;

	font->filename = strdup(filename);

	font->chars.clear();

	font->face = 0;
	FT_New_Face(ft_library, filename, 0, &font->face);
	if (!font->face)
	{
		delete font;
		return NULL;
	}

	return font;
}

void ft_font_destroy(ft_FONT *font)
{
	if (!font) return;
	if (!ft_FontInitialized) ft_font_init();

	if (font->chars.size() > 0)
	{
		for (int i = 0; i < font->chars.size(); i++)
		{
			delete font->chars[i];
			font->chars[i] = NULL;
		}
		font->chars.clear();
	}

	delete font;
}

void grow(unsigned char *in, unsigned char *out, int w, int h, int quad)
{
	for (int q = 0; q < quad; q++)
	{
        for(int y = 0; y < h; y++)
                for(int x = 0; x < w; x++)
                {
                        int c = in[(y*w+x)*quad+q];

                        for (int s_y = -1; s_y <= 1; s_y++)
                            for (int s_x = -1; s_x <= 1; s_x++)
                            {
                                int get_x = x+s_x;
                                int get_y = y+s_y;
                                if (get_x >= 0 && get_y >= 0 && get_x < w && get_y < h)
                                {
                                    int index = get_y*w+get_x;
									index = index*quad+q;
                                    if(in[index] > c)
                                        c = in[index];
                                }
							}

                        out[(y*w+x)*quad+q] = c;
                }
	}
}

inline int next_p2(int a)
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}

ft_FONTCHAR * ft_font_get_char(ft_FONT * font, int c, int size)
{
	if (!font) return NULL;
	if (!ft_FontInitialized) ft_font_init();
  
	if (font->chars.size() > 0)
	{
		for (int i = 0; i < font->chars.size(); i++)
		{
			if (font->chars[i]->id == c && font->chars[i]->size == size) return font->chars[i];
		}
	}
	
	ft_FONTCHAR * chr = new ft_FONTCHAR;
	
	chr->id = c;
	chr->size = size;
	
	chr->texture[0] = 0;
	chr->texture[1] = 0;

	chr->tex_w = 0.0f;
	chr->tex_h = 0.0f;
	chr->x_offset = 0.0f;
	chr->y_offset = 0.0f;
	chr->w = 0.0f;
	chr->h = 0.0f;

	FT_Set_Pixel_Sizes(font->face, 0, size);
	
	FT_Load_Glyph(font->face, FT_Get_Char_Index(font->face, c), FT_LOAD_DEFAULT);
	
	FT_Glyph glyph;
	FT_Get_Glyph(font->face->glyph, &glyph);
	
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	
	FT_Bitmap & bitmap = bitmap_glyph->bitmap;
	
	int width = next_p2(bitmap.width + 6);
	int height = next_p2(bitmap.rows + 6);
	
	if (width > height) height = width; else width = height;
	
	GLubyte * expanded_data = new GLubyte[width * height];
	GLubyte * expanded_data2 = new GLubyte[width * height];
	
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			expanded_data[(i + (j) * width)] = (i < 3 || i >= bitmap.width + 3 || j < 3 || j >= bitmap.rows + 3) ? 0 : bitmap.buffer[i - 3 + (j - 3) * bitmap.width];
		}
	}

	chr->texture[0] = gfx_load_texture_raw(width, height, IMG_ALPHA, expanded_data, IMG_ALPHA, TEXLOAD_NOMIPMAPS);

	int outline_thickness = size >= 18 ? 3 : 2;
	grow(expanded_data, expanded_data2, width, height, 1);
	if (size >= 18)
	{
		grow(expanded_data2, expanded_data, width, height, 1);
		memcpy(expanded_data2, expanded_data, width * height);
	}

	chr->texture[1] = gfx_load_texture_raw(width, height, IMG_ALPHA, expanded_data2, IMG_ALPHA, TEXLOAD_NOMIPMAPS);
	
	delete [] expanded_data;
	delete [] expanded_data2;

	chr->tex_w = (float)(bitmap.width + 6) / (float)width;
	chr->tex_h = (float)(bitmap.rows + 6) / (float)height;
	
	float scale = 1.0f/size;
	int _height = bitmap.rows + outline_thickness * 2 + 2;
	int _width = bitmap.width + outline_thickness * 2 + 2;

	chr->w = _width * scale;
	chr->h = _height * scale;
	chr->x_offset = (font->face->glyph->bitmap_left-1) * scale;
	chr->y_offset = (size - bitmap_glyph->top) * scale;
	chr->advance = (font->face->glyph->advance.x>>6) * scale;
	
	font->chars.push_back(chr);
	
	return chr;
}

static int ft_font_get_index(int pixelsize)
{
	int i;
	for(i = 0; i < NUM_FONT_SIZES; i++)
	{
		if(font_sizes[i] >= pixelsize)
			return i;
	}
	
	return NUM_FONT_SIZES-1;
}

typedef struct
{
    short x, y;
    short width, height;
    short x_offset, y_offset;
    short x_advance;
} FONT_CHARACTER;

typedef struct
{
    short size;
    short width, height;

    short line_height;
    short base;

    FONT_CHARACTER characters[256];

    short kerning[256*256];
} FONT_DATA;

int font_load(FONT *font, const char *filename)
{
    FONT_DATA font_data;
	IOHANDLE file;

	file = engine_openfile(filename, IOFLAG_READ);
	
	if(file)
	{
        int i;

        io_read(file, &font_data, sizeof(FONT_DATA));
        io_close(file);

#if defined(CONF_ARCH_ENDIAN_BIG)
        swap_endian(&font_data, 2, sizeof(FONT_DATA)/2);
#endif

        {
            float scale_factor_x = 1.0f/font_data.size;
            float scale_factor_y = 1.0f/font_data.size;
            float scale_factor_tex_x = 1.0f/font_data.width;
            float scale_factor_tex_y = 1.0f/font_data.height;

            for (i = 0; i < 256; i++)
            {
                float tex_x0 = font_data.characters[i].x*scale_factor_tex_x;
                float tex_y0 = font_data.characters[i].y*scale_factor_tex_y;
                float tex_x1 = (font_data.characters[i].x+font_data.characters[i].width)*scale_factor_tex_x;
                float tex_y1 = (font_data.characters[i].y+font_data.characters[i].height)*scale_factor_tex_y;

                float width = font_data.characters[i].width*scale_factor_x;
                float height = font_data.characters[i].height*scale_factor_y;
                float x_offset = font_data.characters[i].x_offset*scale_factor_x;
                float y_offset = font_data.characters[i].y_offset*scale_factor_y;
                float x_advance = (font_data.characters[i].x_advance)*scale_factor_x;

                font->characters[i].tex_x0 = tex_x0;
                font->characters[i].tex_y0 = tex_y0;
                font->characters[i].tex_x1 = tex_x1;
                font->characters[i].tex_y1 = tex_y1;
                font->characters[i].width = width;
                font->characters[i].height = height;
                font->characters[i].x_offset = x_offset;
                font->characters[i].y_offset = y_offset;
                font->characters[i].x_advance = x_advance;

            }

            for (i = 0; i < 256*256; i++)
            {
                font->kerning[i] = (char)font_data.kerning[i];
            }
        }

        return 0;
    }
    else
        return -1;
}

/*int gfx_load_texture(const char *filename, int store_format, int flags);
#define IMG_ALPHA 2*/

int font_set_load(FONT_SET *font_set, const char *font_filename, const char *text_texture_filename, const char *outline_texture_filename, const char * ft_font_filename, int fonts, ...)
{
    int i;
    va_list va; 

    font_set->font_count = fonts;

    va_start(va, fonts); 
    for (i = 0; i < fonts; i++)
    {
        int size;
        char composed_font_filename[256];
        char composed_text_texture_filename[256];
        char composed_outline_texture_filename[256];
        FONT *font = &font_set->fonts[i];

        size = va_arg(va, int);
        str_format(composed_font_filename, sizeof(composed_font_filename), font_filename, size);
        str_format(composed_text_texture_filename, sizeof(composed_text_texture_filename), text_texture_filename, size);
        str_format(composed_outline_texture_filename, sizeof(composed_outline_texture_filename), outline_texture_filename, size);

        if (font_load(font, composed_font_filename))
        {
            dbg_msg("font/loading", "failed loading font %s.", composed_font_filename);
            va_end(va);
            return -1;
        }

        font->size = size;
        font->text_texture = gfx_load_texture(composed_text_texture_filename, IMG_ALPHA, TEXLOAD_NORESAMPLE);
        font->outline_texture = gfx_load_texture(composed_outline_texture_filename, IMG_ALPHA, TEXLOAD_NORESAMPLE);
    }

    va_end(va);
	
	font_set->ft_font = ft_font_load(ft_font_filename);
	if (!font_set->ft_font)
	{
		dbg_msg("font/loading", "failed loading freetype font %s.", ft_font_filename);
		return -1;
	}
	
    return 0;
}

float font_text_width(FONT *font, const char *text, float size, int length)
{
    float width = 0.0f;

    const unsigned char *c = (unsigned char *)text;
    int chars_left;
    if (length == -1)
        chars_left = strlen(text);
    else
        chars_left = length;

    while (chars_left--)
    {
        float tex_x0, tex_y0, tex_x1, tex_y1;
        float char_width, char_height;
        float x_offset, y_offset, x_advance;
        float advance;

        font_character_info(font, *c, &tex_x0, &tex_y0, &tex_x1, &tex_y1, &char_width, &char_height, &x_offset, &y_offset, &x_advance);

        advance = x_advance + font_kerning(font, *c, *(c+1));

        width += advance;

        c++;
    }

    return width*size;
}

void font_character_info(FONT *font, unsigned char c, float *tex_x0, float *tex_y0, float *tex_x1, float *tex_y1, float *width, float *height, float *x_offset, float *y_offset, float *x_advance)
{
    CHARACTER *character = &font->characters[c];

    *tex_x0 = character->tex_x0;
    *tex_y0 = character->tex_y0;
    *tex_x1 = character->tex_x1;
    *tex_y1 = character->tex_y1;
    *width = character->width;
    *height = character->height;
    *x_offset = character->x_offset;
    *y_offset = character->y_offset;
    *x_advance = character->x_advance;
}

float font_kerning(FONT *font, unsigned char c1, unsigned char c2)
{
    return font->kerning[c1 + c2*256] / (float)font->size;
}

FONT *font_set_pick(FONT_SET *font_set, float size)
{
    int i;
    FONT *picked_font = 0x0;

    for (i = font_set->font_count-1; i >= 0; i--)
    {
        FONT *font = &font_set->fonts[i];

        if (font->size >= size)
            picked_font = font;
    }

    if (!picked_font)
        picked_font = &font_set->fonts[font_set->font_count-1];

    return picked_font;
}
