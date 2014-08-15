/* 08 - CP/DosXXX example for Bullet/REXX */
/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_MkDirDos() [optional]
   - blt_CreateFileDos()
   - blt_AccessFileDos()
   - blt_OpenFileDos()
   - blt_ExpandFileDos()
   - blt_SeekFileDos()
   - blt_WriteFileDos()
   - blt_ReadFileDos()
   - blt_CommitFileDos()
   - blt_CloseFileDos()
   - blt_RenameFileDos()
   - blt_DeleteFileDos()
   - blt_Exit()
*/

/* Typically, each test routine's arg pack (blt_IP., etc.) is set to NOVALUE */
/* so that any unset variables can easily be identified.  In actual use, */
/* this would not be necessary since often arg pack values are already setup */
/* for multiple calls, where blt_?P.variable is already properly set up */

 say "Example: 08cp.cmd  (recommend output be redirected to a file)"

 call RxFuncAdd 'BulletLoadFuncs', 'BREXXI2', 'BulletLoadFuncs'
 call BulletLoadFuncs

 testFilename = "08CP.RAW"
 testNewname  = "08CP.REN"
 testWriteData = "<08 test data3456 (rest of data in this file is uninitialized disk space)>"

 fileID = 0

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

    /* delete lingering file by both original and new names */

    blt_DFP.=NOVALUE
    blt_DFP.filename = testFilename
    rez = blt_DeleteFileDos();
    blt_DFP.filename = testNewname
    rez = blt_DeleteFileDos();
    rez = 0  /* ignore any error */

    /* since there's no blt_RmDir() this defaults to not-done */
    /*
    say
    say "calling blt_MkDirDos()"
    blt_DFP.=NOVALUE
    blt_DFP.filename = ".\08DIR"
    rez = blt_MkDir()
    say " blt_DFP.func is" blt_DFP.func
    say " blt_DFP.stat is" blt_DFP.stat
    if rez = 0 then rez = 0
    */

    if rez = 0 then do
       say
       say "calling blt_CreateFileDos()"
       blt_DFP.=NOVALUE
       blt_DFP.filename = testFilename
       blt_DFP.attr = 0
       rez = blt_CreateFileDos()
       say " blt_DFP.func is" blt_DFP.func
       say " blt_DFP.stat is" blt_DFP.stat
       if rez = 0 then do

          say
          say "calling blt_AccessFileDos()  [asMode=18, or 0x12=R/W access & DENY-R/W share]"
          blt_DFP.=NOVALUE
          blt_DFP.filename = testFilename
          blt_DFP.asMode = 18
          rez = blt_AccessFileDos()
          say " blt_DFP.func is" blt_DFP.func
          say " blt_DFP.stat is" blt_DFP.stat
          if rez = 0 then do

             say
             say "calling blt_OpenFileDos()"
             blt_DFP.=NOVALUE
             blt_DFP.filename = testFilename
             blt_DFP.asMode = 18
             rez = blt_OpenFileDos()
             say " blt_DFP.func is" blt_DFP.func
             say " blt_DFP.stat is" blt_DFP.stat
             if rez = 0 then do

                fileID = blt_DFP.handle
                say " blt_DFP.handle is" fileID
                say
                say "calling blt_ExpandFileDos()  [expanding empty file to 888 bytes]"
                blt_DFP.=NOVALUE
                blt_DFP.handle = fileID
                blt_DFP.bytes = 888
                rez = blt_ExpandFileDos()
                say " blt_DFP.func is" blt_DFP.func
                say " blt_DFP.stat is" blt_DFP.stat
                if rez = 0 then do

                   say
                   say "calling blt_SeekFileDos() [getting filesize]"
                   blt_DFP.=NOVALUE
                   blt_DFP.handle = fileID
                   blt_DFP.seekTo = 0
                   blt_DFP.method = 2
                   rez = blt_SeekFileDos()
                   say " blt_DFP.func is" blt_DFP.func
                   say " blt_DFP.stat is" blt_DFP.stat
                   filesize = blt_DFP.seekTo

                   /* restore file pointer to start of file (without any fuss) */

                   if rez = 0 then do
                      blt_DFP.seekTo = 0
                      blt_DFP.method = 0
                      rez = blt_SeekFileDos()
                      if rez then say "Re-seek after filesize failed:" rez
                   end
                   if rez = 0 then do

                      say " blt_DFP.SeekTo (filesize) is" filesize
                      say
                      say "calling blt_WriteFileDos()"
                      blt_DFP.=NOVALUE
                      blt_DFP.handle = fileID
                      blt_DFP.bytes = length(testWriteData)
                      blt_DFP.bufferData = testWriteData
                      rez = blt_WriteFileDos()
                      say " blt_DFP.func is" blt_DFP.func
                      say " blt_DFP.stat is" blt_DFP.stat
                      if rez = 0 then do

                         bytesWritten = blt_DFP.bytes
                         blt_DFP.seekTo = 0
                         blt_DFP.method = 0
                         rez = blt_SeekFileDos()
                         if rez then say "Re-seek after write failed:" rez
                         if rez = 0 then do

                            say
                            say "calling blt_ReadFileDos()  [read" bytesWritten "bytes written]"
                            blt_DFP.=NOVALUE
                            blt_DFP.handle = fileID
                            blt_DFP.bytes = bytesWritten
                            rez = blt_ReadFileDos()
                            say " blt_DFP.func is" blt_DFP.func
                            say " blt_DFP.stat is" blt_DFP.stat
                            if rez = 0 then do

                               say " blt_ReadFileDos() buffer is '"blt_DFP.bufferData"'"
                               say
                               say "calling blt_CommitFileDos()"
                               blt_DFP.=NOVALUE
                               blt_DFP.handle = fileID
                               rez = blt_CommitFileDos()
                               say " blt_DFP.func is" blt_DFP.func
                               say " blt_DFP.stat is" blt_DFP.stat
                               if rez = 0 then do

                                  say
                                  say "calling blt_CloseFileDos()"
                                  blt_DFP.=NOVALUE
                                  blt_DFP.handle = fileID
                                  rez = blt_CloseFileDos()
                                  say " blt_DFP.func is" blt_DFP.func
                                  say " blt_DFP.stat is" blt_DFP.stat
                                  if rez = 0 then do

                                     fileID = 0  /* handle no longer exists */
                                     say
                                     say "calling blt_RenameFileDos()"
                                     blt_DFP.=NOVALUE
                                     blt_DFP.filename = testFilename
                                     blt_DFP.newName = testNewname
                                     rez = blt_RenameFileDos()
                                     say " blt_DFP.func is" blt_DFP.func
                                     say " blt_DFP.stat is" blt_DFP.stat
                                     if rez = 0 then do

                                        say
                                        say "calling blt_DeleteFileDos()"
                                        blt_DFP.=NOVALUE
                                        blt_DFP.filename = testNewname
                                        rez = blt_DeleteFileDos()
                                        say " blt_DFP.func is" blt_DFP.func
                                        say " blt_DFP.stat is" blt_DFP.stat
                                        if rez = 0 then do

                                           say
                                           say "Excellent!  All calls went as planned"
                                        end
                                        else do
                                           say "* ERROR * blt_DeleteFileDos()"
                                        end

                                     end
                                     else do
                                        say "* ERROR * blt_RenameFileDos()"
                                     end

                                  end
                                  else do
                                     say "* ERROR * blt_CloseFileDos()"
                                  end

                               end
                               else do
                                  say "* ERROR * blt_CommitFileDos()"
                               end

                            end
                            else do
                               say "* ERROR * blt_ReadFileDos()"
                            end

                         end
                         else do
                            say "* ERROR * blt_SeekFileDos()"
                         end

                      end
                      else do
                         say "* ERROR * blt_WriteFileDos()"
                      end

                   end
                   else do
                      say "* ERROR * blt_SeekFileDos()"
                   end

                end
                else do
                   say "* ERROR * blt_ExpandFileDos()"
                end

                if fileID <> 0 then do
                   blt_DFP.handle = fileID
                   rez = blt_CloseFileDos()
                end

             end
             else do
                say "* ERROR * blt_OpenFileDos()"
             end

          end
          else do
             say "* ERROR * blt_AccessFileDos"
          end

          /* get rid of lingering file or leave them to */
          /* as done here so you can check it out on error */
          /*
          blt_DFP.filename = testFilename
          rez = blt_DeleteFileDos();
          blt_DFP.filename = testNewname
          rez = blt_DeleteFileDos();
          */

       end
       else do
          say "* ERROR * blt_CreateFileDos()"
       end

    end
    else do
       say
       say "* ERROR * blt_MkDir()  (optionally called)"
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
