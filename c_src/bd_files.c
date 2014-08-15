
/* bd_files.c -  2-Oct-1996 Cornel Huth
 * This module is called by bd_main.c
 * lots of files open at one time
 *
 * If DOSX32: this requires compiler support in
 * ccdosfn.c, BulletSetHandleCount(), if more than
 * a total of 15 handles are to be used
 *
 */

#include "platform.h"

// external to this module

void PutMsg(CHAR *strg);
void GetMsg(CHAR *strg);

extern CHAR *collateTable;


// public in this module

int bd_files(void);


// GO

int bd_files(void) {

/* 
 * Extend Bullet 2 file resources to the max by opening as many files as 
 * possible (limited by version level so far as max files/max instances)
 * Filenames generated are based on PID, and so multiple processes can
 * be used without filename conflict.  It's recommended that this test be
 * run in a new directory, for no reason other than that you can easily 
 * delete the files created, all of which are of the form: $pidnnnn.DBF
 * where pid is the process ID number, and nnnn is the sequence number
 * of the file generated (0001, 0002, onward). (See below at "For Win95...").
 *
 * This module can create up to 1024 files in the directory so do not
 * use the root directory since this has a finite number of entries,
 * (less than 512, even less if VFAT).
 *
 * For non-multi-process versions (Bullet/X, Win32s), no provision is made for
 * more than one active process (other starts get a 'access denied' or
 * 'file exists' message).
 *
 * No actual add operations are performed in this test (see a later example),
 * but the create/open/close/delete operations are timed.  
 *
 * The mix of file types is one index file for each data file.  So, if the
 * max files is 100, 50 DBFs and 50 index files are generated; if 250 files
 * then 125/125; for 1024 files, 512 DBFs and 512 index files, though the
 * actual number is determined by the number entered from the keyboard
 * (i.e., the user) and also the DLL capability level -- the shareware 
 * DLL has 100 files max, per process, with up to two processes active.
 *
 * ------------------------------------------------------------------------
 * For Win95, VFAT file system, the limit is much higher than MS-DOS:  I've
 * opened 1024 files without problem.  NTFS or HPFS has no preset limit.
 * NOTE: Win95 returns the same PID for things run in an MS-DOS box.  This
 * means that if you run this module simultaneously with another "files
 * blowout" run, you should do so in separate directories since the filenames
 * generated are based on the PID, and if the PIDs are the same, so are the
 * filenames.
 *
 * The shareware Bullet95.DLL of the  version can open and use up to
 * 100 files at the same time, with up to two concurrent processes (except
 * for Win32s, where only 1 process is available).
 *
 * -----------------------------------------------------------------------
 * 
 * For DOSX32, the file limit is about 250, but is dependent on the FILES=
 * statement in CONFIG.SYS.  This module limits it to 250 (125+125), but
 * you may be able to do 251 if nothing else is open (hanles 0-2 are reserved
 * and handle 255 is not a valid handle, leaving 251 max handles).
 *
 * All files are open simultaneously, i.e., no files are closed until all 
 * have been opened, after which all are closed and deleted.  The creates, 
 * opens, and closes are all timed, individually, as well as the sum of all.
 *
 * Note: Since the process cannot easily know which files belong to it
 *       from any previous run, and since there may be multiple processes
 *       generating these files, this program deletes the files created
 *       at the end of the run, rather than at the beginning as most other
 *       examples do.
 */


 // See platform.h for already-defined HANDLES_WANTED, as used in INIT_XB
 // in the bd_main.c module.

#if PLATFORM == ON_DOSX32
 #define MAX_DATA_FILES 127   // may have less than 255 available, such as 208,
 #define MAX_INDEX_FILES 127  // or whatever is left by extender or DPMI provider or DOS

#elif PLATFORM == ON_OS2
 #define MAX_DATA_FILES 512   // your DLL/LIB may not be capable of allowing
 #define MAX_INDEX_FILES 512  // as many as this (see your manual)

#elif PLATFORM == ON_WIN32    // if using NTFS or HPFS, can use more
 #define MAX_DATA_FILES 512   // but VFAT (Win95) is limited to 230 or so
 #define MAX_INDEX_FILES 512  // but NTFS or HPFS under NT can do many more

#else
 #error No PLATFORM set in platform.h
 #error -----------------------------
#endif

 void BuildFieldListFiles(FIELDDESCTYPE fieldList[]);

#pragma pack(1)

 DOSFILEPACK DFP;
 CREATEDATAPACK CDP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
 OPENPACK OP;
 QUERYSETPACK QSP;

#pragma pack()

 int crdFiles=0;  // data files created (so to limit what is deleted)
 int crxFiles=0;  // index files created
 int wkdFiles=0;  // data files opened so far (so to limit what is closed)
 int wkxFiles=0;  // index files opened so far
 int err4=0;      // ran out of handles flag

 time_t startTime,endTime,endTime2,endTime3,endTime4,endTime5,endTime6,endTime7,endTime8;

 LONG rez;                    // return value from Bullet
 LONG i;                      // misc counter

 LONG maxFiles;               // max files DLL allows per process
 LONG userFiles;              // number tester wants (up to maxFiles)
 LONG dataFiles;              // computed number of data files (1/3rd)
 LONG indexFiles;             // computed number of index files (2/3rd)


 // these are used as part of the process-unique filenames generated

#if PLATFORM == ON_OS2
 PTIB pptib;                  // for DosGetInfoBlocks
 PPIB pppib;                  // for DosGetInfoBlocks
 CHAR sPID[4];                // this process ID, ASCII format (e.g.,"0015")

#elif PLATFORM == ON_WIN32
 DWORD PID;                   // process ID (low word only)
 CHAR sPID[4];                // this process ID, ASCII format (e.g.,"0015")
#endif

 CHAR nameData[]=".\\$pid#nnn.dbf";   // data filenames built here
 ULONG dataID[MAX_DATA_FILES]={0};   // handles of data files, max this test uses
 FIELDDESCTYPE fieldList[5];         // 5 fields used in data record (all use same)

 CHAR nameIX3[]=".\\$pid#nnn.ix3";    // index filenames built here
 ULONG indexID[MAX_INDEX_FILES]={0}; // handles of indexes, max this test uses
 CHAR keyExpression[128];            // key expression string buffer (all use same)

 CHAR tmpStr[128];                   // misc stuff, non-Bullet related
 CHAR putStr[128];


 setbuf(stdout,NULL);

 memset(fieldList,0,sizeof(fieldList));  // init unused bytes to 0 (required)
 BuildFieldListFiles(fieldList);

#if PLATFORM == ON_OS2
  // get PID for unique filenames

 rez = DosGetInfoBlocks(&pptib,&pppib);
 sprintf(sPID,"%4.4x",(pppib->pib_ulpid & 0xFFFF)); // pre-build filenames
 strncpy(nameData+3,sPID,4);  // account for leading .\$ characters
 strncpy(nameIX3+3,sPID,4);

#elif PLATFORM == ON_WIN32
 // get PID for unique filenames

 PID = GetCurrentProcessId();
 sprintf(sPID,"%4.4x",(PID & 0xFFFF)); // pre-build filenames (.\$0001nnn.ext)
 strncpy(nameData+3,sPID,4);
 strncpy(nameIX3+3,sPID,4);
#endif

 // find out max files your DLL/LIB version has (100, 250, or 1024)
 // you'll already know this, but for this general purpose example...

 QSP.func = QUERY_SYSVARS_XB;
 QSP.item = 28;
 rez = BULLET(&QSP);
 if (rez) {
    sprintf(putStr,"Failed QUERY_SYSVARS call.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 maxFiles = QSP.itemValue;

#if PLATFORM == ON_DOSX32
 if (maxFiles > 250) maxFiles = 250;   // limit to openable handles
 PutMsg("For DOSX32: Max files that can be opened is dependent on FILES= in\n");
 PutMsg("CONFIG.SYS and compiler support in ccdosfn.c:BulletSetHandleCount()\n");
 PutMsg("and the operating system being used (real DOS, a DOS box, etc.)\n\n");
#endif

 sprintf(putStr,"Max files to generate (half data+half index)? (max %d): ",maxFiles);
 PutMsg(putStr);
 GetMsg(tmpStr);
 userFiles = atol(tmpStr);
 if (userFiles > maxFiles) userFiles = maxFiles;
 if (userFiles < 2)        userFiles = 2;

 dataFiles = userFiles >> 1; // half are data files
 indexFiles = dataFiles;     // and the other half index files

 sprintf(putStr,"Using %d data and %d index files for a total of %d files\n\n",
         dataFiles,
         indexFiles,
         (dataFiles+indexFiles));
 PutMsg(putStr);

 time(&startTime);

 // Create the data file, a standard DBF (ID=3) as defined in fieldList above.

 CDP.func = CREATE_DATA_XB;      // these are all invariant
 CDP.filenamePtr = nameData;     // all DBF files will be similar except in name
 CDP.noFields = 5;
 CDP.fieldListPtr = fieldList;
 CDP.fileID = 0x03;

 for (i=1;i <= dataFiles;i++) {

    sprintf(tmpStr,"%3.3i",i);
    strncpy(nameData+7,tmpStr,3);

    rez = BULLET(&CDP);
    if (rez) {
       sprintf(putStr,"Failed data file #%d create.  Err: %d\n",i,rez);
       PutMsg(putStr);
       goto Abend;
    }
    else {
       sprintf(putStr,"Created: %d\r",i);
       PutMsg(putStr);
       crdFiles++;
    }
 }

 time(&endTime);
 sprintf(putStr,"Created: %d data files - took %u secs.\n",i-1,(endTime - startTime));
 PutMsg(putStr);

 // Open the data files

 OP.func = OPEN_DATA_XB;
 OP.filenamePtr = nameData;
 OP.asMode = READWRITE | DENYNONE;

 for (i=1;i <= dataFiles;i++) {

    sprintf(tmpStr,"%3.3i",i);
    strncpy(nameData+7,tmpStr,3);

    rez = BULLET(&OP);
    if (rez) {
       sprintf(putStr,"Failed data file #%d open.    Err: %d\n",i,rez);
       PutMsg(putStr);
       if (rez==4) {
          sprintf(putStr,"\nYou ran out of handles after %d handles.\n\n",wkdFiles);
          PutMsg(putStr);
          err4=1;
       }
       goto Abend;
    }
    else {
       sprintf(putStr," Opened: %d\r",i);
       PutMsg(putStr);
       dataID[i-1]=OP.handle;
       wkdFiles++;
    }
 }
 time(&endTime2);
 sprintf(putStr," Opened: %d data files - took %u secs.\n\n",i-1,(endTime2 - endTime));
 PutMsg(putStr);

 // Create an index file for each data file.
 // All index files are the same, except for name, for this example

 strcpy(keyExpression,"SSN");

 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = nameIX3;
 CIP.keyExpPtr = keyExpression;
 CIP.sortFunction = NLS_SORT | SORT_SET;  // sort by NLS (SORT_SET defined in PLATFORM.H)
 CIP.codePage = CODEPAGE;
 CIP.countryCode = CTRYCODE;
 CIP.collatePtr = collateTable;
 CIP.nodeSize = 512;

 for (i=1;i <= indexFiles;i++) {

    sprintf(tmpStr,"%3.3i",i);
    strncpy(nameIX3+7,tmpStr,3);

    CIP.xbLink = dataID[i-1];    // its cooresponding DBF handle
    rez = BULLET(&CIP);
    if (rez) {
       sprintf(putStr,"Failed index file #%d create.  Err: %d\n",i,rez);
       PutMsg(putStr);
       goto Abend;
    }
    else {
       sprintf(putStr,"Created: %d\r",i);
       PutMsg(putStr);
       crxFiles++;
    }
 }

 time(&endTime3);
 sprintf(putStr,"Created: %d index files- took %u secs.\n",i-1,(endTime3 - endTime2));
 PutMsg(putStr);

 // Open the index files

 OP.func = OPEN_INDEX_XB;
 OP.filenamePtr = nameIX3;
 OP.asMode = READWRITE | DENYNONE;

 for (i=1;i <= indexFiles;i++) {

    sprintf(tmpStr,"%3.3i",i);
    strncpy(nameIX3+7,tmpStr,3);

    OP.xbLink = dataID[i-1];     // its cooresponding DBF handle
    rez = BULLET(&OP);
    if (rez) {
       sprintf(putStr,"Failed index file #%d open.    Err: %d\n",i,rez);
       PutMsg(putStr);
       if (rez==4) {
          sprintf(putStr,"\nYou ran out of handles after %d handles.\n\n",wkdFiles+wkxFiles);
          PutMsg(putStr);
          err4=1;
       }
       goto Abend;
    }
    else {
       sprintf(putStr," Opened: %d\r",i);
       PutMsg(putStr);
       indexID[i-1]=OP.handle;
       wkxFiles++;
    }
 }
 time(&endTime4);
 sprintf(putStr," Opened: %d index files- took %u secs.\n\n",i-1,(endTime4 - endTime3));
 PutMsg(putStr);

 // Abnormal endings come here
Abend:

 if (rez) time(&endTime4);

 // Close the index files than were opened

 HP.func = CLOSE_INDEX_XB;
 for (i=1;i <= wkxFiles;i++) {
    if (indexID[i-1]) {
       HP.handle = indexID[i-1];
       rez = BULLET(&HP);
       if (rez) {
          sprintf(putStr,"Failed index file #%d close.   Err: %d\n",i,rez);
          PutMsg(putStr);
       }
       else {
          sprintf(putStr," Closed: %d\r",i);
          PutMsg(putStr);
          indexID[i-1]=0;
       }
    }
 }
 time(&endTime5);
 sprintf(putStr," Closed: %d index files- took %u secs.\n",i-1,(endTime5 - endTime4));
 PutMsg(putStr);

 // Close the data files (only closes if handle != 0)

 HP.func = CLOSE_DATA_XB;
 for (i=1;i <= wkdFiles;i++) {
    if (dataID[i-1]) {
       HP.handle = dataID[i-1];
       rez = BULLET(&HP);
       if (rez) {
          sprintf(putStr,"Failed data file #%d close.   Err: %d\n",i,rez);
          PutMsg(putStr);
       }
       else {
          sprintf(putStr," Closed: %d\r",i);
          PutMsg(putStr);
          dataID[i-1]=0;
       }
    }
 }
 time(&endTime6);
 sprintf(putStr," Closed: %d data files - took %u secs.\n\n",i-1,(endTime6 - endTime5));
 PutMsg(putStr);

 // Delete the index files

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = nameIX3;
 for (i=1;i <= crxFiles;i++) {

    sprintf(tmpStr,"%3.3i",i);
    strncpy(nameIX3+7,tmpStr,3);

    rez = BULLET(&DFP);
    if (rez) {
       sprintf(putStr,"Failed index file #%d ('%s') delete.  Err: %d\n",i,DFP.filenamePtr,rez);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"Deleted: %d index files\r",i);
       PutMsg(putStr);
    }
 }
 time(&endTime7);
 sprintf(putStr,"Deleted: %d index files- took %u secs.\n",i-1,(endTime7 - endTime6));
 PutMsg(putStr);

 // Delete the data files

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = nameData;
 for (i=1;i <= crdFiles;i++) {

    sprintf(tmpStr,"%3.3i",i);
    strncpy(nameData+7,tmpStr,3);

    rez = BULLET(&DFP);
    if (rez) {
       sprintf(putStr,"Failed data file #%d ('%s') delete.  Err: %d\n",i,DFP.filenamePtr,rez);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"Deleted: %d data files\r",i);
       PutMsg(putStr);
    }
 }
 time(&endTime8);
 sprintf(putStr,"Deleted: %d data files - took %u secs.\n\n",i-1,(endTime8 - endTime7));
 PutMsg(putStr);

 sprintf(putStr,"Total time: %u seconds.\n",(endTime8 - startTime));
 PutMsg(putStr);

 // put up this message again, since it probably scrolled off

 if (err4) {
    sprintf(putStr,"\nRepeat:  You ran out of handles after %d handles.\n\n",wkdFiles+wkxFiles);
    PutMsg(putStr);
 }

 return rez;  // module exit
}


//------------------------------------
// Init field list items for data file

void BuildFieldListFiles(FIELDDESCTYPE fieldList[]) {

 strcpy(fieldList[0].fieldName, "SSN");  // field names must be upper-case
 fieldList[0].fieldType = 'C';           // field types must be upper-case
 fieldList[0].fieldLen = 9;
 fieldList[0].fieldDC = 0;

 strcpy(fieldList[1].fieldName, "LNAME");
 fieldList[1].fieldType = 'C';
 fieldList[1].fieldLen = 16;
 fieldList[1].fieldDC = 0;

 strcpy(fieldList[2].fieldName, "FNAME");
 fieldList[2].fieldType = 'C';
 fieldList[2].fieldLen = 16;
 fieldList[2].fieldDC = 0;

 strcpy(fieldList[3].fieldName, "HIRED");
 fieldList[3].fieldType = 'D';
 fieldList[3].fieldLen = 8;      // date field type must be 8.0
 fieldList[3].fieldDC = 0;

 strcpy(fieldList[4].fieldName, "DEPT");
 fieldList[4].fieldType = 'C';
 fieldList[4].fieldLen = 6;
 fieldList[4].fieldDC = 0;
 return;
}
