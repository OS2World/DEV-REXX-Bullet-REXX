/* 07 - high-level examples for Bullet/REXX */

 NUMERIC DIGITS 11      /* required for full 4GB number range */

/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_DeleteFileDos()   [to delete test-generated files]
   - blt_CreateData()
   - blt_OpenData()
   - blt_CreateIndex()
   - blt_OpenIndex()
   - blt_Lock()            [XACTION]
   - blt_Insert()          [XACTION]
   - blt_Update()          [XACTION]
   - blt_Reindex()         [XACTION]
   - blt_Relock()          [XACTION]
   - blt_GetXXX()          [XXX=First, Equal, Next, Prev, Last]
   - blt_Unlock()          [XACTION]
   - blt_CloseIndex()
   - blt_CloseData()
   - blt_Exit()
*/

/* Typically, each test routine's arg pack (blt_IP., etc.) is set to NOVALUE */
/* so that any unset variables can easily be identified.  In actual use, */
/* this would not be necessary since often arg pack values are already setup */
/* for multiple calls, where blt_?P.variable is already properly set up */

/* Doing so, however, means Bullet/REXX checks on variables existing are */
/* effectively bypassed, and typically, missing variables are set to 0 */
/* and used without warning (if a valid value) rather than Bullet/REXX */
/* generating an appropriate syntax error */

 say "Example: 07hll.cmd  (recommend output be redirected to a file)"

 call RxFuncAdd 'BulletLoadFuncs', 'BREXXI2', 'BulletLoadFuncs'
 call BulletLoadFuncs

 /* in case Bullet/REXX is still active, close it out */

 rez = blt_Exit()

 say
 say "calling blt_Init()"
 blt_IP.=NOVALUE
 blt_IP.JFTsize=5       /* must be 5 */
 rez = blt_Init()       /* init Bullet/REXX */
 say " blt_IP.func is" blt_IP.func
 say " blt_IP.stat is" blt_IP.stat  /* rez same as stat except for xactions */
 if rez = 0 then do

    say " blt_IP.versionDOS is" blt_IP.versionDOS
    say " blt_IP.versionOS is" blt_IP.versionOS
    say " blt_IP.versionBullet is" blt_IP.versionBullet
    /* say " blt_IP.exitPtr is" blt_IP.exitPtr */

    say
    say "calling blt_DeleteFileDos() x4"
    blt_DFP.=NOVALUE
    blt_DFP.filename = "07HLL.DBF"
    rez = blt_DeleteFileDos();
    say " blt_DFP.func is" blt_DFP.func
    say " blt_DFP.stat is" blt_DFP.stat "(07HLL.DBF) [stat=2, file not found, is possible]"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "07HLL.DBT"  /* may use QuerySysVars for memo .EXT */
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(07HLL.DBT)"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "07HLL_1.IX3"
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(07HLL_1.IX3) [SSN index]"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "07HLL_2.IX3"
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(07HLL_2.IX3) [NAME index]"

    say
    say "calling blt_CreateData()"
    blt_CDP.=NOVALUE
    blt_CDP.filename = "07HLL.DBF"
    blt_CDP.noFields = 3
    blt_CDP.FD.1.fieldName = "SSN"
    blt_CDP.FD.1.fieldType = "N"
    blt_CDP.FD.1.fieldLen = 9
    blt_CDP.FD.1.fieldDC = 0
    blt_CDP.FD.2.fieldName = "NAME"
    blt_CDP.FD.2.fieldType = "C"
    blt_CDP.FD.2.fieldLen = 10
    blt_CDP.FD.2.fieldDC = 0
    blt_CDP.FD.3.fieldName = "ADDR"
    blt_CDP.FD.3.fieldType = "M"
    blt_CDP.FD.3.fieldLen = 10
    blt_CDP.FD.3.fieldDC = 0
    blt_CDP.fileID = 139        /* memo, too  (139 is 0x8B [hex]) */
    rez = blt_CreateData()      /* create data files: DBF and DBT */
    say " blt_CDP.func is" blt_CDP.func
    say " blt_CDP.stat is" blt_CDP.stat

    /* since region locking of data file was demo'ed in 05midm.cmd, this */
    /* locks the file at open time for the data file, and demonstrates */
    /* region locking (could be called on demand locking) for the index */
    /* -- for quick and dirty and simple use, file open locks are okay */
    /* -- but for multi-user environments, in complex programs, region */
    /* -- lock calls (blt_Lock/blt_Unlock) are probably much better since */
    /* -- the file (or region) can be locked on demand, without having */
    /* -- to close/re-open the file in another share mode as would be */
    /* -- needed if region locks weren't available */

    if rez = 0 then do
       say
       say "calling blt_OpenData()"
       blt_OP.=NOVALUE
       blt_OP.filename = blt_CDP.filename
       blt_OP.asMode = 66       /* 66 is 0x0042 [hex] DENYNONE, R/W */
       rez = blt_OpenData()
       say " blt_OP.func is" blt_OP.func
       say " blt_OP.stat is" blt_OP.stat

       /* must have data DBF open before creating index files for it */
       /* and good idea to have at least a shared lock on it while */
       /* index file for it is being created (lock not shown here!)*/

       dataID = 0
       indexID_1 = 0
       indexID_2 = 0

       if rez = 0 then do
          dataID = blt_OP.handle
          say " blt_OP.handle is" dataID
          say
          say "calling blt_CreateIndex()  [SSN index]"
          blt_CIP.=NOVALUE
          blt_CIP.filename = "07HLL_1.IX3"
          blt_CIP.keyExp = "SSN"        /* index is on entire SSN field */
          blt_CIP.xbLink = dataID       /* blt_OP.handle from DBF open above */
          blt_CIP.sortFunction = 1      /* ASCII sort, dups -not- allowed */
          blt_CIP.codePage = 0
          blt_CIP.countryCode = 0
          blt_CIP.collateTable = ""     /* 0-len string for default, else 256-char table of weights */
          blt_CIP.nodeSize = 512
          rez = blt_CreateIndex()
          say " blt_CIP.func is" blt_CIP.func
          say " blt_CIP.stat is" blt_CIP.stat

          /* this example uses two index files for the one data file */

          if rez = 0 then do

             say
             say "calling blt_CreateIndex()  [NAME index]"
             blt_CIP.filename = "07HLL_2.IX3"
             blt_CIP.keyExp = "NAME"         /* index is on entire NAME field */
             blt_CIP.sortFunction = 2+65536  /* NLS sort, dups allowed */
             rez = blt_CreateIndex()
             say " blt_CIP.func is" blt_CIP.func
             say " blt_CIP.stat is" blt_CIP.stat
          end

          if rez = 0 then do

             /* open first index */

             say
             say "calling blt_OpenIndex() [SSN index]"
             blt_OP.=NOVALUE
             blt_OP.filename = "07HLL_1.IX3"
             blt_OP.asMode = 66       /* 66 is 0x0042 [hex] DENYNONE, R/W */

             /* index open requires blt_OP.xbLink set to DBF handle */
             /* whereas .xbLink is not used in a data open */

             blt_OP.xbLink = dataID
             rez = blt_Openindex()
             say " blt_OP.func is" blt_OP.func
             say " blt_OP.stat is" blt_OP.stat

             /* open second index */

             if rez = 0 then do

                indexID_1 = blt_OP.handle
                say " blt_OP.handle is" indexID_1
                say
                say "calling blt_OpenIndex() [NAME index]"
                blt_OP.=NOVALUE
                blt_OP.filename = "07HLL_2.IX3"
                blt_OP.asMode = 66       /* 66 is 0x0042 [hex] DENYNONE, R/W */
                blt_OP.xbLink = dataID
                rez = blt_Openindex()
                say " blt_OP.func is" blt_OP.func
                say " blt_OP.stat is" blt_OP.stat
             end

             if rez = 0 then do

                indexID_2 = blt_OP.handle
                say " blt_OP.handle is" indexID_2

                /* lock the two index files and their data file */
                /* which in this case is the only data file */
                /* note the use of TLP, rather than LP -- TLP stands */
                /* for Transaction Lock Pack */

                say
                say "calling blt_Lock()  [to exclusive lock, read-write] (XACTION)"
                blt_TLP.=NOVALUE
                blt_TLP.1.handle = indexID_1
                blt_TLP.1.xlMode = 0     /* IX3: 0=exclusive lock; 1=shared lock */
                blt_TLP.1.dlMode = 0     /* DBF: 0=exclusive lock; 1=shared lock */
                blt_TLP.1.nextFlag = 1   /* flag that another pack follows */
                blt_TLP.2.handle = indexID_2
                blt_TLP.2.xlMode = 0
                blt_TLP.2.dlMode = 0
                blt_TLP.2.nextFlag = 0   /* flag this as last pack */
                rez = blt_Lock()
                say " blt_Lock() rez= is" rez
                say " blt_TLP.1.func is" blt_TLP.1.func
                rez = abs(rez)   /* where rez may be -2, -1, 0, 1, 2 here */
                if rez <> 0 then do /* and where neg rez is data file, pos rez is index file */
                   say " blt_TLP."rez".stat is" blt_TLP.rez.stat
                end

                if rez = 0 then do

                   /* as with the transaction locks, transaction inserts, */
                   /* updates, and reindexes use the var TAP, rather than AP */

                   /* srec/erec should remain so sized, but erec could be increased easily, say to 109999 */
                   srec = 100001        /* used to fill in SSN and NAME fields */
                   erec = 100100        /* and as range count */
                   trecs = erec-srec+1  /* total records to show in says (hubcaps and all) */

                   say
                   say "calling blt_Insert()"trecs"times [XACTION]"

                   blt_TAP.=NOVALUE

                   /* generates SSN numbers from 465100001 to 465100100 */
                   /* leading delete tag field (space) */
                   /* NAME field set to "name" text followed by same record */
                   /* number -- this technique is used simply to generate */
                   /* different name fields (NOTE: NAME index is allowed to have */
                   /* duplicate keys as set when created (DUPS_ALLOWED=65536) */
                   /* this loop writes 100 records to the DBF, and 100 keys each */
                   /* to the index files */

                   /* raw record is SSN(9), NAME(10), ADDR(10/memo number) */
                   /* with the memo number in ADDR left empty */

                   /* Unlike non-transaction routines, Insert/Update/Reindex/ */
                   /* Lock/Unlock will generate a rexx error (40) if any of */
                   /* the pre-amble calls performed in REXXBLT.DLL fail -- */
                   /* this would happen if you pass a bad handle, for example */
                   /* once the actual BULLET routine is entered, and the */
                   /* transaction begins, errors are returned as expected */

                   /* first setup parts of TAP pack that are constant */

                   blt_TAP.1.handle = indexID_1
                   blt_TAP.1.nextFlag = 1       /* indicate another pack follows */
                   blt_TAP.2.handle = indexID_2
                   blt_TAP.2.nextFlag = 0       /* indicate TAP.2 is last pack */

                   do record = srec to erec until rez <> 0

                      /* index 1 generates keys on SSN part of record */
                      /* index 2 generates keys on NAME part of record */

                      blt_TAP.1.recData = " 465"record"name"record"          "
                      blt_TAP.2.recData = blt_TAP.1.recData

                      /* .recNo must be reset on each call since they */
                      /* are filled in with the actual rec# of insert call */

                      blt_TAP.1.recNo = 0    /* must be set to 0 */
                      blt_TAP.2.recNo = C2D('80000000'X) /* needs NUMERIC DIGITS 11+ */
                      rez = blt_Insert()
                   end
                   say " blt_Insert() rez= is" rez
                   say " blt_TAP.1.func is" blt_TAP.1.func
                   rez = abs(rez)   /* where rez may be -2, -1, 0, 1, 2 here */
                   if rez <> 0 then do /* and where neg rez is data file, pos rez is index file */
                      say " blt_TAP."rez".stat is" blt_TAP.rez.stat
                   end

                   /* note that rez returned from transaction calls is -NOT- */
                   /* an error code but is the number of the pack that failed */
                   /* further defined as data part of pack if negative, and */
                   /* caused by the index part of pack if positive -- the */
                   /* actual error code is in the failed pack's .stat var */

                   if rez = 0 then do

                      say
                      say "calling blt_Update()"trecs"times [XACTION]"

                      /* first setup parts of TAP pack that are constant */

                      blt_TAP.1.handle = indexID_1
                      blt_TAP.1.nextFlag = 1       /* indicate another pack follows */
                      blt_TAP.2.handle = indexID_2
                      blt_TAP.2.nextFlag = 0       /* indicate TAP.2 is last pack */

                      /* as a simple example, this updates records in record number order */
                      /* where each data record is changed so that both index keys require */
                      /* updating (done automatically by the blt_Update() routine) */

                      /* -- the main idea here is to give the transaction-based the record */
                      /* -- number and new record data and allow Bullet to make any necessary */
                      /* -- updates, if any (key access could also be used to locate particular */
                      /* -- records in the database to update, of course) */

                      blt_AP.handle = dataID  /* for blt_GetRecord() call */
                      blt_AP.recNo = 1        /* get each data record, in rec# order */
                      do until rez <> 0

                         rez2 = blt_GetRecord()
                         if rez2 = 8609 then leave  /* 8609 is  expected result after reading all recs */
                         if rez2 = 0 then do

                            blt_TAP.1.recNo = blt_AP.recNo
                            blt_TAP.2.recNo = blt_AP.recNo

                            /* index 1 generates keys on SSN part of record */
                            /* index 2 generates keys on NAME part of record */

                            /* note how SSN field and NAME field are changed from the above insert */
                            /* by changing 100001+  to 600001+ in both SSN and NAME fields */
                            /* this forces key updates in both SSN and NAME index files for all */

                            /* this is purely for example, crude often is effective for this */

                            blt_TAP.1.recData = overlay('6',blt_AP.recData,5)
                            blt_TAP.1.recData = overlay('6',blt_TAP.1.recData,15)
                            blt_TAP.2.recData = blt_TAP.1.recData

                            rez = blt_Update()

                            blt_AP.recNo = blt_AP.recNo + 1  /* prep for next read */
                         end
                         else do
                            say "* ERROR * in blt_GetRecord() part of blt_Update() example"
                            say " blt_GetRecord() rez= is" rez2
                            say " blt_AP.func is" blt_AP.func
                            say " blt_AP.stat is" blt_AP.stat
                            rez = 999  /* special flag for below */
                         end
                      end

                      if rez <> 999 then do
                         say " blt_Update() rez= is" rez
                         say " blt_TAP.1.func is" blt_TAP.1.func
                         rez = abs(rez)   /* where rez may be -2, -1, 0, 1, 2 here */
                         if rez <> 0 then do /* and where neg rez is data file, pos rez is index file */
                            say " blt_TAP."rez".stat is" blt_TAP.rez.stat
                         end
                      end

                      if rez = 0 then do

                         /* purely for example, reindex en masse */

                         say
                         say "calling blt_Reindex()   [XACTION]"
                         blt_TAP.1.handle = indexID_1  /* these are already setup from above */
                         blt_TAP.1.nextFlag = 0        /* but it doesn't hurt to */
                         blt_TAP.2.handle = indexID_2  /* get back, Jack, and do it again */
                         blt_TAP.2.nextFlag = 0
                         rez = blt_Reindex();
                         say " blt_Reindex() rez= is" rez
                         say " blt_TAP.1.func is" blt_TAP.1.func
                         rez = abs(rez)
                         if rez <> 0 then do /* and where neg rez is data file, pos rez is index file */
                            say " blt_TAP."rez".stat is" blt_TAP.rez.stat
                            /* on a reindex error, TAP.rez.recNo/keyData contain */
                            /* data record number/key data at time of failure, if relevent */
                            say " blt_TAP."rez".recNo is" blt_TAP.rez.recNo
                            say " blt_TAP."rez".keyData is" blt_TAP.rez.keyData
                         end

                         if rez = 0 then do

                            /* done writing to file, can release lock, or in this case, relock */
                            /* as blt_Lock(), above, but change locked to shared to */
                            /* allow other processes read-access (but not write) */

                            say
                            say "calling blt_Relock()  [to shared lock, read-only] (XACTION)"
                            blt_TLP.=NOVALUE
                            blt_TLP.1.handle = indexID_1
                            blt_TLP.1.xlMode = 1     /* IX3: 0=exclusive lock; 1=shared lock */
                            blt_TLP.1.dlMode = 1     /* DBF: 0=exclusive lock; 1=shared lock */
                            blt_TLP.1.nextFlag = 1   /* flag that another pack follows */
                            blt_TLP.2.handle = indexID_2
                            blt_TLP.2.xlMode = 1
                            blt_TLP.2.dlMode = 1
                            blt_TLP.2.nextFlag = 0   /* flag this as last pack */
                            rez = blt_Relock()
                            say " blt_Relock() rez= is" rez
                            say " blt_TLP.1.func is" blt_TLP.1.func
                            rez = abs(rez)   /* where rez may be -2, -1, 0, 1, 2 here */
                            if rez <> 0 then do /* and where neg rez is data file, pos rez is index file */
                               say " blt_TLP."rez".stat is" blt_TLP.rez.stat
                            end

                            if rez = 0 then do

                               /* quick exercise of key+record access */

                               say
                               say "calling blt_GetFirst()  [using SSN index here and following]"
                               blt_AP.handle = indexID_1
                               rez = blt_GetFirst()
                               say " blt_AP.func is" blt_AP.func
                               say " blt_AP.stat is" blt_AP.stat
                               if rez = 0 then do

                                  say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                  say " blt_AP.recData is '"blt_AP.recData"'"
                                  say
                                  say "calling blt_GetEqual()  [using GetFirst's key as search key]"
                                  rez = blt_GetEqual()
                                  say " blt_AP.func is" blt_AP.func
                                  say " blt_AP.stat is" blt_AP.stat
                                  if rez = 0 then do

                                     say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                     say " blt_AP.recData is '"blt_AP.recData"'"
                                     say
                                     say "calling blt_GetNext()"
                                     rez = blt_GetNext()
                                     say " blt_AP.func is" blt_AP.func
                                     say " blt_AP.stat is" blt_AP.stat
                                     if rez = 0 then do

                                        say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                        say " blt_AP.recData is '"blt_AP.recData"'"
                                        say
                                        say "calling blt_GetLast()"
                                        rez = blt_GetLast()
                                        say " blt_AP.func is" blt_AP.func
                                        say " blt_AP.stat is" blt_AP.stat
                                        if rez = 0 then do

                                           say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                           say " blt_AP.recData is '"blt_AP.recData"'"
                                           say
                                           say "calling blt_GetPrev()"
                                           rez = blt_GetPrev()
                                           say " blt_AP.func is" blt_AP.func
                                           say " blt_AP.stat is" blt_AP.stat
                                           if rez = 0 then do

                                              say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                              say " blt_AP.recData is '"blt_AP.recData"'"
                                              say
                                              say "Excellent!  All calls went as planned"
                                           end
                                           else do
                                              say "* ERROR * blt_GetPrev()"
                                           end

                                        end
                                        else do
                                           say "* ERROR * blt_GetLast()"
                                        end

                                     end
                                     else do
                                        say "* ERROR * blt_GetNext()"
                                     end

                                  end
                                  else do
                                     say "* ERROR * blt_GetEqual()"
                                  end

                               end
                               else do
                                  say "* ERROR * blt_GetFirst()"
                               end

                            end
                            else do
                               say "* ERROR * blt_Relock()"
                            end

                         end
                         else do
                            say "* ERROR * blt_Reindex()"
                         end

                      end
                      else do
                         say "* ERROR * blt_GetRecord/blt_Update()"
                      end

                   end
                   else do
                      say "* ERROR * blt_Insert()"
                   end

                   /* typically would not wait this long (i.e., the end) */
                   /* to unlock, but this is only a demo */

                   say
                   say "calling blt_Unlock()"
                   blt_TLP.=NOVALUE
                   blt_TLP.1.handle = indexID_1
                   blt_TLP.1.nextFlag = 1
                   blt_TLP.2.handle = indexID_2
                   blt_TLP.2.nextFlag = 0
                   rez = blt_Unlock()
                   say " blt_Unlock() rez= is" rez
                   say " blt_TLP.1.func is" blt_TLP.1.func  /* same for all */
                   rez = abs(rez)
                   if rez <> 0 then do
                      say " blt_TLP."rez".stat is" blt_TLP.rez.stat
                   end

                end
                else do
                   say "* ERROR * blt_Lock()"
                end

                /* if open, it's a good idea to close it (blt_Exit() will if not) */

                blt_HP.=NOVALUE
                blt_HP.handle = indexID_1
                say
                say "calling blt_CloseIndex() on SSN index"
                rez = blt_CloseIndex()
                say " blt_HP.func is" blt_HP.func
                say " blt_HP.stat is" blt_HP.stat

                /* verify that second index opened okay */

                if indexID_2 <> 0 then do
                   blt_HP.=NOVALUE
                   blt_HP.handle = indexID_2
                   say
                   say "calling blt_CloseIndex() on NAME index"
                   rez = blt_CloseIndex()
                   say " blt_HP.func is" blt_HP.func
                   say " blt_HP.stat is" blt_HP.stat
                end

             end
             else do
                say "* ERROR * blt_OpenIndex()"
             end

          end
          else do
             say "* ERROR * blt_CreateIndex()"
          end


          /* if open, it's a good idea to close it (blt_Exit() will if not) */
          /* data file was opened for exclusive access, so not locked */

          blt_HP.=NOVALUE
          blt_HP.handle = dataID
          say
          say "calling blt_CloseData()"
          rez = blt_CloseData()
          say " blt_HP.func is" blt_HP.func
          say " blt_HP.stat is" blt_HP.stat

       end
       else do
          say "* ERROR * blt_OpenData()"
       end

    end
    else do
       say
       say "* ERROR * blt_CreateData()"
    end

    say
    say "calling blt_Exit()"
    blt_EP.=NOVALUE
    rez = blt_Exit()
    say " blt_EP.func is" blt_EP.func
    say " blt_EP.stat is" blt_EP.stat
    say " blt_EP.rxAllocsLeft is" blt_EP.rxAllocsLeft
 end
 else do
    say
    say "* ERROR * blt_Init()"
 end

 call BulletDropFuncs
 exit 0
