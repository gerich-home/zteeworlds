#include <base/system.h>
#include <base/math.hpp>
#include <engine/e_if_gfx.h>

#ifdef CONF_FAMILY_WINDOWS
	#include <windows.h>
#endif

#include <SDL_opengl.h>

// ft2 texture
#include <ft2build.h>
#include FT_FREETYPE_H

#include "ec_new_text_render.h"


int CTextRender::GetFontSizeIndex(int Pixelsize)
{
	for(unsigned i = 0; i < NUM_FONT_SIZES; i++)
	{
		if(aFontSizes[i] >= Pixelsize)
			return i;
	}
	
	return NUM_FONT_SIZES-1;
}

void CTextRender::Grow(unsigned char *pIn, unsigned char *pOut, int w, int h)
{
	for(int y = 0; y < h; y++) 
		for(int x = 0; x < w; x++) 
		{ 
			int c = pIn[y*w+x]; 

			for(int sy = -1; sy <= 1; sy++)
				for(int sx = -1; sx <= 1; sx++)
				{
					int GetX = x+sx;
					int GetY = y+sy;
					if (GetX >= 0 && GetY >= 0 && GetX < w && GetY < h)
					{
						int Index = GetY*w+GetX;
						if(pIn[Index] > c)
							c = pIn[Index]; 
					}
				}

			pOut[y*w+x] = c;
		}
}

void CTextRender::InitTexture(CFontSizeData *pSizeData, int CharWidth, int CharHeight, int Xchars, int Ychars)
{
	static int FontMemoryUsage = 0;
	int Width = CharWidth*Xchars;
	int Height = CharHeight*Ychars;
	void *pMem = mem_alloc(Width*Height, 1);
	mem_zero(pMem, Width*Height);
	
	glEnable(GL_TEXTURE_2D);
	if(pSizeData->m_aTextures[0] == 0)
	{
		glGenTextures(2, pSizeData->m_aTextures);
		for (int i = 0; i < 2; i++)
		{
			pSizeData->m_aTextureIds[i] = gfx_add_texture(pSizeData->m_aTextures[i], Width, Height);
		}
	}
	else
		FontMemoryUsage -= pSizeData->m_TextureWidth*pSizeData->m_TextureHeight*2;
	
	pSizeData->m_NumXChars = Xchars;
	pSizeData->m_NumYChars = Ychars;
	pSizeData->m_TextureWidth = Width;
	pSizeData->m_TextureHeight = Height;
	pSizeData->m_CurrentCharacter = 0;
	
	for(int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, pSizeData->m_aTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		gluBuild2DMipmaps(GL_TEXTURE_2D, m_FontTextureFormat, Width, Height, m_FontTextureFormat, GL_UNSIGNED_BYTE, pMem);
		FontMemoryUsage += Width*Height;
	}
	
	dbg_msg("", "pFont memory usage: %d", FontMemoryUsage);
	
	mem_free(pMem);
}

void CTextRender::IncreaseTextureSize(CFontSizeData *pSizeData)
{
	if(pSizeData->m_TextureWidth < pSizeData->m_TextureHeight)
		pSizeData->m_NumXChars <<= 1;
	else
		pSizeData->m_NumYChars <<= 1;
	InitTexture(pSizeData, pSizeData->m_CharMaxWidth, pSizeData->m_CharMaxHeight, pSizeData->m_NumXChars, pSizeData->m_NumYChars);		
}


void CTextRender::InitIndex(CFont *pFont, int Index)
{
	int OutlineThickness = 1;
	CFontSizeData *pSizeData = &pFont->m_aSizes[Index];
	
	pSizeData->m_FontSize = aFontSizes[Index];
	FT_Set_Pixel_Sizes(pFont->m_FtFace, 0, pSizeData->m_FontSize);
	
	if(pSizeData->m_FontSize >= 18)
		OutlineThickness = 2;
		
	{
		unsigned GlyphIndex;
		int MaxH = 0;
		int MaxW = 0;
		
		int Charcode = FT_Get_First_Char(pFont->m_FtFace, &GlyphIndex);
		while(GlyphIndex != 0)
		{   
			// do stuff
			FT_Load_Glyph(pFont->m_FtFace, GlyphIndex, FT_LOAD_DEFAULT);
			
			if(pFont->m_FtFace->glyph->metrics.width > MaxW) MaxW = pFont->m_FtFace->glyph->metrics.width; // ignore_convention
			if(pFont->m_FtFace->glyph->metrics.height > MaxH) MaxH = pFont->m_FtFace->glyph->metrics.height; // ignore_convention
			Charcode = FT_Get_Next_Char(pFont->m_FtFace, Charcode, &GlyphIndex);
		}
		
		MaxW = (MaxW>>6)+2+OutlineThickness*2;
		MaxH = (MaxH>>6)+2+OutlineThickness*2;
		
		for(pSizeData->m_CharMaxWidth = 1; pSizeData->m_CharMaxWidth < MaxW; pSizeData->m_CharMaxWidth <<= 1);
		for(pSizeData->m_CharMaxHeight = 1; pSizeData->m_CharMaxHeight < MaxH; pSizeData->m_CharMaxHeight <<= 1);
	}
	
	InitTexture(pSizeData, pSizeData->m_CharMaxWidth, pSizeData->m_CharMaxHeight, 8, 8);
}

CFontSizeData *CTextRender::GetSize(CFont *pFont, int Pixelsize)
{
	int Index = GetFontSizeIndex(Pixelsize);
	if(pFont->m_aSizes[Index].m_FontSize != aFontSizes[Index])
		InitIndex(pFont, Index);
	return &pFont->m_aSizes[Index];
}


void CTextRender::UploadGlyph(CFontSizeData *pSizeData, int Texnum, int SlotId, int Chr, const void *pData)
{
	int x = (SlotId%pSizeData->m_NumXChars) * (pSizeData->m_TextureWidth/pSizeData->m_NumXChars);
	int y = (SlotId/pSizeData->m_NumXChars) * (pSizeData->m_TextureHeight/pSizeData->m_NumYChars);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pSizeData->m_aTextures[Texnum]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
		pSizeData->m_TextureWidth/pSizeData->m_NumXChars,
		pSizeData->m_TextureHeight/pSizeData->m_NumYChars,
		m_FontTextureFormat, GL_UNSIGNED_BYTE, pData);
}

int CTextRender::GetSlot(CFontSizeData *pSizeData)
{
	int CharCount = pSizeData->m_NumXChars*pSizeData->m_NumYChars;
	if(pSizeData->m_CurrentCharacter < CharCount)
	{
		int i = pSizeData->m_CurrentCharacter;
		pSizeData->m_CurrentCharacter++;
		return i;
	}

	// kick out the oldest
	// TODO: remove this linear search
	{
		int Oldest = 0;
		for(int i = 1; i < CharCount; i++)
		{
			if(pSizeData->m_aCharacters[i].m_TouchTime < pSizeData->m_aCharacters[Oldest].m_TouchTime)
				Oldest = i;
		}
		
		if(time_get()-pSizeData->m_aCharacters[Oldest].m_TouchTime < time_freq())
		{
			IncreaseTextureSize(pSizeData);
			return GetSlot(pSizeData);
		}
		
		return Oldest;
	}
}

int CTextRender::RenderGlyph(CFont *pFont, CFontSizeData *pSizeData, int Chr)
{
	FT_Bitmap *pBitmap;
	int SlotId = 0;
	int SlotW = pSizeData->m_TextureWidth / pSizeData->m_NumXChars;
	int SlotH = pSizeData->m_TextureHeight / pSizeData->m_NumYChars;
	int SlotSize = SlotW*SlotH;
	int OutlineThickness = 1;
	int x = 1;
	int y = 1;
	int px, py;

	FT_Set_Pixel_Sizes(pFont->m_FtFace, 0, pSizeData->m_FontSize);

	if(FT_Load_Char(pFont->m_FtFace, Chr, FT_LOAD_RENDER|FT_LOAD_NO_BITMAP))
	{
		dbg_msg("pFont", "error loading glyph %d", Chr);
		return -1;
	}

	pBitmap = &pFont->m_FtFace->glyph->bitmap; // ignore_convention
	
	// fetch slot
	SlotId = GetSlot(pSizeData);
	if(SlotId < 0)
		return -1;
	
	// adjust spacing
	if(pSizeData->m_FontSize >= 18)
		OutlineThickness = 2;
	x += OutlineThickness;
	y += OutlineThickness;

	// prepare glyph data
	mem_zero(ms_aGlyphData, SlotSize);

	if(pBitmap->pixel_mode == FT_PIXEL_MODE_GRAY) // ignore_convention
	{
		for(py = 0; py < pBitmap->rows; py++) // ignore_convention
			for(px = 0; px < pBitmap->width; px++) // ignore_convention
				ms_aGlyphData[(py+y)*SlotW+px+x] = pBitmap->buffer[py*pBitmap->pitch+px]; // ignore_convention
	}
	else if(pBitmap->pixel_mode == FT_PIXEL_MODE_MONO) // ignore_convention
	{
		for(py = 0; py < pBitmap->rows; py++)  // ignore_convention
			for(px = 0; px < pBitmap->width; px++) // ignore_convention
			{
				if(pBitmap->buffer[py*pBitmap->pitch+px/8]&(1<<(7-(px%8)))) // ignore_convention
					ms_aGlyphData[(py+y)*SlotW+px+x] = 255;
			}
	}
	
	// upload the glyph
	UploadGlyph(pSizeData, 0, SlotId, Chr, ms_aGlyphData);
	
	if(OutlineThickness == 1)
	{
		Grow(ms_aGlyphData, ms_aGlyphDataOutlined, SlotW, SlotH);
		UploadGlyph(pSizeData, 1, SlotId, Chr, ms_aGlyphDataOutlined);
	}
	else
	{
		Grow(ms_aGlyphData, ms_aGlyphDataOutlined, SlotW, SlotH);
		Grow(ms_aGlyphDataOutlined, ms_aGlyphData, SlotW, SlotH);
		UploadGlyph(pSizeData, 1, SlotId, Chr, ms_aGlyphData);
	}
	
	// set char info
	{
		CFontChar *pFontchr = &pSizeData->m_aCharacters[SlotId];
		float Scale = 1.0f/pSizeData->m_FontSize;
		float Uscale = 1.0f/pSizeData->m_TextureWidth;
		float Vscale = 1.0f/pSizeData->m_TextureHeight;
		int Height = pBitmap->rows + OutlineThickness*2 + 2; // ignore_convention
		int Width = pBitmap->width + OutlineThickness*2 + 2; // ignore_convention
		
		pFontchr->m_Id = Chr;
		pFontchr->m_Height = Height * Scale;
		pFontchr->m_Width = Width * Scale;
		pFontchr->m_OffsetX = (pFont->m_FtFace->glyph->bitmap_left-1) * Scale; // ignore_convention
		pFontchr->m_OffsetY = (pSizeData->m_FontSize - pFont->m_FtFace->glyph->bitmap_top) * Scale; // ignore_convention
		pFontchr->m_AdvanceX = (pFont->m_FtFace->glyph->advance.x>>6) * Scale; // ignore_convention
		
		pFontchr->m_aUvs[0] = (SlotId%pSizeData->m_NumXChars) / (float)(pSizeData->m_NumXChars);
		pFontchr->m_aUvs[1] = (SlotId/pSizeData->m_NumXChars) / (float)(pSizeData->m_NumYChars);
		pFontchr->m_aUvs[2] = pFontchr->m_aUvs[0] + Width*Uscale;
		pFontchr->m_aUvs[3] = pFontchr->m_aUvs[1] + Height*Vscale;
	}
	
	return SlotId;
}

CFontChar *CTextRender::GetChar(CFont *pFont, CFontSizeData *pSizeData, int Chr)
{
	CFontChar *pFontchr = NULL;
	
	// search for the character
	// TODO: remove this linear search
	int i;
	for(i = 0; i < pSizeData->m_CurrentCharacter; i++)
	{
		if(pSizeData->m_aCharacters[i].m_Id == Chr)
		{
			pFontchr = &pSizeData->m_aCharacters[i];
			break;
		}
	}
	
	// check if we need to render the character
	if(!pFontchr)
	{
		int Index = RenderGlyph(pFont, pSizeData, Chr);
		if(Index >= 0)
			pFontchr = &pSizeData->m_aCharacters[Index];
	}
	
	// touch the character
	// TODO: don't call time_get here
	if(pFontchr)
		pFontchr->m_TouchTime = time_get();
		
	return pFontchr;
}

// must only be called from the rendering function as the pFont must be set to the correct size
void CTextRender::RenderSetup(CFont *pFont, int size)
{
	FT_Set_Pixel_Sizes(pFont->m_FtFace, 0, size);
}

float CTextRender::Kerning(CFont *pFont, int Left, int Right)
{
	FT_Vector Kerning = {0,0};
	FT_Get_Kerning(pFont->m_FtFace, Left, Right, FT_KERNING_DEFAULT, &Kerning);
	return (Kerning.x>>6);
}

CTextRender::CTextRender()
{
	m_TextR = 1;
	m_TextG = 1;
	m_TextB = 1;
	m_TextA = 1;

	m_pDefaultFont = 0;

	// GL_LUMINANCE can be good for debugging
	m_FontTextureFormat = GL_ALPHA;
}
	
void CTextRender::Init()
{
	FT_Init_FreeType(&m_FTLibrary);
}
		

CFont *CTextRender::LoadFont(const char *pFilename)
{
	CFont *pFont = (CFont *)mem_alloc(sizeof(CFont), 1);
	
	mem_zero(pFont, sizeof(*pFont));
	str_copy(pFont->m_aFilename, pFilename, sizeof(pFont->m_aFilename));
	
	if(FT_New_Face(m_FTLibrary, pFont->m_aFilename, 0, &pFont->m_FtFace))
	{
		mem_free(pFont);
		return NULL;
	}

	for(unsigned i = 0; i < NUM_FONT_SIZES; i++)
		pFont->m_aSizes[i].m_FontSize = -1;
	
	return pFont;
};

void CTextRender::DestroyFont(CFont *pFont)
{
	mem_free(pFont);
}

void CTextRender::SetDefaultFont(struct CFont *pFont)
{
	m_pDefaultFont = pFont;
}

/*void CTextRender::TextEx(CTextCursor *pCursor, const char *pText, int Length)
{
	CFont *pFont = pCursor->m_pFont;
	CFontSizeData *pSizeData = NULL;
	
	//dbg_msg("textrender", "rendering text '%s'", text);

	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	float FakeToScreenX, FakeToScreenY;
	int ActualX, ActualY;

	int ActualSize;
	int i;
	int GotNewLine = 0;
	float DrawX, DrawY;
	float CursorX, CursorY;
	const char *pEnd;

	float Size = pCursor->m_FontSize;

	// to correct coords, convert to screen coords, round, and convert back
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	
	FakeToScreenX = (Graphics()->ScreenWidth()/(ScreenX1-ScreenX0));
	FakeToScreenY = (Graphics()->ScreenHeight()/(ScreenY1-ScreenY0));
	ActualX = pCursor->m_X * FakeToScreenX;
	ActualY = pCursor->m_Y * FakeToScreenY;

	CursorX = ActualX / FakeToScreenX;
	CursorY = ActualY / FakeToScreenY;

	// same with size
	ActualSize = Size * FakeToScreenY;
	Size = ActualSize / FakeToScreenY;

	// fetch pFont data
	if(!pFont)
		pFont = m_pDefaultFont;
	
	if(!pFont)
		return;

	pSizeData = GetSize(pFont, ActualSize);
	RenderSetup(pFont, ActualSize);
	
	// set length
	if(Length < 0)
		Length = str_length(pText);
		
	pEnd = pText + Length;

	// if we don't want to render, we can just skip the first outline pass
	i = 1;
	if(pCursor->m_Flags&TEXTFLAG_RENDER)
		i = 0;

	for(;i < 2; i++)
	{
		const char *pCurrent = (char *)pText;
		const char *pEnd = pCurrent+Length;
		DrawX = CursorX;
		DrawY = CursorY;

		if(pCursor->m_Flags&TEXTFLAG_RENDER)
		{
			// TODO: Make this better
			glEnable(GL_TEXTURE_2D);
			if (i == 0)
				glBindTexture(GL_TEXTURE_2D, pSizeData->m_aTextures[1]);
			else
				glBindTexture(GL_TEXTURE_2D, pSizeData->m_aTextures[0]);

			Graphics()->QuadsBegin();
			if (i == 0)
				Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.3f*m_TextA);
			else
				Graphics()->SetColor(m_TextR, m_TextG, m_TextB, m_TextA);
		}

		while(pCurrent < pEnd)
		{
			int NewLine = 0;
			const char *pBatchEnd = pEnd;
			if(pCursor->m_LineWidth > 0 && !(pCursor->m_Flags&TEXTFLAG_STOP_AT_END))
			{
				int Wlen = min(WordLength((char *)pCurrent), (int)(pEnd-pCurrent));
				CTextCursor Compare = *pCursor;
				Compare.m_X = DrawX;
				Compare.m_Y = DrawY;
				Compare.m_Flags &= ~TEXTFLAG_RENDER;
				Compare.m_LineWidth = -1;
				TextEx(&Compare, pText, Wlen);
				
				if(Compare.m_X-DrawX > pCursor->m_LineWidth)
				{
					// word can't be fitted in one line, cut it
					CTextCursor Cutter = *pCursor;
					Cutter.m_CharCount = 0;
					Cutter.m_X = DrawX;
					Cutter.m_Y = DrawY;
					Cutter.m_Flags &= ~TEXTFLAG_RENDER;
					Cutter.m_Flags |= TEXTFLAG_STOP_AT_END;
					
					TextEx(&Cutter, (const char *)pCurrent, Wlen);
					Wlen = Cutter.m_CharCount;
					NewLine = 1;
					
					if(Wlen <= 3) // if we can't place 3 chars of the word on this line, take the next
						Wlen = 0;
				}
				else if(Compare.m_X-pCursor->m_StartX > pCursor->m_LineWidth)
				{
					NewLine = 1;
					Wlen = 0;
				}
				
				pBatchEnd = pCurrent + Wlen;
			}
			
			while(pCurrent < pBatchEnd)
			{
				const char *pTmp;
				float Advance = 0;
				int Character = 0;
				int Nextcharacter = 0;
				CFontChar *pChr;

				// TODO: UTF-8 decode
				Character = str_utf8_decode(&pCurrent);
				pTmp = pCurrent;
				Nextcharacter = str_utf8_decode(&pTmp);
				
				if(Character == '\n')
				{
					DrawX = pCursor->m_StartX;
					DrawY += Size;
					DrawX = (int)(DrawX * FakeToScreenX) / FakeToScreenX; // realign
					DrawY = (int)(DrawY * FakeToScreenY) / FakeToScreenY;
					++pCursor->m_LineCount;
					continue;
				}

				pChr = GetChar(pFont, pSizeData, Character);

				if(pChr)
				{
					if(pCursor->m_Flags&TEXTFLAG_RENDER)
					{
						Graphics()->QuadsSetSubset(pChr->m_aUvs[0], pChr->m_aUvs[1], pChr->m_aUvs[2], pChr->m_aUvs[3]);
						Graphics()->QuadsDrawTL(DrawX+pChr->m_OffsetX*Size, DrawY+pChr->m_OffsetY*Size, pChr->m_Width*Size, pChr->m_Height*Size);
					}

					Advance = pChr->m_AdvanceX + Kerning(pFont, Character, Nextcharacter)/Size;
				}
								
				if(pCursor->m_Flags&TEXTFLAG_STOP_AT_END && DrawX+Advance*Size-pCursor->m_StartX > pCursor->m_LineWidth)
				{
					// we hit the end of the line, no more to render or count
					pCurrent = pEnd;
					break;
				}

				DrawX += Advance*Size;
				pCursor->m_CharCount++;
			}
			
			if(NewLine)
			{
				DrawX = pCursor->m_StartX;
				DrawY += Size;
				GotNewLine = 1;
				DrawX = (int)(DrawX * FakeToScreenX) / FakeToScreenX; // realign
				DrawY = (int)(DrawY * FakeToScreenY) / FakeToScreenY;				
				++pCursor->m_LineCount;
			}
		}

		if(pCursor->m_Flags&TEXTFLAG_RENDER)
			Graphics()->QuadsEnd();
	}

	pCursor->m_X = DrawX;
	
	if(GotNewLine)
		pCursor->m_Y = DrawY;
}*/
