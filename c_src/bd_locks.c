
/* bd_locks.c -  4-Oct-1996 Cornel Huth
 * This module is called by bd_main.c
 * region locking
 */

#include "platform.h"

// external to this module

void PutMsg(CHAR *strg);
void GetMsg(CHAR *strg);

extern CHAR *collateTable;


// public in this module

int bd_locks(void);


// GO

int bd_locks(void) {

/* 
 * File-Open locks are not demonstrated here since they typically are not
 * used for online transaction processing since these locks require a file
 * open/close to change the lock type.  The locks demonstrated are region
 * locks/relocks, where physical regions of the file are locked for access,
 * and not the file open itself (other apps can open at any time).
 *
 * This example uses LOCK_XB, RELOCK_XB, and UNLOCK_XB, which are transaction-
 * list routines where a group of related files are all locked in one atomic
 * operation (if all succeed all are locked, if one cannot be locked, none 
 * are locked).  Each individual file can be locked on its own by using the
 * UN/RE/LOCK_INDEX_XB and UN/RE/LOCK_DATA_XB routines.
 *
 * RELOCK_XB, RELOCK_INDEX_XB, and RELOCK_DATA_XB are not supported in 
 * W95 systems (Win/NT does support SHARED region locks, as OS/2, but does
 * not support atomic relocking).
 *
 * RELOCK_XB, RELOCK_INDEX_XB, and RELOCK_DATA_XB are not supported in
 * DOSX32 systems (DOS doesn't support this feature).
 *
 */

 void BuildFieldListLocks(FIELDDESCTYPE fieldList[]);

#pragma pack(1)

 ACCESSPACK AP[2];
 DOSFILEPACK DFP;
 CREATEDATAPACK CDP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
 LOCKPACK LP[2];
 OPENPACK OP;

 struct EmpRecType {
  CHAR tag;              // record tag, init to SPACE, * means deleted
  CHAR empID[9];         // SSN (not 0T string)
  CHAR empLN[16];        // last name
  CHAR empFN[16];        // first name
  CHAR empHire[8];       // "YYYYMMDD" (not 0T string)
  CHAR empDept[6];       // department assigned
 }; // 56 bytes
 struct EmpRecType EmpRec;

#pragma pack()

 LONG rez;                       // return value from Bullet
 LONG rezX;                      // converted pack index (pack that failed ID)
 LONG isLocked;                  // flag indicating active locks

 CHAR nameData[]=".\\$bd_lock.dbf"; // data filename
 ULONG dataID=0;                 // handles of data file
 FIELDDESCTYPE fieldList[5];     // 5 fields used in data record

 CHAR *nameIndex[] = {
 ".\\$bd_lok1.ix3",                 // first index file
 ".\\$bd_lok2.ix3"                  // second...
 };
 ULONG indexID[] ={0,0};
 CHAR *keyExpression[] = {
 "SSN",
 "SUBSTR(LNAME,1,4)+SUBSTR(SSN,6,4)",
 };
 CHAR *keyBuffer[2][68];

 CHAR putStr[128];

 setbuf(stdout,NULL);
 PutMsg("(Self-running module)\n\n");

 memset(fieldList,0,sizeof(fieldList));  // init unused bytes to 0 (required)
 BuildFieldListLocks(fieldList);

 // Delete previous files from any previous run (disregard any error return)

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = nameData;
 rez = BULLET(&DFP);
 DFP.filenamePtr = nameIndex[0];
 rez = BULLET(&DFP);
 DFP.filenamePtr = nameIndex[1];
 rez = BULLET(&DFP);

 PutMsg("Creating one data file ... ");

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

 // Open the data file

 PutMsg("opening ... ");
 OP.func = OPEN_DATA_XB;
 OP.filenamePtr = nameData;
 OP.asMode = READWRITE | DENYNONE;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed data file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 dataID=OP.handle;

 // Create index files

 PutMsg("Done.\n");
 PutMsg("Creating two index files ... ");
 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = nameIndex[0];
 CIP.keyExpPtr = keyExpression[0];
 CIP.xbLink = dataID;
 CIP.sortFunction = ASCII_SORT;
 CIP.codePage = CODEPAGE;
 CIP.countryCode = CTRYCODE;
 CIP.collatePtr = NULL;
 CIP.nodeSize = 512;
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed first index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 CIP.filenamePtr = nameIndex[1];
 CIP.keyExpPtr = keyExpression[1];
 CIP.xbLink = dataID;
 CIP.sortFunction = NLS_SORT | SORT_SET;
 CIP.codePage = CODEPAGE;
 CIP.countryCode = CTRYCODE;
 CIP.collatePtr = collateTable;
 CIP.nodeSize = 512;
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed second index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Open the index files

 PutMsg("opening ... ");
 OP.func = OPEN_INDEX_XB;
 OP.filenamePtr = nameIndex[0];
 OP.asMode = READWRITE | DENYNONE;
 OP.xbLink = dataID;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed first index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID[0] = OP.handle;

 OP.filenamePtr = nameIndex[1];
 OP.xbLink = dataID;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed second index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID[1] = OP.handle;
 PutMsg("Done.\n\n");

 // Lock as one would do in preparation for an update, or any operation
 // where the database needs to be changed, or a status needs to be queried
 // Note: when using the list-processing lock routines (LOCK_XB, UNLOCK_XB,
 //       and RELOCK_XB) only index file handles are used in LP.handle --
 //       their data file will be known internally and locked accordingly.
 //
 // Since index files only are listed, you may wonder how redudant locks
 // are avoid.  The answer is that Bullet locks only the first time, thereafter
 // a lock count is maintained (on a handle basis).  So, while this example
 // does indeed "lock" the data file twice, only the first LP actually does
 // anything regarding the data file (both index files relate to the same
 // data file in this example); the second LP data lock mearly increments
 // the lock count on the data file handle (the first encounter actually
 // locks the data region and reloads the data header).  Since each index
 // file is separate, there are indeed separate physical locks done to each
 // index file (and each maintain separate lock counts, header reloads, etc.).

 PutMsg("Locking the database (exclusive locks) ... ");
 isLocked = 0;

 LP[0].func = LOCK_XB;
 LP[0].handle = indexID[0];
 LP[0].xlMode = 0;       // as are index locks
 LP[0].dlMode = 0;       // data locks are exclusive (initially)
 LP[0].recStart = 0;     // data file's entire region is locked
 LP[0].recCount = 0;     // might as well (complete and all that)
 LP[0].nextPtr = &LP[1];
 LP[1].handle = indexID[1];
 LP[1].xlMode = 0;
 LP[1].dlMode = 0;
 LP[1].recStart = 0;
 LP[1].recCount = 0;
 LP[1].nextPtr = NULL;
 rez = BULLET(&LP[0]);
 if (rez) {

    // since this transaction routine deals with both index and data files
    // rez is returned as the pack that failed: a negative number if the
    // failure was data file related, and positive if index file related
    // in any case, rez is not "the" error code, but is the index of the
    // pack that failed (first pack being pack 1), and in the pack's .stat
    // lies the true error code

    if (rez < 0) {              // data related failure
       rezX = abs(rez)-1;       // minus 1 since C arrays are 0-based
       rez = LP[rezX].stat;     // rez is true error code
       sprintf(putStr,"Lock failed on data, LP[%d], err: %d\n",rezX,rez);
       PutMsg(putStr);
       goto Abend;
    }
    else {
       rezX = rez-1;
       rez = LP[rezX].stat;
       sprintf(putStr,"Lock failed on index, LP[%d], err: %d\n",rezX,rez);
       PutMsg(putStr);
       goto Abend;
    }
 }

 // If it gets here, locks are in place
 // there is no partial locking with LOCK_XB -- it either all succeeds or
 // those locks that were successful in the call are backed out

 PutMsg("Done.\n\n");
 isLocked=1;

 // At this point, the database (the three files in this case) can be
 // written to without concern of corruption (i.e., only this process
 // can write OR read to these files).  Assume that that has taken place...

 PutMsg("INSERT_XB one item into database ... ");
 EmpRec.tag = ' ';
 strncpy(EmpRec.empID,"000000001",9);
 strcpy(EmpRec.empLN,"LockLastName");
 strcpy(EmpRec.empFN,"LockFirstName");
 strncpy(EmpRec.empHire,"19950618",8);
 strcpy(EmpRec.empDept,"XYZ");

 // Inserts only the single item (adds a record, inserts a key into each index)

 AP[0].func = INSERT_XB;
 AP[0].handle = indexID[0];
 AP[0].recNo = 0;                // required
 AP[0].recPtr = &EmpRec;
 AP[0].keyPtr = keyBuffer[0];
 AP[0].nextPtr = &AP[1];
 AP[1].func = INSERT_XB;
 AP[1].handle = indexID[1];
 AP[1].recNo = 0x80000000;       // this pack using record added in first pack
 AP[1].recPtr = &EmpRec;
 AP[1].keyPtr = keyBuffer[1];
 AP[1].nextPtr = NULL;
 rez = BULLET(&AP[0]);
 if (rez) {
    if (rez < 0) {
       rez = abs(rez);
       sprintf(putStr,"INSERT_XB failed, data AP[%d], err: %d\n",rez-1,AP[rez-1].stat);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"INSERT_XB failed, index AP[%d], err: %d\n",rez-1,AP[rez-1].stat);
       PutMsg(putStr);
    }
    goto Abend;
 }

#if RELOCK_AVAIL == 1   // RELOCK_XB is not supported in DOSX32 or WIN95

 // Assume that the insert was successful (it was), and now you want to allow
 // other processes to be able to read the database, but you do not want
 // them to be able to change it (not uncommon).  OS/2 offers two locking
 // modes: 1) exclusive locks, where no other process may read or write
 // to the region locked except the current process (the one locking it),
 // and 2) shared locks, where any other process may READ the locked region,
 // but NO process, INCLUDING the current process, may write to the region.
 // This feature is not available in MS-DOS, except at the file open lock
 // level (but file open locks are too slow to be generally useful).
 // Relock is also an atomic operation, in that there is no chance that
 // another process can steal away the locks during the relock.
 // Note that you can have the initial lock state set to 'shared', rather
 // that exclusive as this example, and then relock to 'exclusive', however
 // you can only call RELOCK_XB if a prior full lock (LOCK_XB) is in force.

 // Note:  When switching from exclusive to shared locks, Bullet automatically
 //        flushes the file (index or data).  This to ensure that a flush
 //        does take place while write-permission is still available -- for
 //        example, you can close files with shared locks last active, and
 //        Bullet does not flush (flush=writes file header, commits to OS)
 //        the file, and cannot, since it can only flush when it has an active
 //        lock, and that lock mode must permit writing (shared does not).
 //        Therefore, whenever you switch from exclusive lock to shared lock,
 //        Bullet flushes the file (which may do nothing if no flush is needed).

 // Since LP[] was previously set, only the changed parms need to be set.
 // Be sure to set each LP[].func in the pack to the new operation, though.

 PutMsg("Done.\n\nChanging locks from exclusive to shared, relock ... ");
 LP[0].func = RELOCK_XB;
 LP[0].xlMode = 1;       // as are index locks
 LP[0].dlMode = 1;       // data locks are now shared
 LP[1].func = RELOCK_XB;
 LP[1].xlMode = 1;       // (also see the Note: above, regarding flushing)
 LP[1].dlMode = 1;
 rez = BULLET(&LP[0]);
 if (rez) {
    if (rez < 0) {
       rezX = abs(rez)-1;
       rez = LP[rezX].stat;
       sprintf(putStr,"Relock failed on data, LP[%d], err: %d\n",rezX,rez);
       PutMsg(putStr);
       goto Abend;
    }
    else {
       rezX = rez-1;
       rez = LP[rezX].stat;
       sprintf(putStr,"Relock failed on index, LP[%d], err: %d\n",rezX,rez);
       PutMsg(putStr);
       goto Abend;
    }
 }

 PutMsg("Done.\n\n...Attempting to write to the database (this will FAIL):\n");

 // At this point, any process may access the database and read from it,
 // but no process, not even this one, may write to it so long as the
 // shared locks are in place.

 // *Attempts* to insert into the database but it will fail because shared
 // locks permit read-only access for all processes, even the locking one.
 // The data record, EmpRec, is changed slightly so that an ERR_KEY_EXISTS
 // error is not returned (only a read is needed to get that far). (o)

 strncpy(EmpRec.empID,"000000002",9); // change keys built (see above)

 AP[0].func = INSERT_XB;
 AP[0].handle = indexID[0];
 AP[0].recNo = 0;                // required (note that this also returns a value)
 AP[0].recPtr = &EmpRec;
 AP[0].keyPtr = keyBuffer[0];
 AP[0].nextPtr = &AP[1];
 AP[1].func = INSERT_XB;
 AP[1].handle = indexID[1];
 AP[1].recNo = 0x80000000;       // this pack using record added in first pack
 AP[1].recPtr = &EmpRec;
 AP[1].keyPtr = keyBuffer[1];
 AP[1].nextPtr = NULL;
 rez = BULLET(&AP[0]);
 if (rez) {
    if (rez < 0) {
       rez = abs(rez);
       sprintf(putStr,"Sure enough, INSERT_XB failed, data AP[%d], err: %d\n",rez-1,AP[rez-1].stat);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"Sure enough, INSERT_XB failed, index AP[%d], err: %d\n",rez-1,AP[rez-1].stat);
       PutMsg(putStr);
    }
 }
 else {
    PutMsg("\n");
    PutMsg("That write should not have worked!  See source.\n");
    goto Abend;
 }

 // switch back to exclusive locks and now write the second item

 PutMsg("\n");
 PutMsg("Changing locks from shared back to exclusive, relock ... ");
 LP[0].func = RELOCK_XB;
 LP[0].xlMode = 0;
 LP[0].dlMode = 0;
 LP[1].func = RELOCK_XB;
 LP[1].xlMode = 0;
 LP[1].dlMode = 0;
 rez = BULLET(&LP[0]);
 if (rez) {
    if (rez < 0) {
       rezX = abs(rez)-1;
       rez = LP[rezX].stat;
       sprintf(putStr,"Relock failed on data, LP[%d], err: %d\n",rezX,rez);
       PutMsg(putStr);
       goto Abend;
    }
    else {
       rezX = rez-1;
       rez = LP[rezX].stat;
       sprintf(putStr,"Relock failed on index, LP[%d], err: %d\n",rezX,rez);
       PutMsg(putStr);
       goto Abend;
    }
 }

 PutMsg("Done.\n\n...Attempting to write to the database (this will WORK) ... ");

 AP[0].func = INSERT_XB;
 AP[0].handle = indexID[0];
 AP[0].recNo = 0;
 AP[0].recPtr = &EmpRec;
 AP[0].keyPtr = keyBuffer[0];
 AP[0].nextPtr = &AP[1];
 AP[1].func = INSERT_XB;
 AP[1].handle = indexID[1];
 AP[1].recNo = 0x80000000;
 AP[1].recPtr = &EmpRec;
 AP[1].keyPtr = keyBuffer[1];
 AP[1].nextPtr = NULL;
 rez = BULLET(&AP[0]);
 if (rez) {
    if (rez < 0) {
       rez = abs(rez);
       sprintf(putStr,"INSERT_XB failed, data AP[%d], err: %d\n",rez-1,AP[rez-1].stat);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"INSERT_XB failed, index AP[%d], err: %d\n",rez-1,AP[rez-1].stat);
       PutMsg(putStr);
    }
    goto Abend;
 }
 else
    PutMsg("Done.\n");

#else

    PutMsg("Done.\n");   // DOSX32 and Win95 do not support RELOCK_XB

#endif


// Fatal errors above come straight to here
Abend:

 // Consider all processing done at this point, and you want to finish
 // up your database access...

 // The operating system will release any locks on files when the files
 // are closed, however, it is only good practice to ensure that you perform
 // this task, if for no other reason than to make sure they are unlocked!
 // As with the previos call, all LP[] parms are already set but .func.
 // Also note that when unlocking, the .xlMode/.dlMode must be set to the
 // current real state -- in other words, since .xlMode=.dlMode=1 at this
 // point (set immediately above), when unlocking these two parms should
 // remain the same.  The reason is that Bullet uses these parms to determine
 // if a flush is necessary.  If they were 1, then the last access could only
 // have been read-only, and so not flush is necessary (also, another process
 // may STILL have a shared lock on the file, so a flush would therefor fail!).
 // If 0, then the standard flush operation is performed.

 PutMsg("\n");
 PutMsg("Unlocking the database (exclusive locks) ... ");
 if (isLocked) {
    LP[0].func = UNLOCK_XB;
    LP[1].func = UNLOCK_XB;
    rez = BULLET(&LP[0]);
    if (rez) {
       if (rez < 0) {
          rezX = abs(rez)-1;
          rez = LP[rezX].stat;
          sprintf(putStr,"Unlock failed on data, LP[%d], err: %d\n",rezX,rez);
          PutMsg(putStr);
       }
       else {
          rezX = rez-1;
          rez = LP[rezX].stat;
          sprintf(putStr,"Unlock failed on index, LP[%d], err: %d\n",rezX,rez);
          PutMsg(putStr);
       }
    }
 }

 PutMsg("Done.\n\n");

 // Note that if there is an error in the unlock process, you may need to
 // manually unlock those packs that lie beyond the failed pack.  Unlikely
 // is such to happen, though.

 if (rez==0) isLocked=0;

 // Close files (index files first then data (recommended but not required))

 PutMsg("Closing files ... ");
 if (indexID[0]) {
    HP.func = CLOSE_INDEX_XB;
    HP.handle = indexID[0];
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed first index file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }

 if (indexID[1]) {
    HP.func = CLOSE_INDEX_XB;
    HP.handle = indexID[1];
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed second index file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }

 if (dataID) {
    HP.func = CLOSE_DATA_XB;
    HP.handle = dataID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed data file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }
 PutMsg("Done.\n");
 return rez;  // module exit
}


//------------------------------------
// Init field list items for data file

void BuildFieldListLocks(FIELDDESCTYPE fieldList[]) {

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
