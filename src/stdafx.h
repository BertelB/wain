// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef STDAFX_H_INC
#define STDAFX_H_INC

#pragma warning( disable : 4996 ) // Yes, I use sprintf and family

#define VC_EXTRALEAN         // Exclude rarely-used stuff from Windows headers
#define OEMRESOURCE

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxmt.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <CRTDBG.H>
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>      // MFC support for Windows Common Controls
#include <afxadv.h>
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxpriv.h>
#include <afxcview.h>
#include <afxtempl.h>
#include <afxinet.h>

#include <vector>
#include <list>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>
#include <direct.h>
#include <io.h>
#include <shlobj.h>
#include <locale.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <cderr.h>
#include <ctype.h>

#pragma warning( disable : 4018 ) // Signed / unsigned mismatch

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// #pragma comment(linker, "\"/manifestdependency:type='Win32' name='Test.Research.SampleAssembly' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='0000000000000000' language='*'\"")

#endif // AFX_STDAFX_H_INC