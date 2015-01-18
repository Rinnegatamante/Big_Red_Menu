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
#include "include/luaplayer.h"
#include "include/luaGraphics.h"

#define stringify(str) #str
#define VariableRegister(lua, value) do { lua_pushinteger(lua, value); lua_setglobal (lua, stringify(value)); } while(0)
#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

static int lua_print(lua_State *L)
{
    int argc = lua_gettop(L);
    if ((argc != 5) && (argc != 6)) return luaL_error(L, "wrong number of arguments");
	int x = luaL_checkint(L, 1);
    int y = luaL_checkint(L, 2);
	char* text = (char*)(luaL_checkstring(L, 3));
	u32 color = luaL_checknumber(L,4);
	u8 alpha = (color >> 24) & 0xFF;
	int screen = luaL_checkint(L,5);
	int side=0;
	if (argc == 6) side = luaL_checkint(L,6);
	if (alpha=255) DrawScreenText(x,y,text,color,screen,side);
	gfxFlushBuffers();
	return 0;
}

static int lua_enable3D(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	gfxSet3D(true);
	return 0;
}

static int lua_disable3D(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	gfxSet3D(false);
	return 0;
}

static int lua_get3D(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushnumber(L, CONFIG_3D_SLIDERSTATE);
	return 1;
}

static int lua_flip(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	gfxSwapBuffers();
	return 0;
}

static int lua_refresh(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	RefreshScreen();	
	return 0;
}

static int lua_Vblank(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	gspWaitForVBlank();
	return 0;
}

static int lua_clearScreen(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	int screen = luaL_checkint(L,1);
	ClearScreen(screen);
	gfxFlushBuffers();
	return 0;
}

static int lua_fillRect(lua_State *L)
{
    int argc = lua_gettop(L);
    if ((argc != 6) && (argc != 7)) return luaL_error(L, "wrong number of arguments");
	int x1 = luaL_checkint(L,1);
	int x2 = luaL_checkint(L,2);
	int y1 = luaL_checkint(L,3);
	int y2 = luaL_checkint(L,4);
	u32 color = luaL_checknumber(L,5);
	u8 alpha = (color >> 24) & 0xFF;
	int screen = luaL_checkint(L,6);
	int side=0;
	if (argc == 7) side = luaL_checkint(L,7);
	if (alpha==255) FillScreenRect(x1,x2,y1,y2,color,screen,side);
	gfxFlushBuffers();
	return 0;
}

static int lua_fillEmptyRect(lua_State *L)
{
    int argc = lua_gettop(L);
    if ((argc != 6) && (argc != 7)) return luaL_error(L, "wrong number of arguments");
	int x1 = luaL_checkint(L,1);
	int x2 = luaL_checkint(L,2);
	int y1 = luaL_checkint(L,3);
	int y2 = luaL_checkint(L,4);
	u32 color = luaL_checknumber(L,5);
	u8 alpha = (color >> 24) & 0xFF;
	int screen = luaL_checkint(L,6);
	int side=0;
	if (argc == 7) side = luaL_checkint(L,7);
	if (alpha==255) FillScreenEmptyRect(x1,x2,y1,y2,color,screen,side);
	gfxFlushBuffers();
	return 0;
}

static int lua_color(lua_State *L) {
    int argc = lua_gettop(L);
    if ((argc != 3) && (argc != 4)) return luaL_error(L, "wrong number of arguments");
    int r = luaL_checkint(L, 1);
    int g = luaL_checkint(L, 2);
	int b = luaL_checkint(L, 3);
	int a = 255;
	if (argc==4) a = luaL_checkint(L, 4);
    u32 color = b | (g << 8) | (r << 16) | (a << 24);
    lua_pushnumber(L,color);
    return 1;
}

static int lua_getB(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
    u32 colour = color & 0xFF;
    lua_pushnumber(L,colour);
    return 1;
}

static int lua_getG(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
    u32 colour = (color >> 8) & 0xFF;
    lua_pushnumber(L,colour);
    return 1;
}

static int lua_getR(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
    u32 colour = (color >> 16) & 0xFF;
    lua_pushnumber(L,colour);
    return 1;
}

static int lua_getA(lua_State *L) {
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
    int color = luaL_checkint(L, 1);
    u32 colour = (color >> 24) & 0xFF;
    lua_pushnumber(L,colour);
    return 1;
}


//Register our Color Functions
static const luaL_Reg Color_functions[] = {
  {"new",                				lua_color},
  {"getR",								lua_getR},
  {"getG",								lua_getG},
  {"getB",								lua_getB},
  {"getA",								lua_getA},
  {0, 0}
};

//Register our Screen Functions
static const luaL_Reg Screen_functions[] = {
  {"debugPrint",					lua_print},
  {"waitVblankStart",				lua_Vblank},
  {"flip",							lua_flip},
  {"refresh",						lua_refresh},
  {"clear",							lua_clearScreen},
  {"fillRect",						lua_fillRect},
  {"fillEmptyRect",					lua_fillEmptyRect},
  {0, 0}
};

void luaScreen_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, Screen_functions, 0);
	lua_setglobal(L, "Screen");
	lua_newtable(L);
	luaL_setfuncs(L, Color_functions, 0);
	lua_setglobal(L, "Color");
	int TOP_SCREEN = 0;
	int BOTTOM_SCREEN = 1;
	int LEFT_EYE = 0;
	int RIGHT_EYE = 1;
	VariableRegister(L,TOP_SCREEN);
	VariableRegister(L,BOTTOM_SCREEN);
	VariableRegister(L,LEFT_EYE);
	VariableRegister(L,RIGHT_EYE);
}