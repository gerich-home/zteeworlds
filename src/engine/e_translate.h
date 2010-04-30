#ifndef E_TRANSLATE_H
#define E_TRANSLATE_H

extern const unsigned long TranslatorLanguagesCount;
extern const char * TranslatorLanguages[];
extern const char * TranslatorLanguageCodes[];

extern struct TranslateTextThreadData;

typedef void TranslatorCallback(TranslateTextThreadData * Data);

typedef struct TranslateTextThreadData
{
	const char * Text;
	unsigned int Language;
	TranslatorCallback * Callback;
	void * Param;
	const char * Translated;
} TranslateTextThreadData;

unsigned long TranslateText(const char * Text, unsigned int Language, TranslatorCallback * Callback, void * Param);
int TranslateGetLanguageIndex(const char * LangName);

#endif