/* 06 - key mid-level examples for Bullet/REXX */
/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_DeleteFileDos()   [to delete test-generated files]
   - blt_CreateData()
   - blt_OpenData()
   - blt_CreateIndex()
   - blt_OpenIndex()
   - blt_AddRecord()
   - blt_BuildKey()
   - blt_LockIndex()
   - blt_StoreKey()
   - blt_UnlockIndex()
   - blt_RelockIndex()
   - blt_xxxKey()           [xxx=First, Equal, Next, Prev, Last]
   - blt_GetRecord()
   - blt_DeleteKey()
   - blt_GetCurrentKey()
   - blt_GetKeyForRecord()
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

 say "Example: 06midk.cmd  (recommend output be redirected to a file)"

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
    say "calling blt_DeleteFileDos() x3"
    blt_DFP.=NOVALUE
    blt_DFP.filename = "06MIDK.DBF"
    rez = blt_DeleteFileDos();
    say " blt_DFP.func is" blt_DFP.func
    say " blt_DFP.stat is" blt_DFP.stat "(06MIDK.DBF) [stat=2, file not found, is possible]"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "06MIDK.DBT"  /* may use QuerySysVars for memo .EXT */
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(06MIDK.DBT)"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "06MIDK.IX3"
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(06MIDK.IX3)"

    say
    say "calling blt_CreateData()"
    blt_CDP.=NOVALUE
    blt_CDP.filename = "06MIDK.DBF"
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
       blt_OP.asMode = 18       /* 18 is 0x0012 [hex] DENYRW, R/W */
       rez = blt_OpenData()     /* data is locked at file level (demo purpose) */
       say " blt_OP.func is" blt_OP.func
       say " blt_OP.stat is" blt_OP.stat

       /* must have data DBF open before creating index for it */
       /* and good idea to have at least a shared lock on it while */
       /* index file for it is being created */

       dataID = 0       /* since two opens are done, cannot keep both handles */
       indexID = 0      /* in the same blt_OP.handle var, so do this */

       if rez = 0 then do
          dataID = blt_OP.handle
          say " blt_OP.handle is" dataID
          say
          say "calling blt_CreateIndex()"
          blt_CIP.=NOVALUE
          blt_CIP.filename = "06MIDK.IX3"
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

          if rez = 0 then do
             say
             say "calling blt_OpenIndex()"
             blt_OP.=NOVALUE
             blt_OP.filename = blt_CIP.filename
             blt_OP.asMode = 66       /* 66 is 0x0042 [hex] DENYNONE, R/W */

             /* index open requires blt_OP.xbLink set to DBF handle */
             /* whereas .xbLink is not used in a data open */

             blt_OP.xbLink = dataID
             rez = blt_Openindex()
             say " blt_OP.func is" blt_OP.func
             say " blt_OP.stat is" blt_OP.stat

             if rez = 0 then do
                indexID = blt_OP.handle
                say " blt_OP.handle is" indexID

                say
                say "calling blt_LockIndex()  [to exclusive lock, read-write]"
                blt_LP.=NOVALUE
                blt_LP.handle = indexID
                blt_LP.xlMode = 0     /* 0=exclusive lock; 1=shared lock */
                rez = blt_LockIndex()
                say " blt_LP.func is" blt_LP.func
                say " blt_LP.stat is" blt_LP.stat
                if rez = 0 then do

                   say
                   say "calling blt_AddRecord(),_BuildKey(),_StoreKey(), 100 times"
                   blt_AP.=NOVALUE

                   /* generates SSN numbers from 465100001 to 465100100 */
                   /* leading delete tag field (space) */
                   /* NAME field just filled in with ... nothing important */
                   /* this loop writes 100 records to disk */
                   /* raw record is SSN(9), NAME(10), ADDR(10/memo number) */
                   /* with the memo number in ADDR left empty */
                   /* -- also builds key for each and stores it to index file */
                   /* -- duplicates not automatically handled by mid-level calls */
                   /* note that, as in all calls, .func is returned only if the */
                   /* final call to Bullet/REXX is actually made; any errors */
                   /* occuring prior to that call will have .func left as it */
                   /* was when last set -- */

                   /* this 3-call procedure is typically handled by */
                   /*                                               */
                   /*           blt_Insert()                        */
                   /*                                               */
                   /* which performs this task much better than can */
                   /* be done by doing it this way                  */

                   f1 = 0
                   f2 = 0
                   f3 = 0
                   do record = 100001 to 100100 until rez <> 0
                      blt_AP.recData = " 465"record"name-nadaZ          "
                      step = 1
                      blt_AP.handle = dataID
                      rez = blt_AddRecord()  /* returns blt_AP.recNo */
                      if f1 = 0 then f1 = blt_AP.func
                      if rez = 0 then do

                         step = 2
                         blt_AP.handle = indexID
                         rez = blt_BuildKey()  /* blt_AP. values already set */
                         if f2 = 0 then f2 = blt_AP.func
                         if rez = 0 then do

                            step = 3
                            rez = blt_StoreKey() /* likewise, already set up */
                            if f3 = 0 then f3 = blt_AP.func
                         end
                      end
                   end
                   say " blt_AddRecord() blt_AP.func is" f1
                   say " blt_BuildKey()  blt_AP.func is" f2
                   say " blt_StoreKey()  blt_AP.func is" f3
                   say " Last blt_AP.stat is" blt_AP.stat
                   if rez = 0 then do

                      /* for demo purpose, this physically removes last */
                      /* physical data record and its key, added last */

                      say
                      say "calling blt_DebumpRecord()"
                      blt_AP.handle = dataID
                      rez = blt_DebumpRecord()  /* again, already set up */
                      say " blt_AP.func is" blt_AP.func
                      say " blt_AP.stat is" blt_AP.stat
                      if rez = 0 then do

                         say
                         say "calling blt_DeleteKey()"
                         blt_AP.handle = indexID
                         rez = blt_DeleteKey()    /* delete key in blt_AP.keyData */
                         say " blt_AP.func is" blt_AP.func
                         say " blt_AP.stat is" blt_AP.stat
                         if rez = 0 then do

                            /* done writing to file, can release lock */
                            /* actually, may have been better to unlock/lock */
                            /* once per call -- in any case, instead of */
                            /* releasing, from demo purpose, relock index */

                            say
                            say "calling blt_RelockIndex()  [to shared lock, read-only]"
                            blt_LP.=NOVALUE
                            blt_LP.handle = indexID
                            blt_LP.xlMode = 1     /* 0=exclusive lock; 1=shared lock */
                            rez = blt_RelockIndex()
                            say " blt_LP.func is" blt_LP.func
                            say " blt_LP.stat is" blt_LP.stat
                            if rez = 0 then do

                               /* quick exercise of key-only access */
                               /* returned is the record number of the key */
                               /* accessed, and from that, its record can */
                               /* be gotten with a call to blt_GetRecord() */
                               /* as shown in the blt_PrevKey() example */

                               say
                               say "calling blt_FirstKey()"
                               rez = blt_FirstKey()
                               say " blt_AP.func is" blt_AP.func
                               say " blt_AP.stat is" blt_AP.stat
                               say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                               if rez = 0 then do

                                  say
                                  say "calling blt_EqualKey()  [using FirstKey as search key]"
                                  rez = blt_EqualKey()
                                  say " blt_AP.func is" blt_AP.func
                                  say " blt_AP.stat is" blt_AP.stat
                                  say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                  if rez = 0 then do

                                     say
                                     say "calling blt_NextKey()"
                                     rez = blt_NextKey()
                                     say " blt_AP.func is" blt_AP.func
                                     say " blt_AP.stat is" blt_AP.stat
                                     say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                     if rez = 0 then do

                                        say
                                        say "calling blt_LastKey()"
                                        rez = blt_LastKey()
                                        say " blt_AP.func is" blt_AP.func
                                        say " blt_AP.stat is" blt_AP.stat
                                        say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                        if rez = 0 then do

                                           say
                                           say "calling blt_PrevKey()"
                                           rez = blt_PrevKey()
                                           say " blt_AP.func is" blt_AP.func
                                           say " blt_AP.stat is" blt_AP.stat
                                           say " blt_AP.keyData is '"blt_AP.keyData"' recNo:" blt_AP.recNo
                                           if rez = 0 then do

                                              say
                                              say "calling blt_GetRecord() for that key search"
                                              blt_AP.handle = dataID
                                              rez = blt_GetRecord()
                                              say " blt_AP.func is" blt_AP.func
                                              say " blt_AP.stat is" blt_AP.stat
                                              say " blt_AP.recData is '"blt_AP.recData"'"
                                              if rez = 0 then do

                                                 say
                                                 say "calling blt_GetCurrentKey()"
                                                 blt_AP.keyData = ""  /* just to verify return */
                                                 blt_AP.handle = indexID
                                                 rez = blt_GetCurrentKey()
                                                 say " blt_AP.func is" blt_AP.func
                                                 say " blt_AP.stat is" blt_AP.stat
                                                 say " blt_AP.recNo is" blt_AP.recNo
                                                 say " blt_AP.keyData is '"blt_AP.keyData"'"
                                                 if rez = 0 then do

                                                    say
                                                    say "calling blt_GetKeyForRecord() with '"blt_AP.recData"' and rec#" blt_AP.recNo

                                                    blt_AP.keyData = "" /* again, to verify return */

                                                    /* this routine finds the exact key for the recData/recNo pair */
                                                    /* typically an extraneous routine, but may be useful at times */

                                                    rez = blt_GetKeyForRecord()
                                                    say " blt_AP.func is" blt_AP.func
                                                    say " blt_AP.stat is" blt_AP.stat
                                                    say " blt_AP.keyData is '"blt_AP.keyData"'"
                                                    if rez = 0 then do

                                                       say
                                                       say "Excellent!  All calls went as planned"
                                                    end
                                                    else do
                                                       say "* ERROR * blt_GetKeyForRecord()"
                                                    end

                                                 end
                                                 else do
                                                    say "* ERROR * blt_GetCurrentKey()"
                                                 end

                                              end
                                              else do
                                                 say "* ERROR * blt_GetRecord()"
                                              end

                                           end
                                           else do
                                              say "* ERROR * blt_PrevKey()"
                                           end

                                        end
                                        else do
                                           say "* ERROR * blt_LastKey()"
                                        end

                                     end
                                     else do
                                        say "* ERROR * blt_NextKey()"
                                     end

                                  end
                                  else do
                                     say "* ERROR * blt_EqualKey()"
                                  end

                               end
                               else do
                                  say "* ERROR * blt_FirstKey()"
                               end

                            end
                            else do
                               say "* ERROR * blt_RelockIndex()"
                            end

                         end
                         else do
                            say "* ERROR * blt_DeleteKey()"
                         end

                      end
                      else do
                         say "* ERROR * blt_DebumpRecord()"
                      end

                   end
                   else do
                      if step = 1 then say "* ERROR * blt_AddRecord()"
                      if step = 2 then say "* ERROR * blt_BuildKey()"
                      if step = 3 then say "* ERROR * blt_StoreKey()"
                      say "  at record" record
                   end

                   /* typically would not wait this long (i.e., the end) */
                   /* to unlock, but this is only a demo */

                   say
                   say "calling blt_UnLockIndex()"
                   blt_LP.=NOVALUE
                   blt_LP.handle = indexID
                   rez = blt_UnlockIndex()
                   say " blt_LP.func is" blt_LP.func
                   say " blt_LP.stat is" blt_LP.stat

                end
                else do
                   say "* ERROR * blt_LockIndex()"
                end

                /* if open, it's a good idea to close it (blt_Exit() will if not) */

                blt_HP.=NOVALUE
                blt_HP.handle = indexID
                say
                say "calling blt_CloseIndex()"
                rez = blt_CloseIndex()
                say " blt_HP.func is" blt_HP.func
                say " blt_HP.stat is" blt_HP.stat

             end
             else do
                say "* ERROR * blt_OpenIndexHeader()"
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
