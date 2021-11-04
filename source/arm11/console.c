/*
 * This code is part of libctru (https://github.com/devkitPro/libctru)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "types.h"
#include "arm11/fmt.h"
#include "drivers/gfx.h"
#include "util.h"
#include "arm11/console.h"

#include "arm11/font_6x10.h"

//set up the palette for color printing
static u16 colorTable[] = {
	RGB8_to_565(  0,  0,  0),	// faint black
	RGB8_to_565(255,  0,  0),	// bright red
	RGB8_to_565(  0,255,  0),	// bright green
	RGB8_to_565(255,255,  0),	// bright yellow
	RGB8_to_565(  0,  0,255),	// bright blue
	RGB8_to_565(255,  0,255),	// bright magenta
	RGB8_to_565(  0,255,255),	// bright cyan
	RGB8_to_565(255,255,255),	// bright white

	RGB8_to_565( 64, 64, 64),	// almost black
	RGB8_to_565(224,  0,  0),	// accent red
	RGB8_to_565( 64,255, 64),	// accent green
	RGB8_to_565(255,255, 32),	// accent yellow
	RGB8_to_565( 64, 64,255),	// accent blue
	RGB8_to_565(255,  0,255),	// bright magenta
	RGB8_to_565(  0,255,255),	// bright cyan
	RGB8_to_565(192,192,192),	// almost white

	RGB8_to_565(128,128,128),	// bright black
	RGB8_to_565( 64,  0,  0),	// faint red
	RGB8_to_565(  0, 64,  0),	// faint green
	RGB8_to_565( 64, 64,  0),	// faint yellow
	RGB8_to_565(  0,  0, 64),	// faint blue
	RGB8_to_565( 64,  0, 64),	// faint magenta
	RGB8_to_565(  0, 64, 64),	// faint cyan
	RGB8_to_565( 96, 96, 96),	// faint white
};

PrintConsole defaultConsole =
{
	//Font:
	{
		default_font, //font gfx
		0, //first ascii character in the set
		256 //number of characters in the font set
	},
	(u16*)NULL,
	0,0,	//cursorX cursorY
	0,0,	//prevcursorX prevcursorY
	53,		//console width
	24,		//console height
	0,		//window x
	0,		//window y
	53,		//window width
	24,		//window height
	3,		//tab size
	7,		// foreground color
	0,		// background color
	0,		// flags
	0,		//print callback
	false	//console initialized
};

PrintConsole currentCopy;

PrintConsole* currentConsole = &currentCopy;

PrintConsole* consoleGetDefault(void){return &defaultConsole;}

void consolePrintChar(int c);
void consoleDrawChar(int c);

//---------------------------------------------------------------------------------
static void consoleCls(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp,rowTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			while(i++ < ((currentConsole->windowHeight * currentConsole->windowWidth) - (rowTemp * currentConsole->consoleWidth + colTemp)))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;
			rowTemp = currentConsole->cursorY ;

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while (i++ < (rowTemp * currentConsole->windowWidth + colTemp))
				consolePrintChar(' ');

			currentConsole->cursorX  = colTemp;
			currentConsole->cursorY  = rowTemp;
			break;
		}
	case '2':
		{
			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowHeight * currentConsole->windowWidth)
				consolePrintChar(' ');

			currentConsole->cursorY  = 0;
			currentConsole->cursorX  = 0;
			break;
		}
	default: ;
	}
}
//---------------------------------------------------------------------------------
static void consoleClearLine(char mode) {
//---------------------------------------------------------------------------------

	int i = 0;
	int colTemp;

	switch (mode)
	{
	case '[':
	case '0':
		{
			colTemp = currentConsole->cursorX ;

			while(i++ < (currentConsole->windowWidth - colTemp)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '1':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < ((currentConsole->windowWidth - colTemp)-2)) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	case '2':
		{
			colTemp = currentConsole->cursorX ;

			currentConsole->cursorX  = 0;

			while(i++ < currentConsole->windowWidth) {
				consolePrintChar(' ');
			}

			currentConsole->cursorX  = colTemp;

			break;
		}
	default: ;
	}
}

__attribute__ ((format (scanf, 2, 3))) int fb_sscanf(const char *s, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);


	int ret = 0;
	const char *const oldS = s;
	while(*fmt)
	{
		if(*fmt == '%')
		{
			bool number = false;

			switch(*++fmt)
			{
				case 'd':
					*va_arg(args, int*) = atoi(s);
					number = true;
					ret++;
					break;
				case 'c':
					*va_arg(args, char*) = *s++;
					ret++;
					break;
				case 'n':
					*va_arg(args, int*) = (int)(s - oldS);
					break;
				default: ;
			}
			if(number) while(*s >= '0' && *s <= '9') s++;
			fmt++;
		}
		else
		{
			if(*fmt != *s) break;
			fmt++;
			s++;
		}
	}

	va_end(args);

	return ret;
}

//---------------------------------------------------------------------------------
ssize_t con_write(UNUSED struct _reent *r,UNUSED void *fd,const char *ptr, size_t len) {
//---------------------------------------------------------------------------------

	char chr;

	int i, count = 0;
	const char *tmp = ptr;

	if(!tmp || (int)len<=0) return -1;

	i = 0;

	while(i<(int)len) {

		chr = *(tmp++);
		i++; count++;

		if ( chr == 0x1b && *tmp == '[' ) {
			bool escaping = true;
			const char *escapeseq = tmp++;
			int escapelen = 1;
			i++; count++;

			do {
				chr = *(tmp++);
				i++; count++; escapelen++;
				int parameter, assigned, consumed;

				// make sure parameters are positive values and delimited by semicolon
				if((chr >= '0' && chr <= '9') || chr == ';')
					continue;

				switch (chr) {
					//---------------------------------------
					// Cursor directional movement
					//---------------------------------------
					case 'A':
						consumed = 0;
						assigned = fb_sscanf(escapeseq,"[%dA%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorY  =  (currentConsole->cursorY  - parameter) < 0 ? 0 : currentConsole->cursorY  - parameter;
						escaping = false;
						break;
					case 'B':
						consumed = 0;
						assigned = fb_sscanf(escapeseq,"[%dB%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorY  =  (currentConsole->cursorY  + parameter) > currentConsole->windowHeight - 1 ? currentConsole->windowHeight - 1 : currentConsole->cursorY  + parameter;
						escaping = false;
						break;
					case 'C':
						consumed = 0;
						assigned = fb_sscanf(escapeseq,"[%dC%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorX  =  (currentConsole->cursorX  + parameter) > currentConsole->windowWidth - 1 ? currentConsole->windowWidth - 1 : currentConsole->cursorX  + parameter;
						escaping = false;
						break;
					case 'D':
						consumed = 0;
						assigned = fb_sscanf(escapeseq,"[%dD%n", &parameter, &consumed);
						if (assigned==0) parameter = 1;
						if (consumed)
							currentConsole->cursorX  =  (currentConsole->cursorX  - parameter) < 0 ? 0 : currentConsole->cursorX  - parameter;
						escaping = false;
						break;
					//---------------------------------------
					// Cursor position movement
					//---------------------------------------
					case 'H':
					case 'f':
					{
						int  x, y;
						char c;
						if(fb_sscanf(escapeseq,"[%d;%d%c", &y, &x, &c) == 3 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(fb_sscanf(escapeseq,"[%d;%c", &y, &c) == 2 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(fb_sscanf(escapeseq,"[;%d%c", &x, &c) == 2 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						x = y = 1;
						if(fb_sscanf(escapeseq,"[;%c", &c) == 1 && (c == 'f' || c == 'H')) {
							currentConsole->cursorX = x;
							currentConsole->cursorY = y;
							escaping = false;
							break;
						}

						// invalid format
						escaping = false;
						break;
					}
					//---------------------------------------
					// Screen clear
					//---------------------------------------
					case 'J':
						if(escapelen <= 3)
							consoleCls(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Line clear
					//---------------------------------------
					case 'K':
						if(escapelen <= 3)
							consoleClearLine(escapeseq[escapelen-2]);
						escaping = false;
						break;
					//---------------------------------------
					// Save cursor position
					//---------------------------------------
					case 's':
						if(escapelen == 2) {
							currentConsole->prevCursorX  = currentConsole->cursorX ;
							currentConsole->prevCursorY  = currentConsole->cursorY ;
						}
						escaping = false;
						break;
					//---------------------------------------
					// Load cursor position
					//---------------------------------------
					case 'u':
						if(escapelen == 2) {
							currentConsole->cursorX  = currentConsole->prevCursorX ;
							currentConsole->cursorY  = currentConsole->prevCursorY ;
						}
						escaping = false;
						break;
					//---------------------------------------
					// Color scan codes
					//---------------------------------------
					case 'm':
						escapeseq++;
						escapelen--;

						do {
							parameter = 0;
							if (escapelen == 1) {
								consumed = 1;
							} else if (memchr(escapeseq,';',escapelen)) {
								fb_sscanf(escapeseq,"%d;%n", &parameter, &consumed);
							} else {
								fb_sscanf(escapeseq,"%dm%n", &parameter, &consumed);
							}

							escapeseq += consumed;
							escapelen -= consumed;

							switch(parameter) {
							case 0: // reset
								currentConsole->flags = 0;
								currentConsole->bg    = 0;
								currentConsole->fg    = 7;
								break;

							case 1: // bold
								currentConsole->flags &= ~CONSOLE_COLOR_FAINT;
								currentConsole->flags |= CONSOLE_COLOR_BOLD;
								break;

							case 2: // faint
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								currentConsole->flags |= CONSOLE_COLOR_FAINT;
								break;

							case 3: // italic
								currentConsole->flags |= CONSOLE_ITALIC;
								break;

							case 4: // underline
								currentConsole->flags |= CONSOLE_UNDERLINE;
								break;

							case 5: // blink slow
								currentConsole->flags &= ~CONSOLE_BLINK_FAST;
								currentConsole->flags |= CONSOLE_BLINK_SLOW;
								break;

							case 6: // blink fast
								currentConsole->flags &= ~CONSOLE_BLINK_SLOW;
								currentConsole->flags |= CONSOLE_BLINK_FAST;
								break;

							case 7: // reverse video
								currentConsole->flags |= CONSOLE_COLOR_REVERSE;
								break;

							case 8: // conceal
								currentConsole->flags |= CONSOLE_CONCEAL;
								break;

							case 9: // crossed-out
								currentConsole->flags |= CONSOLE_CROSSED_OUT;
								break;

							case 21: // bold off
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								break;

							case 22: // normal color
								currentConsole->flags &= ~CONSOLE_COLOR_BOLD;
								currentConsole->flags &= ~CONSOLE_COLOR_FAINT;
								break;

							case 23: // italic off
								currentConsole->flags &= ~CONSOLE_ITALIC;
								break;

							case 24: // underline off
								currentConsole->flags &= ~CONSOLE_UNDERLINE;
								break;

							case 25: // blink off
								currentConsole->flags &= ~CONSOLE_BLINK_SLOW;
								currentConsole->flags &= ~CONSOLE_BLINK_FAST;
								break;

							case 27: // reverse off
								currentConsole->flags &= ~CONSOLE_COLOR_REVERSE;
								break;

							case 29: // crossed-out off
								currentConsole->flags &= ~CONSOLE_CROSSED_OUT;
								break;

							case 30 ... 37: // writing color
								currentConsole->fg = parameter - 30;
								break;

							case 39: // reset foreground color
								currentConsole->fg = 7;
								break;

							case 40 ... 47: // screen color
								currentConsole->bg = parameter - 40;
								break;

							case 49: // reset background color
								currentConsole->fg = 0;
								break;
							default: ;
							}
						} while (escapelen > 0);

						escaping = false;
						break;

					default:
						// some sort of unsupported escape; just gloss over it
						escaping = false;
						break;
				}
			} while (escaping);
			continue;
		}

		consolePrintChar(chr);
	}

	return count;
}

//---------------------------------------------------------------------------------
PrintConsole* consoleInit(u8 screen, PrintConsole* console) {
//---------------------------------------------------------------------------------

	if(console) {
		currentConsole = console;
	} else {
		console = currentConsole;
	}

	*currentConsole = defaultConsole;

	console->consoleInitialised = 1;

	//gfxSetScreenFormat(screen,GSP_RGB565_OES);
	GFX_setDoubleBuffering(screen, false);

	console->frameBuffer = (u16*)GFX_getFramebuffer(screen);

	if(screen==SCREEN_TOP) {
		console->consoleWidth = 66;
		console->windowWidth = 66;
	}


	consoleCls('2');

	return currentConsole;

}

//---------------------------------------------------------------------------------
PrintConsole *consoleSelect(PrintConsole* console){
//---------------------------------------------------------------------------------
	PrintConsole *tmp = currentConsole;
	currentConsole = console;
	return tmp;
}

//---------------------------------------------------------------------------------
PrintConsole *consoleGet(void){
//---------------------------------------------------------------------------------
	return currentConsole;
}

//---------------------------------------------------------------------------------
u16 consoleGetFgColor(void){
//---------------------------------------------------------------------------------
	return colorTable[currentConsole->fg];
}

//---------------------------------------------------------------------------------
void consoleSetFont(PrintConsole* console, ConsoleFont* font){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->font = *font;

}

//---------------------------------------------------------------------------------
static void newRow() {
//---------------------------------------------------------------------------------


	currentConsole->cursorY ++;


	if(currentConsole->cursorY  >= currentConsole->windowHeight)  {
		currentConsole->cursorY --;
		u16 *dst = &currentConsole->frameBuffer[(currentConsole->windowX * 6 * 240) + (239 - (currentConsole->windowY * 10))];
		u16 *src = dst - 10;

		int i,j;

		for (i=0; i<currentConsole->windowWidth*6; i++) {
			u32 *from = (u32*)((int)src & ~3);
			u32 *to = (u32*)((int)dst & ~3);
			for (j=0;j<(((currentConsole->windowHeight-1)*10)/2);j++) *(to--) = *(from--);
			dst += 240;
			src += 240;
		}

		consoleClearLine('2');
	}
}
//---------------------------------------------------------------------------------
void consoleDrawChar(int c) {
//---------------------------------------------------------------------------------
	c -= currentConsole->font.asciiOffset;
	if ( c < 0 || c > currentConsole->font.numChars ) return;

	const u8 *fontdata = currentConsole->font.gfx + (10 * c);

	int writingColor = currentConsole->fg;
	int screenColor = currentConsole->bg;

	if (currentConsole->flags & CONSOLE_COLOR_BOLD) {
		writingColor += 8;
	} else if (currentConsole->flags & CONSOLE_COLOR_FAINT) {
		writingColor += 16;
	}

	if (currentConsole->flags & CONSOLE_COLOR_REVERSE) {
		int tmp = writingColor;
		writingColor = screenColor;
		screenColor = tmp;
	}

	u16 bg = colorTable[screenColor];
	u16 fg = colorTable[writingColor];

	u8 b1 = *(fontdata++);
	u8 b2 = *(fontdata++);
	u8 b3 = *(fontdata++);
	u8 b4 = *(fontdata++);
	u8 b5 = *(fontdata++);
	u8 b6 = *(fontdata++);
	u8 b7 = *(fontdata++);
	u8 b8 = *(fontdata++);
	u8 b9 = *(fontdata++);
	u8 b10 = *(fontdata++);

	if (currentConsole->flags & CONSOLE_UNDERLINE) b10 = 0xff;

	if (currentConsole->flags & CONSOLE_CROSSED_OUT) b5 = 0xff;

	u8 mask = 0x80;


	int i;

	int x = (currentConsole->cursorX + currentConsole->windowX) * 6;
	int y = ((currentConsole->cursorY + currentConsole->windowY) * 10);

	u16 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 9))];

	for (i=0;i<6;i++) {
		if (b10 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b9 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b8 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b7 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b6 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b5 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b4 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b3 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b2 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		if (b1 & mask) { *(screen++) = fg; }else{ *(screen++) = bg; }
		mask >>= 1;
		screen += 240 - 10;
	}

}

//---------------------------------------------------------------------------------
void consolePrintChar(int c) {
//---------------------------------------------------------------------------------
	if (c==0) return;

	if(currentConsole->PrintChar)
		if(currentConsole->PrintChar(currentConsole, c))
			return;

	if(currentConsole->cursorX  >= currentConsole->windowWidth) {
		currentConsole->cursorX  = 0;

		newRow();
	}

	switch(c) {
		/*
		The only special characters we will handle are tab (\t), carriage return (\r), line feed (\n)
		and backspace (\b).
		Carriage return & line feed will function the same: go to next line and put cursor at the beginning.
		For everything else, use VT sequences.

		Reason: VT sequences are more specific to the task of cursor placement.
		The special escape sequences \b \f & \v are archaic and non-portable.
		*/
		case 8:
			currentConsole->cursorX--;

			if(currentConsole->cursorX < 0) {
				if(currentConsole->cursorY > 0) {
					currentConsole->cursorX = currentConsole->windowX - 1;
					currentConsole->cursorY--;
				} else {
					currentConsole->cursorX = 0;
				}
			}

			consoleDrawChar(' ');
			break;

		case 9:
			currentConsole->cursorX  += currentConsole->tabSize - ((currentConsole->cursorX)%(currentConsole->tabSize));
			break;
		case 10:
			newRow();
			// falls through
		case 13:
			currentConsole->cursorX  = 0;
			break;
		default:
			consoleDrawChar(c);
			++currentConsole->cursorX ;
			break;
	}
}

//---------------------------------------------------------------------------------
void consoleClear(void) {
//---------------------------------------------------------------------------------
	ee_printf("\x1b[2J");
}

//---------------------------------------------------------------------------------
void consoleSetWindow(PrintConsole* console, int x, int y, int width, int height){
//---------------------------------------------------------------------------------

	if(!console) console = currentConsole;

	console->windowWidth = width;
	console->windowHeight = height;
	console->windowX = x;
	console->windowY = y;

	console->cursorX = 0;
	console->cursorY = 0;

}

void drawConsoleWindow(PrintConsole* console, int thickness, u8 colorIndex) {

	if(colorIndex >= 16) return;

	if(!console) console = currentConsole;
	
	int startx = console->windowX * 8 - thickness;
	int endx = (console->windowX + console->windowWidth) * 8 + thickness;
	
	int starty = (console->windowY - 1) * 8 - thickness;
	int endy = console->windowHeight * 8 + thickness;

	u16 color = colorTable[colorIndex];
	
	// upper line
	for(int y = starty; y < starty + thickness; y++)
		for(int x = startx; x < endx; x++)
		{
			u16 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 7))];
			*screen = color;
		}
	
	// lower line
	for(int y = endy; y > endy - thickness; y--)
		for(int x = startx; x < endx; x++)
		{
			u16 *screen = &currentConsole->frameBuffer[(x * 240) + (239 - (y + 7))];
			*screen = color;
		}
		
	// left and right
	for(int y = starty; y < endy; y++)
	{
		for(int i = 0; i < thickness; i++)
		{
			u16 *screen = &currentConsole->frameBuffer[((startx + i) * 240) + (239 - (y + 7))];
			*screen = color;
			screen = &currentConsole->frameBuffer[((endx - thickness + i) * 240) + (239 - (y + 7))];
			*screen = color;
		}
	}
}

void consoleSetCursor(PrintConsole* console, int x, int y) {
	console->cursorX = x;
	console->cursorY = y;
}

u16 consoleGetRGB565Color(u8 colorIndex) {
	if(colorIndex >= arrayEntries(colorTable))
		return 0;
	return colorTable[colorIndex];
}
