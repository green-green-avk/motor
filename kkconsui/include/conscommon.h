#ifndef __KONST_UI_FUNC_H_
#define __KONST_UI_FUNC_H_

#include "kkstrtext.h"
#include "conf.h"

#include <string>
#include <vector>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <ctype.h>

#ifdef __sun__
#include <sys/termio.h>
#include <sys/filio.h>
#endif

#ifdef __CYGWIN__
#include <sys/termios.h>
#include <sys/socket.h>
#endif

#undef box
#undef clear
#undef erase
#undef move
#undef refresh

/* Fucking ncurses stuff */

#define boldcolor(c)            COLOR_PAIR(c)|A_BOLD
#define normalcolor(c)          COLOR_PAIR(c)

#ifdef LOCALES_HACK
#define KT_DISP_FILTER(c)       ( (((unsigned char) c > ' ') && (c != '\177')) ? c : ' ' )
#else
#define KT_DISP_FILTER(c)       (!iscntrl((unsigned char) c) ? c : ' ')
#endif

#define VLINE           kintf_graph ? ACS_VLINE         : '|'
#define HLINE           kintf_graph ? ACS_HLINE         : '-'
#define ULCORNER        kintf_graph ? ACS_ULCORNER      : '+'
#define URCORNER        kintf_graph ? ACS_URCORNER      : '+'
#define LLCORNER        kintf_graph ? ACS_LLCORNER      : '+'
#define LRCORNER        kintf_graph ? ACS_LRCORNER      : '+'
#define LTEE            kintf_graph ? ACS_LTEE          : '|'
#define RTEE            kintf_graph ? ACS_RTEE          : '|'
#define TTEE            kintf_graph ? ACS_TTEE          : '+'
#define BTEE            kintf_graph ? ACS_BTEE          : '+'

#define KEY_TAB 9
#define KEY_ESC 27

#ifndef CTRL
#define CTRL(x) ((x) & 0x1F)
#endif

#ifndef ALT
#define ALT(x) (0x200 | (unsigned INT) x)
#endif

#define SHIFT_PRESSED   1
#define ALTR_PRESSED    2
#define ALTL_PRESSED    8
#define CONTROL_PRESSED 4

extern bool kintf_graph, kintf_refresh, use_fribidi;
extern void (*kt_resize_event)(void);

void printchar(char c);
void printstring(const string &s);
INT string2key(const string &adef);

string makebidi(const string &buf, INT lpad = 0);
INT findcolor(const string &s);

__KTOOL_BEGIN_C

void kinterface();
void kendinterface();

INT keypressed(bool wait = false);
INT emacsbind(INT k);

INT getkey();
INT getctrlkeys();

void kwriteatf(INT x, INT y, INT c, const char *fmt, ...);
void kwriteat(INT x, INT y, const char *msg, INT c);
void kgotoxy(INT x, INT y);
void hidecursor();
void showcursor();
void setbeep(INT freq, INT duration);
INT kwherex();
INT kwherey();

__KTOOL_END_C

#endif
