#include "zcurlib.h"
#include <iostream>
using namespace std;


#ifdef WIN32

// codigo windows, basado en http://www.gamedev.net/community/forums/topic.asp?topic_id=308393

#include <windows.h>
#include <conio.h>

void setBackColor(int c) {	
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD wAttrib = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole,&csbi);
	wAttrib = csbi.wAttributes;
	wAttrib &= ~(BACKGROUND_BLUE |
		BACKGROUND_GREEN | 
		BACKGROUND_RED |
		BACKGROUND_INTENSITY);
	if (c & 1)		wAttrib |= BACKGROUND_BLUE;
	if (c & 2)		wAttrib |= BACKGROUND_GREEN;
	if (c & 4)		wAttrib |= BACKGROUND_RED;
	if (c & 8)		wAttrib |= BACKGROUND_INTENSITY;
	SetConsoleTextAttribute(hConsole, wAttrib);
}

void setForeColor(int c) {	
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);	
	WORD wAttrib = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;	
	GetConsoleScreenBufferInfo(hConsole,&csbi);
	wAttrib = csbi.wAttributes;	
	wAttrib &= ~(FOREGROUND_BLUE |
		FOREGROUND_GREEN |
		FOREGROUND_RED |
		FOREGROUND_INTENSITY);
	if (c & 1)		wAttrib |= FOREGROUND_BLUE;
	if (c & 2)		wAttrib |= FOREGROUND_GREEN;
	if (c & 4)		wAttrib |= FOREGROUND_RED;
	if (c & 8)		wAttrib |= FOREGROUND_INTENSITY;
	SetConsoleTextAttribute(hConsole, wAttrib);
}

void clrscr() {
	cout.flush();
	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = {0, 0};	
	DWORD dw;	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwSize;
	GetConsoleScreenBufferInfo(hConsole,&csbi);
	dwSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole,
		' ',
		dwSize,
		coord,
		&dw);
	FillConsoleOutputAttribute(hConsole,
		csbi.wAttributes,
		dwSize,
		coord,
		&dw);
	SetConsoleCursorPosition(hConsole, coord);
}

void gotoXY(int x, int y) {
	HANDLE screen_buffer_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord;
	
	coord.X = x-1;
	coord.Y = y-1;
	
	SetConsoleCursorPosition(screen_buffer_handle, coord);
}

int getKey( void ) {
	int ch = getch();
	if ( ch == 0 || ch == 224 )
		ch = 256 + getch();
	return ch;
}


void hideCursor() {
	CONSOLE_CURSOR_INFO ConCurInf;
	ConCurInf.dwSize = 100;
	ConCurInf.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ConCurInf);
}

void showCursor() {
	CONSOLE_CURSOR_INFO ConCurInf;
	ConCurInf.dwSize = 100;
	ConCurInf.bVisible = TRUE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&ConCurInf);
}

void setTitle(const char* title) {
	SetConsoleTitle(title);
}


#else

#include <termios.h>
#include <unistd.h>

// secuencias de escape POSIX

void setBackColor(const char *color) {	
	static char aux[10]="\033[43m";
	aux[3]=color[7];
	cout<<aux;
}

void setForeColor(const char *color) {
	cout<<color;
}

void clrscr() {
	cout<<"\033[2J";
}

void gotoXY(int x, int y) {
	cout<<"\033["<<y<<';'<<x<<'H'<<flush;
}

int getKey(void) {
	static char buf[256];
	struct termios oldt,
		newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = read(STDIN_FILENO,buf,255); // leer teclas
	if (ch==1)
		ch=buf[0];
	else if (ch==3)
		ch = buf[0]+(buf[1]<<8)+(buf[2]<<16);
	else if (ch==5)
//		ch = buf[0]+(buf[1]<<8)+(buf[2]<<16)+(buf[2]<<24)+(buf[2]<<32); // este no tiene sentido
		ch = buf[1]+(buf[2]<<8)+(buf[2]<<16)+(buf[2]<<24); // este no tiene sentido
	else if (ch==4)
		ch = buf[0]+(buf[1]<<8)+(buf[2]<<16)+(buf[2]<<24);
	else
		ch = buf[0]+(buf[1]<<8);
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

void hideCursor() {
	cout<<"\033[?25l"<<flush;
}

void showCursor() {
	cout<<"\033[?25h"<<flush;
}

void setTitle(const char* title) {
	const static char esc_start[] = { 0x1b, ']', '0', ';', 0 };
	const static char esc_end[] = { 0x07, 0 };
	cout << esc_start << title << esc_end;
}

#endif


