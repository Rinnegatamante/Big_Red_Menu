/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <lpp.nanni@gmail.com> ---------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@gmail.com> -------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Smealum for ctrulib -------------------------------------------------------------------------------------------------#
#- StapleButter for debug font -----------------------------------------------------------------------------------------#
#- Lode Vandevenne for lodepng -----------------------------------------------------------------------------------------#
#- Sean Barrett for stb_truetype ---------------------------------------------------------------------------------------#
#- Special thanks to Aurelio for testing, bug-fixing and various help with codes and implementations -------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <3ds.h>
#include "include/luaGraphics.h"
#include "include/font.h"

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

typedef unsigned short u16;
u8* TopLFB;
u8* TopRFB;
u8* BottomFB;

void DrawPixel(u8* screen, int x,int y, u32 color){
	int idx = ((x)*240) + (239-(y));
	screen[idx*3+0] = (color);
	screen[idx*3+1] = (color) >> 8;
	screen[idx*3+2] = (color) >> 16;
}

void RefreshScreen(){
TopLFB = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
if (CONFIG_3D_SLIDERSTATE != 0) TopRFB = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
BottomFB = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
}

void DrawScreenText(int x, int y, char* str, u32 color, int screen,int side){
u8* buffer;
if (screen == 0){
if (side == 0) buffer = TopLFB;
else buffer = TopRFB;
}else if (screen == 1) buffer = BottomFB;
unsigned short* ptr;
unsigned short glyphsize;
int i, cx, cy;
for (i = 0; str[i] != '\0'; i++)
{
if (str[i] < 0x21)
{
x += 6;
continue;
}
u16 ch = str[i];
if (ch > 0x7E) ch = 0x7F;
ptr = &font[(ch-0x20) << 4];
glyphsize = ptr[0];
if (!glyphsize)
{
x += 6;
continue;
}
x++;
for (cy = 0; cy < 12; cy++)
{
unsigned short val = ptr[4+cy];
for (cx = 0; cx < glyphsize; cx++)
{
if (val & (1 << cx))
DrawPixel(buffer, x+cx, y+cy, color);
}
}
x += glyphsize;
x++;
}
}

void DebugOutput(char* str){
unsigned short* ptr;
unsigned short glyphsize;
int i, cx, cy;
int x=0;
int y=0;
for (i = 0; str[i] != '\0'; i++)
{
if (str[i] == 0x0A){
x=0;
y=y+15;
continue;
}else if(str[i] == 0x0D){
continue;
}
if (str[i] < 0x21)
{
x += 6;
continue;
}
u16 ch = str[i];
if (ch > 0x7E) ch = 0x7F;
ptr = &font[(ch-0x20) << 4];
glyphsize = ptr[0];
if (!glyphsize)
{
x += 6;
continue;
}
x++;
for (cy = 0; cy < 12; cy++)
{
unsigned short val = ptr[4+cy];
for (cx = 0; cx < glyphsize; cx++)
{
if ((x+cx) >= 320){
x=0;
y=y+15;
}
if (val & (1 << cx))
DrawPixel(BottomFB, x+cx, y+cy, 0xFFFFFF);
}
}
x += glyphsize;
x++;
}
}

void FillScreenRect(int x1,int x2,int y1,int y2,u32 color,int screen,int side){
	u8* buffer;
	if (screen == 0){
		if (side == 0) buffer = TopLFB;
		else buffer = TopRFB;
	}else if (screen == 1) buffer = BottomFB;
	if (x1 > x2){
	int temp_x = x1;
	x1 = x2;
	x2 = temp_x;
	}
	if (y1 > y2){
	int temp_y = y1;
	y1 = y2;
	y2 = temp_y;
	}
	int base_y = y1;
	while (x1 <= x2){
		while (y1 <= y2){
			DrawPixel(buffer,x1,y1,color);
			y1++;
		}
		y1 = base_y;
		x1++;
	}
}

void FillScreenEmptyRect(int x1,int x2,int y1,int y2,u32 color,int screen,int side){
	u8* buffer;
	if (screen == 0){
		if (side == 0) buffer = TopLFB;
		else buffer = TopRFB;
	}else if (screen == 1) buffer = BottomFB;
	if (x1 > x2){
	int temp_x = x1;
	x1 = x2;
	x2 = temp_x;
	}
	if (y1 > y2){
	int temp_y = y1;
	y1 = y2;
	y2 = temp_y;
	}
	int base_y = y1;
	while (y1 <= y2){
		DrawPixel(buffer,x1,y1,color);
		DrawPixel(buffer,x2,y1,color);
		y1++;
	}
	while (x1 <= x2){
		DrawPixel(buffer,x1,base_y,color);
		DrawPixel(buffer,x1,y2,color);
		x1++;
	}
}

void ClearScreen(int screen){
	if (screen==1){
		memset(BottomFB,0x00,230400);
	}else{
		memset(TopLFB,0x00,288000);
	if (CONFIG_3D_SLIDERSTATE != 0){
		memset(TopRFB,0x00,288000);
	}
	}
}