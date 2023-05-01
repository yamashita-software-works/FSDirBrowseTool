/******************************************************************************

Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: Common header file containing handy macros and definitions
         used throughout all the applications in the book.
         See Appendix A.
******************************************************************************/

#pragma once   // Include this header file once per compilation unit

////////////// Allow code to compile cleanly at warning level 4 ///////////////


/* nonstandard extension 'single line comment' was used */
#pragma warning(disable:4001)

// unreferenced formal parameter
#pragma warning(disable:4100)

// Note: Creating precompiled header 
#pragma warning(disable:4699)

// function not inlined
#pragma warning(disable:4710)

// unreferenced inline function has been removed
#pragma warning(disable:4514)

// assignment operator could not be generated
#pragma warning(disable:4512)

// cast truncates constant value
#pragma warning(disable:4310)


///////////////////////// Pragma message helper macro /////////////////////////


/* 
When the compiler sees a line like this:
   #pragma chMSG(Fix this later)

it outputs a line like this:

  c:\CD\CmnHdr.h(82):Fix this later

You can easily jump directly to this line and examine the surrounding code.
*/

#ifndef chMSG
#define chSTR2(x)   #x
#define chSTR(x)    chSTR2(x)
#define chMSG(desc) message(__FILE__ "(" chSTR(__LINE__) "):" #desc)
#endif

////////////////////////////// chINRANGE Macro ////////////////////////////////


// This macro returns TRUE if a number is between two others
#define chINRANGE(low, Num, High) (((low) <= (Num)) && ((Num) <= (High)))


//////////////////////////////// chDIMOF Macro ////////////////////////////////


// This macro evaluates to the number of elements in an array. 
#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))


///////////////////////////// chSIZEOFSTRING Macro ////////////////////////////


// This macro evaluates to the number of bytes needed by a string.
#define chSIZEOFSTRING(psz)   ((lstrlen(psz) + 1) * sizeof(TCHAR))


/////////////////// chROUNDDOWN & chROUNDUP inline functions //////////////////


// This inline function rounds a value down to the nearest multiple
template <class TV, class TM>
inline TV chROUNDDOWN(TV Value, TM Multiple) {
   return((Value / Multiple) * Multiple);
}


// This inline function rounds a value down to the nearest multiple
template <class TV, class TM>
inline TV chROUNDUP(TV Value, TM Multiple) {
   return(chROUNDDOWN(Value, Multiple) + 
      (((Value % Multiple) > 0) ? Multiple : 0));
}


///////////////////////////// chBEGINTHREADEX Macro ///////////////////////////


// This macro function calls the C runtime's _beginthreadex function. 
// The C runtime library doesn't want to have any reliance on Windows' data 
// types such as HANDLE. This means that a Windows programmer needs to cast
// values when using _beginthreadex. Since this is terribly inconvenient, 
// I created this macro to perform the casting.
typedef unsigned (__stdcall *PTHREAD_START) (void *);

#define chBEGINTHREADEX(psa, cbStack, pfnStartAddr, \
   pvParam, fdwCreate, pdwThreadId)                 \
      ((HANDLE)_beginthreadex(                      \
         (void *)        (psa),                     \
         (unsigned)      (cbStack),                 \
         (PTHREAD_START) (pfnStartAddr),            \
         (void *)        (pvParam),                 \
         (unsigned)      (fdwCreate),               \
         (unsigned *)    (pdwThreadId)))

/////////////////////////// Software Exception Macro //////////////////////////

// Useful macro for creating your own software exception codes
#define MAKESOFTWAREEXCEPTION(Severity, Facility, Exception) \
   ((DWORD) ( \
   /* Severity code    */  (Severity       ) |     \
   /* MS(0) or Cust(1) */  (1         << 29) |     \
   /* Reserved(0)      */  (0         << 28) |     \
   /* Facility code    */  (Facility  << 16) |     \
   /* Exception code   */  (Exception <<  0)))

// Put up a message box if an assertion fails in a debug build.
#ifdef _DEBUG
#define chASSERT(x) if (!(x)) chASSERTFAIL(__FILE__, __LINE__, #x)
#else
#define chASSERT(x)
#endif


// Put up a failure message box in a debug build.
#ifdef _DEBUG
#define chFAIL() chASSERTFAIL(__FILE__, __LINE__, "")
#else
#define chFAIL()
#endif

// Assert in debug builds, but don't remove the code in retail builds.
#ifdef _DEBUG
#define chVERIFY(x) chASSERT(x)
#else
#define chVERIFY(x) (x)
#endif

//////////////////////// Dialog Box Icon Setting Macro ////////////////////////

// Sets the dialog box icons
inline void chSETDLGICONS(HWND hwnd, int idi) {
   SendMessage(hwnd, WM_SETICON, TRUE,  (LPARAM) 
      LoadIcon((HINSTANCE)(ULONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 
         MAKEINTRESOURCE(idi)));
   SendMessage(hwnd, WM_SETICON, FALSE, (LPARAM) 
      LoadIcon((HINSTANCE)(ULONG_PTR)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 
      MAKEINTRESOURCE(idi)));
}

///////////////////////////////// End of File /////////////////////////////////
