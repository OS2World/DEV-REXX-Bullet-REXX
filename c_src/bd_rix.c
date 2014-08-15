
/* bd_rix.c -  4-Oct-1996 Cornel Huth
 * This module is called by bd_main.c
 * REINDEX_XB
 */

#include "platform.h"

// external to this module

void PutMsg(CHAR *strg);
void GetMsg(CHAR *strg);

extern CHAR *collateTable;

#if FOR_WINDOWS == 1
 void DoWinThing(int waitFlag);
#endif

// public in this module

int bd_rix(void);

void __cdecl CallbackRix(CALLBACKPACK *CBP);  // *CBP on stack; caller OR callee may clean stack
void APIENTRY ThreadRix(ACCESSPACK *AP);      // OS-called thread routine

LONG rezRix=0;   // thread reindex return code
TID tidRix;      // reindex thread ID  (OS/2=TID, Win32=DWORD)
HANDLE hRix;     // reindex thread's handle (Win32 only)
LONG recs2add;   // records to add en-masse (also used in CallbackRix())

// GO

int bd_rix(void) {

 void BuildFieldListRix(FIELDDESCTYPE fieldList[]);

#pragma pack(1)

 ACCESSPACK AP;
 DOSFILEPACK DFP;
 CREATEDATAPACK CDP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
 OPENPACK OP;
 QUERYSETPACK QSP;
 STATINDEXPACK SIP;   // for separate thread reindex percentage done

 struct EmpRecType {
  CHAR tag;              // record tag, init to SPACE, * means deleted
  CHAR empID[9];         // SSN (not 0T string)
  CHAR empLN[16];        // last name
  CHAR empFN[16];        // first name
  CHAR empHire[8];       // "YYYYMMDD" (not 0T string)
  CHAR empDept[6];       // department assigned
 }; // 56 bytes
 struct EmpRecType EmpRec;

#pragma pack()   // no more Bullet-accessed data structures follow

 time_t startTime, endTime;
 int display = 0;          // display results or not flag
 int thread = 1;           // dispatch reindex in a thread flag
 int lastNameVar;          // used to construct on-the-fly data record...
 int lastFourVar;          // ...that is unique

 LONG rez;                 // return value from Bullet

 CHAR nameIX3[] = ".\\$bd_rix.ix3"; // name of index file created
 ULONG indexID=0;                // handle of index file
 CHAR keyExpression[128];        // key expression string buffer (159 max)
 CHAR keyBuffer[68];             // buffer used to store/receive key values

 CHAR nameData[] = ".\\$bd_rix.dbf";// name of data file created
 ULONG dataID=0;                 // handle of data file
 FIELDDESCTYPE fieldList[5];     // 5 fields used in data record

 LONG i;                 // counter
 CHAR tmpStr[128];       // misc stuff, non-Bullet related
 CHAR putStr[128];
 ULONG tmpU;

 SIP.func = 0; // init+ref it for DOSX32 compile

 // set field descriptor info for DBF data file created later

 memset(fieldList,0,sizeof(fieldList));  // init unused bytes to 0 (required)
 BuildFieldListRix(fieldList);


 // Use 300K for workspace in reindex module (min is 48K, max is whatever you want)
 // Note: larger is not necessarily faster; it depends on the data order
 #define NEW_XBUFF_SIZE (300*1024)

 QSP.func = SET_SYSVARS_XB;
 QSP.item = REINDEX_BUFFER_SIZE;
 QSP.itemValue = NEW_XBUFF_SIZE;
 rez = BULLET(&QSP);
 if (rez) {
    sprintf(putStr,"Failed SET_SYSVARS_XB #%d (reindex buffer size) call.  Err: %d\n",QSP.item,rez);
    PutMsg(putStr);
    goto Abend;
 }

 tmpU = QSP.itemValue;              // 0=default, or 144KB total buffer space
 if (tmpU == 0) tmpU = (144*1024);
 sprintf(putStr,"Bullet reindex tmp buffer was %d KB, now %d KB\n", tmpU/1024,
                                                                     NEW_XBUFF_SIZE/1024);
 PutMsg(putStr);

 // Delete previous files from any previous run (disregard any error return)

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = nameData;
 rez = BULLET(&DFP);
 DFP.filenamePtr = nameIX3;
 rez = BULLET(&DFP);


 // Create the data file, a standard DBF (ID=3) as defined in fieldList above.

 CDP.func = CREATE_DATA_XB;
 CDP.filenamePtr = nameData;
 CDP.noFields = 5;
 CDP.fieldListPtr = fieldList;
 CDP.fileID = 0x03;
 rez = BULLET(&CDP);
 if (rez) {
    sprintf(putStr,"Failed data file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }


 // Open the data file (required before creating an index file for it)

 OP.func = OPEN_DATA_XB;
 OP.filenamePtr = nameData;
 OP.asMode = READWRITE | DENYNONE;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed data file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 dataID = OP.handle;


 // Create an index file for the data file opened above.
 // This example uses a multi-part key composed of parts of last name and SSN.

 strcpy(keyExpression,"SUBSTR(LNAME,1,4)+SUBSTR(SSN,6,4)");

 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = nameIX3;
 CIP.keyExpPtr = keyExpression;
 CIP.xbLink = dataID;            // the handle of the data file
 CIP.sortFunction = NLS_SORT | SORT_SET;  // sort by NLS (SORT_SET defined in PLATFORM.H)
 CIP.codePage = CODEPAGE;        // code page to use, or 0 for system default
 CIP.countryCode = CTRYCODE;     // country code...
 CIP.collatePtr = collateTable;  // (see #define INT2165xxNS above)
 CIP.nodeSize = 512;             // 512-byte node size (or 1024, 2048 bytes)
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }


 // Open the index file (what we just created above).
 // As with the index-create, the index-open requires the handle of the data
 // file which this index file indexes.

 OP.func = OPEN_INDEX_XB;
 OP.filenamePtr = nameIX3;
 OP.asMode = READWRITE | DENYNONE;
 OP.xbLink = dataID;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID = OP.handle;


 PutMsg("Display all data accessed (slower results)? (y/N) ");
 GetMsg(tmpStr);
 if (*tmpStr=='y') display = 1;

#if PLATFORM == ON_OS2 || PLATFORM == ON_WIN32 && FOR_WINDOWS == 0
  PutMsg("  Dispatch REINDEX_XB in a separate thread? (Y/n) ");
  GetMsg(tmpStr);
  if (*tmpStr=='n') thread = 0;
#else
  thread = 0;
#endif

 PutMsg("  How many records do you want for this test run? ");
 GetMsg(tmpStr);
 recs2add = atol(tmpStr);

 if (recs2add < 1) recs2add = 1;         // would you rather end the test?
 if (recs2add > 9999999) recs2add = 1;   // why wait around for 10M?


 // Add the data records, which are created here, on-the-fly, varying enough
 // to make unique records.

 lastFourVar = 1;                        // 0001 start (as used in strncpy() below)
 lastNameVar = 0;                        // 0000 start

 EmpRec.tag = ' ';                       // set to not-deleted
 strncpy(EmpRec.empID,"465990001",9);    // only changing last 4 in test
 strcpy(EmpRec.empLN,"0000LastNameNum"); // only changing first 4 in test
 strcpy(EmpRec.empFN,"YourFirstName");   // everyone has this first name!
 strncpy(EmpRec.empHire,"19950527",8);   // YYYYMMDD DBF form, no \0 on date
 strcpy(EmpRec.empDept,"MIS");           // everyone works for MIS!

 sprintf(putStr,"    Adding %d records...  ",recs2add);
 PutMsg(putStr);

 time(&startTime);

 AP.func = ADD_RECORD_XB;
 AP.handle = dataID;
 AP.recPtr = &EmpRec;
 for (i = 1; i <= recs2add; i++) {

    sprintf(tmpStr,"%4.4i",lastFourVar++);
    strncpy(EmpRec.empID+5,tmpStr,4);            // update last 4 of empID
    rez = BULLET(&AP);                           // AP. already setup
    if (rez) {
       sprintf(putStr,"Failed while adding record %d.  Err: %d\n",i,rez);
       PutMsg(putStr);
       goto Abend;
    }
    if (lastFourVar > 9999) {                    // changes every 10,000 adds
       lastFourVar = 0;                          // where name takes carry ...
       sprintf(tmpStr,"%4.4i",++lastNameVar);
       strncpy(EmpRec.empLN,tmpStr,4);           // update first 4 of empLN
    }

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_ADD == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);
 sprintf(putStr,"took %u secs.\n",(endTime - startTime));
 PutMsg(putStr);


 // Reindex

 sprintf(putStr,"Reindexing %d records...  \r",recs2add);
 PutMsg(putStr);

 time(&startTime);

 AP.func = REINDEX_XB;           // this is all there is to reindexing even...
 AP.handle = indexID;            // ...million-record databases
 AP.keyPtr = keyBuffer;          // if dup key error...(see manual for details)
 AP.nextPtr = NULL;              // just this one index file

 if (thread) {

#if PLATFORM == ON_DOSX32

    PutMsg(" Threads are not supported on this OS\n");
    goto Abend;

#elif PLATFORM == ON_OS2

    rez = DosCreateThread(&tidRix,
                          (PFNTHREAD) &ThreadRix,
                          (ULONG) &AP,
                          CREATE_READY | STACK_SPARSE,
                          32768);  // min is 8K stack
    if (rez) {
       sprintf(putStr,"Could not start reindex thread.  Err: %d\n",rez);
       PutMsg(putStr);
       goto Abend;
    }

    DosSleep(32);  // wait a bit to let reindex code set progress to non-zero

    // could also use a callback routine to put out a reindex % number

    SIP.func = STAT_INDEX_XB;
    SIP.handle = indexID;
    rez = BULLET(&SIP);
    while (rez==0) {
       DosSleep(100);
       sprintf(putStr,"Reindexing %d records... %3.3u%%\r",recs2add,SIP.progress);
       PutMsg(putStr);
       rez = BULLET(&SIP);
       if (SIP.progress==0) {
          sprintf(putStr,"Reindexing %d records...  ",recs2add);
          PutMsg(putStr);
          break;
       }
    }
    if (rez) {
       sprintf(putStr,"Failed progress check.  Err: %d\n",rez);
       PutMsg(putStr);
    } // continue

    // Can actually get here _before_ the reindex thread fully exits, but
    // won't get here until SIP.progress is back to 0.

    time(&endTime);

    // It's likely that the thread has exited completely by now, but if
    // it was blocked after setting SIP.progress to 0 (hung up in the cache,
    // etc.), then it's possible to get here well before Bullet has exited.
    // Since calling Bullet while Bullet is busy (for most routines) results
    // in a rc=640 being returned (mutex timeout), just call this to wait
    // for the thread to exit completely.  Note that this has only been seen
    // under Win95 (done, but blocked), but this little routine won't hurt.
    // Ignore any error, which there will be if the thread has indeed exited.
    // Another option is to use a non-0 mutex-timeout value, via SET_SYSVARS_XB.

    rez = DosWaitThread(&tidRix,DCWW_WAIT);
    // ignore error (such as "invalid thread ID")

    DosSleep(1); // allow the reindex thread to exit BULLET() (and set rez)

    rez = rezRix;           // get return code set by ThreadRix()
    if (rez) {              // rez is already AP.stat
       sprintf(putStr,"Failed reindex.  Err: %d\n",rez);
       PutMsg(putStr);
       goto Abend;
    }
    sprintf(putStr,"took %u secs.\n",(endTime - startTime));
    PutMsg(putStr);

#elif PLATFORM == ON_WIN32

    hRix = CreateThread(NULL,
                        32768,               // 8KB min.
                        (LPTHREAD_START_ROUTINE) &ThreadRix,
                        (PACCESSPACK) &AP,
                        0,                   // immediate start
                        &tidRix);


    if (hRix==0) {
       rez = GetLastError();
       sprintf(putStr,"Could not start reindex thread.  Err: %d\n",rez);
       PutMsg(putStr);
       goto Abend;
    }

    Sleep(32);   // wait a bit to let reindex code set progress to non-zero

    SIP.func = STAT_INDEX_XB;
    SIP.handle = indexID;
    rez = BULLET(&SIP);
    while (rez==0) {
       Sleep(100);
       sprintf(putStr,"Reindexing %d records... %3.3u%%\r",recs2add,SIP.progress);
       PutMsg(putStr);
       rez = BULLET(&SIP);
       if (SIP.progress==0) {
          sprintf(putStr,"Reindexing %d records...  ",recs2add);
          PutMsg(putStr);
          break;
       }
    }
    if (rez) {
       sprintf(putStr,"Failed progress check.  Err: %d\n",rez);
       PutMsg(putStr);
    } // continue


    // Can actually get here _before_ the reindex thread fully exits, but
    // won't get here until SIP.progress is back to 0.

    time(&endTime);

    // It's likely that the thread has exited completely by now, but if
    // it was blocked after setting SIP.progress to 0 (hung up in the cache,
    // etc.), then it's possible to get here well before Bullet has exited.
    // Since calling Bullet while Bullet is busy (for most routines) results
    // in a rc=640 being returned (mutex timeout), just call this to wait
    // for the thread to exit completely.  Note that this has only been seen
    // under Win95 (done, but blocked), but this little routine won't hurt.
    // Ignore any error, which there will be if the thread has indeed exited.
    // Another option is to use a non-0 mutex-timeout value, via SET_SYSVARS_XB.
    // Error code 640 is documented in BULLET_2.H.

    rez = WaitForSingleObject(hRix,10*1000); // 10 secs plenty of flush time
    // ignore error

    Sleep(32);   // allow the reindex thread to exit BULLET() (and set rez)
                 // why 32ms?  Only because OS/2 min std resolution is 32ms

    rez = rezRix;       // get return code set by ThreadRix()
    if (rez) {              // rez is already AP.stat
       sprintf(putStr,"Failed reindex.  Err: %d\n",rez);
       PutMsg(putStr);
       goto Abend;
    }
    sprintf(putStr,"took %u secs.\n",(endTime - startTime));
    PutMsg(putStr);

#endif

 }
 else {

    // no separate thread, so use callback (especially useful for DOSX32 and Win32s)

    QSP.func = SET_SYSVARS_XB;
    QSP.item = CALLBACK_PTR;
    QSP.itemValue = (ULONG) &CallbackRix;
    rez = BULLET(&QSP);
    if (rez) {
       sprintf(putStr,"Failed SET_SYSVARS_XB #%d (callback address) call.  Err: %d\n",QSP.item,rez);
       PutMsg(putStr);
       goto Abend;
    }

    // now make actual REINDEX_XB call, set up above

    rez = BULLET(&AP);       // rez=0 if okay, or =1 if pack failed with ecode in AP.stat
    if (rez) {
       rez = AP.stat;
       sprintf(putStr,"Failed reindex.  Err: %d\n",rez);
       PutMsg(putStr);
       goto Abend;
    }
    time(&endTime);
    sprintf(putStr,"Reindexing %d records...  took %u secs.\n",recs2add,(endTime - startTime));
    PutMsg(putStr);

    // turn off callback

    QSP.func = SET_SYSVARS_XB;
    QSP.item = CALLBACK_PTR;
    QSP.itemValue = 0;
    rez = BULLET(&QSP);
    if (rez) {
       sprintf(putStr,"Failed SET_SYSVARS_XB #%d (callback clear) call.  Err: %d\n",QSP.item,rez);
       PutMsg(putStr);
       goto Abend;
    }

 }


 // Get key data

 memset(keyBuffer,0,sizeof(keyBuffer));
 sprintf(putStr," Accessing %d keys...     ",recs2add);
 PutMsg(putStr);
 if (display) PutMsg("\n");
 time(&startTime);
 AP.func = FIRST_KEY_XB;
 AP.handle = indexID;
 AP.keyPtr = keyBuffer;
 rez = BULLET(&AP);
 i=0;
 while (rez==0) {
    if (display) {
       sprintf(putStr,"%s  %9.9lu\r", keyBuffer, AP.recNo);
       PutMsg(putStr);
    }
    i++;
    AP.func = NEXT_KEY_XB;
    rez = BULLET(&AP);

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_KEY == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);
 if (display) PutMsg("\n...");

 // expected rez is EXB_END_OF_FILE

 if (rez == EXB_END_OF_FILE) rez=0;
 if (rez) {
    sprintf(putStr,"Failed KEY access.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 sprintf(putStr,"took %u secs. for %d keys\n",(endTime - startTime),i);
 PutMsg(putStr);


 // Get key and record data

 sprintf(putStr," Accessing %d keys+recs...",recs2add);
 PutMsg(putStr);
 if (display) PutMsg("\n");
 time(&startTime);

 AP.func = GET_FIRST_XB;
 AP.handle = indexID;
 AP.keyPtr = keyBuffer;
 AP.recPtr = &EmpRec;
 rez = BULLET(&AP);
 i=0;
 while (rez==0) {
    if (display) {
       sprintf(putStr,"%s  %9.9lu   %s\r", keyBuffer, AP.recNo, &EmpRec); // partial show
       PutMsg(putStr);
    }
    i++;
    AP.func = GET_NEXT_XB;
    rez = BULLET(&AP);

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_GET == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);
 if (display) PutMsg("\n...");

 // expected rez is EXB_END_OF_FILE

 if (rez == EXB_END_OF_FILE) rez=0;
 if (rez) {
    sprintf(putStr,"Failed GET access.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 sprintf(putStr,"took %u secs. for %d keys & records\n",(endTime - startTime),i);
 PutMsg(putStr);


 // Fatal errors above come straight to here
Abend:

 // Close files

 if (indexID) {
    HP.func = CLOSE_INDEX_XB;
    HP.handle = indexID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed index file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }

 // Unlikely the above could fail, considering how far it has gotten so far!
 // But logic says that we want to continue closing other open files...

 if (dataID) {
    HP.func = CLOSE_DATA_XB;
    HP.handle = dataID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed data file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }
 return rez;  // module exit
}

////////////////////
// Support Routines
////////////////////

#if PLATFORM == ON_OS2 || PLATFORM == ON_WIN32

// -------------------------
// Reindex thread, thread #2

void APIENTRY ThreadRix(ACCESSPACK *AP) {

 rezRix = BULLET(AP);

 // Reindex is a xaction-list routine and so must check AP.stat for any
 // error code -- rez as returned from the Bullet call is the list item
 // that failed.  Since this example has but the single item, rez=1 on
 // failure, with the error code in AP.stat.

 if (rezRix) rezRix = AP->stat;
}
#endif


//------------------------------------
// Init field list items for data file

void BuildFieldListRix(FIELDDESCTYPE fieldList[]) {

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


// --------------------------------
// Callback for reindex (no thread)
// This must get parm from stack
// Stack clean up may be caller OR callee
// CBP->callMode == 0 (0=reindex; 1=pack records)
// CBP->data1 is 1 to 99 when working (it starts
// at 1), and is 0 when finished (same as SxP.progress)

void __cdecl CallbackRix(CALLBACKPACK *CBP) {

 static ULONG pct=0;
 CHAR putStr[128];

 if (CBP->data1 > pct) {   // this logic prevents final 0% (all done) from showing
    // W10a fails to carry-forward %% in sprintf so percent sign doesn't display
    pct = CBP->data1;
    sprintf(putStr,"Reindexing %d records... %3.3u%%\r",recs2add,pct);
    PutMsg(putStr);
 }

 // when reindex is complete data1 is always set to 0
 // take the opportunity to re-init the static pct var
 // else it'll stay at its last value

 if (CBP->data1 == 0) pct=0;

 return;

 // could also get % this way, but why if already in CBP->data1

#if 0
 STATINDEXPACK SIP;

 SIP.func = STAT_INDEX_XB;
 SIP.handle = indexID;
 rez = BULLET(&SIP);
 if (SIP.progress != 0) {
    sprintf(putStr,"Reindexing %d records... %3.3u%%\r",recs2add,SIP.progress);
    PutMsg(putStr);
 }
 return;
#endif

}
