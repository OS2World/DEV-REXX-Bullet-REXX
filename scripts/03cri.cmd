/* 03 - Create index example for Bullet/REXX */
/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_DeleteFileDos()   [to delete test-generated files]
   - blt_CreateData()
   - blt_OpenData()        [file-level locks used]
   - blt_CreateIndex()
   - blt_OpenIndex()
   - blt_FlushIndexHeader()
   - blt_ReadIndexHeader()
   - blt_CopyIndexHeader()
   - blt_ZapIndexHeader()
   - blt_StatIndex()
   - blt_CloseIndex()
   - blt_CloseData()
   - blt_Exit()
*/

/* Typically, each test routine's arg pack (blt_IP., etc.) is set to NOVALUE */
/* so that any unset variables can easily be identified.  In actual use, */
/* this would not be necessary since often arg pack values are already setup */
/* for multiple calls, where blt_?P.variable is already properly set up */

 say "Example: 03cri.cmd  (recommend output be redirected to a file)"

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
    say "calling blt_DeleteFileDos() x4 (03CRI.DB?, 03CRI.IX3 O3CRIcpy.IX3)"
    blt_DFP.=NOVALUE
    blt_DFP.filename = "03CRI.DBF"
    rez = blt_DeleteFileDos();
    say " blt_DFP.func is" blt_DFP.func
    say " blt_DFP.stat is" blt_DFP.stat "(03CRI.DBF) [stat=2, file not found, is possible]"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "03CRI.DBT"  /* may use QuerySysVars for memo .EXT */
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(03CRI.DBT)"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "03CRI.IX3"
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(03CRI.IX3)"

    blt_DFP.=NOVALUE
    blt_DFP.filename = "03CRIcpy.IX3"   /* from CopyIndexHeader() */
    rez = blt_DeleteFileDos();
    say " blt_DFP.stat is" blt_DFP.stat "(03CRIcpy.IX3)"

    say
    say "calling blt_CreateData()"
    blt_CDP.=NOVALUE
    blt_CDP.filename = "03CRI.DBF"
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

       /* so far, similar to 02CRD.CMD */
       /* must have data DBF open before creating index for it */

       dataID = 0       /* since two opens are done, cannot keep both handles */
       indexID = 0      /* in the same blt_OP.handle var, so do this */

       if rez = 0 then do
          dataID = blt_OP.handle
          say " blt_OP.handle is" dataID
          say
          say "calling blt_CreateIndex()"
          blt_CIP.=NOVALUE
          blt_CIP.filename = "03CRI.IX3"
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
             blt_OP.asMode = 18

             /* index open requires blt_OP.xbLink set to DBF handle */
             /* whereas .xbLink is not used in a data open */

             blt_OP.xbLink = dataID
             rez = blt_Openindex()
             say " blt_OP.func is" blt_OP.func
             say " blt_OP.stat is" blt_OP.stat

             if rez = 0 then do
                indexID = blt_OP.handle
                say " blt_OP.handle is" indexID
                blt_HP.=NOVALUE
                blt_HP.handle = indexID
                say
                say "calling blt_FlushIndexHeader()"  /* purely for demo purpose */
                rez = blt_FlushIndexHeader()
                say " blt_HP.func is" blt_HP.func
                say " blt_HP.stat is" blt_HP.stat

                if rez = 0 then do
                   blt_HP.=NOVALUE
                   blt_HP.handle = indexID
                   say
                   say "calling blt_ReadIndexHeader()"  /* purely for demo purpose */
                   rez = blt_ReadIndexHeader()
                   say " blt_HP.func is" blt_HP.func
                   say " blt_HP.stat is" blt_HP.stat

                   if rez = 0 then do
                      blt_CP.=NOVALUE
                      blt_CP.handle = indexID
                      blt_CP.filename = "03CRIcpy.IX3"
                      say
                      say "calling blt_CopyIndexHeader()"  /* purely for demo purpose */
                      rez = blt_CopyIndexHeader()
                      say " blt_CP.func is" blt_CP.func
                      say " blt_CP.stat is" blt_CP.stat

                      if rez = 0 then do
                         blt_HP.=NOVALUE
                         blt_HP.handle = indexID        /* zaps .IX3 */
                         say
                         say "calling blt_ZapIndexHeader()"  /* purely for demo purpose */
                         rez = blt_ZapIndexHeader()
                         say " blt_HP.func is" blt_HP.func
                         say " blt_HP.stat is" blt_HP.stat

                         if rez = 0 then do
                            say
                            say "calling blt_StatIndex()"
                            blt_SIP.=NOVALUE
                            blt_SIP.handle = indexID
                            rez = blt_StatIndex()
                            say " blt_SIP.func is" blt_SIP.func
                            say " blt_SIP.stat is" blt_SIP.stat
                            if rez = 0 then do

                               say " blt_SIP.handle is" blt_SIP.handle
                               say " blt_SIP.fileType is" blt_SIP.fileType
                               say " blt_SIP.flags is" blt_SIP.flags
                               say " blt_SIP.progress is" blt_SIP.progress
                               say " blt_SIP.morePtr is" blt_SIP.morePtr
                               say " blt_SIP.xbLink is" blt_SIP.xbLink
                               say " blt_SIP.asMode is" blt_SIP.asMode
                               say " blt_SIP.filename is '"blt_SIP.filename"'"
                               say " blt_SIP.fileID is '"blt_SIP.fileID"'"
                               say " blt_SIP.keyExp is" "'"blt_SIP.keyExp"'"
                               say " blt_SIP.keys is" blt_SIP.keys
                               say " blt_SIP.keyLength is" blt_SIP.keyLength
                               say " blt_SIP.keyRecNo is" blt_SIP.keyRecNo
                               say " blt_SIP.key is '"blt_SIP.key"'"
                               say " blt_SIP.herePtr is" blt_SIP.herePtr
                               say " blt_SIP.codePage is" blt_SIP.codePage
                               say " blt_SIP.countryCode is" blt_SIP.countryCode
                               /* will be null-string, be aware that if was not default */
                               /* many of the table bytes are not printable characters */
                               say " blt_SIP.collateTable is '"blt_SIP.collatetable"'"
                               say " blt_SIP.nodeSize is" blt_SIP.nodeSize
                               dupsFlag = blt_SIP.sortFunction % 65536
                               sortFunc = blt_SIP.sortFunction - (dupsFlag * 65536)
                               say " blt_SIP.sortFunction is" blt_SIP.sortFunction
                               say "    dups allowed flag is" dupsFlag
                               say "        sort function is" sortFunc
                               say " blt_SIP.lockCount is" blt_SIP.lockCount
                            end
                            else do
                               say "* ERROR * blt_StatIndex()"
                            end

                         end
                         else do
                            say "* ERROR * blt_ZapIndexHeader()"
                         end

                      end
                      else do
                         say "* ERROR * blt_CopyIndexHeader()"
                      end

                   end
                   else do
                     say "* ERROR * blt_ReadIndexHeader()"
                   end

                end
                else do
                   say "* ERROR * blt_FlushIndexHeader()"
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
