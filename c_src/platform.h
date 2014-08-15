
/*
 *
 * platform.h -  12-Oct-1996 Cornel Huth
 * This is included by all bd*.c
 *
 * Purpose is to select the platform the compile is for, and
 * also includes needed header files (standard ones, too).
 * I put all this in here just to make the bd_* files less
 * cluttered, and is meant for use only with the bd_* project.
 *
 * See bullet_2.h for more on Bullet-only issues.
 *
 */

#define ON_DOSX32 3
#define ON_OS2    4
#define ON_WIN32  5

// * * *  SELECT ONE  * * *  of the three below ***

#ifndef PLATFORM
// #define PLATFORM ON_DOSX32
// #define PLATFORM ON_OS2
// #define PLATFORM ON_WIN32
#endif

#if PLATFORM == ON_DOSX32
   #include <dos.h>                 // for _harderr
   typedef unsigned long HANDLE;
   typedef unsigned long TID;
   #define USE_ANSI 0               // set to 1 if ANSI screen control wanted
   #define HANDLES_WANTED 255       // limited to FILES= in config.sys
   #define FOR_WINDOWS 0

#elif PLATFORM == ON_OS2
   #define INCL_DOSPROCESS          // for OS/2 threads
   #define INCL_DOSMISC             // for DosError()
   #include <os2.h>
   typedef unsigned long HANDLE;
   #define USE_ANSI 1
   #define HANDLES_WANTED 1030
   #define FOR_WINDOWS 0

#elif PLATFORM == ON_WIN32
   // set FOR_WINDOWS==1 if doing a windowed app (win32s target) or 0 if a console app
   #ifndef FOR_WINDOWS
    #define FOR_WINDOWS 0
   #endif
   #if FOR_WINDOWS == 1
    #define Q_INS 200   // deal with message queue at least every 200 inserts
    #define Q_ADD 1000  // every 1000 adds
    #define Q_UPD 150   // every 150 updates
    #define Q_GET 750   // every 750 get key+record accesses
    #define Q_KEY 1000  // every 1000 get key accesses
   #endif

   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>
   typedef DWORD TID;
   #define USE_ANSI 0
   #define HANDLES_WANTED 1030

#else
   #error No PLATFORM defined in platform.h
   #error ---------------------------------

#endif

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


// bullet_2.h requires PLATFORM defined

#include "bullet_2.h"   


// DOSX32 and OS/2 use OEM character set, Windows the Windows character set (aka 'ANSI').
// ccdosfn.c shows both OEM and ANSI character set tables.
// DOSX32 and OS/2 do not generally have or use an ANSI character set, only OEM.
// Windows generally uses the ANSI char set, but may also use an OEM char set.
// If neither is specific (CIP.sortFunction), USE_OEM_CHARSET is used by default (needed
// only if an NLS_SORT or a custom sort-compare).

#if PLATFORM == ON_DOSX32
   #define SORT_SET  USE_OEM_CHARSET

#elif PLATFORM == ON_OS2
   #define SORT_SET  USE_OEM_CHARSET

#elif PLATFORM == ON_WIN32
   #define SORT_SET  USE_ANSI_CHARSET

#endif

