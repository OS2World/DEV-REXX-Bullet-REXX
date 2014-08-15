
/*
 *
 * Bullet 2 OS support routines
 * ccdosfn.c - Copyright (C)1996 Cornel Huth
 * 12-Oct-1996 -chh
 *
 */

#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\locking.h>
#include <share.h>
#include <direct.h>
#include <errno.h>

#include <i86.h>
#include <dos.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>

#define LIBENTRY __cdecl

#ifndef VOID
 #define VOID void
 #define LONG long
 typedef unsigned long ULONG;
 typedef unsigned char *PSZ;
 typedef const unsigned char *cPSZ;
 typedef VOID *PVOID;
#endif

// * For Bullet/X (DOSX32):
// Of the following routines, these four MUST be provided to BULLETX.LIB:
//
// 1. LONG LIBENTRY BulletFree(VOID *baseAddr);
// 2. LONG LIBENTRY BulletMalloc(ULONG bytes, VOID **baseAddrPtr);
// 3. LONG LIBENTRY BulletGetMemoryAvail(void); // __WATCOM__
// 4. LONG LIBENTRY BulletGetTmpDir(char *bufferPtr);
//
// Function pointers to these routines are to be set using SET_VECTORS_XB
// (QSP.item = VECTOR_MALLOC; QSP.itemValue = (ULONG)&BulletMalloc();...)
// If these are not set, when they are called the internal dispatcher returns
// error = 1, function not supported.
//
// * For Bullet/2 and Bullet/95:
// These may be used, or recoded, but they will never be as efficient as
// the internal calls.  See SET_VECTORS_XB for more.
//
// Define FULL_COMPILER_SUPPORT to compile all routines in this module, or
// leave undefined to compile just the four required routines (for DOSX32).
//
// ------------------------------------------------------------------------
//
// Routine protos listed below that end in "// __WATCOM__" include
// compiler-specific code (usually intdos(), or a non-ANSI RTL call)
// and need to be modified for use on other compilers.  In all cases
// where __WATCOM__ is not defined, a suitable default is used, though
// only suitable enough to compile and run.

// Watcom compiler flags are non-critical.  Since the code generated here
// is not called often, you may compile with no optimizations without any
// performance loss.  However, the calling convention must be C, with args
// passed on the stack.  Since these are always called by pointer, the name
// doesn't matter (i.e., leading _, or lack of, doesn't matter).  The
// code must be compiled by a 32-bit flat-mode compiler (such as Watcom's
// wcc386 compiler).  A suitable DOS-extender is also required.  Watcom's
// package includes Tenberry's extender, but most any can be used, such as
// Causeway and PMODE.  Borland's PowerPack may work, but it has not been
// tested (Borland's extender is more of a Windows-based extender, rather
// than a DOS-based extender; it is likely that intdos() calls are NOT
// available using Borland's extender support, but with this source, you
// probably will be able to do what is needed to get Bullet/X going just
// fine.)
//
// Any structure packing must be on 1-byte aligns (i.e., no padding).
//
// Actual compile line used in testing (include set to watcom\h):
// wcc386 ccdosfn.c -w4 -e25 -za -s -za -od -of -zl -zld -mf -4s -bt=DOS

// This code may be compiled to .obj form and added to the BULLETX.LIB file
// using LIB/WLIB or simply added at LINK time to the final EXE, or it may
// be left in source form and compiled along with the rest of your project.
// Consult your compiler docs on how to build an EXE using an external LIB
// file if you need more details.


  // -------------------------------- //
  // DOSX32 REQUIRED SUPPORT ROUTINES //
  // -------------------------------- //


#ifdef __cplusplus
 extern "C" {
#endif

LONG LIBENTRY BulletFree(VOID *baseAddr);
LONG LIBENTRY BulletMalloc(ULONG bytes, VOID **baseAddrPtr);
LONG LIBENTRY BulletGetMemoryAvail(void); // __WATCOM__
LONG LIBENTRY BulletGetTmpDir(char *bufferPtr);

#ifdef __cplusplus
}
#endif

//:FREE MEMORY
// Must be flat (ds=es=ss)

LONG LIBENTRY BulletFree(VOID *baseAddr) {

 free(baseAddr);
 return 0;

}

//:ALLOCATE MEMORY
// typical (0-filled required so use calloc)

LONG LIBENTRY BulletMalloc(ULONG bytes, VOID **baseAddrPtr) {

 *baseAddrPtr = calloc(bytes,1); // checked for NULL internally
 return 0;

}

//:GET DOS MEMORY (available)
// Watcom-specific compile returns _memmax(), else 128K
// This is a required routine since internally it WOULD depend on
// the BulletMalloc() (which used to be INT21/48/and for this call,
// ebx was set to =1). This is not a critical-need routine in any case.

LONG LIBENTRY BulletGetMemoryAvail(void) {

 size_t mem;

#ifdef __WATCOMC__

 mem = _memmax();

#else

 mem=128*1024;

#endif

 return (LONG)mem;
}


//:GET TMP DIR
// typical

LONG LIBENTRY BulletGetTmpDir(char *bufferPtr) {

 char *ePtr;
 int eLen;

 ePtr=getenv("TMP");
 if (ePtr != NULL) {

    eLen = strlen(ePtr);
    if ((eLen != 0) && (eLen < 81-8)) {
       strcpy(bufferPtr,ePtr);
    }
    else {
       ePtr=NULL;
    }
 }
 return (LONG)ePtr;   //return NULL (0) if TMP was not found
}


  //---------------------------------------------------------------//
  //                                                               //
  // The following routines are not required.  They may be used in //
  // whole or in part.  Make them active by using SET_VECTORS_XB.  //
  //                                                               //
  //---------------------------------------------------------------//


#ifdef FULL_COMPILER_SUPPORT

#ifdef __cplusplus
   extern "C" {
#endif

//----------------------------------------------------------------------------
//[FILE ROUTINES] ============================================================

LONG LIBENTRY BulletCloseFile(ULONG handle);
LONG LIBENTRY BulletCreateDir(cPSZ pathNamePtr);
LONG LIBENTRY BulletCreateFile(cPSZ pathNamePtr);
LONG LIBENTRY BulletDeleteFile(cPSZ pathNamePtr);
LONG LIBENTRY BulletMoveFile(cPSZ orgPathNamePtr, cPSZ newPathNamePtr);
LONG LIBENTRY BulletOpenFile(cPSZ pathNamePtr, LONG openMode, LONG *handlePtr);
LONG LIBENTRY BulletReadFile(LONG handle, LONG *bytesPtr, VOID *bufferPtr);
LONG LIBENTRY BulletSeekFile(LONG handle,LONG seekMode,LONG *offsetPtr);
LONG LIBENTRY BulletUpdateDirEntry(LONG handle);
LONG LIBENTRY BulletWriteFile(LONG handle, LONG *bytesPtr, VOID *bufferPtr);

//----------------------------------------------------------------------------
//[NETWORK OS FUNCTIONS] =====================================================

LONG LIBENTRY BulletLockFile(LONG handle, LONG lockMode, LONG lockOffset, ULONG lockBytes, ULONG timeout);
LONG LIBENTRY BulletIsDriveRemote(ULONG drive, ULONG *isRemotePtr, ULONG *isSharePtr); // __WATCOM__
LONG LIBENTRY BulletIsFileRemote(ULONG handle, ULONG *isRemotePtr, ULONG *isSharePtr); // __WATCOM__

//----------------------------------------------------------------------------
//[GENERAL OS FUNCTIONS] =====================================================

LONG LIBENTRY BulletGetSortTable (ULONG codePage, ULONG countryCode, VOID *bufferPtr, ULONG flags);
LONG LIBENTRY BulletGetCountryInfo(ULONG *codePagePtr, ULONG *countryCodePtr, ULONG flags); // __WATCOM__
LONG LIBENTRY BulletGetExtendedError(LONG ecode, ULONG *classPtr, ULONG *actionPtr, ULONG *locusPtr);
LONG LIBENTRY BulletGetVersionDOS(ULONG *verPtr, ULONG *rezPtr, ULONG *maxPathPtr, ULONG *maxCompPtr); // __WATCOM__
LONG LIBENTRY BulletSetHandleCount(ULONG *handlesPtr); // __WATCOM__

//----------------------------------------------------------------------------
//[MISC OS FUNCTIONS] ========================================================

LONG LIBENTRY BulletGetTimeInfo(ULONG *timePtr, ULONG *datePtr, ULONG *dayPtr, LONG *tzPtr);
LONG LIBENTRY BulletUpperCase(char *strPtr, ULONG strLen);

//----------------------------------------------------------------------------
//[SEMAPHORE OS FUNCTIONS]====================================================

LONG LIBENTRY BulletCloseMutexSem(ULONG handle);
LONG LIBENTRY BulletCreateMutexSem(ULONG rez1, ULONG *handlePtr, ULONG rez2, ULONG rez3);
LONG LIBENTRY BulletRequestMutexSem(ULONG handle, ULONG timeout);
LONG LIBENTRY BulletReleaseMutexSem(ULONG handle);

#ifdef __cplusplus
}
#endif


//-------------------------------
// Convert CLIB errno to OS error
// finer conversion is possible to non-90xx
// local use

int LocalGetErrorCC(void) {

 #define bltEZERO     0    /* 0  No error */
 #define bltENOENT    2    /* 1  No such file or directory */
 #define bltE2BIG     9002 /* 2  Arg list too big */
 #define bltENOEXEC   9003 /* 3  Exec format error */
 #define bltEBADF     6    /* 4  Bad file number */
 #define bltENOMEM    8    /* 5  Not enough memory */
 #define bltEACCES    5    /* 6  Permission denied */
 #define bltEEXIST    80   /* 7  File exists */
 #define bltEXDEV     9008 /* 8  Cross-device link */
 #define bltEINVAL    9009 /* 9  Invalid argument */
 #define bltENFILE    9010 /* 10 File table overflow */
 #define bltEMFILE    4    /* 11 Too many open files */
 #define bltENOSPC    39   /* 12 No space left on device */
                           /*    File locking error */
 #define bltEDEADLK   5    /* 15 Resource deadlock would occur */
 #define bltEDEADLOCK 5    /* 15 ... */
 #define bltEINTR     9016 /* 16 interrupt */
 #define bltECHILD    9017 /* 17 Child does not exist */
                           /*    POSIX errors */
 #define bltEAGAIN    9018 /* 18 Resource unavailable, try again */
 #define bltEBUSY     142  /* 19 Device or resource busy */
 #define bltEFBIG     9020 /* 20 File too large */
 #define bltEIO       31   /* 21 I/O error */
 #define bltEISDIR    9022 /* 22 Is a directory */
 #define bltENOTDIR   9023 /* 23 Not a directory */
 #define bltEMLINK    9024 /* 24 Too many links */
 #define bltENOTBLK   9025 /* 25 Block device required */
 #define bltENOTTY    9026 /* 26 Not a character device */
 #define bltENXIO     9027 /* 27 No such device or address */
 #define bltEPERM     9028 /* 28 Not owner */
 #define bltEPIPE     9029 /* 29 Broken pipe */
 #define bltEROFS     5    /* 30 Read-only file system */
 #define bltESPIPE    9031 /* 31 Illegal seek */
 #define bltESRCH     9032 /* 32 No such process */
 #define bltETXTBSY   9033 /* 33 Text file busy */
 #define bltEFAULT    9034 /* 34 Bad address */
 #define bltENAMETOOLONG  9035 /* 35 Name too long */
 #define bltENODEV    9036 /* 36 No such device */
 #define bltENOLCK    9037 /* 37 No locks available in system */
 #define bltENOSYS    1    /* 38 Unknown system call */
 #define bltENOTEMPTY 9039 /* 39 Directory not empty */

 int rez;

 rez=errno;
 switch (rez) {
  case EZERO        : rez=bltEZERO       ;break;
  case ENOENT       : rez=bltENOENT      ;break;
  case E2BIG        : rez=bltE2BIG       ;break;
  case ENOEXEC      : rez=bltENOEXEC     ;break;
  case EBADF        : rez=bltEBADF       ;break;
  case ENOMEM       : rez=bltENOMEM      ;break;
  case EACCES       : rez=bltEACCES      ;break;
  case EEXIST       : rez=bltEEXIST      ;break;
  case EXDEV        : rez=bltEXDEV       ;break;
  case EINVAL       : rez=bltEINVAL      ;break;
  case ENFILE       : rez=bltENFILE      ;break;
  case EMFILE       : rez=bltEMFILE      ;break;
  case ENOSPC       : rez=bltENOSPC      ;break;
  //case EDEADLK      : rez=bltEDEADLK     ;break; // same as EDEADLOCK
  case EDEADLOCK    : rez=bltEDEADLOCK   ;break;
  case EINTR        : rez=bltEINTR       ;break;
  case ECHILD       : rez=bltECHILD      ;break;

  case EAGAIN       : rez=bltEAGAIN      ;break;
  case EBUSY        : rez=bltEBUSY       ;break;
  case EFBIG        : rez=bltEFBIG       ;break;
  case EIO          : rez=bltEIO         ;break;
  case EISDIR       : rez=bltEISDIR      ;break;
  case ENOTDIR      : rez=bltENOTDIR     ;break;
  case EMLINK       : rez=bltEMLINK      ;break;
  case ENOTBLK      : rez=bltENOTBLK     ;break;
  case ENOTTY       : rez=bltENOTTY      ;break;
  case ENXIO        : rez=bltENXIO       ;break;
  case EPERM        : rez=bltEPERM       ;break;
  case EPIPE        : rez=bltEPIPE       ;break;
  case EROFS        : rez=bltEROFS       ;break;
  case ESPIPE       : rez=bltESPIPE      ;break;
  case ESRCH        : rez=bltESRCH       ;break;
  case ETXTBSY      : rez=bltETXTBSY     ;break;
  case EFAULT       : rez=bltEFAULT      ;break;
  case ENAMETOOLONG : rez=bltENAMETOOLONG;break;
  case ENODEV       : rez=bltENODEV      ;break;
  case ENOLCK       : rez=bltENOLCK      ;break;
  case ENOSYS       : rez=bltENOSYS      ;break;
  case ENOTEMPTY    : rez=bltENOTEMPTY   ;break;
  default: rez=9099;
 }
 return rez;
}




//----------------------------------------------------------------------------
//[FILE ROUTINES] ============================================================

//:CLOSE FILE
// typical

LONG LIBENTRY BulletCloseFile(ULONG handle) {

 int rez;

 rez = close((int)handle);
 if (rez== -1) rez=LocalGetErrorCC();
 return (LONG)rez;
}


//:CREATE DIRECTORY
// typical

LONG LIBENTRY BulletCreateDir(cPSZ pathNamePtr) {

 int rez;

 rez = mkdir(pathNamePtr);
 if (rez== -1) rez=LocalGetErrorCC();
 return (LONG)rez;
}


//:CREATE FILE
// typical

LONG LIBENTRY BulletCreateFile(cPSZ pathNamePtr) {

 int rez;
 int handle;

 handle = sopen(pathNamePtr,
                O_CREAT | O_EXCL | O_RDWR | O_BINARY,
                SH_DENYRW,
                S_IREAD | S_IWRITE);
 if (handle== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    close(handle);   // only create it, so close after a good create
    rez=0;
 }
 return (LONG)rez;
}


//:DELETE FILE
// typical

LONG LIBENTRY BulletDeleteFile(cPSZ pathNamePtr) {

 int rez;

 rez = remove(pathNamePtr);
 if (rez== -1) rez=LocalGetErrorCC();
 return (LONG)rez;
}


//:RENAME (Move) FILE
// typical

LONG LIBENTRY BulletMoveFile(cPSZ orgPathNamePtr, cPSZ newPathNamePtr) {

 int rez;

 rez = rename(orgPathNamePtr, newPathNamePtr);
 if (rez== -1) rez=LocalGetErrorCC();
 return (LONG)rez;
}


//:OPEN FILE
// if handle <=2 is returned to Bullet, Bullet will leave the handle
// alone and issue ERR_SYSTEM_HANDLE (8305) returned for rez.  Your
// app level code (OPEN_DATA_XB, etc.) should reissue the open request.

LONG LIBENTRY BulletOpenFile(cPSZ pathNamePtr, LONG openMode, LONG *handlePtr) {

 int rez;
 int handle;
 int oflag, shflag;

 *(handlePtr)=0;

 switch (openMode & 0x03) {
  case 0: oflag = O_RDONLY | O_BINARY; break;
  case 1: oflag = O_WRONLY | O_BINARY; break;
  default:oflag = O_RDWR   | O_BINARY; break;
 }

 switch (openMode & 0x70) {             // no-inherit is not supported by sopen()
  case 0x10: shflag = SH_DENYRW; break; // deny read-write
  case 0x20: shflag = SH_DENYWR; break; // deny write
  case 0x30: shflag = SH_DENYRD; break; // deny read
  case 0x40: shflag = SH_DENYNO; break; // deny none
  default: shflag = SH_DENYNO;   break; // SH_COMPAT is not a good idea so deny none when case==0
 }

 handle = sopen(pathNamePtr, oflag, shflag);
 if (handle== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    *(handlePtr)=(LONG)handle;
    rez=0;
 }
 return (LONG)rez;
}


//:READ FILE
// extender typically handles requests for >64K bytes  --if not, shrink
// Bullet pack/reindex buffers so that no single read request is larger than
// 65520 bytes (or 32767 for some C RTLs)
//
// if bytesRead is not the same as bytes-requested this routine does
// NOT return an error (simply return the bytes actually read)

LONG LIBENTRY BulletReadFile(LONG handle, LONG *bytesPtr, VOID *bufferPtr) {

 int rez;
 int bytesRead;

 bytesRead = read((int)handle, bufferPtr, (unsigned int) *(bytesPtr));
 if (bytesRead== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    *(bytesPtr)=(LONG)bytesRead;
    rez=0;
 }
 return (LONG)rez;
}


//:SEEK TO FILE POSITION
// typical

LONG LIBENTRY BulletSeekFile(LONG handle,LONG seekMode,LONG *offsetPtr) {

 int rez;
 long currPos;

 currPos = lseek((int)handle, *(offsetPtr), (int)seekMode);
 if (currPos== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    *(offsetPtr)=currPos;
    rez=0;
 }
 return (LONG)rez;
}


//:UPDATE DIR ENTRY
// typical

LONG LIBENTRY BulletUpdateDirEntry(LONG handle) {

 int rez;
 int handleCommit;

 handleCommit=dup(handle);
 if (handleCommit== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    close(handleCommit);
    rez=0;
 }
 return (LONG)rez;
}


//:WRITE FILE
// extender typically handles requests for >64K bytes  --if not, shrink
// Bullet pack/reindex buffers so that no single read request is larger than
// 65520 bytes (or 32767 for some C RTLs)
//
// if bytesWritten is not the same as bytes-requested this routine does
// NOT return an error (simply return the bytes actually written)
// (a ERR_DISK_FULL is generated internally by Bullet if this is the case)
//
// If *bytesPtr=0 (write 0 bytes) file is to be truncated at its current
// position.  In DOS, this is the case (i.e., the OS does this when bytes-to-write
// is 0).  chsize() may be an alternate option when 0.

LONG LIBENTRY BulletWriteFile(LONG handle, LONG *bytesPtr, VOID *bufferPtr) {

 int rez;
 int bytesWritten;

 bytesWritten = write((int)handle, bufferPtr, (unsigned int) *(bytesPtr));
 if (bytesWritten== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    *(bytesPtr)=(LONG)bytesWritten;
    rez=0;
 }
 return (LONG)rez;
}




//----------------------------------------------------------------------------
//[NETWORK OS FUNCTIONS] =====================================================

//:LOCK/UNLOCK FILE
// lockMode: bit0=0 lock
//           bit0=1 unlock
//    n/a    bit1=0 exclusive access to locked region by process (R/W)
//    n/a    bit1=1 read access to locked region, but not write for any
//    n/a    bit2=1 atomic lock operation (unlock then lock, for relock only)
//    n/a = not available in DOSX32

LONG LIBENTRY BulletLockFile(LONG handle, LONG lockMode, LONG lockOffset, ULONG lockBytes, ULONG timeout) {

 int rez, rez2;
 int lockType;

 LONG currPos=0;
 LONG newPos;

 if ((lockMode & 1)==1) {
    lockType=LK_UNLCK;     // unlock if bit0=1
 }
 else {
    lockType=LK_NBLCK;     // lock if bit0=1 (non-blocking, or use timeout as basis to block)
 }

 // locking() used over lock() since more likely supported by the RTL

 rez = BulletSeekFile(handle,1,&currPos);
 if (rez==0) {

    newPos = lockOffset;
    rez = BulletSeekFile(handle,0,&newPos);
    if (rez==0) {
       rez = locking(handle,lockType,lockBytes);
       if (rez==-1) rez=LocalGetErrorCC();
    }

    //even though Bullet file operations are atomic, the position is restored
    rez2 = BulletSeekFile(handle,0,&currPos); // restore position

    if (rez==0) rez=rez2;
 }
 return (LONG)rez;

 timeout;  // ref it-unreachable-use it if you can
}


//:QUERY DRIVE REMOTE
// used for informational use only (non-critical)
// Requires making INT call if not otherwise supported by a compiler call.
// The source below compiles with a Watcom compiler (10a, for example), if not:
//
// This code sets remote and share flags to 1, indicating that the file
// is remote and that SHARE.EXE (or compatible) is installed.  The purpose
// is information, and is used to determine whether locking is required.
// Since it's better to lock and not need to than to not lock and need to,
// this code returns 1 in both cases.  If SHARE is not installed, a run-time
// error is returned by BulletLockFile(), above (try it).  This may also
// happen if run in a Windows DOS box since Windows always reports that
// SHARE is installed (it should be if it isn't).

LONG LIBENTRY BulletIsDriveRemote(ULONG drive, ULONG *isRemotePtr, ULONG *isSharePtr) {

 int rez;

#ifdef __WATCOMC__

 union REGS iregs, oregs;

 *(isRemotePtr)=1;
 *(isSharePtr)=1;

 iregs.x.eax = 0x4409;
 iregs.x.ebx = drive;   // 0=current/default, 1=A:, 2=B:, 3=C:...
 iregs.x.edx = 0;
 rez = intdos(&iregs, &oregs);
 if ((oregs.x.cflag & 1)==0) {
    if ((oregs.x.edx & (1 << 12))==1) *(isRemotePtr)=1;  // bit12=1 then remote
    rez = 0;
 }

 oregs.x.eax = -1;      // init to non-zero for 0-compare after call
 iregs.x.eax = 0x1000;  // ignore error since not likely, and it won't matter
 int386(0x2F, &iregs, &oregs);
 if ((oregs.h.al)==0) {  // installed only if al=FF
    *(isSharePtr)=0;
 }

#else

 *(isRemotePtr)=1;
 *(isSharePtr)=1;
 rez=0;

#endif

 return (LONG)rez;
}


//:QUERY FILE REMOTE
// see notes for QUERY DRIVE REMOTE

LONG LIBENTRY BulletIsFileRemote(ULONG handle, ULONG *isRemotePtr, ULONG *isSharePtr) {

 int rez;

#ifdef __WATCOMC__

 union REGS iregs, oregs;

 *(isRemotePtr)=1;
 *(isSharePtr)=1;

 iregs.x.eax = 0x440A;
 iregs.x.ebx = handle;
 iregs.x.edx = 0;
 rez = intdos(&iregs, &oregs);
 if ((oregs.x.cflag & 1)==0) {
    if ((oregs.x.edx & (1 << 15))==1) *(isRemotePtr)=1;  // bit15=1 then remote
    rez = 0;
 }

 oregs.x.eax = -1;      // init to non-zero for 0-compare after call
 iregs.x.eax = 0x1000;  // ignore error since not likely, and it won't matter
 int386(0x2F, &iregs, &oregs);
 if ((oregs.h.al)==0) {  // installed only if al=FF
    *(isSharePtr)=0;
 }

#else

 *(isRemotePtr)=1;
 *(isSharePtr)=1;
 rez=0;

#endif

 return (LONG)rez;
}




//----------------------------------------------------------------------------
//[GENERAL OS FUNCTIONS] =====================================================



//:GET COLLATE SEQUENCE
//default is for US codePage

LONG LIBENTRY BulletGetSortTable (ULONG codePage, ULONG countryCode, VOID *bufferPtr, ULONG flags) {

 // OEM cp=437 table along MS-DOS, OS/2 lines (similar to Windows OEM)

 static char sortTableOEM[] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x43,0x55,0x45,0x41,0x41,0x41,0x41,0x43,0x45,0x45,0x45,0x49,0x49,0x49,0x41,0x41,
0x45,0x41,0x41,0x4F,0x4F,0x4F,0x55,0x55,0x59,0x4F,0x55,0x24,0x24,0x24,0x24,0x24,
0x41,0x49,0x4F,0x55,0x4E,0x4E,0xA6,0xA7,0x3F,0xA9,0xAA,0xAB,0xAC,0x21,0x22,0x22,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0x53,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

 // ANSI cp=1252 table along Windows lines

 static char sortTableANSI[] = {
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x8A,0x9B,0x8C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xF7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0x9F
};

 if (flags & 1) {

    // USE_ANSI_SET flag in CREATE_INDEX_XB

    memmove(bufferPtr,sortTableANSI,256);
 }
 else {
    memmove(bufferPtr,sortTableOEM,256);
 }
 return 0;

 codePage;
 countryCode;
 flags;

#if 0

 // --------------------------------------------------------------------------
 // the original assembly is shown for reference if you want
 // to do inline assembly to get the DOS table -- results
 // may be less than perfect (DOS may return an invalid table)
 // NOTE that this is the  DOSX32  original version
 // and both

;--------------------
;GET COLLATE SEQUENCE
; in: ecx=code page ID
;     edx=country code
;     edi->buffer to store sequence table (256 bytes)
;     esi=OEM (0) or ANSI (1) flag (for Windows use only, selects char set)
;out: eax=0, or error if CF=1
;use:
;nts: uses DPMI call to ax=6 verify selector!
;     if ERR_216506 returned, instruct user to provide own collate table
;DOS4GW does return 6,seg:off, but only a 16-bit offset! (06,ofs16,selector)

PROC
 LOCAL \
 sBuffer[20]:DWORD

 mov     ebx,ecx         ;ebx=cp / edx=cc
 mov     ecx,5           ;get 5 bytes worth
 mov     ax,6506h        ;es:edi->buffer (use dest buffer for a sec)
 int 21h
 jc      ExitNotSupported
 cmp     BPTR [edi],6    ;valid identifier?
 jne     ExitNotSupported ;no!

 movzx   esi,WPTR [edi+1];16-bit offset so movzx
 mov     bx,WPTR [edi+3] ;and selector to bx for testing

 mov     ax,6            ;DPMI Get Segment Base Address
 int 31h                 ;used only to validate selector
 jc      ExitNotSupported

 push    ds              ;(save ds)
 mov     ds,bx           ;ds:esi->table size word
 movzx   ecx,WPTR [esi]  ;get size in bytes
 cmp     ecx,256         ;valid size?
 jne     ExitNS2         ;no
 add     esi,2
 shr     ecx,2           ;move dwords at a time (256/4=64 dwords)
 rep movsd
 pop     ds              ;(back ds)
 sub     eax,eax
ExitGen:
 ret

ExitNotSupported:
 mov     eax,ERR_216506 ;(ERR_216506 = 8256)
 stc
 jmp     ExitGen
ExitNS2:
 pop     ds              ;(back ds)
 jmp     ExitNotSupported
ENDP

  // -------------------------------------------------------------------------

#endif
}


//:GET COUNTRY INFO
// requires WATCOM-specific intdos() support, else returns parms same as args

LONG LIBENTRY BulletGetCountryInfo(ULONG *codePagePtr, ULONG *countryCodePtr, ULONG flags) {

 int rez;
 ULONG cc, cp;

#ifdef __WATCOMC__

 char buffer[44];
 unsigned short *tmpPtr;

 union REGS iregs, oregs;

 iregs.x.eax = 0x6501;
 iregs.x.ecx = 40;
 iregs.x.edi = (ULONG)(&buffer)+3;  // for alignment
 rez = intdos(&iregs, &oregs);
 if ((oregs.x.cflag & 1)==0) {

    tmpPtr = (unsigned short*) &buffer[3];
    cc = *(tmpPtr);
    cp = *(++tmpPtr);
 }
 else {
    rez = 8251;   // Bullet ERR_216501 (extender support lacking)
 }

#else

    // one way to do it (easier to change this way since common exit assigns done)

    cc = *countryCodePtr;
    cp = *codePagePtr;
    rez=0;

#endif

 if (*countryCodePtr==0) *countryCodePtr = cc;
 if (*codePagePtr==0) *codePagePtr = cp;
 return (LONG)rez;

 flags;
}


//:GET EXTENDED ERROR
// for informational use; not supported so returned *Ptr==0

LONG LIBENTRY BulletGetExtendedError(LONG ecode, ULONG *classPtr, ULONG *actionPtr, ULONG *locusPtr) {

 *classPtr=0;
 *actionPtr=0;
 *locusPtr=0;
 return 0;

 ecode;
}


//:GET DOS VERSION
// Watcom-specific compile uses intdos(), else returns version 500 (for DOS 5.0)

LONG LIBENTRY BulletGetVersionDOS(ULONG *verPtr, ULONG *rezPtr, ULONG *maxPathPtr, ULONG *maxCompPtr) {

 int rez;

#ifdef __WATCOMC__

 union REGS iregs, oregs;

 iregs.x.eax = 0x3000;
 rez = intdos(&iregs, &oregs);
 *verPtr = (iregs.h.al * 100) + (iregs.h.ah);

#else

 *verPtr = 500;   // assume DOS 5.0
 rez = 0;

#endif

 strncpy((char *)rezPtr,"2TLB",4);
 *maxPathPtr = 81;
 *maxCompPtr = 8; // preferred over 11
 return (LONG)rez;
}




//:SET HANDLE COUNT
// Watcom-specific compile uses intdos(), else no action (max 15 handles then)
//
// *handlesPtr is to return the max number of handles available
// if *handlesPtr==0 on entry, return max number of handles available
// in DOS, this is not a supported service, so tracked internally
// (i.e., leave value in *handlesPtr as is)
//
// Note: Even though Watcom's _grow_handle() does return the max
// handles that can be open (with restrictions), it returns 256
// when 255 is used as its arg.  DOS nevers uses handle FF since
// this is reserved for the 'unused' slot marker in the file
// tables, so the max possible is 255 (numbered 0-254).  A safe
// bet here is to let Bullet handles this case, and just leave
// *handlesPtr as-is.

LONG LIBENTRY BulletSetHandleCount(ULONG *handlesPtr) {

 int rez;

#ifdef __WATCOMC__

 // The Watcom RTL 'open' support requires that _grow_handles()
 // be used; simply using DOS INT21/67 is not sufficient, even
 // though the open support routines used DOS handles directly
 // (this may be related to the extender)
 //
 //union REGS iregs, oregs;
 //
 //iregs.x.eax = 0x6700;
 //iregs.x.ebx = *handlesPtr;
 //rez = intdos(&iregs, &oregs);
 //if ((oregs.x.cflag & 1)==0) rez=0;

 rez = _grow_handles((int)*handlesPtr);
 if (rez== -1) {
    rez=LocalGetErrorCC();
 }
 else {
    rez=0;
 }

#else

 // you won't be able to open more than 15 or so handles concurrently
 // unless this is coded for your compiler -- also be sure to increase
 // FILES= in config.sys (set to no more than FILES=255)

 rez=0;

#endif

 return (LONG)rez;
}




//----------------------------------------------------------------------------
//[MISC OS FUNCTIONS] ========================================================

//:GET DATE/TIME
// typical

LONG LIBENTRY BulletGetTimeInfo(ULONG *timePtr, ULONG *datePtr, ULONG *dayPtr, LONG *tzPtr) {

 #define daTZ  0xFF00

 struct tm *daTime;
 time_t daTicks;

 static int huns = 99;

 // hundreths are needed so simulate

 huns++;
 if (huns==100) huns=0;

 time(&daTicks);
 daTime = localtime(&daTicks);

 *(timePtr) = (daTime->tm_hour) + (daTime->tm_min << 8) + (daTime->tm_sec << 16) + (huns << 24);
 *(datePtr) = (daTime->tm_year +1900) + ((daTime->tm_mon+1) << 16) + (daTime->tm_mday << 24);
 *(dayPtr)  = (daTime->tm_wday);
 *(tzPtr)   = (LONG) daTZ;

 return 0;
}


//:UPPERCASE STRING
// Used to upper-case fieldnames, key expression, and value in UPPER().
// This likely will not work for char > 127, but this should not pose
// a problem since field names (and hence, key expression and UPPER()
// value) should be standard ASCII only.
// Filenames do NOT have their case changed (that's FILE names).
// String likely is NOT 0-terminated,
// String will NOT contain embedded 0s.

LONG LIBENTRY BulletUpperCase(char *strPtr, ULONG strLen) {

 while (strLen--) {
    toupper(*strPtr++);
 }

 return 0;
}




//----------------------------------------------------------------------------
//[SEMAPHORE OS FUNCTIONS]====================================================

//:CLOSE MUTEX SEMAPHORE
// used at EXIT_XB

LONG LIBENTRY BulletCloseMutexSem(ULONG handle) {

 return 0;

 handle;
}


//:CREATE MUTEX SEMAPHORE
// used at first Bullet call

LONG LIBENTRY BulletCreateMutexSem(ULONG rez1, ULONG *handlePtr, ULONG rez2, ULONG rez3) {

 *handlePtr=1;  // return non-zero to prevent later calls
 return 0;

 rez1;   // passed=0, unnamed semaphore
 rez2;   // passed=0, not shared with other processes
 rez3;   // passed=0, initiallly unowned semaphore
 handlePtr;
}


//:REQUEST MUTEX SEMAPHORE
// used at each Bullet call

LONG LIBENTRY BulletRequestMutexSem(ULONG handle, ULONG timeout) {

 return 0;

 handle;
 timeout;   // passed=as set by sysvar
}


//:RELEASE MUTEX SEMAPHORE
// used at each Bullet call

LONG LIBENTRY BulletReleaseMutexSem(ULONG handle) {

 return 0;

 handle;
}

#endif // #ifdef FULL_COMPILER_SUPPORT

// <EOF>
