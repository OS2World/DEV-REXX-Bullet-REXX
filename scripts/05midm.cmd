/* 05 - memo mid-level examples for Bullet/REXX */
/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_DeleteFileDos()   [to delete test-generated files]
   - blt_CreateData()
   - blt_OpenData()
   - blt_LockData()
   - blt_AddMemo()
   - blt_AddRecord()
   - blt_RelockData()
   - blt_GetRecord()
   - blt_GetMemoSize()
   - blt_GetMemo()
   - blt_UpdateMemo()
   - blt_UpdateRecord()
   - blt_DeleteMemo()
   - blt_DeleteRecord()
   - blt_MemoBypass()
   - blt_UnlockData()
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

 say "Example: 05midm.cmd  (recommend output be redirected to a file)"

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
    say "calling blt_DeleteFileDos() x2 (05MIDM.DB?)"
    blt_DFP.=NOVALUE
    blt_DFP.filename = "05MIDM.DBF"
    rez = blt_DeleteFileDos();
    say " blt_DFP.func is" blt_DFP.func
    say " blt_DFP.stat is" blt_DFP.stat "(05MIDM.DBF) [stat=2, file not found, is possible]"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "05MIDM.DBT"  /* may use QuerySysVars for memo .EXT */
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(05MIDM.DBT)"

    say
    say "calling blt_CreateData()"
    blt_CDP.=NOVALUE
    blt_CDP.filename = "05MIDM.DBF"
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

    if rez = 0 then do
       say
       say "calling blt_OpenData()"
       blt_OP.=NOVALUE
       blt_OP.filename = blt_CDP.filename
       blt_OP.asMode = 66       /* 66 is 0x0042 [hex] DENYNONE, R/W */
       rez = blt_OpenData()
       say " blt_OP.func is" blt_OP.func
       say " blt_OP.stat is" blt_OP.stat

       /* lock is in place for more calls that normally would be done */
       /* but for this demo it simplifies the number of lines shown -- */
       /* normally, a lock is released as soon as its immediate duty */
       /* (protecting the following single call) has completed */

       if rez = 0 then do
          say " blt_OP.handle is" blt_OP.handle
          say
          say "calling blt_LockData()  (also implicitly locks memo file)"
          blt_LP.=NOVALUE
          blt_LP.handle = blt_OP.handle
          blt_LP.dlMode = 0     /* 0=exclusive lock; 1=shared lock */
          blt_LP.recStart = 0   /* lock everything */
          blt_LP.recCount = 0   /* not used nor required if .recStart=0 */
          rez = blt_LockData()
          say " blt_LP.func is" blt_LP.func
          say " blt_LP.stat is" blt_LP.stat
          if rez = 0 then do

             /* adding a memo first, before adding its record, is simpler */
             /* since you have to write the record with the memo number */
             /* returned from the add memo call, so it only makes sense to */
             /* add the record (if it doesn't already exist, of course) */
             /* after adding the memo */

             say
             say "calling blt_AddMemo()"
             blt_MDP.=NOVALUE
             blt_MDP.dbfHandle = blt_OP.handle
             blt_MDP.memoData = "1313 Mockingbird Lane/Chicago IL 60606"
             blt_MDP.memoBytes = length(blt_MDP.memoData)
             rez = blt_AddMemo()
             say " blt_MDP.func is" blt_MDP.func
             say " blt_MDP.stat is" blt_MDP.stat
             if rez = 0 then do

                /* blt_MDP.memoNo returns as properly formatted number string */
                /* with leading 0s already  (e.g., '0000000001') */

                say
                say "calling blt_AddRecord()"
                blt_AP.=NOVALUE
                blt_AP.handle = blt_OP.handle
                blt_AP.recData = " 465100001name-nadaZ"blt_MDP.memoNo
                rez = blt_Addrecord()
                say " blt_AP.func is" blt_AP.func
                say " blt_AP.stat is" blt_AP.stat
                if rez = 0 then do

                   say
                   say "calling blt_RelockData()  [to shared lock]"
                   blt_LP.=NOVALUE
                   blt_LP.handle = blt_OP.handle
                   blt_LP.dlMode = 1     /* 0=exclusive lock; 1=shared lock */
                   blt_LP.recStart = 0   /* lock everything */
                   blt_LP.recCount = 0   /* not used nor required if .recStart=0 */
                   rez = blt_RelockData()
                   say " blt_LP.func is" blt_LP.func
                   say " blt_LP.stat is" blt_LP.stat
                   if rez = 0 then do

                      say
                      say "calling blt_GetRecord()"
                      blt_AP.=NOVALUE
                      blt_AP.handle = blt_OP.handle
                      blt_AP.recNo = 1
                      rez = blt_GetRecord()
                      say " blt_AP.func is" blt_AP.func
                      say " blt_AP.stat is" blt_AP.stat
                      if rez = 0 then do

                         /* tmpVar is set to memo field data */
                         /* it's at offset 21 (skip tag byte, SSN, and NAME) */

                         tmpVar = substr(blt_AP.recData,21,10)

                         say " rec#" blt_AP.recNo "'"blt_AP.recData"'"
                         say
                         say "calling blt_GetMemoSize()"
                         blt_MDP.=NOVALUE
                         blt_MDP.dbfHandle = blt_OP.handle
                         blt_MDP.memoNo = tmpVar
                         rez = blt_GetMemoSize()
                         say " blt_MDP.func is" blt_MDP.func
                         say " blt_MDP.stat is" blt_MDP.stat
                         if rez = 0 then do

                            say " blt_MDP.memoBytes is" blt_MDP.memoBytes
                            say
                            say "calling blt_GetMemo()"

                            /* does not set to NOVALUE here since */
                            /* many fields gotten above are needed here */
                            /* dbfHandle, memoNo are already set */
                            /* as is memoBytes (if getting entire memo data */

                            /* blt_MDP.dbfHandle = blt_OP.handle */
                            /* blt_MDP.memoNo = tmpVar */
                            /* blt_MDP.memoBytes = (already set above) */
                            blt_MDP.memoOffset = 0  /* start at beginning */
                            rez = blt_GetMemo()
                            say " blt_MDP.func is" blt_MDP.func
                            say " blt_MDP.stat is" blt_MDP.stat
                            if rez = 0 then do

                               say " blt_MDP.memoBytes is" blt_MDP.memoBytes
                               say " blt_MDP.memoData is '"blt_MDP.memoData"'"

                               /* switch lock back to exclusive since writing */

                               say
                               say "calling blt_RelockData()  [to exclusive lock]"
                               blt_LP.=NOVALUE
                               blt_LP.handle = blt_OP.handle
                               blt_LP.dlMode = 0     /* 0=exclusive lock; 1=shared lock */
                               blt_LP.recStart = 0   /* lock everything */
                               blt_LP.recCount = 0   /* not used nor required if .recStart=0 */
                               rez = blt_RelockData()
                               say " blt_LP.func is" blt_LP.func
                               say " blt_LP.stat is" blt_LP.stat
                               if rez = 0 then do

                                  say
                                  say "calling blt_UpdateMemo() (change 1313 to 1919)"
                                  /* other blt_MDP. members set from above calls */
                                  blt_MDP.memoData = "1919"
                                  blt_MDP.memoOffset = 0
                                  blt_MDP.memoBytes = length(blt_MDP.memoData)
                                  rez = blt_UpdateMemo()
                                  say " blt_MDP.func is" blt_MDP.func
                                  say " blt_MDP.stat is" blt_MDP.stat
                                  if rez = 0 then do

                                     /* use GetMemo() if you don't think it's 1919! */
                                     /* not demo'ed here */

                                     /* memo updated, will not have moved memo */
                                     /* since did not increase memo size, only */
                                     /* overwrote existing data */

                                     /* for demo purpose, the memo is now deleted */

                                     say
                                     say "calling blt_DeleteMemo()"
                                     blt_MDP.=NOVALUE
                                     blt_MDP.dbfHandle = blt_OP.handle
                                     blt_MDP.memoNo = 1  /* can use 000000001 */
                                     rez = blt_DeleteMemo()
                                     say " blt_MDP.func is" blt_MDP.func
                                     say " blt_MDP.stat is" blt_MDP.stat
                                     if rez = 0 then do

                                        /* update DBF record's memo field to indicate no memo */

                                        blt_MDP.memoNo = copies(' ',10) /* 10 spaces = no memo */
                                        blt_AP.recData = substr(blt_AP.recData,1,20)blt_MDP.memoNo
                                        say
                                        say "calling blt_UpdateRecord() with '"blt_AP.recData"'"
                                        /* blt_AP.handle, recNo, recData already set */
                                        rez = blt_UpdateRecord()
                                        say " blt_AP.func is" blt_AP.func
                                        say " blt_AP.stat is" blt_AP.stat
                                        if rez = 0 then do

                                           say     /* purely demo purpose */
                                           say "calling blt_MemoBypass()  [BYPASS_FLUSH_MEMO_HEADER]"
                                           blt_MDP.=NOVALUE
                                           blt_MDP.dbfHandle = blt_OP.handle
                                           blt_MDP.memoBypass = 5 /* flush memo header */
                                           rez = blt_MemoBypass()
                                           say " blt_MDP.func is" blt_MDP.func
                                           say " blt_MDP.stat is" blt_MDP.stat
                                           if rez = 0 then do

                                              say
                                              say "Excellent!  All calls went as planned"
                                           end
                                           else do
                                              say "* ERROR * blt_MemoBypass()"
                                           end

                                        end
                                        else do
                                           say "* ERROR * blt_UpdateRecord()"
                                        end

                                     end
                                     else do
                                        say "* ERROR * blt_DeleteMemo()"
                                     end

                                  end
                                  else do
                                     say "* ERROR * blt_UpdateMemo()"
                                  end

                               end
                               else do
                                  say "* ERROR * blt_RelockData()"
                               end

                            end
                            else do
                               say "* ERROR * blt_GetMemo()"
                            end

                         end
                         else do
                            say "* ERROR * blt_GetMemoSize()"
                         end

                      end
                      else do
                         say "* ERROR * blt_GetRecord()"
                      end

                   end
                   else do
                      say "* ERROR * blt_RelockData()"
                   end

                end
                else do
                   say "* ERROR * blt_AddRecord()"
                end

             end
             else do
                say "* ERROR * blt_AddMemo()"
             end

             blt_LP.=NOVALUE
             blt_LP.handle = blt_OP.handle
             blt_LP.recStart = 0
             blt_LP.recCount = 0
             say
             say "calling blt_UnlockData()"
             rez = blt_UnlockData()
             say " blt_LP.func is" blt_LP.func
             say " blt_LP.stat is" blt_LP.stat

          end
          else do
             say "* ERROR * blt_LockData()"
          end

          /* if open, it's a good idea to close it (blt_Exit() will if not) */

          blt_HP.=NOVALUE
          blt_HP.handle = blt_OP.handle
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
