#include "e_translate.h"

#include <base/system.h>
#include <curl/curl.h>

const unsigned long TranslatorLanguagesCount = 88;

const char * TranslatorLanguages[88] = {
	"Afrikaans",
	"Albanian",
	"Amharic",
	"Arabic",
	"Armenian",
	"Azerbaijani",
	"Basque",
	"Belarusian",
	"Bengali",
	"Bihari",
	"Bulgarian",
	"Burmese",
	"Catalan",
	"Cherokee",
	"Chinese",
	"Chinese Simplified",
	"Chinese Traditional",
	"Croatian",
	"Czech",
	"Danish",
	"Dhivehi",
	"Dutch",
	"English",
	"Esperanto",
	"Estonian",
	"Filipino",
	"Finnish",
	"French",
	"Galician",
	"Georgian",
	"German",
	"Greek",
	"Guarani",
	"Gujarati",
	"Hebrew",
	"Hindi",
	"Hungarian",
	"Icelandic",
	"Indonesian",
	"Inuktitut",
	"Italian",
	"Japanese",
	"Kannada",
	"Kazakh",
	"Khmer",
	"Korean",
	"Kurdish",
	"Kyrgyz",
	"Laothian",
	"Latvian",
	"Lithuanian",
	"Macedonian",
	"Malay",
	"Malayalam",
	"Maltese",
	"Marathi",
	"Mongolian",
	"Nepali",
	"Norwegian",
	"Oriya",
	"Pashto",
	"Persian",
	"Polish",
	"Portuguese",
	"Punjabi",
	"Romanian",
	"Russian",
	"Sanskrit",
	"Serbian",
	"Sindhi",
	"Sinhalese",
	"Slovak",
	"Slovenian",
	"Spanish",
	"Swahili",
	"Swedish",
	"Tajik",
	"Tamil",
	"Tagalog",
	"Telugu",
	"Thai",
	"Tibetan",
	"Turkish",
	"Ukrainian",
	"Urdu",
	"Uzbek",
	"Uighur",
	"Vietnamese",
};

const char * TranslatorLanguageCodes[88] = {
	"af", // 0
	"sq", // 1
	"am", // 2
	"ar", // 3
	"hy", // 4
	"az", // 5
	"eu", // 6
	"be", // 7
	"bn", // 8
	"bh", // 9
	"bg", // 10
	"my", // 11
	"ca", // 12
	"chr", // 13
	"zh", // 14
	"zh-CN", // 15
	"zh-TW", // 16
	"hr", // 17
	"cs", // 18
	"da", // 19
	"dv", // 20
	"nl", // 21
	"en", // 22
	"eo", // 23
	"et", // 24
	"tl", // 25
	"fi", // 26
	"fr", // 27
	"gl", // 28
	"ka", // 29
	"de", // 30
	"el", // 31
	"gn", // 32
	"gu", // 33
	"iw", // 34
	"hi", // 35
	"hu", // 36
	"is", // 37
	"id", // 38
	"iu", // 39
	"it", // 40
	"ja", // 41
	"kn", // 42
	"kk", // 43
	"km", // 44
	"ko", // 45
	"ku", // 46
	"ky", // 47
	"lo", // 48
	"lv", // 49
	"lt", // 50
	"mk", // 51
	"ms", // 52
	"ml", // 53
	"mt", // 54
	"mr", // 55
	"mn", // 56
	"ne", // 57
	"no", // 58
	"or", // 59
	"ps", // 60
	"fa", // 61
	"pl", // 62
	"pt-PT", // 63
	"pa", // 64
	"ro", // 65
	"ru", // 66
	"sa", // 67
	"sr", // 68
	"sd", // 69
	"si", // 70
	"sk", // 71
	"sl", // 72
	"es", // 73
	"sw", // 74
	"sv", // 75
	"tg", // 76
	"ta", // 77
	"tl", // 78
	"te", // 79
	"th", // 80
	"bo", // 81
	"tr", // 82
	"uk", // 83
	"ur", // 84
	"uz", // 85
	"ug", // 86
	"vi", // 87
};

const unsigned long TranslationBufferSize = 4096;

size_t curlWriteFunc(char *data, size_t size, size_t nmemb, char * buffer)  
{  
        size_t result = 0;  

        if (buffer != NULL)  
        {
			result = size * nmemb;
			memcpy(buffer + strlen(buffer), data, result <= TranslationBufferSize - strlen(buffer) - 1 ? result : TranslationBufferSize - strlen(buffer) - 1);
        }  

        return result;  
}

char * UnescapeStep1(char * From)
{
	unsigned long Len = str_length(From);
	char * Result = (char *)mem_alloc(Len * 4 + 1, 1);
	memset(Result, 0, Len * 4 + 1);
	
	unsigned long i = 0;
	unsigned long j = 0;
	while (i < Len)
	{
		if (From[i] == '\\')
		{
			if (From[i + 1] == '\\')
			{
				Result[j++] = '\\';
				i += 2;
			} else {
				if (From[i + 2] == 'u')
				{
					unsigned long Code = 0;
					int q;
					for (q = 0; q < 4; q++)
					{
						if (From[i + 3 + q] >= '0' && From[i + 3 + q] <= '9')
							q = (q * 16) + From[i + 3 + q] - '0';
						else if (From[i + 3 + q] >= 'a' && From[i + 3 + q] <= 'f')
							q = (q * 16) + 10 + From[i + 3 + q] - 'a';
						else if (From[i + 3 + q] >= 'A' && From[i + 3 + q] <= 'F')
							q = (q * 16) + 10 + From[i + 3 + q] - 'A';
						else break;
					}
					j += str_utf8_encode(Result, Code);
					i += 2 + q;
				}
			}
		} else {
			Result[j++] = From[i++];
		}
	}
	
	return Result;
}

char * UnescapeStep2(char * From)
{
	unsigned long Len = str_length(From);
	char * Result = (char *)mem_alloc(Len * 4 + 1, 1);
	memset(Result, 0, Len * 4 + 1);
	
	unsigned long i = 0;
	unsigned long j = 0;
	while (i < Len)
	{
		if (From[i] == '%')
		{
			if (From[i + 1] != '&')
			{
				Result[j++] = From[i++];
			} else {
				unsigned long Code = 0;
				int q;
				for (q = 0; q < 4; q++)
				{
					if (From[i + 3 + q] >= '0' && From[i + 3 + q] <= '9')
						q = (q * 16) + From[i + 3 + q] - '0';
					else if (From[i + 3 + q] >= 'a' && From[i + 3 + q] <= 'f')
						q = (q * 16) + 10 + From[i + 3 + q] - 'a';
					else if (From[i + 3 + q] >= 'A' && From[i + 3 + q] <= 'F')
						q = (q * 16) + 10 + From[i + 3 + q] - 'A';
					else break;
				}
				j += str_utf8_encode(Result, Code);
				i += 2 + q;
				if (From[i] == ';') i++;
			}
		} else {
			Result[j++] = From[i++];
		}
	}
	
	return Result;
}

char * UnescapeStr(char * From)
{
	char * First = UnescapeStep1(From);
	char * Second = UnescapeStep2(First);
	mem_free((void *)First);
	return Second;
}

char * EscapeStrByLong(const char * From)
{
	unsigned long Len = str_length(From);
	unsigned long DestLen = Len * 6;
	char * Result = (char *)mem_alloc(DestLen + 1, 1);
	memset(Result, 0, DestLen + 1);
	
	unsigned long Char;
	const char * Text = From;
	char * ToText = Result;
	unsigned long i;
	while (Char = str_utf8_decode(&Text))
	{
		*(ToText++) = '\\';
		*(ToText++) = 'u';
		str_format(ToText, 5, "%04x", Char);
		ToText += 4;
	}
	
	return Result;
}

char * EscapeStr(const char * From)
{
	unsigned long Len = str_length(From);
	unsigned long DestLen = Len * 4;
	char * Result = (char *)mem_alloc(DestLen + 1, 1);
	memset(Result, 0, DestLen + 1);
	
	unsigned long Char;
	const char * Text = From;
	char * ToText = Result;
	unsigned long i;
	for (i = 0; i < Len; i++)
	{
		if ((From[i] >= 'a' && From[i] <= 'z') || (From[i] >= 'A' && From[i] <= 'Z') || (From[i] >= '0' && From[i] <= '9'))
			*(ToText++) = From[i];
		else
		{
			*(ToText++) = '%';
			str_format(ToText, 5, "%02x", ((unsigned int)From[i])%0x100);
			ToText += 2;
		}
	}
	
	return Result;
}

void TranslateTextThreadFunc(void * Param)
{		
	CURL * curl = NULL;

	char * Request = NULL;
	char * Escaped = NULL;
	char * Str = NULL;
	char * Result = NULL;
	
	TranslateTextThreadData * Data = (TranslateTextThreadData *)Param;
	
	try
	{		
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, "http://ajax.googleapis.com/ajax/services/language/translate");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_POST, 1);

		Request = (char *)mem_alloc(TranslationBufferSize, 1);
		Escaped = (char *)EscapeStr(Data->Text);
		str_format(Request, TranslationBufferSize, "v=1.0&q=%s&langpair=%%7C%s", Escaped, TranslatorLanguageCodes[Data->Language]);
		mem_free((void *)Escaped);
		Escaped = NULL;
		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, Request);

		Str = (char *)mem_alloc(TranslationBufferSize, 1);
		memset(Str, 0, TranslationBufferSize);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, Str);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);

		CURLcode curlResult = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		
		mem_free(Request);
		Request = NULL;
		
		const char * TranslatedText = str_find_nocase(Str, "translatedText\":\"");
		if (TranslatedText)
		{
			TranslatedText += strlen("translatedText\":\"");
			char * TranslationEnd = (char *)str_find_nocase(TranslatedText, "\"");
			if (TranslationEnd) TranslationEnd[0] = 0;
			Result = strdup(TranslatedText);
		} else Result = strdup(Data->Text);
		
		mem_free(Str);
		Str = NULL;
		
		Data->Translated = UnescapeStr((char *)Result);
		free((void *)Result);
		Result = NULL;

		(*(Data->Callback))(Data);
	}
	catch(...)
	{
		if (Request) mem_free(Request);
		if (Escaped) mem_free(Escaped);
		if (Str) mem_free(Str);
		if (Result) free(Result);
	}
	try
	{
		free((void *)Data->Text);
		mem_free((void *)Data->Translated);
		mem_free((void *)Data);
	}
	catch(...)
	{
	}
}

unsigned long TranslateText(const char * Text, unsigned int Language, TranslatorCallback * Callback, void * Param)
{
	TranslateTextThreadData * Data = (TranslateTextThreadData *)mem_alloc(sizeof(TranslateTextThreadData), 1);
	Data->Text = strdup(Text);
	Data->Language = Language;
	Data->Callback = Callback;
	Data->Param = Param;

	return (unsigned long)thread_create(TranslateTextThreadFunc, (void *)Data);
}

int TranslateGetLanguageIndex(const char * LangName)
{
	for (unsigned long i = 0; i < TranslatorLanguagesCount; i++)
	{
		if (str_comp_nocase(LangName, TranslatorLanguages[i]) == 0 || str_comp_nocase(LangName, TranslatorLanguageCodes[i]) == 0)
			return i;
	}
	return -1;
}
