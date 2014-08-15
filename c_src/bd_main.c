
/* This main() is for console apps.  For Win32s see bd_mainw.c, instead.
 *
 * bd_main.c - 30-Sep-1996 Cornel Huth
 * This module calls the other bd_*.c modules
 * Bullet 2 Demonstration Program
 *
 * These samples should be read as if a book, where you start
 * with bd_main.c, then to bd_rix.c, then bd_ko, bd_ins, bd_upd,
 * bd_join, bd_memo, bd_files, bd_lock, and bd_cust ...
 *
 * This is because the later examples do not explain what has
 * already been explained in previous samples.
 *
 * Filenames used in these demo modules are prefixed with .\ to
 * ensure that no path searching is done (if ever the OS would).
 *
 */

#define PRG_DATE "[12-Oct-96]"

#include "platform.h"           // defines platform, brings in includes

#if FOR_WINDOWS == 1
 #define main main2

// external to this module (in bd_mainw.c)


 extern HWND gHwnd;
 extern int gCurrStr;           // current element in gPutStr[n][0]  (n=1 to 39)
 extern CHAR gPutStr[40][82];   // output from bd_*.c modules should be less than this size

 void DoWinThing(int waitFlag); // GetMessage() loop (lack of threads is somewhat awkward)
 extern MSG gMsg;
 extern int gInputReady;        // flag when CR hit
 extern int gCurrChar;          // current character pointer, in gGetStr[] (n=0 to 80)
 extern CHAR gGetStr[82];       // input from bd_*.c (GetMsg())
 extern BOOL gDie;              // pull the plug

#endif

// external to this module

int bd_rix();     // add, reindex, threads, access
int bd_ko();      // key-only index managing
int bd_ins();     // insert, transactions
int bd_upd();     // update, transactions
int bd_join();    // join, multi-table view
int bd_memo();    // memo fields/memo files
int bd_files();   // lots of open files
int bd_locks();   // region locks
int bd_rixu();    // similar to bd_rix() but uses custom sort-compare


// public in this module

void PutMsg(CHAR *strg);
void GetMsg(CHAR *strg);


#if PLATFORM == ON_DOSX32

 int __far INT24HANDLER(unsigned deverr,unsigned errcode,unsigned far *devhdr) {

    //if (!(deverr & 0x8000)) {
    //
    //   // Bit15=0 is disk error
    //
    //   return 0;                      // 0=_HARDERR_IGNORE
    //}

    // you may prefer to abort, rather than ignore, if not a disk error
    // see your compiler for details
    // #include <conio.h>           // for cprintf()
    // cprintf( "Critical error other than from Bullet: " );
    // cprintf( "deverr=%4.4X errcode=%d\r\n",deverr, errcode );
    // cprintf( "devhdr = %Fp\r\n", devhdr );

    return 0;

    deverr;
    errcode;
    devhdr;
 }

#endif

CHAR *collateTable = NULL; // use OS-supplied collate table (DOSX32 see ccdosfn.c for more)
CHAR gCLS[6] = "";         // used here and in bd_join

// Main program start ////////////////////////////////////////////////////////

int main(void) {

 //MSG msg;

#pragma pack(1)

 EXITPACK EP;
 INITPACK IP;

#if PLATFORM == ON_DOSX32
 QUERYSETPACK QSP;

#ifdef __cplusplus
 extern "C" {
#endif
  // these are defined in ccdosfn.c
  long __cdecl BulletMalloc(ULONG bytes, VOID **baseAddrPtr);
  long __cdecl BulletFree(VOID *baseAddr);
  long __cdecl BulletGetMemoryAvail(void);
  long __cdecl BulletGetTmpDir(CHAR *bufferPtr);

#ifdef __cplusplus
 }
#endif
#endif

#pragma pack()

 int rez=0;

 CHAR tmpStr[128];
 CHAR putStr[128];

 CHAR aPRGDATE[] = PRG_DATE;
 CHAR op1[] = "Import/Add/REINDEX, with import data generated on-the-fly to DBF.";
 CHAR op2[] = "Key I/O on non-Bullet data file (only index file is maintained).";
 CHAR op3[] = "Atomic INSERT (transaction-list of up to 512 files).";
 CHAR op4[] = "Atomic UPDATE (transaction-list of up to 512 files).";
 CHAR op5[] = "Two-table joined view of EMPLOYEE and DEPARTMENT tables.";
 CHAR op6[] = "Memo support for text/BLOB data to DBT.";
 CHAR op7[] = "Files blowout -- many files opened simultaneously.";
 CHAR op8[] = "Lock file, region, & record; exclusive/shared relock.";
 CHAR op9[] = "Import/Add/REINDEX (as #1) using Custom sortFunction (float key).";

 CHAR aGRN[] = "";
 CHAR aWHT[] = "";

#if FOR_WINDOWS == 1
 strcpy(gCLS,"\x0C");
#elif USE_ANSI == 0
 strcpy(gCLS,"\n");
#else
 strcpy(gCLS,"\x1B[2J");
 CHAR aGRN[] = "\x1B[1;32m";
 CHAR aWHT[] = "\x1B[0;37m";
#endif

 setbuf(stdout,NULL);

#if PLATFORM == ON_DOSX32

 // set minimum required external function vectors for DOSX32 compiler support
 // these must be setup before calling INIT_XB to prevent chicken-and-the-egg dilemma

 QSP.func = SET_VECTORS_XB;
 QSP.item = VECTOR_MALLOC;
 QSP.itemValue = (ULONG) &BulletMalloc;   // in ccdosfn.c
 rez = BULLET(&QSP);
 if (rez==0) {
    QSP.item = VECTOR_FREE;
    QSP.itemValue = (ULONG) &BulletFree;
    rez = BULLET(&QSP);
    if (rez==0) {
       QSP.item = VECTOR_GET_MEMORY;
       QSP.itemValue = (ULONG) &BulletGetMemoryAvail;
       rez = BULLET(&QSP);
       if (rez==0) {
          QSP.item = VECTOR_GET_TMP_DIR;
          QSP.itemValue = (ULONG) &BulletGetTmpDir;
          rez = BULLET(&QSP);
       }
    }
 }
 if (rez) {
    sprintf(putStr,"Failed SET_VECTORS_XB #%d (DOSX32 support setup) call.  Err: %d\n",QSP.item,rez);
    PutMsg(putStr);
    goto ExitNow;
 }

#endif

 IP.func = INIT_XB;
 IP.JFTsize = HANDLES_WANTED;

 rez = BULLET(&IP);
 if (rez) {
    sprintf(putStr,"Failed Bullet initialization.  Err: %d\n",rez);
    PutMsg(putStr);
    goto ExitNow;
 }

#if PLATFORM == ON_DOSX32

 // register EXIT_XB with the startup code's shutdown list so that Bullet
 // is exited with files closed, etc.

 rez = atexit(IP.exitPtr);
 if (rez) PutMsg("\a\n<*> Could not register EXIT_XB with atexit().  Continuing...\n");

 _harderr(INT24HANDLER);

#elif PLATFORM == ON_OS2

 // Instead of using _harderr() as is done for DOSX32, in OS/2 all one needs to
 // do to prevent pop-ups is to call DosError(FERR_DISABLEHARDERR) (or 0).
 // You may prefer to place it around Bullet sections only (use
 // FERR_ENABLEHARDERR, or 1, to restore pop-ups).  This does not disable
 // numeric or other exceptions, which would be FERR_DISABLEEXCEPTION, or 2.

 DosError(FERR_DISABLEHARDERR);

#elif PLATFORM == ON_WIN32

 // register EXIT_XB with the startup code's shutdown list so that Bullet
 // is exited with files closed, etc.

 rez = atexit(IP.exitPtr);
 if (rez) PutMsg("\a\n<*> Could not register EXIT_XB with atexit().  Continuing...\n");

 SetErrorMode(SEM_FAILCRITICALERRORS);

#endif

 while (1) {

    PutMsg(gCLS);
    sprintf(putStr,"%sBullet %u.%3.3u Demo.%s  NLS IX3/DBF/DBT/external %s\n\n",aGRN,
                     IP.versionBullet/1000 ,IP.versionBullet % 1000,aWHT,aPRGDATE);
    PutMsg(putStr);
    PutMsg("Take your pick (each has selectable run options):\n\n");
    PutMsg(" 0. Exit demo\n");
    sprintf(putStr," 1. %s\n",op1);
    PutMsg(putStr);
    sprintf(putStr," 2. %s\n",op2);
    PutMsg(putStr);
    sprintf(putStr," 3. %s\n",op3);
    PutMsg(putStr);
    sprintf(putStr," 4. %s\n",op4);
    PutMsg(putStr);
    sprintf(putStr," 5. %s\n",op5);
    PutMsg(putStr);
    sprintf(putStr," 6. %s\n",op6);
    PutMsg(putStr);
    sprintf(putStr," 7. %s\n",op7);
    PutMsg(putStr);
    sprintf(putStr," 8. %s\n",op8);
    PutMsg(putStr);
    sprintf(putStr," 9. %s\n\n",op9);
    PutMsg(putStr);
    PutMsg("Selection: ");

    GetMsg(tmpStr);
    switch(*tmpStr) {
    case '1':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op1,aWHT);
       PutMsg(putStr);
       rez = bd_rix();
       break;
    case '2':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op2,aWHT);
       PutMsg(putStr);
       rez = bd_ko();
       break;
    case '3':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op3,aWHT);
       PutMsg(putStr);
       rez = bd_ins();
       break;
    case '4':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op4,aWHT);
       PutMsg(putStr);
       rez = bd_upd();
       break;
    case '5':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op5,aWHT);
       PutMsg(putStr);
       rez = bd_join();
       break;
    case '6':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op6,aWHT);
       PutMsg(putStr);
       rez = bd_memo();
       break;
    case '7':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op7,aWHT);
       PutMsg(putStr);
       rez = bd_files();
       break;
    case '8':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op8,aWHT);
       PutMsg(putStr);
       rez = bd_locks();
       break;
    case '9':
       PutMsg(gCLS);
       sprintf(putStr,"%s%s%s\n\n",aGRN,op9,aWHT);
       PutMsg(putStr);
       rez = bd_rixu();
       break;
    case '0':
       break;
    }

    if ((*tmpStr=='0') | (rez)) break;

    PutMsg("\nPress ENTER for select menu...");
    GetMsg(tmpStr);

 }
 EP.func = EXIT_XB;
 rez = BULLET(&EP);

ExitNow:

#if FOR_WINDOWS == 1
 gDie = TRUE;
#endif

 return rez;
}


//-----------------
// Write out string

void PutMsg(CHAR *strg) {

#if FOR_WINDOWS != 1
 printf(strg);
 //fflush(stdout); // already have setbuf(stdout,NULL);
 return;

#else
 static int skipIncFlag=0;
 int tmpLen;

 if (skipIncFlag==0) gCurrStr++;
 if (gCurrStr > 39) {
    gCurrStr=1;
    PutMsg(gCLS);
 }
 strncpy(&gPutStr[gCurrStr][0],strg,82);

 InvalidateRect(gHwnd,NULL,FALSE);
 DoWinThing(0);   // do it, no waiting around

 tmpLen = strlen(&gPutStr[gCurrStr][0]);  // reuse current string line if \r
 if (tmpLen) {
    if (gPutStr[gCurrStr][tmpLen-1] == '\r') {
       skipIncFlag=1;
    }
    else {
       skipIncFlag=0;
    }
 }
 return;

#endif
}


//---------------
// Read in string

void GetMsg(CHAR *strg) {

#if FOR_WINDOWS != 1
 gets(strg);
 return;

#else

 gCurrChar = 0;
 ShowCaret(gHwnd);

 // wait for input

 while (gInputReady==0) {
    DoWinThing(1);   // 1=waits for message
 }

 HideCaret(gHwnd);
 gInputReady = 0;
 strcpy(strg,gGetStr);
 PutMsg("\n");
 return;

#endif
}
