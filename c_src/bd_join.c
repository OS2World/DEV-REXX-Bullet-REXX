
/* bd_join.c -  2-Oct-1996 Cornel Huth
 * This module is called by bd_main.c
 * multiple table view, join
 */

#include "platform.h"

// external to this module

void PutMsg(CHAR *strg);
void GetMsg(CHAR *strg);

extern CHAR *collateTable;
extern CHAR gCLS[6];

// public in this module

int bd_join(void);


// GO

int bd_join(void) {

 void BuildEmpFieldList(FIELDDESCTYPE fieldList[]);
 void BuildDptFieldList(FIELDDESCTYPE fieldList[]);

 // structures below are byte-aligned by virtue of being of all char type
 typedef struct _EmpRecType {
 CHAR tag;               // record tag, init to SPACE, * means deleted
 CHAR empID[9];          // SSN (not 0T string)
 CHAR empLN[16];         // last name
 CHAR empFN[16];         // first name
 CHAR empHire[8];        // "YYYYMMDD" (not 0T string)
 CHAR empDept[6];        // department assigned
 } EmpRecType; // 56 bytes

 typedef struct _DptRecType {
 CHAR tag;               // record tag, init to SPACE, * means deleted
 CHAR dptID[6];          // department (same format as empDept)
 CHAR dptName[16];       // department name
 CHAR dptMgrID[9];       // manager of (same format as empID)
 CHAR dptNumber[4];      // number of employees assigned to department
 } DptRecType; // 36 bytes

 // Sample data records for the database EMP-DEPT
 // C++ probably doesn't like these exact-fit, non-zero-terminating strings

 EmpRecType empSampleRecords[] = {
 //   123456789   1234567890123456   1234567890123456   12345678   123456
 ' ',"465309999","Que",             "Barbie",          "19900131","BOSS",
 ' ',"445038888","Stewart",         "Jackie",          "19910228","ACC",
 ' ',"760443232","Whitman",         "Kelly",           "19920414","HUM",
 ' ',"845309944","Beatty",          "Leslie",          "19940122","PRG",
 ' ',"555033388","Jasper",          "Amy",             "19930230","PRG",
 ' ',"430443222","Hauntos",         "Poco",            "19920414","PRG",
 ' ',"365502949","Hopkins",         "Lisa",            "19910121","PRG",
 ' ',"685733868","Leonard",         "Rosina",          "19850218","PRG",
 ' ',"500945242","Morton",          "Holly",           "19950406","PSY",
 ' ',"335209939","Buckly",          "Lois",            "19930715","GO4",
 ' ',"745338218","Parker",          "Angie",           "19940412","MKT",
 ' ',"860461892","Sosa",            "Rhoda",           "19940623","R&D",
 ' ',"225374865","Jefferson",       "Weezie",          "19941106","R&D",
 ' ',"115036578","Chung",           "Connie",          "19941205","PRG",
 ' ',"240443355","Baker",           "Rosinda",         "19940304","PRG",
 };

 DptRecType dptSampleRecords[] = {
 //   123456   1234567890123456   123456789   1234
 ' ',"BOSS",  "40th Floor",      "465309999","   1",
 ' ',"GO4",   "Secretarial",     "335209939","   1",
 ' ',"PRG",   "Programming",     "845309944","   7",
 ' ',"R&D",   "Research & Dev",  "860461892","   2",
 ' ',"MKT",   "Marketing",       "745338218","   1",
 ' ',"ACC",   "Accounting",      "445038888","   1",
 ' ',"PSY",   "Psycho Healers",  "500945242","   1",
 ' ',"HUM",   "Human Resources", "760443232","   1",
 };


#pragma pack(1)         // recommended around Bullet-accessed data

 ACCESSPACK AP[3];
 DOSFILEPACK DFP;
 CREATEDATAPACK CDP;
 CREATEINDEXPACK CIP;
 HANDLEPACK HP;
 OPENPACK OP;

 CHAR *indexFilename[] = {
 ".\\$bd_jnss.ix3",        // employee SSN index
 ".\\$bd_jnns.ix3",        // employee Last name and SSN last-four index
 ".\\$bd_jnid.ix3"         // department ID index
 };

 ULONG indexID[] ={0,0,0};

 CHAR *keyExpression[] = {
 "SSN",
 "SUBSTR(LNAME,1,4)+SUBSTR(SSN,6,4)",
 "DEPT_ID"
 };

 // The max key length Bullet uses is 64 bytes (this is a big key)
 // I've sized the arrays at 68 bytes so that I can append a 0T
 // if I want to make the key (if it ever were exactly 64 bytes)
 // a C string (by appending a 0T, of course).

 CHAR *keyBuffer[3][68];

 CHAR *dataFilename[] = {
 ".\\$bd_jnem.dbf",        // employee data
 ".\\$bd_jndp.dbf"         // department data
 };

 ULONG dataID[] ={0,0};

 FIELDDESCTYPE empFieldList[5];  // 5 fields used in Employee data record
 EmpRecType empRec;

 FIELDDESCTYPE dptFieldList[4];  // 4 fields used in Department data record
 DptRecType dptRec;

#pragma pack()

 LONG rez;               // return value from Bullet
 LONG i;                 // counter
 CHAR tmpStr[128];       // misc stuff, non-Bullet related
 CHAR putStr[128];

 LONG empRecs2Add = sizeof(empSampleRecords) / sizeof(empRec);
 LONG dptRecs2Add = sizeof(dptSampleRecords) / sizeof(dptRec);

 // Clap for the Wolfman

 PutMsg(gCLS);
 PutMsg("Employee-Department database using two data files & three index files\n\n");

 // Assign fieldlist members (after first zeroing)

 memset(empFieldList,0,sizeof(empFieldList));
 BuildEmpFieldList(empFieldList);
 memset(dptFieldList,0,sizeof(dptFieldList));
 BuildDptFieldList(dptFieldList);

 // Delete previous files from any previous run (disregard any error return)

 DFP.func = DELETE_FILE_DOS;
 DFP.filenamePtr = dataFilename[0];
 rez = BULLET(&DFP);
 DFP.filenamePtr = dataFilename[1];
 rez = BULLET(&DFP);

 for (i=0;i<3;i++) {
    DFP.filenamePtr = indexFilename[i];
    rez = BULLET(&DFP);
 }

 // Create the data files

 CDP.func = CREATE_DATA_XB;
 CDP.filenamePtr = dataFilename[0];
 CDP.noFields = 5;
 CDP.fieldListPtr = empFieldList;
 CDP.fileID = 0x03;
 rez = BULLET(&CDP);
 if (rez) {
    sprintf(putStr,"Failed EMP data file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 CDP.filenamePtr = dataFilename[1];
 CDP.noFields = 4;
 CDP.fieldListPtr = dptFieldList;
 rez = BULLET(&CDP);
 if (rez) {
    sprintf(putStr,"Failed DPT data file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Open the data files

 OP.func = OPEN_DATA_XB;
 OP.filenamePtr = dataFilename[0];
 OP.asMode = READWRITE | DENYNONE;
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed EMP data file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 dataID[0] = OP.handle;

 OP.filenamePtr = dataFilename[1];
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed DPT data file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 dataID[1] = OP.handle;

 // Create index files
 // First two index EMP data file, third indexes DPT data file

 CIP.func = CREATE_INDEX_XB;
 CIP.filenamePtr = indexFilename[0];
 CIP.keyExpPtr = keyExpression[0];
 CIP.xbLink = dataID[0];
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

 CIP.filenamePtr = indexFilename[1];
 CIP.keyExpPtr = keyExpression[1];
 CIP.xbLink = dataID[0];
 CIP.sortFunction = NLS_SORT | SORT_SET;
 CIP.codePage = CODEPAGE;
 CIP.countryCode = CTRYCODE;
 CIP.collatePtr = collateTable;
 CIP.nodeSize = 512;
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed EMP NAME index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 CIP.filenamePtr = indexFilename[2];
 CIP.keyExpPtr = keyExpression[2];
 CIP.xbLink = dataID[1];
 CIP.sortFunction = ASCII_SORT;
 CIP.codePage = CODEPAGE;
 CIP.countryCode = CTRYCODE;
 CIP.collatePtr = NULL;
 CIP.nodeSize = 512;
 rez = BULLET(&CIP);
 if (rez) {
    sprintf(putStr,"Failed DPT index file create.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Open the index files

 OP.func = OPEN_INDEX_XB;
 OP.filenamePtr = indexFilename[0];
 OP.asMode = READWRITE | DENYNONE;
 OP.xbLink = dataID[0];
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed SSN index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID[0] = OP.handle;

 OP.filenamePtr = indexFilename[1];
 OP.xbLink = dataID[0];
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed EMP NAME index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID[1] = OP.handle;

 OP.filenamePtr = indexFilename[2];
 OP.asMode = READWRITE | DENYNONE;
 OP.xbLink = dataID[1];
 rez = BULLET(&OP);
 if (rez) {
    sprintf(putStr,"Failed DPT index file open.  Err: %d\n",rez);
    PutMsg(putStr);
    goto Abend;
 }
 indexID[2] = OP.handle;

 // Insert into the database.  This example has separate inserts for the
 // employee file and the department file.  These could be done together,
 // with just the single call, however, since this is done in a loop, for
 // the number of sample records of each (different counts), and since an
 // insert into EMP is distinct from an insert into DPT (not dependent on
 // each other), it does not matter that they are done separately -- and
 // actually, makes more sense to do so, considering that they are distinct.

 // Insert into the EMP data file (two index, one data)

 AP[0].func = INSERT_XB;
 AP[0].handle = indexID[0];
 AP[0].keyPtr = keyBuffer[0];
 AP[0].nextPtr = &AP[1];
 AP[1].func = INSERT_XB;
 AP[1].handle = indexID[1];
 AP[1].keyPtr = keyBuffer[1];
 AP[1].nextPtr = NULL;

 for (i=0;i < empRecs2Add;i++) {
    AP[0].recNo = 0;
    AP[0].recPtr = &empSampleRecords[i];
    AP[1].recNo = 0x80000000;
    AP[1].recPtr = &empSampleRecords[i];
    rez = BULLET(&AP[0]);
    if (rez!=0) break;
 }
 if (rez) {
    if (rez < 0) {
       rez = abs(rez);
       sprintf(putStr,"INSERT_XB #%d failed, data pack# %d, err: %d\n",i,rez,AP[rez-1].stat);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"INSERT_XB #%d failed, index pack# %d, err: %d\n",i,rez,AP[rez-1].stat);
       PutMsg(putStr);
    }
    goto Abend;
 }

 // Insert into the DPT data file (one index, one data)

 AP[2].func = INSERT_XB;         // invariants out of loop
 AP[2].handle = indexID[2];
 AP[2].keyPtr = keyBuffer[2];
 AP[2].nextPtr = NULL;

 for (i=0;i < dptRecs2Add;i++) {
    AP[2].recNo = 0;
    AP[2].recPtr = &dptSampleRecords[i];
    rez = BULLET(&AP[2]);
    if (rez!=0) break;
 }
 if (rez) {
    if (rez < 0) {
       sprintf(putStr,"INSERT_XB #%d failed, data pack# %d, err: %d\n",i,rez,AP[2].stat);
       PutMsg(putStr);
    }
    else {
       sprintf(putStr,"INSERT_XB #%d failed, index pack# %d, err: %d\n",i,rez,AP[2].stat);
       PutMsg(putStr);
    }
    goto Abend;
 }

 // Shows a view on the two tables, EMP-DPT, so that all employee info is
 // shown, along with the department info that that employee is assigned.
 // EMP.DEPT_ID (datafile.fieldname) is a foreign key into DPT.DEPT_ID, and
 // so EMP.DEPT_ID for each EMP record is joined with the DPT info for that
 // department.  First in SSN order, then in LNAME+last-4 order.  After this,
 // a view on DPT-EMP is shown, listing the managers of each department, and
 // their EMP info.

 AP[0].func = GET_FIRST_XB;      // using AP[0] since it's convenient to do so
 AP[0].handle = indexID[0];
 AP[0].recPtr = &empRec;
 AP[0].keyPtr = keyBuffer[0];

 AP[2].func = GET_EQUAL_XB;      // accessing DPT by foreign key (exact) first
 AP[2].handle = indexID[2];
 AP[2].recPtr = &dptRec;
 AP[2].keyPtr = keyBuffer[2];

 i=0;
 //       123456789 1234567890123456 1234567890123456 12345678 1234567890123456
 PutMsg(" EMP.SSN   LNAME            FNAME            HIRED    DEPARTMENT\n");
 rez=BULLET(&AP[0]);
 while (rez==0) {
    memcpy(keyBuffer[2],empRec.empDept,6);       // foreign key to key buffer
    rez=BULLET(&AP[2]);                          // and get it to dptRec
    if (rez!=0) strcpy(dptRec.dptName,"Error!");

    sprintf(putStr," %9.9s %-16.16s %-16.16s %8.8s %-16.16s\n",
            empRec.empID,
            empRec.empLN,
            empRec.empFN,
            empRec.empHire,
            dptRec.dptName);
    PutMsg(putStr);

    if (rez==0) {
       i++;
       AP[0].func = GET_NEXT_XB;
       rez=BULLET(&AP[0]);
    }
 }
 if (rez == EXB_END_OF_FILE) rez=0; // expected is ERR_END_OF_FILE
 if (rez) {
    sprintf(putStr,"(SSN) Failed EMP-DPT view #%d, err: %d\n",i,rez);
    PutMsg(putStr);
    goto Abend;
 }

 PutMsg("\nThat was in SSN order.  Press <Enter> for LNAME+last-4 order... ");
 GetMsg(tmpStr);
 PutMsg(gCLS);

 // now the same thing, but in LNAME+last-4 order

 AP[1].func = GET_FIRST_XB;      // using AP[1] since it's convenient to do so
 AP[1].handle = indexID[1];
 AP[1].recPtr = &empRec;
 AP[1].keyPtr = keyBuffer[1];

 AP[2].func = GET_EQUAL_XB;      // accessing DPT by foreign key (exact) first
 AP[2].handle = indexID[2];
 AP[2].recPtr = &dptRec;
 AP[2].keyPtr = keyBuffer[2];

 i=0;
 //       123456789 1234567890123456 1234567890123456 12345678 1234567890123456
 PutMsg(" EMP.SSN   LNAME            FNAME            HIRED    DEPARTMENT\n");
 rez=BULLET(&AP[1]);
 while (rez==0) {
    memcpy(keyBuffer[2],empRec.empDept,6);       // foreign key to key buffer
    rez=BULLET(&AP[2]);                          // and get it to dptRec
    if (rez!=0) strcpy(dptRec.dptName,"Error!");

    sprintf(putStr," %9.9s %-16.16s %-16.16s %8.8s %-16.16s\n",
            empRec.empID,
            empRec.empLN,
            empRec.empFN,
            empRec.empHire,
            dptRec.dptName);
    PutMsg(putStr);

    if (rez==0) {
       i++;
       AP[1].func = GET_NEXT_XB;
       rez=BULLET(&AP[1]);
    }
 }
 if (rez == EXB_END_OF_FILE) rez=0; // expected is ERR_END_OF_FILE
 if (rez) {
    sprintf(putStr,"(LNAME) Failed EMP-DPT view #%d, err: %d\n",i,rez);
    PutMsg(putStr);
    goto Abend;
 }

 // now for something a little different... the managers' info

 PutMsg("\nThose were EMP-DPT views.  Press <Enter> for DPT-EMP view (DEPT_ID order)...");
 GetMsg(tmpStr);
 PutMsg(gCLS);

 AP[2].func = GET_FIRST_XB;
 AP[2].handle = indexID[2];
 AP[2].recPtr = &dptRec;
 AP[2].keyPtr = keyBuffer[2];

 AP[0].func = GET_EQUAL_XB;      // accessing EMP by foreign key (exact) first
 AP[0].handle = indexID[0];      // indexID[0] is the SSN (aka MGR_ID)
 AP[0].recPtr = &empRec;
 AP[0].keyPtr = keyBuffer[0];

 i=0;
 //       1234567890123456 123456- 1234---8 1234567890123456 1234567890123456
 PutMsg(" DEPARTMENT       DEPT_ID ASSIGNED MANAGER\n");
 rez=BULLET(&AP[2]);
 while (rez==0) {
    memcpy(keyBuffer[0],dptRec.dptMgrID,9);      // foreign key to key buffer
    rez=BULLET(&AP[0]);                          // and get it to empRec
    if (rez!=0) strcpy(empRec.empLN,"Error!");

    sprintf(putStr," %-16.16s %7.6s %4.4s     %s %s\n",
            dptRec.dptName,
            dptRec.dptID,
            dptRec.dptNumber,
            empRec.empFN,
            empRec.empLN);
    PutMsg(putStr);

    if (rez==0) {
       i++;
       AP[2].func = GET_NEXT_XB;
       rez=BULLET(&AP[2]);
    }
 }
 if (rez == EXB_END_OF_FILE) rez=0; // expected is ERR_END_OF_FILE
 if (rez) {
    sprintf(putStr,"(DEPT_ID) Failed DPT-EMP view #%d, err: %d\n",i,rez);
    PutMsg(putStr);
    goto Abend;
 }

 // Fatal errors above come straight to here
 Abend:

 // Close files (index files first then data (recommended but not required))

 HP.func = CLOSE_INDEX_XB;
 for (i=2;i >= 0;i--) {
    if (indexID[i]) {
       HP.handle = indexID[i];
       rez = BULLET(&HP);
       if (rez) {
          sprintf(putStr,"Failed index #%d file close.  Err: %d\n",i,rez);
          PutMsg(putStr);
       }
    }
 }

 HP.func = CLOSE_DATA_XB;
 for (i=1;i >= 0;i--) {
    if (dataID[i]) {
       HP.handle = dataID[i];
       rez = BULLET(&HP);
       if (rez) {
          sprintf(putStr,"Failed data #%d file close.  Err: %d\n",i,rez);
          PutMsg(putStr);
       }
    }
 }

 return rez;
}

// field list assigns moved down here to just get them the heck out of the way!
// -- actually, all these rez=BULLET() sections would be, should be,
// wrapped up into generic routines (e.g., InitDatabase(xyz),
// CreateDatabase(abc), GetEqualOrGreater(), and so on -- all the building
// blocks are already done, you just need to put them together the way you
// want them.

//---------------------------------------------
// Init field list items for employee data file

void BuildEmpFieldList(FIELDDESCTYPE fieldList[]) {

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

 strcpy(fieldList[4].fieldName, "DEPT_ID");
 fieldList[4].fieldType = 'C';
 fieldList[4].fieldLen = 6;
 fieldList[4].fieldDC = 0;
 return;
}


//-----------------------------------------------
// Init field list items for department data file

void BuildDptFieldList(FIELDDESCTYPE fieldList[]) {

 strcpy(fieldList[0].fieldName, "DEPT_ID");
 fieldList[0].fieldType = 'C';
 fieldList[0].fieldLen = 6;
 fieldList[0].fieldDC = 0;

 strcpy(fieldList[1].fieldName, "NAME");
 fieldList[1].fieldType = 'C';
 fieldList[1].fieldLen = 16;
 fieldList[1].fieldDC = 0;

 strcpy(fieldList[2].fieldName, "MGR_ID");
 fieldList[2].fieldType = 'C';
 fieldList[2].fieldLen = 9;
 fieldList[2].fieldDC = 0;

 strcpy(fieldList[3].fieldName, "ASSIGNED");
 fieldList[3].fieldType = 'N';
 fieldList[3].fieldLen = 4;
 fieldList[3].fieldDC = 0;
 return;
}
