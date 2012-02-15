#ifndef ZCURLIB_H
#define ZCURLIB_H

#ifdef WIN32

#define COLOR_T int
#define COLOR_BLACK	0
#define COLOR_BLUE	1
#define COLOR_GREEN	2
#define COLOR_CYAN	3
#define COLOR_RED	4
#define COLOR_MAGENTA	5
#define COLOR_BROWN	6
#define COLOR_WHITE	7
#define COLOR_GRAY	8
#define COLOR_LBLUE	9
#define COLOR_LGREEN	10
#define COLOR_LCYAN	11
#define COLOR_LRED	12
#define COLOR_LMAGENTA	13
#define COLOR_YELLOW	14
#define COLOR_BWHITE	15   

#define KEY_ESC   27
#define KEY_UP    256+72
#define KEY_DOWN  256+80
#define KEY_LEFT  256+75
#define KEY_RIGHT 256+77
#define KEY_DELETE 256+83
#define KEY_INSERT 256+82
#define KEY_ENTER 13

#else

#define COLOR_T const char*
#define COLOR_BLACK	"\033[0m\033[30m"
#define COLOR_BLUE	"\033[0m\033[34m"
#define COLOR_GREEN	"\033[0m\033[32m"
#define COLOR_CYAN	"\033[0m\033[36m"
#define COLOR_RED	"\033[0m\033[31m"
#define COLOR_MAGENTA "\033[0m\033[35m"
#define COLOR_BROWN	"\033[0m\033[33m"
#define COLOR_WHITE	"\033[0m\033[37m"
#define COLOR_GRAY	"\033[1m\033[30m"
#define COLOR_LBLUE	"\033[1m\033[34m"
#define COLOR_LGREEN "\033[1m\033[32m"
#define COLOR_LCYAN	"\033[1m\033[36m"
#define COLOR_LRED	"\033[1m\033[31m"
#define COLOR_LMAGENTA "\033[1m\033[35m"
#define COLOR_YELLOW "\033[1m\033[33m"
#define COLOR_BWHITE	"\033[1m\033[37m"



#define KEY_ESC   27
#define KEY_TAB   9
#define KEY_UP    27+(91<<8)+(65<<16)
#define KEY_DOWN  27+(91<<8)+(66<<16)
#define KEY_LEFT  27+(91<<8)+(68<<16)
#define KEY_RIGHT 27+(91<<8)+(67<<16)
#define KEY_DELETE 27+(91<<8)+(51<<16)
#define KEY_INSERT 27+(91<<8)+(50<<16)
#define KEY_ENTER 13

#endif

void setBackColor(COLOR_T color);	
void setForeColor(COLOR_T color);
void clrscr();
void gotoXY(int x, int y);
int getKey(void);
void hideCursor();
void showCursor();
void setTitle(const char* title);

#endif


