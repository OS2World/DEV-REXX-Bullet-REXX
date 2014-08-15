/* 01 - Init example for Bullet/REXX */
/* 3-Aug-96
   Calls made in this example:
   - blt_Init()
   - blt_Memory()
   - blt_GetErrorClass()
   - blt_SetSysVars()
   - blt_QuerySysVars()
   - blt_Exit()
*/

/* Typically, each test routine's arg pack (blt_IP., etc.) is set to NOVALUE */
/* so that any unset variables can easily be identified.  In actual use, */
/* this would not be necessary since often arg pack values are already setup */
/* for multiple calls, where blt_?P.variable is already properly set up */

 say "Example: 01init.cmd  (recommend output be redirected to a file)"

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
    say "calling blt_Memory()"
    blt_MP.=NOVALUE
    rez = blt_Memory()  /* get private arena memory avail */
    say " blt_MP.func is" blt_MP.func
    say " blt_MP.stat is" blt_MP.stat
    say " blt_MP.memory is" blt_MP.memory

    say
    say "calling blt_GetErrorClass()"
    blt_XEP.=NOVALUE
    blt_XEP.stat = 0    /* use any OS error code */
    rez = blt_GetErrorClass()
    say " blt_XEP.func is" blt_XEP.func
    say " blt_XEP.stat is" blt_XEP.stat
    say " blt_XEP.errClass is" blt_XEP.errClass
    say " blt_XEP.action is" blt_XEP.action
    say " blt_XEP.location is" blt_XEP.location

    say
    say "calling blt_SetSysVars() to set lock file region timeout to 500ms"
    blt_QSP.=NOVALUE
    blt_QSP.item = 30   /* SET lock file region timeout */
    blt_QSP.itemValue = 500  /* 500ms timeout, previous timeout on return */
    rez = blt_SetSysVars()
    say " blt_QSP.func is" blt_QSP.func
    say " blt_QSP.stat is" blt_QSP.stat
    say " blt_QSP.item is" blt_QSP.item
    say " blt_QSP.itemValue previous timeout was" blt_QSP.itemValue

    say
    say "calling blt_QuerySysVars() to verify change to 500ms"
    blt_QSP.=NOVALUE
    blt_QSP.item = 30   /* QUERY lock file region timeout (just set) */
    rez = blt_QuerySysVars()
    say " blt_QSP.func is" blt_QSP.func
    say " blt_QSP.stat is" blt_QSP.stat
    say " blt_QSP.item is" blt_QSP.item
    say " blt_QSP.itemValue is" blt_QSP.itemValue

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
