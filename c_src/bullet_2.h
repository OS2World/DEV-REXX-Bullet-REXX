
/* BULLET_2.H    10-Oct-96-chh
 *
 *  Bullet header for 32-bit C/C++ (DOSX32, OS/2, and Win32s/Win32)
 *  Bullet call numbers, parameter packs, and error number equates
 *
 *  Requires PLATFORM defined and set to ON_DOSX32 (3), ON_OS2 (4),
 *  or ON_WIN32 (5) before getting here.  For example:
 *    #define PLATFORM ON_DOSX32  (ON_DOSX32 defined as 3)
 *
 */

#ifndef __BULLET_H
#define __BULLET_H

/*
 * The #pragma pack(1)/#pragma pack() is no longer required since all
 * structure members in this header will align properly -- all are
 * 32-bit size except for the structure "FieldDescType", but fieldDA
 * member (a LONG) is at a 32-bit alignment already (at byte offset +12).
 * The altFieldLength member, same structure, is also already at
 * proper alignment for a 16-bit value (at byte offset +18).  If, for
 * some reason, your compiler aligns the members differently, then you
 * must use the appropriate compiler pragma to prevent this -- the
 * FieldDescType size is 32 bytes exactly.  It is not likely that any
 * conforming compiler will alter this structure, but, now you know what
 * to do if it does.
 *
 * #pragma pack(1)
 *
 * NOTE: In your program source code, when you layout your record buffer
 * structure, you must use the #pragma pack(1)/#pragma pack() directives
 * around it since it will be, most likely, modified.  The reason is that
 * this structure MUST start with the implicit TAG field (a BYTE), and so,
 * unless you use only BYTE/CHAR members in your structure (Bullet can use
 * binary field values), or take special care to align the record layout
 * so no padding is performed by the compiler, then you will need to use
 * the pack(1) pragma.
 *
 * #pragma pack()
 */

/* Re: Bullet/X for DOSX32:
 * (refer to ccdosfn.c for more)
 * Bullet ccdosfn.c provides no OS call support to determine the
 * system country code and code page ID.  This can be coded in
 * ccdosfn.c, but it is quite compiler- and extender-dependent.
 * The current state is to supply a collate sequence table for
 * country code=1 and code page=437 (the table is statically coded).
 * For other sort tables, modify as required.  If support for this
 * at the system level is made available at run-time, you may
 * want to change the following to 0, for both CTRYCODE and CODEPAGE.
 * Until this is so coded, you cannot use 0 here (as with the Win32
 * and OS/2 versions), else error EXB_216501 (8251) is the result.
 * This table is added to each index file (if NLS or a user sort).
 */

#ifndef ON_DOSX32
 ON_DOSX32 3
 ON_OS2    4
 ON_WIN32  5
#endif

#if PLATFORM == ON_DOSX32
 #define CTRYCODE 1     /* 0 signifies default country code (at index create) */
 #define CODEPAGE 437   /* 0 signifies default code page (at index create) */
                        /* but DOS extender may not support OS call to get info */
                        /* see ccdosfn.c for making changes for DOSX32 platform */
 #define RELOCK_AVAIL 0 /* relock not supported */

 #define VOID void      /* these are already defined if Win32 or OS/2, but not DOS */
 #define SHORT short
 #define LONG long
 #define CHAR char

 typedef unsigned char BYTE;
 typedef unsigned short USHORT;
 typedef unsigned long ULONG;
 typedef unsigned char *PSZ;
 typedef VOID *PVOID;

 #define APIENTRY __cdecl

#elif PLATFORM == ON_OS2
 #define CTRYCODE 0
 #define CODEPAGE 0
 #define RELOCK_AVAIL 1    /* relock is supported */

 /* above types are assumed defined in os2def.h */

#elif PLATFORM == ON_WIN32
 #define CTRYCODE 0
 #define CODEPAGE 0     /* may be ANSI or OEM code page value, depending on SORT_SET flag */
 #define RELOCK_AVAIL 0 /* relock not supported (is only on NT) */
 
 /* above types are assume defined in windef.h and winnt.h */

#else
 #error No PLATFORM specified
 #error ---------------------

#endif

#ifndef __BLT_DYNA    // define this if using run-time loading of BULLET*.DLL
                      // via LoadLibrary(Win32) or DosLoadModule(OS/2)
 #ifdef __cplusplus
  extern "C" LONG APIENTRY BULLET(PVOID datapack);
 #else
  extern LONG APIENTRY BULLET(PVOID datapack);
 #endif

#else

  LONG (* APIENTRY BULLET)(PVOID datapack);

#endif



/* The following on mutex-semaphore protection does not apply to Bullet/X */
/* unless the semaphore routines (in ccdosfn.c) are coded to do something */

/* All Bullet routines are mutex-semaphore protected except the following:
 *
 * MEMORY_XB            STAT_HANDLE_XB          GET_ERROR_CLASS_XB
 * QUERY_SYSVARS_XB     QUERY_VECTORS_XB        CHECK_REMOTE_XB
 * STAT_DATA_XB         STAT_INDEX_XB
 *
 * This means that any thread can call the above routines at any time.  All
 * other calls in the current process block until the previous thread exits 
 * BULLET.  The default mutex wait is 0 milliseconds, and can be set via 
 * SET_SYSVARS_XB using the MUTEX_SEM_TIMEOUT index.  In the case of
 * STAT_DATA_XB and STAT_INDEX_XB, these should be used only when there
 * is no chance that another thread may close that file handle while the
 * routine is working.
 *
 */



/* ************************************************************************
 *
 * xxx.func call numbers
 *
 * ************************************************************************/

#define GEN_ERR_XB              0
#define INIT_XB                 1  /* system */
#define EXIT_XB                 2
#define MEMORY_XB               4
#define BACKUP_FILE_XB          6
#define STAT_HANDLE_XB          7
#define GET_ERROR_CLASS_XB      8

#define QUERY_SYSVARS_XB        10 /* advanced system */
#define SET_SYSVARS_XB          11
#define SET_DVMON_XB            12 /* reserved */.
#define QUERY_VECTORS_XB        13
#define SET_VECTORS_XB          14

#define CREATE_DATA_XB          20 /* data control mid-level */
#define OPEN_DATA_XB            21
#define CLOSE_DATA_XB           22
#define STAT_DATA_XB            23
#define READ_DATA_HEADER_XB     24
#define FLUSH_DATA_HEADER_XB    25
#define COPY_DATA_HEADER_XB     26
#define ZAP_DATA_HEADER_XB      27

#define CREATE_INDEX_XB         30 /* key control mid-level */
#define OPEN_INDEX_XB           31
#define CLOSE_INDEX_XB          32
#define STAT_INDEX_XB           33
#define READ_INDEX_HEADER_XB    34
#define FLUSH_INDEX_HEADER_XB   35
#define COPY_INDEX_HEADER_XB    36
#define ZAP_INDEX_HEADER_XB     37

#define GET_DESCRIPTOR_XB       40 /* data access mid-level */
#define GET_RECORD_XB           41
#define ADD_RECORD_XB           42
#define UPDATE_RECORD_XB        43
#define DELETE_RECORD_XB        44
#define UNDELETE_RECORD_XB      45
#define DEBUMP_RECORD_XB        46
#define PACK_RECORDS_XB         47

#define GET_MEMO_SIZE_XB        50 /* memo access mid-level */
#define GET_MEMO_XB             51
#define ADD_MEMO_XB             52
#define UPDATE_MEMO_XB          53
#define DELETE_MEMO_XB          54
#define MEMO_BYPASS_XB          59 /* see below for bypass ordinals */

#define BYPASS_CREATE_MEMO       1 /* The bypass routines are automatically */
#define BYPASS_OPEN_MEMO         2 /* performed by BULLET but can be done */
#define BYPASS_CLOSE_MEMO        3 /* manually, if needed - these numbers are */
#define BYPASS_READ_MEMO_HEADER  4 /* put in MDP.memoBypass, with MDP.func */
#define BYPASS_FLUSH_MEMO_HEADER 5 /* set to MEMO_BYPASS_XB */

#define FIRST_KEY_XB            60 /* key access mid-level */
#define EQUAL_KEY_XB            61
#define NEXT_KEY_XB             62
#define PREV_KEY_XB             63
#define LAST_KEY_XB             64
#define STORE_KEY_XB            65
#define DELETE_KEY_XB           66
#define BUILD_KEY_XB            67
#define GET_CURRENT_KEY_XB      68
#define GET_KEY_FOR_RECORD_XB   69

#define GET_FIRST_XB            70 /* key and data access high-level */
#define GET_EQUAL_XB            71
#define GET_NEXT_XB             72
#define GET_PREV_XB             73
#define GET_LAST_XB             74
#define INSERT_XB               75
#define UPDATE_XB               76
#define REINDEX_XB              77

#define LOCK_XB                 80 /* network control */
#define UNLOCK_XB               81
#define LOCK_INDEX_XB           82
#define UNLOCK_INDEX_XB         83
#define LOCK_DATA_XB            84
#define UNLOCK_DATA_XB          85
#define CHECK_REMOTE_XB         86
#define RELOCK_XB               87
#define RELOCK_INDEX_XB         88
#define RELOCK_DATA_XB          89

#define DELETE_FILE_DOS         90 /* DOS file I/O low-level */
#define RENAME_FILE_DOS         91
#define CREATE_FILE_DOS         92
#define OPEN_FILE_DOS           93
#define SEEK_FILE_DOS           94
#define READ_FILE_DOS           95
#define WRITE_FILE_DOS          96
#define CLOSE_FILE_DOS          97
#define ACCESS_FILE_DOS         98
#define EXPAND_FILE_DOS         99
#define MAKE_DIR_DOS            100
#define COMMIT_FILE_DOS         101

/* ************************************************************************
 *
 * operating system file I/O equates
 *
 * ************************************************************************/

#define READONLY        0x00000000 /* std file access mode */
#define WRITEONLY       0x00000001 /* no underscore used for std equates */
#define READWRITE       0x00000002

#define DENYREADWRITE   0x00000010 /* std file share mode, cannot be 0 */
#define DENYWRITE       0x00000020
#define DENYREAD        0x00000030
#define DENYNONE        0x00000040
#define NOINHERIT       0x00000080

#define NO_LOCALITY     0x00000000 /* optional cache modes */
#define SEQ_LOCALITY    0x00010000
#define RND_LOCALITY    0x00020000
#define MIX_LOCALITY    0x00030000
#define SKIP_CACHE      0x00100000 /* not inherited by child process */
#define WRITE_THROUGH   0x00400000 /* not inherited by child process */


#define LOCK_SHARED      1         /* for LP.xlMode and LP.dlMode */
#define LOCK_EXCLUSIVE   0         /* N/A for DOSX32 or Win95/Win32s */

/* ************************************************************************
 *
 * .sortFunction IDs, Query/SetSysVars|Vectors item IDs
 *
 * ************************************************************************/

// SORT_SET flag:  USE_*_SET defaults to OEM character set, and is used (either
// OEM or ANSI) whenever the .sortFunction used is NLS or a custom sort-compare.

#define USE_OEM_CHARSET  (0 << 17) /* for DOSX32, OS/2, and Windows */
#define USE_ANSI_CHARSET (1 << 17) /* for Windows (.sortFunction flag) */

#define DUPS_ALLOWED (1 << 16) /* allow duplicate keys (.sortFunction flag) */

/* All Bullet system vars set to default values at INIT_XB */
/* Sorts 1-19 also used as CIP.sortFunction (can be OR'ed with DUPS_ALLOWED) */
/* Intrinsic sorts (1-6) are read-only (R-O) */

#define ASCII_SORT 1    /* sort by: ASCII value (R-O) */
#define NLS_SORT   2    /* NLS (R-O) */
#define S16_SORT   3    /* 16-bit signed integer (R-O) */
#define U16_SORT   4    /* 16-bit unsigned integer (R-O) */
#define S32_SORT   5    /* 32-bit signed integer (R-O) */
#define U32_SORT   6    /* 32-bit unsigned integer (R-O) */

/* sorts 7 to 9 are reserved */
/* Custom sort-compare functions are from 10 to 19 */

#define BUILD_KEY_FUNC  20      /* key build function ptr */
#define PARSER_FUNC     21      /* key expression parser function ptr */

#define MUTEX_SEM_HANDLE     29 /* handle of Bullet's mutex semaphore (R-O) */
#define LOCK_TIMEOUT         30 /* lock-wait timeout (default=0, no wait)*/
#define MUTEX_SEM_TIMEOUT    31 /* mutex semaphore-wait timeout (def=0,none) */
#define PACK_BUFFER_SIZE     32 /* pack buffer size (def=0, min autosize) */
#define REINDEX_BUFFER_SIZE  33 /* reindex buffer size (def=0, min autosize) */
#define REINDEX_PACK_PCT     34 /* reindex node pack % (default=100, max) */
#define TMP_PATH_PTR         35 /* temporary file path ptr (default=NULL) */
#define REINDEX_SKIP_TAG     36 /* index skip tag select (default=0, none) */
#define COMMIT_AT_EACH       37 /* commit each insert/update in pack (def=0) */
#define MEMO_BLOCKSIZE       38 /* memo block size (default=512 bytes) */
#define MEMO_EXTENSION       39 /* memo filename extension (default='DBT\0') */
#define MAX_DATAFILE_SIZE    40 /* max data size (default=0x7FEFFFFF=2095MB) */
#define MAX_INDEXFILE_SIZE   41 /* max index size (default=0x7FEFFFFF=2095MB)*/
#define ATOMIC_MODE          42 /* bit0=1 atomic next/prev key access (def=0)*/
#define CALLBACK_PTR         43 /* callback at reindex/pack (def=0, none) */

/* ************************************************************************
 *
 * Query/SetVectors vector IDs
 *
 * ************************************************************************/

#define VECTOR_CLOSE_FILE           2
#define VECTOR_CREATE_DIR           3
#define VECTOR_CREATE_FILE          4
#define VECTOR_CREATE_UNIQUE_FILE   5
#define VECTOR_DELETE_FILE          6
#define VECTOR_LENGTH_FILE          7
#define VECTOR_MOVE_FILE            8
#define VECTOR_OPEN_FILE            9
#define VECTOR_READ_FILE           10
#define VECTOR_SEEK_FILE           11
#define VECTOR_UPDATE_DIR_ENTRY    12
#define VECTOR_WRITE_FILE          13
#define VECTOR_LOCK_FILE           14
#define VECTOR_IS_DRIVE_REMOTE     15
#define VECTOR_IS_FILE_REMOTE      16
#define VECTOR_EXITLIST            17
#define VECTOR_REMOVE_EXITLIST     18
#define VECTOR_FREE                19
#define VECTOR_GET_SORT_TABLE      20
#define VECTOR_GET_COUNTRY_INFO    21
#define VECTOR_GET_ERROR_CLASS     22
#define VECTOR_GET_MEMORY          23
#define VECTOR_GET_TMP_DIR         24
#define VECTOR_GET_VERSION         25
#define VECTOR_MALLOC              26
#define VECTOR_SET_HANDLE_COUNT    27
#define VECTOR_GET_TIME_INFO       28
#define VECTOR_UPPERCASE           29
#define VECTOR_CLOSE_MUTEX_SEM     30
#define VECTOR_CREATE_MUTEX_SEM    31
#define VECTOR_RELEASE_MUTEX_SEM   32
#define VECTOR_REQUEST_MUTEX_SEM   33


/* ************************************************************************
 *
 * Parameter pack structures, typedefs
 *
 * ************************************************************************/

/* AP, CP, CDP, etc., are suggested variable names */

typedef struct _ACCESSPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle of Bullet file to access */
LONG  recNo;          /* IO, record number */
PVOID recPtr;         /* I, programmer's record buffer */
PVOID keyPtr;         /* I, programmer's key buffer */
PVOID nextPtr;        /* I, NULL if not xaction, else next AP in list */
} ACCESSPACK; /* AP */
typedef ACCESSPACK *PACCESSPACK;

/* CBP is the structure received by the callback procedure */
/* structure members are filled in by Bullet */

typedef struct _CALLBACKPACK {
ULONG sizeIs;         /* structure size (current 16 bytes) */
ULONG callMode;       /* 0=from reindex; 1=from DBF pack */
ULONG handle;         /* file handle */
ULONG data1;          /* for callMode=0/1: progress percent (1-99,0) */
} CALLBACKPACK; /* CBP */
typedef CALLBACKPACK *PCALLBACKPACK;

typedef struct _COPYPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle of Bullet file to copy */
PSZ   filenamePtr;    /* I, filename to use (drv+path must exist if used) */
} COPYPACK; /* CP */
typedef COPYPACK *PCOPYPACK;

typedef struct _CREATEDATAPACK {
ULONG func;
ULONG stat;
PSZ   filenamePtr;    /* I, filename to use */
ULONG noFields;       /* I, 1 to 254 */
PVOID fieldListPtr;   /* I, descriptor list, 1 per field */
ULONG fileID;         /* I, 0x03 for standard DBF, 0x8B if memo file also */
} CREATEDATAPACK; /* CDP */
typedef CREATEDATAPACK *PCREATEDATAPACK;

typedef struct _CREATEINDEXPACK {
ULONG func;
ULONG stat;
PSZ   filenamePtr;    /* I, filename to use */
PSZ   keyExpPtr;      /* I, e.g., "SUBSTR(LNAME,1,4)+SSN" */
LONG  xbLink;         /* I, opened data file handle this indexes */
ULONG sortFunction;   /* I, 1-9 system, 10-19 custom */
ULONG codePage;       /* I, 0=use process default */
ULONG countryCode;    /* I, 0=use process default */
PVOID collatePtr;     /* I, NULL=use cc/cp else use passed table for sort */
ULONG nodeSize;       /* I, 512, 1024, or 2048 */
} CREATEINDEXPACK; /* CIP */
typedef CREATEINDEXPACK *PCREATEINDEXPACK;

typedef struct _FIELDDESCTYPE {
BYTE  fieldName[11];  /* IO, upper A-Z and _; 1-10 chars, 0-filled, 0-term */
BYTE  fieldType;      /* IO, C,D,L,N, or M */
LONG  fieldDA;        /* x, offset within record (run-time storage option) */
BYTE  fieldLen;       /* IO, C=1-255,D=8,L=1,N=1-19,M=10 */
BYTE  fieldDC;        /* IO, fieldType=N then 0-15 else 0 */
USHORT altFieldLength;/* IO, 0 */
BYTE  filler[12];     /* I, 0 */
} FIELDDESCTYPE; /* nested in _DESCRIPTORPACK */
typedef FIELDDESCTYPE *PFIELDDESCTYPE;

typedef struct _DESCRIPTORPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle of DBF file */
ULONG fieldNumber;    /* IO, first field is 1 */
ULONG fieldOffset;    /* O, offset of field within record (tag=offset 0) */
FIELDDESCTYPE FD;     /* IO FD.fieldName only, O for rest of FD */
} DESCRIPTORPACK; /* DP */
typedef DESCRIPTORPACK *PDESCRIPTORPACK;

typedef struct _DOSFILEPACK {
ULONG func;
ULONG stat;
PSZ   filenamePtr;    /* I, filename to use */
ULONG handle;         /* IO, handle of open file */
ULONG asMode;         /* I, access-sharing mode */
ULONG bytes;          /* IO, bytes to read, write, length of */
LONG  seekTo;         /* IO, seek to offset, current offset */
ULONG method;         /* I, seek method (0=start of file, 1=current, 2=end) */
PVOID bufferPtr;      /* I, buffer to read into or write from */
ULONG attr;           /* I, attribute to create file with */
PSZ   newNamePtr;     /* I, name to use on rename */
} DOSFILEPACK; /* DFP */
typedef DOSFILEPACK *PDOSFILEPACK;

typedef struct _EXITPACK {
ULONG func;
ULONG stat;
} EXITPACK; /* EP */
typedef EXITPACK *PEXITPACK;

typedef struct _HANDLEPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle of Bullet file */
} HANDLEPACK; /* HP */
typedef HANDLEPACK *PHANDLEPACK;

typedef struct _INITPACK {
ULONG func;
ULONG stat;
ULONG JFTsize;        /* I, max opened files (20-1024+) */
ULONG versionDOS;     /* O, e.g., 230 for 2.30 */
ULONG versionBullet;  /* O, e.g., 2019 for 2.019 */
ULONG versionOS;      /* O, e.g., 4=OS/2 32-bit */
PVOID exitPtr;        /* O, function pointer to EXIT_XB routine */
} INITPACK; /* IP */
typedef INITPACK *PINITPACK;

typedef struct _LOCKPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle of Bullet file to lock */
ULONG xlMode;         /* I, index lock mode (0=exclusive, 1=shared) */
ULONG dlMode;         /* I, data lock mode (0=exclusive, 1=shared) */
LONG  recStart;       /* I, if data, first record # to lock, or 0 for all */
ULONG recCount;       /* I, if data and recStart!=0, # records to lock */
PVOID nextPtr;        /* I, NULL if not xaction, else next LP in list */
} LOCKPACK; /* LP */
typedef LOCKPACK *PLOCKPACK;

typedef struct _MEMODATAPACK {
ULONG func;
ULONG stat;
ULONG dbfHandle;      /* I, handle of DBF file to which this memo file belongs */
ULONG memoBypass;     /* I, memo bypass function to do, if any */
PVOID memoPtr;        /* I, ptr to memo record buffer */
ULONG memoNo;         /* IO, memo record number (aka block number) */
ULONG memoOffset;     /* I, position within record to start read/update */
ULONG memoBytes;      /* IO, number of bytes to read/update */
} MEMODATAPACK; /* MDP */
typedef MEMODATAPACK *PMEMODATAPACK;

typedef struct _MEMORYPACK {
ULONG func;
ULONG stat;
ULONG memory;         /* O, not used in OS/2 */
} MEMORYPACK; /* MP */
typedef MEMORYPACK *PMEMORYPACK;

typedef struct _OPENPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* O, handle of file opened */
PSZ   filenamePtr;    /* I, Bullet file to open */
ULONG asMode;         /* I, access-sharing-cache mode */
LONG  xbLink;         /* I, if index open, xbLink=handle of its opened DBF */
} OPENPACK; /* OP */
typedef OPENPACK *POPENPACK;

typedef struct _QUERYSETPACK {
ULONG func;
ULONG stat;
ULONG item;           /* I, Bullet sysvar item to get/set */
ULONG itemValue;      /* IO, current/new value */
} QUERYSETPACK; /* QSP */
typedef QUERYSETPACK *PQUERYSETPACK;

typedef struct _REMOTEPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle of file, or if 0, use RP.drive */
ULONG drive;          /* I, drive (1=A,2=B,3=C,...0=current) to check */
ULONG isRemote;       /* O, =1 of handle/drive is remote, =0 if local */
ULONG flags;          /* O, 0 */
ULONG isShare;        /* O, 1 */
} REMOTEPACK; /* RP */
typedef REMOTEPACK *PREMOTEPACK;

typedef struct _STATDATAPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle to check */
ULONG fileType;       /* O, bit0=1 data file */
ULONG flags;          /* O, bit0=1 dirty, bit1=1 full-lock, bit2=1 shared */
ULONG progress;       /* O, 0,1-99% pack progress */
PVOID morePtr;        /* O, 0 */
ULONG fields;         /* O, fields per record */
ULONG asMode;         /* O, access-sharing-cache mode */
PSZ   filenamePtr;    /* O, filename used in open */
ULONG fileID;         /* O, first byte of DBF file */
ULONG lastUpdate;     /* O, high word=year,low byte=day, high byte=month */
ULONG records;        /* O, data records (including "deleted") */
ULONG recordLength;   /* O, record length */
ULONG xactionFlag;    /* O, 0 */
ULONG encryptFlag;    /* O, 0 */
PVOID herePtr;        /* O, this file's control address */
ULONG memoHandle;     /* O, handle of open memo file (0 if none) */
ULONG memoBlockSize;  /* O, memo file block size */
ULONG memoFlags;      /* O, bit0=1 dirty */
ULONG memoLastRecord; /* O, last accessed memo record (0 if none) */
ULONG memoLastSize;   /* O, size of last accessed memo record (in bytes, +8) */
ULONG lockCount;      /* O, number of full-locks in force */
} STATDATAPACK; /* SDP */
typedef STATDATAPACK *PSTATDATAPACK;

typedef struct _STATHANDLEPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle to check */
LONG  ID;             /* O, bit0=1 data file, bit0=1 index file */
} STATHANDLEPACK; /* SHP */
typedef STATHANDLEPACK *PSTATHANDLEPACK;

typedef struct _STATINDEXPACK {
ULONG func;
ULONG stat;
ULONG handle;         /* I, handle to check */
ULONG fileType;       /* O, bit0=0 index file */
ULONG flags;          /* O, bit0=1 dirty, bit1=1 full-lock, bit2=1 shared */
ULONG progress;       /* O, 0,1-99% reindex progress */
PVOID morePtr;        /* O, 0 */
ULONG xbLink;         /* O, XB file link handle */
ULONG asMode;         /* O, access-sharing-cache mode */
PSZ   filenamePtr;    /* O, pointer to filename used in open */
ULONG fileID;         /* O, "31ch" */
PSZ   keyExpPtr;      /* O, pointer to key expression */
ULONG keys;           /* O, keys in file */
ULONG keyLength;      /* O, key length */
ULONG keyRecNo;       /* O, record number of current key */
PVOID keyPtr;         /* O, ptr to current key value (valid to keyLength) */
PVOID herePtr;        /* O, this file's control address */
ULONG codePage;       /* O, code page at create time */
ULONG countryCode;    /* O, country code at create time */
PVOID CTptr;          /* O, collate table ptr, NULL=no collate table present */
ULONG nodeSize;       /* O, node size */
ULONG sortFunction;   /* O, sort function ID */
ULONG lockCount;      /* O, number of full-locks in force */
} STATINDEXPACK; /* SIP */
typedef STATINDEXPACK *PSTATINDEXPACK;

typedef struct _XERRORPACK {
ULONG func;
ULONG stat;           /* I, error to check */
ULONG errClass;       /* O, class of error */
ULONG action;         /* O, action recommended for error */
ULONG location;       /* O, location of error */
} XERRORPACK; /* XEP */
typedef XERRORPACK *PXERRORPACK;


/* ************************************************************************
 *
 * Error codes
 *
 * ************************************************************************/

#define EXB_NOT_ENOUGH_MEMORY   8  /* cannot get memory requested */
#define ERR_INVALID_DRIVE       15 /* not a valid drive letter */
#define EXB_UNEXPECTED_EOF      38 /* unexpect EOF (bytes read != bytes asked) */
#define EXB_DISK_FULL           39 /* disk full on WriteFile */
#define EXB_FILE_EXISTS         80 /* cannot create file since it already exists */
#define EXB_SEM_OWNER_DIED      105 /* in place of Win32 error 80h (mutex) */
#define EXB_TIMEOUT             640 /* in place of Win32 error 102h (mutex) */

/* Other operating system errors are as returned by OS itself */

/* System/general error codes */

#define EXB_OR_WITH_FAULTS      8192 /* 8192+1 to +4, close-type errors */

                                     /* ERR_216501/6 are for Bullet/x only */
#define EXB_216501              8251 /* INT21/6501h not supported by DOS extender */
                                     /* (do not use default cc/cp) */
#define EXB_216506              8256 /* INT21/6506h not supported by DOS extender */
                                     /* (provide a custom collate table) */

#define EXB_ILLEGAL_CMD         8300 /* function not allowed */
#define EXB_OLD_DOS             8301 /* OS version < MIN_DOS_NEEDED */
#define EXB_NOT_INITIALIZED     8302 /* init not active, must do INIT_XB */
#define EXB_ALREADY_INITIALIZED 8303 /* init already active, must do EXIT_XB */
#define EXB_TOO_MANY_HANDLES    8304 /* more than 1024 opens requested */
#define EXB_SYSTEM_HANDLE       8305 /* Bullet won't use or close handles 0-2 */
#define EXB_FILE_NOT_OPEN       8306 /* file not open (not Bullet handle, including xbLink) */
#define EXB_FILE_IS_DIRTY       8307 /* tried to reload header but current still dirty */
#define EXB_BAD_FILETYPE        8308 /* tried key op on non-key file, data op on non... */
#define EXB_TOO_MANY_PACKS      8309 /* too many INSERT,UPDATE,REINDEX,LOCK_XB packs */
#define EXB_NULL_RECPTR         8310 /* null record pointer passed to Bullet */
#define EXB_NULL_KEYPTR         8311 /* null key pointer passed to Bullet */
#define EXB_NULL_MEMOPTR        8312 /* null memo pointer passed to Bullet */
#define EXB_EXPIRED             8313 /* evaluation time period has expired */
#define EXB_BAD_INDEX           8314 /* Query/SetSysVars index beyond last one */
#define EXB_RO_INDEX            8315 /* SetSysVar index item is read-only */
#define EXB_FILE_BOUNDS         8316 /* file size > 4GB, or > system var sets */

/* Multi-access error codes */

#define EXB_BAD_LOCK_MODE       8401 /* lock mode (LP) not valid */
#define EXB_NOTHING_TO_RELOCK   8402 /* cannot relock without existing full-lock */
#define EXB_SHARED_LOCK_ON      8403 /* write access needed but lock is shared (flush on backup) */

/* Index error codes */

#define EXB_KEY_NOT_FOUND       8501 /* exact match of key not found */
#define EXB_KEY_EXISTS          8502 /* key exists already and dups not allowed */
#define EXB_END_OF_FILE         8503 /* already at last index order */
#define EXB_TOP_OF_FILE         8504 /* already at first index order */
#define EXB_EMPTY_FILE          8505 /* nothing to do since no keys */
#define EXB_CANNOT_GET_LAST     8506 /* cannot locate last key */
#define EXB_BAD_INDEX_STACK     8507 /* index file is corrupt */
#define EXB_BAD_INDEX_READ0     8508 /* index file is corrupt */
#define EXB_BAD_INDEX_WRITE0    8509 /* index file is corrupt */

#define EXB_OLD_INDEX           8521 /* old index, run through ReindexOld to update */
#define EXB_UNKNOWN_INDEX       8522 /* not a Bullet index file */
#define EXB_KEY_TOO_LONG        8523 /* keylength > 62 (or 64 if unique), or is 0 */

#define EXB_PARSER_NULL         8531 /* parser function pointer is NULL */
#define EXB_BUILDER_NULL        8532 /* build key function pointer is NULL */
#define EXB_BAD_SORT_FUNC       8533 /* CIP.sortFunction not valid */
#define EXB_BAD_NODE_SIZE       8534 /* CIP.nodeSize is not 512, 1024, or 2048 */
#define EXB_FILENAME_TOO_LONG   8535 /* CIP.filenamePtr->pathname > max path length */

#define EXB_KEYX_NULL           8541 /* expression is effectively NULL */
#define EXB_KEYX_TOO_LONG       8542 /* CIP.keyExpPtr->expression > 159 */
#define EXB_KEYX_SYM_TOO_LONG   8543 /* fieldname/funcname in expression > 10 chars */
#define EXB_KEYX_SYM_UNKNOWN    8544 /* fieldname/funcname in expression unknown */
#define EXB_KEYX_TOO_MANY_SYMS  8545 /* too many symbols/fields used in expression */
#define EXB_KEYX_BAD_SUBSTR     8546 /* invalid SUBSTR() operand in expression */
#define EXB_KEYX_BAD_SUBSTR_SZ  8547 /* SUBSTR() exceeds field's size */
#define EXB_KEYX_BAD_FORM       8548 /* didn't match expected symbol in expression */

#define EXB_NO_READS_FOR_RUN    8551 /* unlikely, use different reindex buffer size */
#define EXB_TOO_MANY_RUNS       8552 /* unlikely, too many runs (64K or more runs) */
#define EXB_TOO_MANY_RUNS_FOR_BUFFER 8553 /* unlikely, too many runs for run buffer */
#define EXB_TOO_MANY_DUPLICATES 8554 /* more than 64K "identical" keys */

#define EXB_INSERT_RECNO_BAD    8561 /* AP.recNo cannot be > 0 if inserting */
#define EXB_PREV_APPEND_EMPTY   8562 /* no prev append for insert yet AP.recNo==80000000h */
#define EXB_PREV_APPEND_MISMATCH 8563 /* prev append's xbLink does not match this */
#define EXB_INSERT_KBO_FAILED   8564 /* could not back out key at INSERT_XB */
#define EXB_INSERT_DBO_FAILED   8565 /* could not back out data records at INSERT_XB */

#define WRN_NOTHING_TO_UPDATE   8571 /* all AP.recNo=0 at UPDATE_XB */
#define EXB_INTERNAL_UPDATE     8572 /* internal error UPDATE_XB, not in hdl/rec# list */

#define EXB_FAILED_DATA_RESTORE 8573 /* could not restore original data record (*) */
#define EXB_FAILED_KEY_DELETE   8574 /* could not remove new key (*) */
#define EXB_FAILED_KEY_RESTORE  8575 /* could not restore original key(*) */
/* *original error, which forced a back-out, has been replaced by this error */ 
/* this error is always returned in the first AP.stat (-1 on data, 1 on index) */

/* Data error codes */

#define EXB_EXT_XBLINK          8601 /* xbLink handle is not internal DBF (is -1) */
#define EXB_FIELDNAME_TOO_LONG  8602 /* fieldname is > 10 characters */
#define EXB_RECORD_TOO_LONG     8603 /* record length is > 64K */
#define EXB_FIELD_NOT_FOUND     8604 /* fieldname not found in descriptor info */
#define EXB_BAD_FIELD_COUNT     8605 /* fields <= 0 or >= MAX_FIELDS (Init,Open) */
                                     /* and also GetDescriptor by field number */
#define EXB_BAD_HEADER          8606 /* bad header (reclen=0, etc., from LocateTo, Flush) */
#define EXB_BUFFER_TOO_SMALL    8607 /* buffer too small (pack buffer < reclen in pack) */
#define EXB_INTERNAL_PACK       8608 /* internal error in PackRecords */
#define EXB_BAD_RECNO           8609 /* record number=0 or > records in data file hdr */
                                     /* or Pack on empty data file */
#define WRN_RECORD_TAGGED       8610 /* record's tag field matches skip tag */

/* Memo error codes */

#define WRN_CANNOT_OPEN_MEMO    8701 /* DBF says memo file but memo open fails */
#define EXB_MEMO_NOT_OPEN       8702 /* no open memo file for operation */
#define EXB_BAD_BLOCKSIZE       8703 /* memo blocksize must be at least 24 bytes */
#define EXB_MEMO_DELETED        8704 /* memo is deleted */
#define EXB_MEMO_PAST_END       8705 /* memo data requested is past end of record */
#define EXB_BAD_MEMONO          8706 /* memo number is not valid */
#define EXB_MEMO_IN_USE         8707 /* memo add encountered likely corrupt memo file */
#define EXB_BAD_AVAIL_LINK      8708 /* memo avail link cannot be valid (is 0) */
#define EXB_MEMO_ZERO_SIZE      8709 /* memo data has no size */
#define EXB_MEMO_IS_SMALLER     8710 /* memo attempt to shrink but already <= size */

#endif /* ifndef __BULLET_H */
