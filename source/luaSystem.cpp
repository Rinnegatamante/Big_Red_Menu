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

int FREAD = 0;
int FWRITE = 1;
int FCREATE = 2;
int NAND = 0;
int SDMC = 1;
FS_archive main_extdata_archive;

void unicodeToChar(char* dst, u16* src)
{
if(!src || !dst)return;
while(*src)*(dst++)=(*(src++))&0xFF;
*dst=0x00;
}

static int lua_exit(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	char string[20];
	strcpy(string,"lpp_exit_0456432");
	return luaL_error(L, string); // NOTE: This is a fake error
}

static int lua_dofile (lua_State *L) {
  int argc = lua_gettop(L);
  if (argc != 1) return luaL_error(L, "wrong number of arguments");
  const char *fname = luaL_checkstring(L, 1);
  Handle fileHandle;
  u64 size;
  u32 bytesRead;
  unsigned char *buffer;
  FS_path filePath=FS_makePath(PATH_CHAR, fname);
  FS_archive script=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
  FSUSER_OpenFileDirectly(NULL, &fileHandle, script, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
  FSFILE_GetSize(fileHandle, &size);
  buffer = (unsigned char*)(malloc((size+1) * sizeof (char)));
  FSFILE_Read(fileHandle, &bytesRead, 0x0, buffer, size);
  buffer[size]=0;
  FSFILE_Close(fileHandle);
  svcCloseHandle(fileHandle);
  lua_settop(L, 1);
  if (luaL_loadbuffer(L, (const char*)buffer, strlen((const char*)buffer), NULL) != LUA_OK)
    return lua_error(L);
  lua_CFunction dofilecont = (lua_CFunction)(lua_gettop(L) - 1);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  return (int)dofilecont;
}

static int lua_openfile(lua_State *L)
{
    int argc = lua_gettop(L);
    if ((argc != 2) && (argc != 3)) return luaL_error(L, "wrong number of arguments");
	const char *file_tbo = luaL_checkstring(L, 1);
	int type = luaL_checkint(L, 2);
	u64 archive_id;
	bool extdata = false;
	if (argc == 3){
		archive_id = luaL_checknumber(L,3);
		extdata = true;
	}
	Handle fileHandle;
	Result ret;
	if (extdata){
		mediatypes_enum mtype;
		FS_archiveIds atype;
		if (archive_id < 0x2000){
			mtype = mediatype_SDMC;
			atype = ARCH_EXTDATA;
		}else{
			mtype = mediatype_NAND;
			atype = ARCH_SHARED_EXTDATA;
		}
		u32 main_extdata_archive_lowpathdata[3] = {mtype, archive_id, 0};
		FS_archive main_extdata_archive = (FS_archive){atype, (FS_path){PATH_BINARY, 0xC, (u8*)main_extdata_archive_lowpathdata}};
		Result ret = FSUSER_OpenArchive(NULL, &main_extdata_archive);
		if(ret!=0) return luaL_error(L, "cannot access extdata archive");
		switch(type){
			case 0:
				ret = FSUSER_OpenFile(NULL, &fileHandle, main_extdata_archive, FS_makePath(PATH_CHAR, file_tbo), FS_OPEN_READ, 0);
				break;
			case 1:
				ret = FSUSER_OpenFile(NULL, &fileHandle, main_extdata_archive, FS_makePath(PATH_CHAR, file_tbo), FS_OPEN_WRITE, 0);
				break;
		}
	}else{
	FS_archive sdmcArchive=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FS_path filePath=FS_makePath(PATH_CHAR, file_tbo);
		switch(type){
			case 0:
				ret=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
				break;
			case 1:
				ret=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_WRITE, FS_ATTRIBUTE_NONE);
				break;
			case 2:
				ret=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_CREATE|FS_OPEN_WRITE, FS_ATTRIBUTE_NONE);
				break;
		}
		if(ret) return luaL_error(L, "error opening file");
	}
	lua_pushnumber(L,fileHandle);
	return 1;
}

static int lua_checkexist(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *file_tbo = luaL_checkstring(L, 1);
	Handle fileHandle;
	FS_archive sdmcArchive=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FS_path filePath=FS_makePath(PATH_CHAR, file_tbo);
	Result ret=FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if (!ret){
	FSFILE_Close(fileHandle);
	}
	svcCloseHandle(fileHandle);
	lua_pushboolean(L,!ret);
	return 1;
}

static int lua_isGW(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 0) return luaL_error(L, "wrong number of arguments");
	lua_pushboolean(L,GW_MODE);
	return 1;
}

static int lua_getsize(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 1) return luaL_error(L, "wrong number of arguments");
	Handle fileHandle = luaL_checknumber(L, 1);
	u64 size;
	Result ret=FSFILE_GetSize(fileHandle, &size);
	if(ret) return luaL_error(L, "error getting size");
	lua_pushnumber(L,size);
	return 1;
}

static int lua_closefile(lua_State *L)
{
    int argc = lua_gettop(L);
    if ((argc != 1) && (argc != 2)) return luaL_error(L, "wrong number of arguments");
	Handle fileHandle = luaL_checknumber(L, 1);
	Result ret=FSFILE_Close(fileHandle);
	svcCloseHandle(fileHandle);
	if (argc == 2) FSUSER_CloseArchive(NULL, &main_extdata_archive);
	if(ret) return luaL_error(L, "error closing file");
	return 0;
}

static int lua_readfile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 3) return luaL_error(L, "wrong number of arguments");
	Handle fileHandle = luaL_checknumber(L, 1);
	u64 init = luaL_checknumber(L, 2);
	u64 size = luaL_checknumber(L, 3);
	u32 bytesRead;
	unsigned char *buffer = (unsigned char*)(malloc((size+1) * sizeof (char)));
	Result ret=FSFILE_Read(fileHandle, &bytesRead, init, buffer, size);
	buffer[size] = 0;
	if(ret || size!=bytesRead) return luaL_error(L, "error reading file");
	lua_pushlstring(L,(const char*)buffer,size);
	free(buffer);
	return 1;
}


static int lua_writefile(lua_State *L)
{
    int argc = lua_gettop(L);
    if (argc != 4) return luaL_error(L, "wrong number of arguments");
	Handle fileHandle = luaL_checknumber(L, 1);
	u64 init = luaL_checknumber(L, 2);
	const char *text = luaL_checkstring(L, 3);
	u64 size = luaL_checknumber(L, 4);
	u32 bytesWritten;
	Result ret=FSFILE_Write(fileHandle, &bytesWritten, init, text, size, FS_WRITE_FLUSH);
	if(ret || size!=bytesWritten) return luaL_error(L, "error writing file");
	return 0;
}

static int lua_getCurrentDirectory(lua_State *L)
{
    lua_pushstring(L, cur_dir);
    return 1;
}

static int lua_setCurrentDirectory(lua_State *L)
{
    const char *path = luaL_checkstring(L, 1);
    if(!path) return luaL_error(L, "System.currentDirectory(file) takes a filename as a string argument.");
    strcpy(cur_dir,path);
    return 1;
}

static int lua_curdir(lua_State *L) {
    int argc = lua_gettop(L);
    if(argc == 0) return lua_getCurrentDirectory(L);
    if(argc == 1) return lua_setCurrentDirectory(L);
    return luaL_error(L, "System.currentDirectory([file]) takes zero or one argument");
}

static int lua_listdir(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *path = luaL_checkstring(L, 1);
	Handle dirHandle;
	FS_dirent entry;
	FS_path dirPath=FS_makePath(PATH_CHAR, path);
	FS_archive sdmcArchive = (FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);
	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
	u32 entriesRead;
	lua_newtable(L);
	int i = 1;
	static char name[1024];
	for (;;){
		entriesRead=0;
		FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
		if (entriesRead){
			lua_pushnumber(L, i++);
			lua_newtable(L);
			lua_pushstring(L, "name");
			unicodeToChar(&name[0],entry.name);
			lua_pushstring(L, name);
			lua_settable(L, -3);
			lua_pushstring(L, "size");
			lua_pushnumber(L, entry.fileSize);
			lua_settable(L, -3);
			lua_pushstring(L, "directory");
			lua_pushboolean(L, entry.isDirectory);
			lua_settable(L, -3);
			lua_settable(L, -3);
		}else break;
	}
	FSDIR_Close(dirHandle);
	FSUSER_CloseArchive(NULL, &sdmcArchive);
	return 1;
}

//AM service support, partially stolen by libctru
static Handle amHandle = 0;

Result amInit()
{
	if(srvGetServiceHandle(&amHandle, "am:net") == 0)
		return (Result)0;
	else
		return srvGetServiceHandle(&amHandle, "am:u");
}

Result amExit()
{
	return svcCloseHandle(amHandle);
}

Result AM_StartCiaInstall(u8 mediatype, Handle *ciahandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04020040;
	cmdbuf[1] = mediatype;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	*ciahandle = cmdbuf[3];
	
	return (Result)cmdbuf[1];
}

Result AM_FinishCiaInstall(u8 mediatype, Handle *ciahandle)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x04050002;
	cmdbuf[1] = 0x10;
	cmdbuf[2] = *ciahandle;

	if((ret = svcSendSyncRequest(amHandle))!=0) return ret;

	return (Result)cmdbuf[1];
}

Result AM_GetTitleCount(u8 mediatype, u32 *count)
{
Result ret = 0;
u32 *cmdbuf = getThreadCommandBuffer();
cmdbuf[0] = 0x00010040;
cmdbuf[1] = mediatype;
if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
*count = cmdbuf[2];
return (Result)cmdbuf[1];
}

Result AM_GetTitleList(u8 mediatype, u32 count, void *buffer)
{
Result ret = 0;
u32 *cmdbuf = getThreadCommandBuffer();
cmdbuf[0] = 0x00020082;
cmdbuf[1] = count;
cmdbuf[2] = mediatype;
cmdbuf[3] = ((count*8) << 4) | 12;
cmdbuf[4] = (u32)buffer;
if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
return (Result)cmdbuf[1];
}

Result AM_GetTitleProductCode(u8 mediatype, u64 titleid, char* product_code)
{
Result ret = 0;
u32 *cmdbuf = getThreadCommandBuffer();
cmdbuf[0] = 0x000500C0;
cmdbuf[1] = mediatype;
cmdbuf[2] = titleid & 0xffffffff;
cmdbuf[3] = (titleid >> 32) & 0xffffffff;
if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
sprintf(product_code,"%s",(char*)(&cmdbuf[2]));
return (Result)cmdbuf[1];
}

Result AM_DeleteTitle(u8 mediatype, u64 titleid)
{
Result ret = 0;
u32 *cmdbuf = getThreadCommandBuffer();
cmdbuf[0] = 0x041000C0;
cmdbuf[1] = mediatype;
cmdbuf[2] = titleid & 0xffffffff;
cmdbuf[3] = (titleid >> 32) & 0xffffffff;
if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
return (Result)cmdbuf[1];
}

Result AM_DeleteAppTitle(u8 mediatype, u64 titleid)
{
Result ret = 0;
u32 *cmdbuf = getThreadCommandBuffer();
cmdbuf[0] = 0x000400C0;
cmdbuf[1] = mediatype;
cmdbuf[2] = titleid & 0xffffffff;
cmdbuf[3] = (titleid >> 32) & 0xffffffff;
if((ret = svcSendSyncRequest(amHandle))!=0) return ret;
return (Result)cmdbuf[1];
}
//Finish AM support

int MAX_RAM_ALLOCATION = 1048576;

static int lua_installCia(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char* path = luaL_checkstring(L, 1);
	Handle fileHandle;
	Handle ciaHandle;
	u64 size;
	u32 bytes;
	FS_archive sdmcArchive=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FS_path filePath=FS_makePath(PATH_CHAR, path);
	FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	amInit();
	AM_StartCiaInstall(mediatype_SDMC, &ciaHandle);
	FSFILE_GetSize(fileHandle, &size);
	Console* console = (Console*)malloc(sizeof(Console));
	console->screen = 0;
	if (size > 1024){
		if (size > 1024 * 1024){
			sprintf(console->text,"Importing...\n0 / %lli MBs",size / (1024*1024));
		}else{
			sprintf(console->text,"Importing...\n0 / %lli KBs",size / 1024);
		}
	}else{
		sprintf(console->text,"Importing...\n0 / %llu Bytes",size);
	}
	gspWaitForVBlank();
	RefreshScreen();
	ClearScreen(0);
	ConsoleOutput(console);
	gfxFlushBuffers();
	gfxSwapBuffers();
	if (size < MAX_RAM_ALLOCATION){
		char* cia_buffer = (char*)(malloc((size) * sizeof (char)));
		FSFILE_Read(fileHandle, &bytes, 0x0, cia_buffer, size);
		FSFILE_Write(ciaHandle, &bytes, 0, cia_buffer, size, 0x10001);
		free(cia_buffer);
	}else{
		u64 i = 0;
		char* cia_buffer;
		while (i < size){
			u64 bytesToRead;
			if	(i+MAX_RAM_ALLOCATION > size){
				cia_buffer = (char*)(malloc((size-i) * sizeof(char)));
				bytesToRead = size - i;
			}else{
				cia_buffer = (char*)(malloc((MAX_RAM_ALLOCATION) * sizeof(char)));
				bytesToRead = MAX_RAM_ALLOCATION;
			}
			FSFILE_Read(fileHandle, &bytes, i, cia_buffer, bytesToRead);
			FSFILE_Write(ciaHandle, &bytes, i, cia_buffer, bytesToRead, 0x10001);
			i = i + bytesToRead;
			sprintf(console->text,"Importing...\n%lli / %lli MBs",i / (1024*1024), size / (1024*1024));
			free(cia_buffer);	
			gspWaitForVBlank();
			RefreshScreen();
			ClearScreen(0);
			ConsoleOutput(console);
			gfxFlushBuffers();
			gfxSwapBuffers();
		}
	}
	AM_FinishCiaInstall(mediatype_SDMC, &ciaHandle);
	FSFILE_Close(fileHandle);
	svcCloseHandle(fileHandle);
	amExit();
	return 0;
}

struct TitleId{
	u32 uniqueid;
	u16 category;
	u16 platform;
};
/* CIA categories
0 = Application
1 = System
2 = DLC
3 = Patch
4 = TWL
*/
static int lua_listCia(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	amInit();
	u32 cia_nums;
	AM_GetTitleCount(mediatype_SDMC, &cia_nums);
	TitleId* TitleIDs = (TitleId*)malloc(cia_nums * sizeof(TitleId));
	AM_GetTitleList(mediatype_SDMC,cia_nums,TitleIDs);
	u32 i = 1;
	lua_newtable(L);
	while (i <= cia_nums){
		lua_pushnumber(L, i);
		lua_newtable(L);
		lua_pushstring(L, "unique_id");
		lua_pushnumber(L, (TitleIDs[i-1].uniqueid));
		lua_settable(L, -3);
		lua_pushstring(L, "mediatype");
		lua_pushnumber(L, 1);
		lua_settable(L, -3);
		lua_pushstring(L, "platform");
		lua_pushnumber(L, (TitleIDs[i-1].platform));
		lua_settable(L, -3);
		u64 id = TitleIDs[i-1].uniqueid | ((u64)TitleIDs[i-1].category << 32) | ((u64)TitleIDs[i-1].platform << 48);
		char product_id[16];
		AM_GetTitleProductCode(mediatype_SDMC, id, product_id);
		lua_pushstring(L, "product_id");
		lua_pushstring(L, product_id);
		lua_settable(L, -3);
		lua_pushstring(L, "access_id");
		lua_pushnumber(L, i);
		lua_settable(L, -3);
		lua_pushstring(L, "category");
		if(((TitleIDs[i-1].category) & 0x8000) == 0x8000) lua_pushnumber(L, 4);
		else if (((TitleIDs[i-1].category) & 0x10) == 0x10) lua_pushnumber(L, 1);
		else if(((TitleIDs[i-1].category) & 0x6) == 0x6) lua_pushnumber(L, 3);
		else if(((TitleIDs[i-1].category) & 0x2) == 0x2) lua_pushnumber(L, 2);
		else lua_pushnumber(L, 0);
		lua_settable(L, -3);
		lua_settable(L, -3);
		i++;
	}
	free(TitleIDs);
	u32 z = 1;
	AM_GetTitleCount(mediatype_NAND, &cia_nums);
	TitleIDs = (TitleId*)malloc(cia_nums * sizeof(TitleId));
	AM_GetTitleList(mediatype_NAND,cia_nums,TitleIDs);
	while (z <= cia_nums){
		lua_pushnumber(L, i);
		lua_newtable(L);
		lua_pushstring(L, "unique_id");
		lua_pushnumber(L, (TitleIDs[i-1].uniqueid));
		lua_settable(L, -3);
		lua_pushstring(L, "mediatype");
		lua_pushnumber(L, 2);
		lua_settable(L, -3);
		lua_pushstring(L, "platform");
		lua_pushnumber(L, (TitleIDs[i-1].platform));
		lua_settable(L, -3);
		u64 id = TitleIDs[i-1].uniqueid | ((u64)TitleIDs[i-1].category << 32) | ((u64)TitleIDs[i-1].platform << 48);
		char product_id[16];
		AM_GetTitleProductCode(mediatype_NAND, id, product_id);
		lua_pushstring(L, "product_id");
		lua_pushstring(L, product_id);
		lua_settable(L, -3);
		lua_pushstring(L, "access_id");
		lua_pushnumber(L, z);
		lua_settable(L, -3);
		lua_pushstring(L, "category");
		if(((TitleIDs[i-1].category) & 0x8000) == 0x8000) lua_pushnumber(L, 4);
		else if (((TitleIDs[i-1].category) & 0x10) == 0x10) lua_pushnumber(L, 1);
		else if(((TitleIDs[i-1].category) & 0x6) == 0x6) lua_pushnumber(L, 3);
		else if(((TitleIDs[i-1].category) & 0x2) == 0x2) lua_pushnumber(L, 2);
		else lua_pushnumber(L, 0);
		lua_settable(L, -3);
		lua_settable(L, -3);
		i++;
		z++;
	}
	free(TitleIDs);
	amExit();
	return 1;
}

static int lua_uninstallCia(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	u32 delete_id = luaL_checknumber(L,1);
	u32 mediatype = luaL_checknumber(L,2);
	mediatypes_enum media;
	if (mediatype == 1) media = mediatype_SDMC;
	else media = mediatype_NAND;
	amInit();
	u32 cia_nums;
	AM_GetTitleCount(media, &cia_nums);
	TitleId* TitleIDs = (TitleId*)malloc(cia_nums * sizeof(TitleId));
	AM_GetTitleList(media,cia_nums,TitleIDs);
	u64 id = TitleIDs[delete_id-1].uniqueid | ((u64)TitleIDs[delete_id-1].category << 32) | ((u64)TitleIDs[delete_id-1].platform << 48);
	AM_DeleteAppTitle(media, id);
	AM_DeleteTitle(media, id);
	amExit();
	free(TitleIDs);
	return 0;
}

u32 Endian_UInt32_Conversion(u32 value){
   return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
}

static int lua_ciainfo(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char* file = luaL_checkstring(L, 1);
	char title[16];
	Handle fileHandle;
	u32 bytesRead;
	FS_archive sdmcArchive=(FS_archive){ARCH_SDMC, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FS_path filePath=FS_makePath(PATH_CHAR, file);
	FSUSER_OpenFileDirectly(NULL, &fileHandle, sdmcArchive, filePath, FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	u32 unique_id;
	FSFILE_Read(fileHandle, &bytesRead, 0x3A50, title, 16);
	FSFILE_Read(fileHandle, &bytesRead, 0x2C20, &unique_id, 4);
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "title");
	lua_pushstring(L, title);
	lua_settable(L, -3);
	lua_pushstring(L, "unique_id");
	lua_pushnumber(L, Endian_UInt32_Conversion(unique_id));
	lua_settable(L, -3);
	FSFILE_Close(fileHandle);
	svcCloseHandle(fileHandle);
	return 1;
}
static int lua_freespace(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc != 0) return luaL_error(L, "wrong number of arguments");
	u32 freeBlocks;
	u32 blockSize;
	FSUSER_GetSdmcArchiveResource(NULL, NULL, &blockSize, NULL, &freeBlocks);
	lua_pushnumber(L,freeBlocks*blockSize);
	return 1;
}

static int lua_delfile(lua_State *L) {
	int argc = lua_gettop(L);
	if (argc != 1) return luaL_error(L, "wrong number of arguments");
	const char *path = luaL_checkstring(L, 1);
	FS_archive sdmcArchive = (FS_archive){0x9, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);
	FS_path filePath=FS_makePath(PATH_CHAR, path);
	FSUSER_DeleteFile(NULL,sdmcArchive,filePath);
	FSUSER_CloseArchive(NULL, &sdmcArchive);
	return 0;
}

static int lua_launchCia(lua_State *L){
	int argc = lua_gettop(L);
	if (argc != 2) return luaL_error(L, "wrong number of arguments");
	u32 delete_id = luaL_checknumber(L,1);
	u32 mediatype = luaL_checknumber(L,2);
	mediatypes_enum media;
	if (mediatype == 1) media = mediatype_SDMC;
	else media = mediatype_NAND;
	amInit();
	u32 cia_nums;
	AM_GetTitleCount(media, &cia_nums);
	TitleId* TitleIDs = (TitleId*)malloc(cia_nums * sizeof(TitleId));
	AM_GetTitleList(media,cia_nums,TitleIDs);
	u64 id = TitleIDs[delete_id-1].uniqueid | ((u64)TitleIDs[delete_id-1].category << 32) | ((u64)TitleIDs[delete_id-1].platform << 48);
	free(TitleIDs);
	amExit();
	u8 buf0[0x300];
	u8 buf1[0x20];
	memset(buf0, 0, 0x300);
	memset(buf1, 0, 0x20);
	aptOpenSession();
	APT_PrepareToDoAppJump(NULL, 0, id, mediatype);
	APT_DoAppJump(NULL, 0x300, 0x20, buf0, buf1);
	aptCloseSession();
	return 0;
}

//Register our System Functions
static const luaL_Reg System_functions[] = {
  {"exit",					lua_exit},
  {"isGWMode",				lua_isGW},
  {"currentDirectory",		lua_curdir},
  {"doesFileExist",			lua_checkexist},
  {"listDirectory",			lua_listdir},
  {"installCIA",			lua_installCia},
  {"listCIA",				lua_listCia},
  {"deleteFile",			lua_delfile},
  {"uninstallCIA",			lua_uninstallCia},
  {"getFreeSpace",			lua_freespace},
  {"extractCIA",			lua_ciainfo},
  {"launchCIA",				lua_launchCia},
// I/O Module and Dofile Patch
  {"openFile",				lua_openfile},
  {"getFileSize",			lua_getsize},
  {"closeFile",				lua_closefile},
  {"readFile",				lua_readfile},
  {"writeFile",				lua_writefile},
  {"dofile",				lua_dofile},
// End I/O Module and Dofile Patch
  {0, 0}
};

void luaSystem_init(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, System_functions, 0);
	lua_setglobal(L, "System");
	VariableRegister(L,FREAD);
	VariableRegister(L,FWRITE);
	VariableRegister(L,FCREATE);
}