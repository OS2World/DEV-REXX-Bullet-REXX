
/* bd_memo.c -  2-Oct-1996 Cornel Huth
 * This module is called by bd_main.c
 * memo fields
 */

#include "platform.h"

// external to this module

void PutMsg(CHAR *strg);
void GetMsg(CHAR *strg);

extern CHAR *collateTable;


// public in this module

int bd_memo(void);


// GO

int bd_memo(void) {

 void BuildEmpFieldListMemo(FIELDDESCTYPE fieldList[]);

 typedef struct _EmpRecType {
 CHAR tag;               // record tag, init to SPACE, * means deleted
 CHAR empID[9];          // SSN (not 0T string)
 CHAR empLN[16];         // last name
 CHAR empFN[16];         // first name
 CHAR empHire[8];        // "YYYYMMDD" (not 0T string)
 CHAR empNotes[10];      // a memo field, called Notes
 } EmpRecType; // 60 bytes

 // Sample data records for the database EMP
 // 10-space data at end is the blank memo field value (means no memo yet)
 // C++ probably doesn't like these exact-fit, non-zero-terminating strings

 EmpRecType empSampleRecords2[] = {
 //   123456789   1234567890123456   1234567890123456   12345678   123456
 ' ',"465309999","Que",             "Barbie",          "19900131","          ",
 ' ',"445038888","Stewart",         "Jackie",          "19910228","          ",
 ' ',"760443232","Whitman",         "Kelly",           "19920414","          ",
 ' ',"845309944","Beatty",          "Leslie",          "19940122","          ",
 ' ',"555033388","Jasper",          "Amy",             "19930230","          ",
 ' ',"430443222","Hauntos",         "Poco",            "19920414","          ",
 ' ',"365502949","Hopkins",         "Lisa",            "19910121","          ",
 ' ',"685733868","Leonard",         "Rosina",          "19850218","          ",
 ' ',"500945242","Morton",          "Holly",           "19950406","          ",
 ' ',"335209939","Buckly",          "Lois",            "19930715","          ",
 ' ',"745338218","Parker",          "Angie",           "19940412","          ",
 ' ',"860461892","Sosa",            "Rhoda",           "19940623","          ",
 ' ',"225374865","Jefferson",       "Weezie",          "19941106","          ",
 ' ',"115036578","Chung",           "Connie",          "19941205","          ",
 ' ',"240443355","Baker",           "Rosinda",         "19940304","          ",
 };

 // Sample memos for the database EMP.dbt
 // Anything goes

 CHAR *empSampleMemos[] = {
 "This is memo sample number uno",
 "This would then be number two",
 "And number three",
 "Number four, too",
 "Don't forget memo number five!",
 "Then comes memo six",
 "And since seven follows six, memo seven",
 "Eight is close to the last memo",
 "But nine is even close to the last memo",
 "Ten, now this is the last memo",
 "Well, no, that wasn't -- this is eleven",
 "Memo twelve is this",
 "Thirteen here, or is this unlucky?",
 "Memo Fourteen, now we're getting some where",
 "Memo Fifteen, this is the last (one for each employee record)",
 };

#pragma pack(1)

 ACCESSPACK AP;
 DOSFILEPACK DFP;
 CREATEDATAPACK CDP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
 MEMODATAPACK MDP;
 OPENPACK OP;

 CHAR indexFilename[] = ".\\$bd_memo.ix3";
 ULONG indexID=0;

 CHAR keyExpression[] = "SSN";
 CHAR keyBuffer[68];

 CHAR dataFilename[] = ".\\$bd_memo.dbf";
 ULONG dataID=0;

 // the memo file is created implicitly when the DBF is, but to ease deleting
 // the memo file at program startup, the DBT (as it would be by default)
 // is set here

 CHAR memoFilename[] = ".\\$bd_memo.dbt";

 FIELDDESCTYPE empFieldList[5];  // 5 fields used in Employee data record
 EmpRecType empRec;

#pragma pack()

 LONG rez;               // return value from Bullet
 LONG i;                 // counter
 CHAR tmpStr[256];       // misc stuff, non-Bullet related (so far as not needing pack() goes)
 CHAR putStr[128];

 LONG empRecs2Add = sizeof(empSampleRecords2) / sizeof(empRec);

 PutMsg("Memo file example (sort order is by SSN, not in memo number order)\n\n");

 // Assign fieldlist members (after first zeroing)

 memset(empFieldList,0,sizeof(empFieldList));
 BuildEmpFieldListMemo(empFieldList);

 // Delete previous files from any previous run (disregard any error return)

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = dataFilename;
 rez = BULLET(&DFP);
 DFP.filenamePtr = memoFilename;
 rez = BULLET(&DFP);
 DFP.filenamePtr = indexFilename;
 rez = BULLET(&DFP);

 // Create the data files

 CDP.func = CREATE_DATA_XB;
 CDP.filenamePtr = dataFilename;
 CDP.noFields = 5;
 CDP.fieldListPtr = empFieldList;
 CDP.fileID = 0x8B;              // bit7&3=1 then also create memo file
 rez = BULLET(&CDP);
 if (rez) {
    sprintf(putStr,"Failed EMP data file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Open the data file

 OP.func = OPEN_DATA_XB;
 OP.filenamePtr = dataFilename;
 OP.asMode = READWRITE | DENYNONE;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed EMP data file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 dataID = OP.handle;

 // Create index file

 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = indexFilename;
 CIP.keyExpPtr = keyExpression;
 CIP.xbLink = dataID;
 CIP.sortFunction = ASCII_SORT;
 CIP.codePage = CODEPAGE;
 CIP.countryCode = CTRYCODE;
 CIP.collatePtr = NULL;
 CIP.nodeSize = 512;
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed EMP SSN index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Open the index file

 OP.func = OPEN_INDEX_XB;
 OP.filenamePtr = indexFilename;
 OP.asMode = READWRITE | DENYNONE;
 OP.xbLink = dataID;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed SSN index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID = OP.handle;

 // Insert into the EMP data file, adding the memo record in the process

 AP.func = INSERT_XB;
 AP.handle = indexID;
 AP.keyPtr = keyBuffer;
 AP.nextPtr = NULL;

 MDP.func = ADD_MEMO_XB;
 MDP.dbfHandle = dataID;   // this remains the same, as does the song

 for (i=0;i < empRecs2Add;i++) {

    // it may seem odd to add a memo before its DBF record exists, but
    // this is not so odd -- if you were adding a new employee record,
    // say, you'd enter the info via some form of dialog, getting the
    // standard record info (DBF field data) and also any memo data
    // (comments, other non-field data), and then post that data to the
    // database.  Since the DBF record's memo field needs the memo number
    // (which is returned from the ADD_MEMO_XB call), it only makes sense
    // to first add the memo, then fill in the DBF field's memo number
    // and then INSERT_XB the employee record (DBF, the DBT having already
    // been written to disk) -- and that is what this does

    MDP.memoPtr = empSampleMemos[i];
    MDP.memoBytes = strlen(empSampleMemos[i])+1;  // +1 so it stores \0, too
    rez = BULLET(&MDP);

    if (rez!=0) {
       sprintf(putStr,"ADD_MEMO_XB #%d failed, err: %d\n",i,MDP.stat);
       PutMsg(putStr);
       goto Abend;
    }

    sprintf(tmpStr,"%10.10u",MDP.memoNo);  // "0000000001" is first memo...
    strncpy(empSampleRecords2[i].empNotes,tmpStr,10);  // put memo number in

    AP.recNo = 0;
    AP.recPtr = &empSampleRecords2[i];

    rez = BULLET(&AP);
    if (rez!=0) {
       if (rez < 0) {
          rez = abs(rez);
          sprintf(putStr,"INSERT_XB #%d failed, data pack# %d, err: %d\n",i,rez,AP.stat);
          PutMsg(putStr);
       }
       else {
          sprintf(putStr,"INSERT_XB #%d failed, index pack# %d, err: %d\n",i,rez,AP.stat);
          PutMsg(putStr);
       }
       goto Abend;
    }
 }

 // Shows a view on the EMP table, along with memo notes (first 30 or so bytes)
 // This is just a crude output method -- the memo routines do return the
 // size of each memo data, and so on -- enough info to manage them quite well,
 // much better than can be done in dBASE

 AP.func = GET_FIRST_XB;
 AP.handle = indexID;
 AP.recPtr = &empRec;
 AP.keyPtr = keyBuffer;


 // for demo use, get bytes of memo data
 // ====================================
 // ===========================


 //MDP.func = GET_MEMO_XB; // no real point setting this now, since it's changed in the loop
 MDP.memoPtr = tmpStr;     // read memo into this (other MDP. parms already set)
 MDP.memoOffset = 0;       // starting at first byte of memo

 i=0;
 //       123456789 1234567890123456 1234567890123456 12345678 123456789012345...
 PutMsg(" EMP.SSN   LNAME            FNAME            HIRED    NOTES (memo, '>' added)\n");
 rez=BULLET(&AP);
 while (rez==0) {

    // don't want to use atol(empRec.empNotes) since this field is not zero-terminated
    // and fills the 10 bytes completely -- use sscanf(), or atol() on left 10 bytes
    // MDP.memoNo = atol(empRec.empNotes); // scratch this

    sscanf(empRec.empNotes,"%10u",&MDP.memoNo);

    // MDP.memoNo set from above, get number of bytes of real data of this memo record
    // this size returned INCLUDES the 0T since that's the way it was written above
    // as:  MDP.memoBytes = strlen(empSampleMemos[i])+1;  // +1 so it stores \0, too

    MDP.func = GET_MEMO_SIZE_XB;
    rez = BULLET(&MDP);
    if (rez!=0) {
       sprintf(putStr,"GET_MEMO_SIZE_XB #%d failed, err: %d\n",i,MDP.stat);
       PutMsg(putStr);
       goto Abend;
    }

    // MDP.memoBytes set from above, .memoPtr/Offset from several lines up

    MDP.func = GET_MEMO_XB;
    // other members already setup
    rez = BULLET(&MDP);
    if (rez!=0) {
       sprintf(putStr,"GET_MEMO_XB #%d failed, err: %d\n",MDP.memoNo,MDP.stat);
       PutMsg(putStr);
       goto Abend;
    }

    // tmpStr has the memo record, including the 0T that was written originally
    // only the first 24 characters of the memo record are printed (with an
    // added > to show that more follow, and to mark the end of shorter ones)

    sprintf(putStr," %9.9s %-16.16s %-16.16s %8.8s %-.24s>\n",
            empRec.empID,
            empRec.empLN,
            empRec.empFN,
            empRec.empHire,
            tmpStr);
    PutMsg(putStr);

    if (rez==0) {
       i++;
       AP.func = GET_NEXT_XB;
       rez=BULLET(&AP);
    }
    
 }
 if (rez == EXB_END_OF_FILE) rez=0; // expected is EXB_END_OF_FILE
 if (rez) {
    sprintf(putStr,"(SSN) Failed EMP view #%d, err: %d\n",i,rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Fatal errors above come straight to here
 Abend:

 // Close files
 // closing the data file closes its memo file

 HP.func = CLOSE_INDEX_XB;
 if (indexID) {
    HP.handle = indexID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed index #%d file close.  Err: %d\n",i,rez);
       PutMsg(putStr);
    }
 }

 HP.func = CLOSE_DATA_XB;
 if (dataID) {
    HP.handle = dataID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed data #%d file close.  Err: %d\n",i,rez);
       PutMsg(putStr);
    }
 }

 return rez;
}


//---------------------------------------------
// Init field list items for employee data file

void BuildEmpFieldListMemo(FIELDDESCTYPE fieldList[]) {

 strcpy(fieldList[0].fieldName, "SSN");
 fieldList[0].fieldType = 'C';
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
 fieldList[3].fieldLen = 8;
 fieldList[3].fieldDC = 0;

 strcpy(fieldList[4].fieldName, "NOTES");
 fieldList[4].fieldType = 'M';
 fieldList[4].fieldLen = 10;
 fieldList[4].fieldDC = 0;
 return;
}
