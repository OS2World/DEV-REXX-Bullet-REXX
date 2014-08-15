
/* This WinMain() is for Windows apps (Win32s specifically).
 * For console apps see bd_main.c, instead.
 *
 * bd_winmn.c - 12-Oct-1996 Cornel Huth
 *
 * This module calls the bd_main.c module if FOR_WINDOWS==1
 * (and only if ON_WIN32==1) otherwise this is a null module.
 *
 */


#include "platform.h"         // defines platform, brings in includes

#if FOR_WINDOWS == 1          // for ON_WIN32 only

// external to this module

int main2();             // the regular _main()
void PutMsg(CHAR *strg); // writes text
void GetMsg(CHAR *strg); // gets text


// public to this module

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM uParam,LPARAM lParam);

// global to this exe

HWND gHwnd=0;
int gCurrStr=0;         // current element in gPutStr[n][0]  (n=1 to 39 strings, init to 0)
CHAR gPutStr[40][82];   // output from bd_*.c modules should be less than this size (PutMsg())

int gInputReady=0;      // flag when CR hit
int gCurrChar=0;        // current character pointer, in gGetStr[] (n=0 to 79)
CHAR gGetStr[82];       // input from bd_*.c (GetMsg())

BOOL gDie=FALSE;
MSG gMsg;
void DoWinThing(int waitFlag);
void DoWinThing2(void);


// WinMain program start ////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow) {

 static CHAR appName[] = "bd9wdemo";
 HWND hwnd;
 WNDCLASS wc;


 if (!hPrevInstance) {
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC) WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = appName;
    wc.lpszClassName = appName;

    RegisterClass(&wc);
 }
 else {
    if ((GetVersion() & 0x80000000)==0) return 0; // only play once if win32s in win31
 }


 hwnd = CreateWindow(appName,"Bullet 2 Demo Starter for Windows", WS_OVERLAPPEDWINDOW,
                     CW_USEDEFAULT,CW_USEDEFAULT,
                     CW_USEDEFAULT,CW_USEDEFAULT,
                     NULL,NULL,hInstance,NULL);

 if (!hwnd) return (FALSE);
 gHwnd = hwnd;
 ShowWindow(hwnd, nCmdShow);
 UpdateWindow(hwnd);

 // do the hustle

 main2();

 if (gDie==FALSE) DoWinThing(1);   // wait for death
 return gMsg.wParam;

 lpCmdLine;
}


// Handle message queue here

void DoWinThing(int waitFlag) {

 if (waitFlag == 0) {
    while (PeekMessage(&gMsg,NULL,0,0,PM_NOREMOVE)) {
       DoWinThing2();
    }
 }
 else {
    DoWinThing2();
 }
 return;
}


// Handle
void DoWinThing2(void) {

 if (GetMessage(&gMsg, NULL, 0, 0)) {
    TranslateMessage(&gMsg);
    DispatchMessage(&gMsg);
 }
 else {
    gDie = TRUE;
 }
 return;
}


// Windows handler

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM uParam,LPARAM lParam) {

 TEXTMETRIC tm;
 PAINTSTRUCT ps;
 HDC hdc;

 int i,j,k,t;

 static int cxChar,cyChar;
 static int outRow,outCol;

 CHAR c;
 CHAR lnBuffer[128];
 CHAR *lnPtr;
 int lnLen;
 int outColFlag;

 switch (message) {
 case WM_CREATE:
    hdc = GetDC(hwnd);
    SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));
    GetTextMetrics(hdc,&tm);
    cxChar = tm.tmAveCharWidth;
    cyChar = tm.tmHeight;
    ReleaseDC(hwnd,hdc);
    outRow = 0;
    outCol = 0;
    break;

 case WM_SIZE:
    break;

 case WM_SETFOCUS:
    CreateCaret(hwnd,(HBITMAP)1,cxChar,cyChar);
    ShowCaret(hwnd);
    break;

 case WM_KILLFOCUS:
    HideCaret(hwnd);
    DestroyCaret();
    break;

 case WM_PAINT:
    hdc = BeginPaint(hwnd,&ps);
    SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));

    outRow=0;
    outCol=0;
    outColFlag = 0;
    lnPtr = lnBuffer;
    lnLen = 0;

    for (i=1; i <= gCurrStr; i++) {
       k = strlen(&gPutStr[i][0]);
       for (j=0; j < k; j++) {
          c = gPutStr[i][j];
          switch (c) {
          case '\n':
             if (lnLen) TextOut(hdc,(outCol*cxChar),(outRow*cyChar),lnBuffer,lnLen);
             lnPtr = lnBuffer;
             lnLen = 0;
             outCol = 0;
             outRow++;
             break;
          case '\r':
             outColFlag = 1;
             break;
          case '\x0C':  // form-feed (rest of gPutStr[] line ignored)
             // cls
             gCurrStr=0;
             InvalidateRect(gHwnd,NULL,TRUE);
             break;

          default:
             if (c >= 32) {
                *lnPtr++ = c;
                lnLen++;
             }
          }
       }
       if (lnLen) {
          TextOut(hdc,(outCol*cxChar),(outRow*cyChar),lnBuffer,lnLen);
          outCol = outCol + lnLen;
          lnPtr = lnBuffer;
          lnLen = 0;
       }
       if (outColFlag) {
          outCol = 0;
          outColFlag = 0;
       }
    }

    SetCaretPos(outCol*cxChar,outRow*cyChar);
    EndPaint(hwnd,&ps);
    break;

 case WM_CHAR:
    HideCaret(hwnd);
    for (i=0; i < (int)LOWORD(lParam); i++) {
       switch(LOWORD(uParam)) {
       case '\r':
          outRow++;
          outCol=0;
          gGetStr[gCurrChar] = '\x0';
          gInputReady = 1;             // used in GetMsg() in bd_main.c
          break;
       case '\n':
          break;
       case '\b':
          if (gCurrChar > 0) {
             gCurrChar--;
             gGetStr[gCurrChar] = '\x0';
             t = strlen(&gPutStr[gCurrStr][0]);
             if (t > 0) gPutStr[gCurrStr][t-1] = 0;
             if (outCol > 0) outCol--;  // play it safe
             hdc = GetDC(hwnd);
             SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));
             TextOut(hdc,(outCol*cxChar),(outRow*cyChar)," ",1);
             ReleaseDC(hwnd, hdc);
          }
          break;
       default:
          if (gCurrChar < sizeof(gGetStr)-1) {
             c = (CHAR)LOWORD(uParam);
             if ((c >= 32) & (c < 127)) {
                gGetStr[gCurrChar] = c;
                t = strlen(&gPutStr[gCurrStr][0]);
                gPutStr[gCurrStr][t] = c;
                gPutStr[gCurrStr][t+1] = '\x0';
                hdc = GetDC(hwnd);
                SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));
                TextOut(hdc,(outCol*cxChar),(outRow*cyChar),&c,1);
                ReleaseDC(hwnd, hdc);
                gCurrChar++;
                outCol++;
             }
          }
          else {
             gGetStr[gCurrChar] = '\x0';
             gInputReady = 1;
          }
          break;
       }
    }
    SetCaretPos(outCol*cxChar,outRow*cyChar);
    ShowCaret(hwnd);
    break;

 case WM_DESTROY:
    PostQuitMessage(0);
    break;

 default:
    return (DefWindowProc(hwnd, message, uParam, lParam));
 }
 return 0;
}

#endif  // FOR_WINDOWS==1












