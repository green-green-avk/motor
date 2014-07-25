#ifndef __KONST_STRING_H_
#define __KONST_STRING_H_

#include <string>
#include <vector>
#include <fstream>

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "conf.h"

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

//
// Gives out a number with the both limits _included_
//

#define randlimit(l, h) ((INT)((float)rand()/(float)RAND_MAX*(float)(h+1-l)+(float)l))

#define SWAPVAL(v1, v2) { \
    INT SWAPVAL__v = v2; \
    v2 = v1, v1 = SWAPVAL__v; \
}

#define PRINTNULL(p) (p ? p : "")
#define TAB_SIZE      8

struct quotedblock {
    INT begin, end;
};

void charpointerfree(void *p);
void nothingfree(void *p);
INT stringcompare(void *s1, void *s2);
INT intcompare(void *s1, void *s2);

string leadcut(const string &base, const string &delim = "\t\n\r ");
string trailcut(const string &base, const string &delim = "\t\n\r ");

string getword(string &base, const string &delim = "\t\n\r ");
string getrword(string &base, const string &delim = "\t\n\r ");

string getwordquote(string &base, const string &quote = "\"", const string &delim = "\t\n\r ");
string getrwordquote(string &base, const string &quote = "\"", const string &delim = "\t\n\r ");

INT rtabmargin(bool fake, INT curpos, const char *p = 0);
INT ltabmargin(bool fake, INT curpos, const char *p = 0);

void breakintolines(string text, vector<string> &lst, INT linelen);
void breakintolines(const string &text, vector<string> &lst);

void find_gather_quoted(vector<quotedblock> &lst, const string &str,
    const string &quote, const string &escape);

INT find_quoted(const string &str, const string &needle, INT offs = 0,
    const string &quote = "\"'", const string &escape = "\\");

INT find_quoted_first_of(const string &str, const string &needle,
    INT offs = 0, const string &quote = "\"'",
    const string &escape = "\\");

void splitlongtext(string text, vector<string> &lst,
    INT size = 440, const string cont = "\n[continued]");

string strdateandtime(time_t stamp, const string &fmt = "");
string strdateandtime(struct tm *tms, const string &fmt = "");

bool iswholeword(const string &s, INT so, INT eo);

INT hex2int(const string &ahex);

vector<INT> getquotelayout(const string &haystack, const string &qs,
    const string &aescs);

vector<INT> getsymbolpositions(const string &haystack,
    const string &needles, const string &qoutes, const string &esc);

#define VGETSTRING(c, fmt) \
    { \
	va_list vgs__ap; char vgs__buf[1024]; \
	va_start(vgs__ap, fmt); \
	vsprintf(vgs__buf, fmt, vgs__ap); c = vgs__buf; \
	va_end(vgs__ap); \
    }

string justfname(const string &fname);
string justpathname(const string &fname);
string textscreen(const string &text);
string i2str(INT i);
string ui2str(INT i);

bool getconf(string &st, string &buf, ifstream &f, bool passemptylines = false);
bool getstring(istream &f, string &buf);

string unmime(const string &text);
string mime(const string &text);

string toutf8(const string &text);
string fromutf8(const string &text);

string ruscase(const string &s, const string &mode);
string siconv(const string &text, const string &fromcs, const string &tocs);

const INT chCutBR = 1;
const INT chLeaveLinks = 2;

string cuthtml(const string &html, INT flags = 0);
string striprtf(const string &s, const string &charset);

__KTOOL_BEGIN_C

char *strcut(char *strin, INT frompos, INT count);

char *trimlead(char *str, char *chr);
char *trimtrail(char *str, char *chr);
char *trim(char *str, char *chr);

char *strimlead(char *str);
char *strimtrail(char *str);
char *strim(char *str);

const char *strqpbrk(
    const char *s, INT offset,
    const char *accept,
    const char *q,
    const char *esc = "");

const char *strqcasestr(
    const char *s,
    const char *str,
    const char *q,
    const char *esc = "");
    
const char *strqstr(const char *s,
    const char *str,
    const char *q,
    const char *esc = "");

char *strccat(char *dest, char c);
char *strinsert(char *buf, INT pos, char *ins);
char *strcinsert(char *buf, INT pos, char ins);

INT strchcount(char *s, char *accept);
INT stralone(char *buf, char *startword, INT wordlen, char *delim);

char *time2str(const time_t *t, char *mask, char *sout);
time_t str2time(char *sdate, char *mask, time_t *t);

char *unmime(char *text);
char *mime(char *dst, const char *src);

char *utf8_to_str(const char *pin);
char *str_to_utf8(const char *pin);

__KTOOL_END_C

#endif
