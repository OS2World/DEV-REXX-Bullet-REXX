/* 04 - data mid-level examples for Bullet/REXX */
/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_DeleteFileDos()   [to delete test-generated files]
   - blt_CreateData()
   - blt_OpenData()        [file-level locks used]
   - blt_GetDescriptor()
   - blt_AddRecord()
   - blt_GetRecord()
   - blt_UpdateRecord()
   - blt_DeleteRecord()
   - blt_UndeleteRecord()
   - blt_DebumpRecord()
   - blt_PackRecords()
   - blt_CloseData()
   - blt_Exit()
*/

/* Typically, each test routine's arg pack (blt_IP., etc.) is set to NOVALUE */
/* so that any unset variables can easily be identified.  In actual use, */
/* this would not be necessary since often arg pack values are already setup */
/* for multiple calls, where blt_?P.variable is already properly set up */

 say "Example: 04midd.cmd  (recommend output be redirected to a file)"

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
    say "calling blt_DeleteFileDos() x2 (04MIDD.DB?)"
    blt_DFP.=NOVALUE
    blt_DFP.filename = "04MIDD.DBF"
    rez = blt_DeleteFileDos();
    say " blt_DFP.func is" blt_DFP.func
    say " blt_DFP.stat is" blt_DFP.stat "(04MIDD.DBF) [stat=2, file not found, is possible]"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "04MIDD.DBT"  /* may use QuerySysVars for memo .EXT */
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(04MIDD.DBT)"

    say
    say "calling blt_CreateData()"
    blt_CDP.=NOVALUE
    blt_CDP.filename = "04MIDD.DBF"
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
       blt_OP.asMode = 18       /* 18 is 0x0012 [hex] DENYRW, R/W */
       rez = blt_OpenData()     /* see blt_Lock() for range locking */
       say " blt_OP.func is" blt_OP.func
       say " blt_OP.stat is" blt_OP.stat

       if rez = 0 then do
          say " blt_OP.handle is" blt_OP.handle
          say
          say "calling blt_GetDescriptor()  [by field position, first descriptor]"
          blt_DP.=NOVALUE
          blt_DP.handle = blt_OP.handle
          blt_DP.fieldNumber = 1
          rez = blt_GetDescriptor()
          say " blt_DP.func is" blt_DP.func
          say " blt_DP.stat is" blt_DP.stat
          if rez = 0 then do

             say " blt_DP.fieldNumber is" blt_DP.fieldNumber "(the following are for this)"
             say " blt_DP.fieldOffset is" blt_DP.fieldOffset
             say " blt_DP.FD.fieldName is '"blt_DP.FD.fieldName"'"
             say " blt_DP.FD.fieldType is '"blt_DP.FD.fieldType"'"
             say " blt_DP.FD.fieldLen is" blt_DP.FD.fieldLen
             say " blt_DP.FD.fieldDC is" blt_DP.FD.fieldDC

             tmpStr = blt_DP.FD.fieldName
             say
             say "calling blt_GetDescriptor()  [by fieldName, using fieldname just gotten]"
             blt_DP.=NOVALUE
             blt_DP.handle = blt_OP.handle
             blt_DP.fieldNumber = 0
             blt_DP.FD.fieldName = tmpStr
             rez = blt_GetDescriptor()
             say " blt_DP.func is" blt_DP.func
             say " blt_DP.stat is" blt_DP.stat
             if rez = 0 then do

                say " blt_DP.FD.fieldName is '"blt_DP.FD.fieldName"' (the following are for this)"
                say " blt_DP.fieldNumber is" blt_DP.fieldNumber
                say " blt_DP.fieldOffset is" blt_DP.fieldOffset
                say " blt_DP.FD.fieldType is '"blt_DP.FD.fieldType"'"
                say " blt_DP.FD.fieldLen is" blt_DP.FD.fieldLen
                say " blt_DP.FD.fieldDC is" blt_DP.FD.fieldDC

                say
                say "calling blt_AddRecord(), 100 times"
                blt_AP.=NOVALUE
                blt_AP.handle = blt_OP.handle

                /* generates SSN numbers from 465100001 to 465100100 */
                /* leading delete tag field (space) */
                /* NAME field just filled in with ... nothing important */
                /* this loop writes 100 records to disk */
                /* raw record is SSN(9), NAME(10), ADDR(10/memo number) */
                /* with the memo number in ADDR left empty */

                do record = 100001 to 100100 until rez <> 0
                   blt_AP.recData = " 465"record"name-nadaZ          "
                   rez = blt_AddRecord()
                end
                say " blt_AP.func is" blt_AP.func
                say " blt_AP.stat is" blt_AP.stat
                if rez = 0 then do

                   tmpVar = record - 100001   /* record here is do limit +1 */
                   say " Done writing the" tmpVar "test records"
                   say
                   say "calling blt_GetRecord()," tmpVar "times, unparsed output"
                   blt_AP.=NOVALUE            /* always reset for demo purposes only */
                   blt_AP.handle = blt_OP.handle
                   blt_AP.recNo = 0
                   do until rez <> 0
                      blt_AP.recNo = blt_AP.recNo + 1
                      rez = blt_GetRecord()
                      if rez = 0 then do

                         say " rec#" blt_AP.recNo "'"blt_AP.recData"'"
                         tmpVarStr = blt_AP.recData  /* save last valid data */
                      end
                   end

                   /* if read all records written, and result is 8609 */
                   /* (expected) then okay */

                   if ((blt_AP.recNo-1) = tmpVar) & (blt_AP.stat = 8609) then do
                      blt_AP.stat = 0
                      rez = 0
                   end
                   say " blt_AP.func is" blt_AP.func
                   say " blt_AP.stat is" blt_AP.stat
                   if rez = 0 then do

                      /* non-index update of data record, see blt_Update() */
                      /* for data+index updating */

                      tmpVar = blt_AP.recNo - 1  /* last valid rec# read, used again below */

                      say
                      say "calling blt_UpdateRecord()  (update last record w/ new name)"
                      blt_AP.=NOVALUE
                      blt_AP.handle = blt_OP.handle
                      blt_AP.recNo = tmpVar     /* last recNo, last data just read */
                      blt_AP.recData = substr(tmpVarStr,1,10)"newnameXYZ          "
                      rez = blt_UpdateRecord()
                      say " blt_AP.func is" blt_AP.func
                      say " blt_AP.stat is" blt_AP.stat
                      if rez = 0 then do

                         /* blt_AP.handle and .recNo already set */
                         say
                         say "calling blt_GetRecord() on the updated record -- note newnameXYZ"
                         rez = blt_GetRecord()
                         if rez = 0 then do

                            say " rec#" blt_AP.recNo "'"blt_AP.recData"'"
                            say
                            say "calling blt_DeleteRecord() on first record, then"
                            say "calling blt_UndeleteRecord() on first record, then"
                            say "calling blt_DeleteRecord() on first record (again)"
                            blt_AP.=NOVALUE
                            blt_AP.handle = blt_OP.handle
                            blt_AP.recNo = 1
                            rez = blt_DeleteRecord()
                            say " blt_AP.func is" blt_AP.func " (DeleteRecord)"
                            say " blt_AP.stat is" blt_AP.stat

                            /* here the first record has had its delete */
                            /* tag byte set to a * -- use blt_GetRecord() */
                            /* to verify if you don't believe it */
                            /* This does -not- physically remove the record */

                            if rez = 0 then do

                               rez = blt_UndeleteRecord()
                               say " blt_AP.func is" blt_AP.func " (UndeleteRecord)"
                               say " blt_AP.stat is" blt_AP.stat
                               if rez = 0 then do

                                  rez = blt_DeleteRecord() /* for pack test */
                                  say " blt_AP.func is" blt_AP.func " (DeleteRecord -second time)"
                                  say " blt_AP.stat is" blt_AP.stat
                               end
                            end

                            /* slightly different flow ('end's above) just */
                            /* to minimize indents, etc. */

                            if rez = 0 then do

                               say
                               say "calling blt_DebumpRecord() (physical removal of last record)"
                               blt_AP.=NOVALUE
                               blt_AP.handle = blt_OP.handle
                               blt_AP.recNo = tmpVar    /* tmpVar=last recNo set above */
                               rez = blt_DebumpRecord()
                               say " blt_AP.func is" blt_AP.func
                               say " blt_AP.stat is" blt_AP.stat
                               if rez = 0 then do

                                  say
                                  say "calling blt_PackRecords() (removes records marked as deleted)"
                                  blt_AP.=NOVALUE
                                  blt_AP.handle = blt_OP.handle
                                  rez = blt_PackRecords()
                                  say " blt_AP.func is" blt_AP.func
                                  say " blt_AP.stat is" blt_AP.stat
                                  if rez = 0 then do

                                     say
                                     say "Excellent!  All calls went as planned"
                                  end
                                  else do
                                     say "* ERROR * blt_PackRecords()"
                                  end

                               end
                               else do
                                  say "* ERROR * blt_DebumpRecord"
                               end

                            end
                            else do
                               say "* ERROR * blt_DeleteRecord or UndeleteRecord()"
                            end

                         end
                         else do
                            say "* ERROR * blt_GetRecord()"
                         end

                      end
                      else do
                         say "* ERROR * blt_UpdateRecord()"
                      end

                   end
                   else do
                      say "* ERROR * blt_GetRecord() at record"record+1
                   end

                end
                else do
                   say "* ERROR * blt_AddRecord() at record"record
                end

             end
             else do
                say "* ERROR * blt_GetDescriptor() [by fieldName]"
             end

          end
          else do
             say "* ERROR * blt_GetDescriptor() [by fieldNumber]"
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
