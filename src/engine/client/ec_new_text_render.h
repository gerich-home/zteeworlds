#ifndef ENGINE_TEXTRENDER_H
#define ENGINE_TEXTRENDER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

enum
{
	MAX_CHARACTERS = 64,
};


static int aFontSizes[] = {8,9,10,11,12,13,14,15,16,17,18,19,20,36};
#define NUM_FONT_SIZES (sizeof(aFontSizes)/sizeof(int))

struct CFontChar
{
	int m_Id;
	
	// these values are scaled to the pFont size
	// width * font_size == real_size
	float m_Width;
	float m_Height;
	float m_OffsetX;
	float m_OffsetY;
	float m_AdvanceX;
	
	float m_aUvs[4];
	int64 m_TouchTime;
};

struct CFontSizeData
{
	int m_FontSize;
	FT_Face *m_pFace;

	unsigned m_aTextures[2];
	int m_aTextureIds[2];
	
	int m_TextureWidth;
	int m_TextureHeight;
	
	int m_NumXChars;
	int m_NumYChars;
	
	int m_CharMaxWidth;
	int m_CharMaxHeight;
	
	CFontChar m_aCharacters[MAX_CHARACTERS*MAX_CHARACTERS];
	
	int m_CurrentCharacter;	
};

struct CFont
{
	char m_aFilename[128];
	FT_Face m_FtFace;
	CFontSizeData m_aSizes[NUM_FONT_SIZES];
};

class CTextCursor
{
public:
	int m_Flags;
	int m_LineCount;
	int m_CharCount;
	
	float m_StartX;
	float m_StartY;
	float m_LineWidth;
	float m_X, m_Y;
	
	struct CFont *m_pFont;
	float m_FontSize;
};

class CTextRender
{
private:
	float m_TextR;
	float m_TextG;
	float m_TextB;
	float m_TextA;
	
	int m_FontTextureFormat;

	struct CFont *m_pDefaultFont;

	FT_Library m_FTLibrary;
	
	// 8k of data used for rendering glyphs
	unsigned char ms_aGlyphData[(4096/64) * (4096/64)];
	unsigned char ms_aGlyphDataOutlined[(4096/64) * (4096/64)];
	
	int GetFontSizeIndex(int Pixelsize);
	void Grow(unsigned char *pIn, unsigned char *pOut, int w, int h);
	void InitTexture(CFontSizeData *pSizeData, int CharWidth, int CharHeight, int Xchars, int Ychars);
	void IncreaseTextureSize(CFontSizeData *pSizeData);
	void InitIndex(CFont *pFont, int Index);
	void UploadGlyph(CFontSizeData *pSizeData, int Texnum, int SlotId, int Chr, const void *pData);
	int GetSlot(CFontSizeData *pSizeData);
	int RenderGlyph(CFont *pFont, CFontSizeData *pSizeData, int Chr);
public:
	CTextRender();
	
	CFontChar *GetChar(CFont *pFont, CFontSizeData *pSizeData, int Chr);
	void RenderSetup(CFont *pFont, int size);
	float Kerning(CFont *pFont, int Left, int Right);
	CFontSizeData *GetSize(CFont *pFont, int Pixelsize);
	
	void Init();
	
	CFont *LoadFont(const char *pFilename);
	void DestroyFont(CFont *pFont);
	
	void SetDefaultFont(struct CFont *pFont);
	
	CFont * GetDefaultFont() { return m_pDefaultFont; }
};

#endif
