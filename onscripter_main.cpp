/* -*- C++ -*-
 * 
 *  onscripter_main.cpp -- main function of ONScripter
 *
 *  Copyright (c) 2001-2014 Ogapee. All rights reserved.
 *            (C) 2014 jh10001 <jh10001@live.cn>
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ONScripter.h"
#include "Utils.h"
#include "gbk2utf16.h"
#include "sjis2utf16.h"
#include "version.h"

ONScripter ons;
Coding2UTF16 *coding2utf16 = nullptr;

#if defined(IOS)
#import <Foundation/NSArray.h>
#import <UIKit/UIKit.h>
#import "DataCopier.h"
#import "DataDownloader.h"
#import "ScriptSelector.h"
#import "MoviePlayer.h"
#endif

#ifdef WINRT
#include "ScriptSelector.h"
#endif

#if defined(PSP)
#include <pspkernel.h>
#include <psputility.h>
#include <psppower.h>
#include <ctype.h>

PSP_HEAP_SIZE_KB(-1);

int psp_power_resume_number = 0;

int exit_callback(int arg1, int arg2, void *common)
{
    ons.endCommand();
    sceKernelExitGame();
    return 0;
}

int power_callback(int unknown, int pwrflags, void *common)
{
    if (pwrflags & PSP_POWER_CB_RESUMING) psp_power_resume_number++;
    return 0;
}

int CallbackThread(SceSize args, void *argp)
{
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    cbid = sceKernelCreateCallback("Power Callback", power_callback, NULL);
    scePowerRegisterCallback(0, cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int SetupCallbacks(void)
{
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0) sceKernelStartThread(thid, 0, 0);
    return thid;
}
#endif

void optionHelp()
{
    printf( "Usage: onscripter [option ...]\n" );
    printf( "      --cdaudio\t\tuse CD audio if available\n");
    printf( "      --cdnumber no\tchoose the CD-ROM drive number\n");
    printf( "  -f, --font file\tset a TTF font file\n");
    printf( "      --registry file\tset a registry file\n");
    printf( "      --dll file\tset a dll file\n");
    printf( "  -r, --root path\tset the root path to the archives\n");
    printf( "      --fullscreen\tstart in fullscreen mode\n");
    printf( "      --window\t\tstart in windowed mode\n");
    printf( "      --force-button-shortcut\tignore useescspc and getenter command\n");
    printf( "      --enable-wheeldown-advance\tadvance the text on mouse wheel down\n");
    printf( "      --disable-rescale\tdo not rescale the images in the archives\n");
    printf( "      --render-font-outline\trender the outline of a text instead of casting a shadow\n");
    printf( "      --edit\t\tenable online modification of the volume and variables when 'z' is pressed\n");
    printf( "      --key-exe file\tset a file (*.EXE) that includes a key table\n");
	printf( "      --enc:sjis\tuse sjis coding script\n");
	printf( "      --debug:1\tprint debug info\n");
    printf( "  -h, --help\t\tshow this help and exit\n");
    printf( "  -v, --version\t\tshow the version information and exit\n");
    exit(0);
}

void optionVersion()
{
    printf("Written by Ogapee <ogapee@aqua.dti2.ne.jp>\n\n");
    printf("Copyright (c) 2001-2014 Ogapee.\n\
    		          (C) 2014 jh10001");
    printf("This is free software; see the source for copying conditions.\n");
    exit(0);
}

#ifdef ANDROID && !SDL_VERSION_ATLEAST(2,0,0)
extern "C"
{
#include <jni.h>

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

JNIEXPORT jint JNICALL 
JAVA_EXPORT_NAME(ONScripter_nativeGetWidth) ( JNIEnv*  env, jobject thiz )
{
	return ons.getWidth();
}

JNIEXPORT jint JNICALL 
JAVA_EXPORT_NAME(ONScripter_nativeGetHeight) ( JNIEnv*  env, jobject thiz )
{
	return ons.getHeight();
}
}
#endif

#if defined(IOS)
extern "C" void playVideoIOS(const char *filename, bool click_flag, bool loop_flag)
{
    NSString *str = [[NSString alloc] initWithUTF8String:filename];
    id obj = [MoviePlayer alloc];
    [[obj init] play:str click:click_flag loop:loop_flag];
    [obj release];
}
#endif

#ifdef WINRT
#include "windows.h"
BOOL WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	if (dwSize < dwMinSize) {
		return FALSE;
	}
	WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, lpszStr, dwSize, NULL, FALSE);
	return TRUE;
}
#endif

#if (defined(QWS) || defined(ANDROID)) && !SDL_VERSION_ATLEAST(2,0,0)
int SDL_main( int argc, char **argv )
#elif defined(PSP)
extern "C" int main( int argc, char **argv )
#else
int main( int argc, char *argv[] )
#endif
{
    utils::printInfo("ONScripter-Jh version %s(%d.%02d)\n", ONS_VERSION, NSC_VERSION/100, NSC_VERSION%100 );

#if defined(PSP)
    ons.disableRescale();
    ons.enableButtonShortCut();
    SetupCallbacks();
#elif defined(WINRT)
	char currentDir[256];
	auto appInstallDirectory = Windows::Storage::ApplicationData::Current->LocalFolder->Path + "\\";
	const wchar_t *wText = appInstallDirectory->Begin();
	WCharToMByte(wText,currentDir,256);
	char* cptr = currentDir;
	int i, len = strlen(currentDir);
	for (i = len - 1; i>0; i--) {
		if (cptr[i] == '\\' || cptr[i] == '/')
			break;
	}
	cptr[i] = '\0';
	{
		ScriptSelector ss(currentDir);
		ons.setArchivePath(ss.selectedPath);
	}
	ons.disableRescale();
#elif defined(WINCE)
    char currentDir[256];
    strcpy(currentDir, argv[0]);
    char* cptr = currentDir;
    int i, len = strlen(currentDir);
    for(i=len-1; i>0; i--){
        if(cptr[i] == '\\' || cptr[i] == '/')
            break;
    }
    cptr[i] = '\0';
    ons.setArchivePath(currentDir);
    ons.disableRescale();
    ons.enableButtonShortCut();
#elif defined(ANDROID) 
    ons.enableButtonShortCut();
#endif

#if defined(IOS)
#if defined(HAVE_CONTENTS)
    if ([[[DataCopier alloc] init] copy]) exit(-1);
#endif

    // scripts and archives are stored under /Library/Caches
    NSArray* cpaths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString* cpath = [[cpaths objectAtIndex:0] stringByAppendingPathComponent:@"ONS"];
    char filename[256];
    strcpy(filename, [cpath UTF8String]);
    ons.setArchivePath(filename);

    // output files are stored under /Documents
    NSArray* dpaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* dpath = [[dpaths objectAtIndex:0] stringByAppendingPathComponent:@"ONS"];
    strcpy(filename, [dpath UTF8String]);
    ons.setSaveDir(filename);

#if defined(ZIP_URL)
    if ([[[DataDownloader alloc] init] download]) exit(-1);
#endif

#if defined(USE_SELECTOR)
    // scripts and archives are stored under /Library/Caches
    cpath = [[[ScriptSelector alloc] initWithStyle:UITableViewStylePlain] select];
    strcpy(filename, [cpath UTF8String]);
    ons.setArchivePath(filename);

    // output files are stored under /Documents
    dpath = [[dpaths objectAtIndex:0] stringByAppendingPathComponent:[cpath lastPathComponent]];
    strcpy(filename, [dpath UTF8String]);
    ons.setSaveDir(filename);
#endif

#if defined(RENDER_FONT_OUTLINE)
    ons.renderFontOutline();
#endif
#endif

    // ----------------------------------------
    // Parse options
    argv++;
    while( argc > 1 ){
        if ( argv[0][0] == '-' ){
            if ( !strcmp( argv[0]+1, "h" ) || !strcmp( argv[0]+1, "-help" ) ){
                optionHelp();
            }
            else if ( !strcmp( argv[0]+1, "v" ) || !strcmp( argv[0]+1, "-version" ) ){
                optionVersion();
            }
            else if ( !strcmp( argv[0]+1, "-cdaudio" ) ){
                ons.enableCDAudio();
            }
            else if ( !strcmp( argv[0]+1, "-cdnumber" ) ){
                argc--;
                argv++;
                ons.setCDNumber(atoi(argv[0]));
            }
            else if ( !strcmp( argv[0]+1, "f" ) || !strcmp( argv[0]+1, "-font" ) ){
                argc--;
                argv++;
                ons.setFontFile(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-registry" ) ){
                argc--;
                argv++;
                ons.setRegistryFile(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-dll" ) ){
                argc--;
                argv++;
                ons.setDLLFile(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "r" ) || !strcmp( argv[0]+1, "-root" ) ){
                argc--;
                argv++;
                ons.setArchivePath(argv[0]);
            }
            else if ( !strcmp( argv[0]+1, "-fullscreen" ) ){
                ons.setFullscreenMode();
            }
            else if ( !strcmp( argv[0]+1, "-window" ) ){
                ons.setWindowMode();
            }
            else if ( !strcmp( argv[0]+1, "-force-button-shortcut" ) ){
                ons.enableButtonShortCut();
            }
            else if ( !strcmp( argv[0]+1, "-enable-wheeldown-advance" ) ){
                ons.enableWheelDownAdvance();
            }
            else if ( !strcmp( argv[0]+1, "-disable-rescale" ) ){
                ons.disableRescale();
            }
            else if ( !strcmp( argv[0]+1, "-render-font-outline" ) ){
                ons.renderFontOutline();
            }
            else if ( !strcmp( argv[0]+1, "-edit" ) ){
                ons.enableEdit();
            }
            else if ( !strcmp( argv[0]+1, "-key-exe" ) ){
                argc--;
                argv++;
                ons.setKeyEXE(argv[0]);
            }
			else if (!strcmp(argv[0] + 1, "-enc:sjis")) {
				if (coding2utf16 == nullptr) coding2utf16 = new SJIS2UTF16();
			}
			else if (!strcmp(argv[0] + 1, "-debug:1")) {
				ons.setDebugLevel(1);
			}
#if defined(ANDROID) 
#if SDL_VERSION_ATLEAST(2,0,0)
            else if (!strcmp(argv[0] + 1, "-compatible")){
				ons.setCompatibilityMode(); 
            }
#else
            else if ( !strcmp( argv[0]+1, "-open-only" ) ){
                argc--;
                argv++;
                if (ons.openScript()) exit(-1);
                return 0;
            }
#endif //SDL_VERSION_ATLEAST(2,0,0)
#endif
            else{
                utils::printInfo(" unknown option %s\n", argv[0] );
            }
        }
        else{
            optionHelp();
        }
        argc--;
        argv++;
    }

	if (coding2utf16 == nullptr) coding2utf16 = new GBK2UTF16();
    
    // ----------------------------------------
    // Run ONScripter
    if (ons.openScript()) exit(-1);
    if (ons.init()) exit(-1);
    ons.executeLabel();
	delete coding2utf16;
    exit(0);
}
