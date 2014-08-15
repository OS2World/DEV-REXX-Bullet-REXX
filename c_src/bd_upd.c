
/* bd_upd.c -  1-Oct-1996 Cornel Huth
 * This module is called by bd_main.c
 * UPDATE_XB
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

int bd_upd(void);

// GO

int bd_upd(void) {

void BuildFieldListUpd(FIELDDESCTYPE fieldList[]);

#pragma pack(1)

 ACCESSPACK AP;
 DOSFILEPACK DFP;
 CREATEDATAPACK CDP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
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

 time_t startTime, endTime;
 int display = 0;                // display results or not flag
 int sameSequence = 1;           // use same random seed each time
 int rndSeqVar = 0;              // random "key" generated (9 digits worth)
 int doOptimize = 1;             // optimize index prior update and key access
 LONG rez;                       // return value from Bullet

 CHAR nameIX3[] = ".\\$bd_upd.ix3"; // name of index file created
 ULONG indexID=0;                // handle of index file
 CHAR keyExpression[128];        // key expression string buffer (159 max)
 CHAR keyBuffer[68];             // buffer used to store/receive key values

 CHAR nameData[] = ".\\$bd_upd.dbf";// name of data file created
 ULONG dataID=0;                 // handle of data file
 FIELDDESCTYPE fieldList[5];     // 5 fields used in data record

 LONG recs2add;          // records to add
 LONG i;                 // counter
 LONG recNo;             // counter
 CHAR tmpStr[128];       // misc stuff, non-Bullet related
 CHAR putStr[128];

 PutMsg("UPDATE_XB, an all-or-nothing transaction update into up to\n");
 PutMsg("256 files from a single call.\n\n");

 // Implicit is the tag field at the first byte, then comes...

 memset(fieldList,0,sizeof(fieldList));  // init unused bytes to 0 (required)
 BuildFieldListUpd(fieldList);

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
 // This example uses a single, 9-byte key -- the SSN.

 strcpy(keyExpression,"SSN");

 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = nameIX3;
 CIP.keyExpPtr = keyExpression;
 CIP.xbLink = dataID;            // the handle of the data file
 CIP.sortFunction = ASCII_SORT;  // sort by ASCII order (fine for text numbers)
 CIP.codePage = CODEPAGE;        // code page
 CIP.countryCode = CTRYCODE;     // country code
 CIP.collatePtr = NULL;          // since ASCII_SORT, no collate table
 CIP.nodeSize = 512;             // 512-byte node size (or 1024, 2048 bytes)
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }


 // Open the index file (what we just created above).

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

 PutMsg("           Repeat same random key sequence? (Y/n) ");
 GetMsg(tmpStr);
 if (*tmpStr=='n') sameSequence = 0;

 PutMsg("               Optimize index after update? (Y/n) ");
 GetMsg(tmpStr);
 if (*tmpStr=='n') doOptimize = 0;

 PutMsg("  How many records do you want for this test run? ");
 GetMsg(tmpStr);
 recs2add = atol(tmpStr);
 if (recs2add < 1) recs2add = 1;  // would you rather end the test?
 if (recs2add > 9999999) recs2add = 1; // why wait around for 10M?

 // Add the data records, which are created here, on-the-fly, varying enough
 // to make unique records.  The key is inserted for each record added.

 EmpRec.tag = ' ';                       // set to not-deleted
 strncpy(EmpRec.empID,"000000000",9);    // the key.field
 strcpy(EmpRec.empLN,"YourLastName");    // everyone has the same last name
 strcpy(EmpRec.empFN,"YourFirstName");   // everyone has this first name!
 strncpy(EmpRec.empHire,"19950618",8);   // YYYYMMDD DBF form, no \0 on date
 strcpy(EmpRec.empDept,"MIS");           // everyone works for MIS!

 // The record construction creates a pretty random SSN number (.empID)
 // and that is used as the key -- the key is kept unique (i.e., duplicate
 // .empID values are not allowed).  In case of a duplicate (likely given
 // the SSN generation technque), the changes made to the database are
 // backed out by BULLET.  Another insert is made to cover the count requested.

 if (sameSequence)
    srand(1);
 else
    srand((unsigned)time(0));

 sprintf(putStr,"Inserting %d recs/keys... ",recs2add);
 PutMsg(putStr);
 time(&startTime);

 AP.func = INSERT_XB;
 AP.handle = indexID;
 AP.keyPtr = keyBuffer;
 AP.recPtr = &EmpRec;
 AP.nextPtr = NULL;

 for (i = 1; i <= recs2add; i++) {

    rndSeqVar = (rand() >> 1) * (rand() >> 2);   // 16383*8191=134193153 max
    sprintf(tmpStr,"%9.9i",rndSeqVar);
    strncpy(EmpRec.empID,tmpStr,9);              // update SSN

    AP.recNo = 0;

    rez = BULLET(&AP);
    if (rez) {

       // negative rc means failed during data record add

       if (rez < 0) {
          rez = AP.stat;
          sprintf(putStr,"Failed during data portion, sequence %d.  Err: %d\n",i,rez);
          PutMsg(putStr);
          goto Abend;
       }

       // otherwise during index key insert

       else {
          rez = AP.stat;
          if (rez == EXB_KEY_EXISTS) {
             rez = 0;
             i--;
          }
          else {
             sprintf(putStr,"Failed during index portion, sequence %d.  Err: %d\n",i,rez);
             PutMsg(putStr);
             goto Abend;
          }
       }
    }

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_INS == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);
 sprintf(putStr,"took %u secs.\n",(endTime - startTime));
 PutMsg(putStr);

 PutMsg("\nUpdating the entire database by adding 200000000 to key field\n");
 sprintf(putStr," Updating %d recs/keys... ",recs2add);
 PutMsg(putStr);
 time(&startTime);

 recNo=1;
 AP.func = GET_RECORD_XB;        // direct access, see above notes
 AP.handle = dataID;             // GET_RECORD_XB uses only the data file
 AP.recNo = recNo;               // get the first record to start
 AP.keyPtr = keyBuffer;          // ...to be used by UPDATE_XB
 AP.recPtr = &EmpRec;
 AP.nextPtr = NULL;
 rez = BULLET(&AP);
 i=0;

 while (rez==0) {

    // got the data record, force a key update by changing key field
    // all this does is change the first digit of the nine, same effect
    // as adding 200000000

    *(EmpRec.empID)+=2;

    AP.func = UPDATE_XB;
    AP.handle = indexID;
    rez = BULLET(&AP);
    if (rez) {
       if (rez < 0) {
          rez = AP.stat;
          sprintf(putStr,"Failed data update, rec#: %d. Err: %d\n",AP.recNo,rez);
          PutMsg(putStr);
          goto Abend;
       }

       // otherwise during index key update

       else {
          rez = AP.stat;
          sprintf(putStr,"Failed index update, rec#: %d. Err: %d\n",AP.recNo,rez);
          PutMsg(putStr);
          goto Abend;
       }
    }
    i++;

    // change for GET_RECORD_XB...

    AP.func = GET_RECORD_XB;
    AP.handle = dataID;
    AP.recNo=++recNo;
    rez = BULLET(&AP);

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_UPD == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);

 // expected error is EXB_BAD_RECNO

 if (rez == EXB_BAD_RECNO) rez=0;
 if (rez) {
    sprintf(putStr,"Failed GET_RECORD_XB at %d.  Err: %d\n",AP.recNo,rez);
    PutMsg(putStr);
    goto Abend;
 }
 sprintf(putStr,"took %u secs. for %d updates\n",(endTime - startTime),i);
 PutMsg(putStr);

 // all records now are > 200000000, as are their keys

 if (doOptimize) {
    PutMsg("Optimizing index... (no callback in this example)");
    AP.func = REINDEX_XB;        // this is all there is to reindexing even...
    AP.handle = indexID;         // ...million-record databases
    AP.keyPtr = keyBuffer;       // if dup key error...(see manual for details)
    AP.nextPtr = NULL;           // just this one index file
    rez = BULLET(&AP);
    if (rez)  {
       rez = AP.stat;
       sprintf(putStr,"Failed optimize (reindex).  Err: %d\n",rez);
       PutMsg(putStr);
       goto Abend;
    }
    PutMsg("Done\n");
 }

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
    i++;
    if (display) {
       sprintf(putStr,"%s  %9.9u\r", keyBuffer, AP.recNo);
       PutMsg(putStr);
    }
    AP.func = NEXT_KEY_XB;
    rez = BULLET(&AP);

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_KEY == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);
 if (display) PutMsg("\n...");

 // expected rez is ERR_END_OF_FILE

 if (rez == EXB_END_OF_FILE) rez=0;
 if (rez) {
    sprintf(putStr,"Failed KEY access.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 sprintf(putStr,"took %u secs. for %d keys\n",(endTime - startTime),i);
 PutMsg(putStr);

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
    i++;
    if (display) {
       sprintf(putStr,"%s  %9.9u   %s\r", keyBuffer, AP.recNo, &EmpRec);
       PutMsg(putStr);
    }
    AP.func = GET_NEXT_XB;
    rez = BULLET(&AP);

#if FOR_WINDOWS == 1                   // deal with message queue every now and then
    if (i % Q_GET == 0) DoWinThing(0); // 0=no waiting around
#endif

 }
 time(&endTime);
 if (display) PutMsg("\n...");

 // expected rez is ERR_END_OF_FILE

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

 if (dataID) {
    HP.func = CLOSE_DATA_XB;
    HP.handle = dataID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed data file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }

 return rez;
}


//------------------------------------
// Init field list items for data file

void BuildFieldListUpd(FIELDDESCTYPE fieldList[]) {

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
 fieldList[3].fieldLen = 8;      // date field type must be 8.0
 fieldList[3].fieldDC = 0;

 strcpy(fieldList[4].fieldName, "DEPT");
 fieldList[4].fieldType = 'C';
 fieldList[4].fieldLen = 6;
 fieldList[4].fieldDC = 0;
 return;
}
