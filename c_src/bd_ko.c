
/* bd_ko.c - 30-Sep-1996 Cornel Huth
 * This module is called by bd_main.c
 * Key-only (uses external data (non-BULLET data file) key store)
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

int bd_ko(void);


// GO

int bd_ko(void) {

#pragma pack(1)

 ACCESSPACK AP;
 DOSFILEPACK DFP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
 OPENPACK OP;

#pragma pack()

 time_t startTime, endTime;
 int display = 0;                // display results or not flag
 int sameSequence = 1;           // use same sequence for keys or new flag
 int binaryKey = 0;              // store key as binary 32-bit flag

 LONG rndSeqVar = 0;             // used to construct on-the-fly key

 CHAR nameIX3[] = ".\\$bd_ko.ix3";  // name of index file created
 ULONG indexID=0;                // handle of index file
 CHAR keyExpression[128];        // key expression string buffer (159 max)
 CHAR keyBuffer[68];             // buffer used to store/receive key values

 LONG keys2add;                  // keys to store
 LONG i;                         // counter
 CHAR tmpStr[128];               // misc stuff, non-Bullet related
 CHAR putStr[128];               // message output string
 LONG rez;                       // return value from Bullet
 LONG dupHits = 0;               // duplicate keys generated from rand()


 // Delete previous files from any previous run (disregard any error return)

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = nameIX3;
 rez = BULLET(&DFP);

 strcpy(keyExpression,"EXTERNAL DATA SO NOT PARSED BUT IS STORED");

 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = nameIX3;
 CIP.keyExpPtr = keyExpression;
 CIP.xbLink = -1;                // must be -1 to use external data
                                 // sort by ASCII (or any valid sort-cmp func)
 CIP.sortFunction = (8 << 8) | ASCII_SORT;  // the 8 is the key size

 CIP.codePage = CODEPAGE;        // from bullet_2.h
 CIP.countryCode = CTRYCODE;     //
 CIP.collatePtr = NULL;          // since ASCII_SORT, no collate table is used
 CIP.nodeSize = 512;             // 512-byte node size (or 1024, 2048 bytes)
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 OP.func = OPEN_INDEX_XB;
 OP.filenamePtr = nameIX3;
 OP.asMode = READWRITE | DENYNONE; // must always specify a sharing mode
 OP.xbLink = -1;                   // must be -1 to use external data
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID = OP.handle;


 PutMsg("Display keys and key data (slower results)? (y/N) ");
 GetMsg(tmpStr);
 if (*tmpStr=='y') display = 1;

 PutMsg("           Repeat same random key sequence? (Y/n) ");
 GetMsg(tmpStr);
 if (*tmpStr=='n') sameSequence = 0;

 PutMsg("                          How many keys to store? ");
 GetMsg(tmpStr);
 keys2add = atol(tmpStr);
 if (keys2add < 1) keys2add = 1;       // would you rather end the test?
 if (keys2add > 9999999) keys2add = 1; // why wait around for so many?

 if (sameSequence)
    srand(1);
 else
    srand((unsigned)time(0));

 sprintf(putStr,"  Storing %d keys... ",keys2add);
 PutMsg(putStr);
 if (display) PutMsg("\n");

 time(&startTime);

 AP.func = STORE_KEY_XB;
 AP.handle = indexID;
 AP.keyPtr = keyBuffer;
 AP.recNo = 1;           // 32-bit data attached, start at 1 (why not)

 if (binaryKey) {
    PutMsg("binaryKey version is not coded\n");
 }
 else {
    for (i = 1; i <= keys2add; i++) {
       rndSeqVar = (rand() >> 3) * (rand() >> 3);   // 4095*4095=16769025 max
       sprintf(tmpStr,"%8.8i",rndSeqVar);
       strncpy(keyBuffer,tmpStr,8);

       rez = BULLET(&AP);
       if (rez) {

          // tried to insert a 'random' key that was already in the index; since
          // this is not INSERT_XB, dups are not automatically handled -- here
          // they're just skipped and another store is done to cover the total

          if (rez==EXB_KEY_EXISTS) {
             dupHits++;
             i--;                // do another key store (reuse AP.recNo)
          }
          else {
             sprintf(putStr,"Failed storing key %s  %d.  Err: %d\n",keyBuffer,i,rez);
             PutMsg(putStr);
             goto Abend;
          }
       }
       else {
           if (display) {
              sprintf(putStr,"%8.8s  %9.9lu\r", keyBuffer, AP.recNo);
              PutMsg(putStr);
           }
          AP.recNo++;
       }

#if FOR_WINDOWS == 1                      // deal with message queue every now and then
       if (i % Q_INS == 0) DoWinThing(0); // 0=no waiting around
#endif

    }

 }
 time(&endTime);
 if (display) PutMsg("\n...");

 sprintf(putStr,"took %u secs.",(endTime - startTime));
 PutMsg(putStr);

 if (dupHits) {
    sprintf(putStr," (had %d duplicates--ignored)\n",dupHits);
    PutMsg(putStr);
 }
 else {
    PutMsg("\n");
 }

 memset(keyBuffer,0,sizeof(keyBuffer)); // gives the \0 to the returned key

 sprintf(putStr,"Accessing %d keys in order... ",keys2add);
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
       sprintf(putStr,"%8.8s  %9.9lu\r", keyBuffer, AP.recNo);
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

 // expected rez is EXB_END_OF_FILE

 if (rez == EXB_END_OF_FILE) rez=0;
 if (rez) {
    sprintf(putStr,"Failed KEY access.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 sprintf(putStr,"took %u secs. for %d keys\n",(endTime - startTime),i);
 PutMsg(putStr);


 // Fatal errors above come straight to here
 Abend:

 // Close file

 if (indexID) {
    HP.func = CLOSE_INDEX_XB;
    HP.handle = indexID;
    rez = BULLET(&HP);
    if (rez) {
       sprintf(putStr,"Failed index file close.  Err: %d\n",rez);
       PutMsg(putStr);
    }
 }
 return rez;  // module exit
}
