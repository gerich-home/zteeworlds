#ifndef _LANGUAGE_H
#define _LANGUAGE_H

void lang_init();
void lang_free();
int lang_load(const char * filename);

const char * _T(int gid, const char * str);
int get_gid(const char * str);

#define __T(gid, str) _T( gid, str )

#ifndef __GNUC__
#define _t(str) __T( get_gid( __FILE__ ) , str )
#else
#define _t(str) __T( get_gid( __BASE_FILE__ ) , str )
#endif

#endif
