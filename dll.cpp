/*!
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/userex/functions/openthemedata.asp
* \file dll.cpp
* \brief Main DLL File
*
* This file contains the main DCX dll routines.
*
* \author David Legault ( clickhere at scriptsdb dot org )
* \version 1.0
*
* \b Revisions
*
* � ScriptsDB.org - 2006
*/

#include "defines.h"
#include "classes/dcxdialogcollection.h"
#include "classes/dcxdialog.h"
#include "classes/dcxcontrol.h"

#include "classes/custom/divider.h"
#include "classes/custom/dcxtrayicon.h"

#include "classes/xpopup/xmenubar.h"

#include "classes/xpopup/xpopupmenumanager.h"
#include "classes/xpopup/xpopupmenu.h"

#include <exdisp.h>
#include <mshtml.h>
#include <exdispid.h>

// DCX Stuff
DcxDialogCollection Dialogs; //!< blah

mIRCDLL mIRCLink; //!< blah

// Windows 2000+ Pointers
PFNGETWINDOWINFO GetWindowInfoUx = NULL;
PFNANIMATEWINDOW AnimateWindowUx = NULL;
PFNINSENDMESSAGEEX InSendMessageExUx = NULL;
PFNFLASHWINDOWEX FlashWindowExUx = NULL;

// XP+ function pointers
// Theme functions
PFNSETTHEME SetWindowThemeUx = NULL; //!< blah
PFNISTHEMEACTIVE IsThemeActiveUx = NULL; //!< blah
PFNOPENTHEMEDATA OpenThemeDataUx = NULL;
PFNCLOSETHEMEDATA CloseThemeDataUx = NULL;
PFNDRAWTHEMEBACKGROUND DrawThemeBackgroundUx = NULL;
PFNGETTHEMEBACKGROUNDCONTENTRECT GetThemeBackgroundContentRectUx = NULL;
PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT IsThemeBackgroundPartiallyTransparentUx = NULL;
PFNDRAWTHEMEPARENTBACKGROUND DrawThemeParentBackgroundUx = NULL;
PFNDRAWTHEMETEXT DrawThemeTextUx = NULL;
PFNGETTHEMEBACKGROUNDREGION GetThemeBackgroundRegionUx = NULL;
PFNGETWINDOWTHEME GetWindowThemeUx = NULL;
PFNDRAWTHEMEEDGE DrawThemeEdgeUx = NULL;
PFNGETTHEMECOLOR GetThemeColorUx = NULL;

// Others
PFNUPDATELAYEREDWINDOW UpdateLayeredWindowUx = NULL;
PFNSETLAYEREDWINDOWATTRIBUTES SetLayeredWindowAttributesUx = NULL;
PFNDRAWSHADOWTEXT DrawShadowTextUx = NULL;
PFNPICKICONDLG PickIconDlgUx = NULL;

// Vista Function pointers.
#ifdef DCX_USE_WINSDK
PFNDRAWTHEMEPARENTBACKGROUNDEX DrawThemeParentBackgroundExUx = NULL;
//PFNGETTHEMEBITMAP GetThemeBitmapUx = NULL;
PFNBUFFEREDPAINTINIT BufferedPaintInitUx = NULL;
PFNBUFFEREDPAINTUNINIT BufferedPaintUnInitUx = NULL;
PFNBEGINBUFFEREDPAINT BeginBufferedPaintUx = NULL;
PFNENDBUFFEREDPAINT EndBufferedPaintUx = NULL;
#endif
PFNDWMISCOMPOSITIONENABLED DwmIsCompositionEnabledUx = NULL;

BOOL XPPlus = FALSE;                 //!< Is OS WinXP+ ?

//FILE * logFile;

IClassFactory * g_pClassFactory = NULL; //!< Web Control Factory


// Tray Icon stuff
DcxTrayIcon *trayIcons = NULL; // tray icon manager


// XMenuBar stuff
XMenuBar g_XMenuBar; // Global XMenuBar object
HMENU g_OriginalMenuBar = NULL;
XPopupMenu *g_mIRCScriptMenu = NULL; // Wrapper for the mIRC scriptable menu.


// XPopup Stuff
XPopupMenuManager g_XPopupMenuManager; //!< Global XPopupMenu Manager

XPopupMenu * g_mIRCPopupMenu = NULL;
XPopupMenu * g_mIRCMenuBar = NULL;

BOOL isMenuBar = FALSE;
BOOL isSysMenu = FALSE;

BOOL bIsActiveMircPopup = FALSE;
BOOL bIsActiveMircMenubarPopup = FALSE;

HWND mhMenuOwner = NULL; //!< Menu Owner Window Which Processes WM_ Menu Messages

WNDPROC g_OldmIRCWindowProc = NULL;

SIGNALSWITCH dcxSignal;

/*!
* \brief mIRC DLL Load Function
*
* This function is called when the DLL is loaded.
*
* It initializes all what the DLL needs and links mIRC received information to the mIRCDLL \b mIRCLink
* data structure to be used later in various functions in the DLL.
*
* \param load mIRC Load Structure Pointer
*/
void WINAPI LoadDll(LOADINFO * load) {
	int cnt = 0;

	load->mKeep = TRUE; // keep it loaded, this is TRUE by default with 6.21+, but FALSE by default with previous versions.
	ZeroMemory(&mIRCLink,sizeof(mIRCLink));
	// If mIRC V6.2+ then try & create our own unique mapfile.
	// damn mIRC reports as 6.2 instead of 6.20!
	// meaning mirc v6.17 appears to be a higher version.
	if ((HIWORD(load->mVersion) == 2) && (LOWORD(load->mVersion) == 6))
		mIRCLink.m_bmIRCSixPointTwoZero = true;
	else
		mIRCLink.m_bmIRCSixPointTwoZero = false;

	// Check if this is mIRC V6.30+
	if ((HIWORD(load->mVersion) >= 30) && (LOWORD(load->mVersion) == 6))
		mIRCLink.m_bmIRCSixPointThree = true;
	else
		mIRCLink.m_bmIRCSixPointThree = false;

	if (mIRCLink.m_bmIRCSixPointTwoZero || ((HIWORD(load->mVersion) >= 21) && (LOWORD(load->mVersion) == 6))) {
		TString map_name;
		cnt = 1;
		mIRCLink.m_hFileMap = NULL;

		while ((mIRCLink.m_hFileMap == NULL) && (cnt < 256)) {
			// create mapfile name.
			map_name.sprintf("mIRC%d",cnt);
			// create mapfile.
			mIRCLink.m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,0,PAGE_READWRITE,0,4096,map_name.to_chr());

			// if create failed, fall back on old method.
			if ((mIRCLink.m_hFileMap == NULL) || (mIRCLink.m_hFileMap == INVALID_HANDLE_VALUE)) {
				cnt = 0;
				break;
			}

			// if mapfile already exists then close & try another name.
			if (GetLastError() == ERROR_ALREADY_EXISTS) {
				CloseHandle(mIRCLink.m_hFileMap);
				mIRCLink.m_hFileMap = NULL;
			}
			else
				break;

			cnt++;
		}

		if (cnt == 256)
			cnt = 0;
	}

	mIRCLink.m_map_cnt = cnt; // set mapfile counter for SendMessage()'s

	// use old method for < mirc 6.2 or when new method fails.
	if (cnt == 0) {
		mIRCLink.m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, 4096, "mIRC");
	}

	mIRCLink.m_pData = (LPSTR) MapViewOfFile(mIRCLink.m_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	mIRCLink.m_mIRCHWND = load->mHwnd;
	mIRCLink.m_bGhosted = false;
	mIRCLink.m_bDoGhostDrag = 255;
	mIRCLink.m_bAero = FALSE;
	mIRCLink.m_bVista = false;

	// Check if we're in debug mode
	TString isDebug((UINT)255);
	mIRCeval("$debug", isDebug.to_chr(), 255);

	isDebug.trim();
	mIRCLink.m_bisDebug = (isDebug.len() > 0);
	DCX_DEBUG("LoadDLL", "Debug mode detected...");

	// Initializing OLE Support
	DCX_DEBUG("LoadDLL", "Initializing OLE Support...");
	//CoInitialize( NULL );

	// Initializing OLE Support
	OleInitialize(NULL);

	SetupOSCompatibility();

	//get IClassFactory* for WebBrowser
	DCX_DEBUG("LoadDLL", "Generating class for WebBrowser...");
	CoGetClassObject(CLSID_WebBrowser, CLSCTX_INPROC_SERVER, 0, IID_IClassFactory, (void**) &g_pClassFactory);
	//6BF52A52-394A-11D3-B153-00C04F79FAA6

	// RichEdit DLL Loading
	DCX_DEBUG("LoadDLL", "Generating class for RichEdit...");
	LoadLibrary("RICHED20.DLL");

	// Load Control definitions
	DCX_DEBUG("LoadDLL", "Loading control classes");
	INITCOMMONCONTROLSEX icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | 
		ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS | ICC_DATE_CLASSES | 
		ICC_TAB_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS;
	InitCommonControlsEx( &icex ); 

	WNDCLASSEX wc;
	ZeroMemory((void*)&wc , sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

//#define dcxRegisterClassM(szClass, szDcxClass, CClass) { \
//	GetClassInfoEx(NULL, (TCHAR *)(szClass), &wc); \
//	wc.lpszClassName = (TCHAR *)(szDcxClass); \
//	(CClass)::m_DefaultWindowProc = wc.wc.lpfnWndProc; \
//	wc.lpfnWndProc = (CClass)::WindowProc; \
//	RegisterClassEx(&wc); \
//};
#define dcxRegisterClass(szClass, szDcxClass) { \
	GetClassInfoEx(NULL, (TCHAR *)(szClass), &wc); \
	wc.lpszClassName = (TCHAR *)(szDcxClass); \
	RegisterClassEx(&wc); \
};

	// Custom ProgressBar
	DCX_DEBUG("LoadDLL", "Registering ProgressBar...");
	dcxRegisterClass(PROGRESS_CLASS, DCX_PROGRESSBARCLASS);

	// Custom TreeView
	DCX_DEBUG("LoadDLL", "Registering TreeView...");
	dcxRegisterClass(WC_TREEVIEW, DCX_TREEVIEWCLASS);

	// Custom Toolbar
	DCX_DEBUG("LoadDLL", "Registering ToolBar...");
	dcxRegisterClass(TOOLBARCLASSNAME, DCX_TOOLBARCLASS);

	// Custom StatusBar
	DCX_DEBUG("LoadDLL", "Registering StatusBar...");
	dcxRegisterClass(STATUSCLASSNAME, DCX_STATUSBARCLASS);

	// Custom ListView
	DCX_DEBUG("LoadDLL", "Registering Listview...");
	dcxRegisterClass(WC_LISTVIEW, DCX_LISTVIEWCLASS);

	// Custom ComboEx
	DCX_DEBUG("LoadDLL", "Registering ComboEx...");
	dcxRegisterClass(WC_COMBOBOXEX, DCX_COMBOEXCLASS);

	// Custom TrackBar
	DCX_DEBUG("LoadDLL", "Registering TrackBar...");
	dcxRegisterClass(TRACKBAR_CLASS, DCX_TRACKBARCLASS);

	// Custom RichEdit
	DCX_DEBUG("LoadDLL", "Registering RichEdit...");
	dcxRegisterClass("RichEdit20A", DCX_RICHEDITCLASS);

	// Custom RebarCtrl
	DCX_DEBUG("LoadDLL", "Registering ReBar...");
	dcxRegisterClass(REBARCLASSNAME, DCX_REBARCTRLCLASS);

	// Custom Color Combo
	DCX_DEBUG("LoadDLL", "Registering ComboBox...");
	dcxRegisterClass("COMBOBOX", DCX_COLORCOMBOCLASS);

	// Custom TabCtrl
	DCX_DEBUG("LoadDLL", "Registering Tab...");
	dcxRegisterClass(WC_TABCONTROL, DCX_TABCTRLCLASS);

	// Custom UpDown
	DCX_DEBUG("LoadDLL", "Registering UpDown...");
	dcxRegisterClass(UPDOWN_CLASS, DCX_UPDOWNCLASS);

	// Custom IpAddress
	DCX_DEBUG("LoadDLL", "Registering IpAddress...");
	dcxRegisterClass(WC_IPADDRESS, DCX_IPADDRESSCLASS);

	// Init Divider Control
	//InitDivider( GetModuleHandle( NULL ) );

	// Custom Divider
	DCX_DEBUG("LoadDLL", "Registering Divider...");
	wc.cbSize         = sizeof(WNDCLASSEX);
	wc.style          = 0;
	wc.lpfnWndProc    = DividerWndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = GetModuleHandle(NULL);
	wc.hIcon          = NULL;
	wc.hCursor        = NULL;
	wc.hbrBackground  = (HBRUSH) (COLOR_3DFACE +1);
	wc.lpszMenuName   = 0;
	wc.lpszClassName  = DCX_DIVIDERCLASS;
	wc.hIconSm        = NULL;
	RegisterClassEx(&wc);

	//GetClassInfoEx( NULL, DIVIDERCLASSNAME, &wc );
	//wc.lpszClassName = ;
	//RegisterClassEx( &wc );

	/*
	// Custom Panel
	wc.cbSize			    = sizeof( WNDCLASSEX );
	wc.style		  	  = 0;
	wc.lpfnWndProc	  = DefWindowProc;
	wc.cbClsExtra		  = 0;
	wc.cbWndExtra		  = 0;
	wc.hInstance		  = GetModuleHandle( NULL );
	wc.hIcon			    = LoadCursor( GetModuleHandle( NULL ), IDC_ARROW );
	wc.hCursor			  = NULL;
	wc.hbrBackground	= NULL; //(HBRUSH)(COLOR_3DFACE+1);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= DCX_PANELCLASS;
	wc.hIconSm			  = NULL;
	RegisterClassEx( &wc );
	*/

	// Custom Panel
	//DCX_DEBUG("LoadDLL", "Registering Panel (#32770)...");
	//GetClassInfoEx(NULL, "#32770", &wc); // NB: using this class causes tooltips in toolbar children to not show
	//wc.lpszClassName = DCX_PANELCLASS;		// Also causes listview/treeview label edit events to fail.
	//RegisterClassEx(&wc);
	DCX_DEBUG("LoadDLL", "Registering Panel...");
	wc.cbSize         = sizeof(WNDCLASSEX);
	wc.style          = CS_PARENTDC;
	wc.lpfnWndProc    = DefWindowProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = GetModuleHandle(NULL);
	wc.hIcon          = NULL;
	wc.hCursor        = LoadCursor( GetModuleHandle( NULL ), IDC_ARROW );
	wc.hbrBackground  = NULL; //(HBRUSH) (COLOR_3DFACE +1);
	wc.lpszMenuName   = 0;
	wc.lpszClassName  = DCX_PANELCLASS;
	wc.hIconSm        = NULL;
	RegisterClassEx(&wc);

	// Custom Box
	//DCX_DEBUG("LoadDLL", "Registering Box (#32770)...");
	//GetClassInfoEx(NULL, "#32770", &wc); // NB: using this class causes tooltips in toolbar children to not show
	//wc.lpszClassName = DCX_BOXCLASS;			// Also causes listview/treeview label edit events to fail.
	//RegisterClassEx(&wc);
	DCX_DEBUG("LoadDLL", "Registering Box...");
	wc.cbSize         = sizeof(WNDCLASSEX);
	wc.style          = CS_PARENTDC;
	wc.lpfnWndProc    = DefWindowProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 0;
	wc.hInstance      = GetModuleHandle(NULL);
	wc.hIcon          = NULL;
	wc.hCursor        = LoadCursor( GetModuleHandle( NULL ), IDC_ARROW );
	wc.hbrBackground  = NULL; //(HBRUSH) (COLOR_3DFACE +1);
	wc.lpszMenuName   = 0;
	wc.lpszClassName  = DCX_BOXCLASS;
	wc.hIconSm        = NULL;
	RegisterClassEx(&wc);

	// Custom Button
	DCX_DEBUG("LoadDLL", "Registering Button...");
	dcxRegisterClass("BUTTON", DCX_BUTTONCLASS);

	// Custom Calendar
	DCX_DEBUG("LoadDLL", "Registering Calendar...");
	dcxRegisterClass(MONTHCAL_CLASS, DCX_CALENDARCLASS);

	// Custom DateTime
	DCX_DEBUG("LoadDLL", "Registering DateTime...");
	dcxRegisterClass(DATETIMEPICK_CLASS, DCX_DATETIMECLASS);

	// Custom Pager
	DCX_DEBUG("LoadDLL", "Registering Pager...");
	dcxRegisterClass(WC_PAGESCROLLER, DCX_PAGERCLASS);

	// Shadow Class
	DCX_DEBUG("LoadDLL", "Registering Shadow...");
	wc.cbSize = sizeof(WNDCLASSEX); 
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = DefWindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = GetModuleHandle( NULL );
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = DCX_SHADOWCLASS;
	wc.hIconSm       = NULL;
	RegisterClassEx(&wc);

	// Vista Dialog Class
	DCX_DEBUG("LoadDLL", "Registering Vista Dialog...");
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = DefWindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = GetModuleHandle( NULL );
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = DCX_VISTACLASS;
	wc.hIconSm       = NULL;
	RegisterClassEx(&wc);

	/***** XPopup Stuff *****/
	//GetClassInfoEx(NULL,"#32768",&wc); // menu
	//HWND tmp_hwnd = CreateWindowEx(0,"#32768",NULL,WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandle(NULL),NULL);
	//if (tmp_hwnd != NULL) {
	//	g_OldmIRCMenusWindowProc = (WNDPROC)SetClassLongPtr(tmp_hwnd,GCLP_WNDPROC,(LONG_PTR)mIRCMenusWinProc);
	//	DestroyWindow(tmp_hwnd);
	//}
	DCX_DEBUG("LoadDLL", "Registering XPopup...");
	g_OldmIRCWindowProc = SubclassWindow(mIRCLink.m_mIRCHWND, mIRCSubClassWinProc);

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = XPopupMenu::XPopupWinProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = GetModuleHandle( NULL );
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = XPOPUPMENUCLASS;
	wc.hIconSm       = NULL;
	RegisterClassEx(&wc);
	//WNDCLASS wcpop;
	//ZeroMemory(&wcpop, sizeof(WNDCLASS));
	//wcpop.hInstance = GetModuleHandle(NULL);
	//wcpop.lpszClassName = XPOPUPMENUCLASS;
	//wcpop.lpfnWndProc = XPopupMenu::XPopupWinProc;
	//RegisterClass(&wcpop);

	DCX_DEBUG("LoadDLL", "Creating menu owner...");
	mhMenuOwner = CreateWindow(XPOPUPMENUCLASS, NULL, 0, 0, 0, 0, 0, (isXP() ? HWND_MESSAGE : 0), 0, GetModuleHandle(NULL), 0);

	g_mIRCPopupMenu = new XPopupMenu("mirc",(HMENU)NULL);
	g_mIRCMenuBar = new XPopupMenu("mircbar",GetMenu(mIRCLink.m_mIRCHWND));

	DCX_DEBUG("LoadDLL", "Initialising UltraDock...");
	InitUltraDock();
//#ifndef NDEBUG
//	InitCustomDock();
//#endif
	// Initialise signals of diff types
	dcxSignal.xdock = false;
	dcxSignal.xstatusbar = true;
	dcxSignal.xtray = true;


	// XMenuBar stuff
	HMENU menu = GetMenu(mIRCLink.m_mIRCHWND);
	int i = 0;
	const unsigned int buffSize = 30;
	TString label(buffSize);
	MENUITEMINFO mii;

	ZeroMemory(&mii, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING;
	mii.cch = buffSize;
	mii.dwTypeData = label.to_chr();

	while (GetMenuItemInfo(menu, i, TRUE, &mii)) {
		// We've found the tools menu, next one is the scriptable popup.
		if (label == "&Tools") {
			HMENU scriptable = GetSubMenu(menu, i +1);;

			g_mIRCScriptMenu = new XPopupMenu("scriptpopup", scriptable);
			break;
		}

		// Reset buffer size
		mii.cch = buffSize;
		i++;
	}
}

/*!
* \brief mIRC DLL UnLoad Function
*
* This function is called when the DLL is unloaded.
*
* It uninitializes all the stuff DCX set up and frees up memory fragments.
*
* \param timeout Unload trigger indicator (0 = /dll -u (or on pre mIRC 6.30, exit) - 1 = timeout unload after 10 min - 2 = exit on mIRC 6.30+)
*/
int WINAPI UnloadDll(int timeout) {
	// DLL unloaded because mIRC exits or /dll -u used
	if (timeout != 1) {
		if (Dialogs.closeDialogs()) { // if unable to close dialogs stop unload.
			mIRCError("Unable to Unload DLL from within the DLL");
			return 0; // NB: This DOESN'T stop the unload, & mIRC will still crash.
		}

		CloseUltraDock(); // UnDock All.

		UnregisterClass(DCX_PROGRESSBARCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_TREEVIEWCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_TOOLBARCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_STATUSBARCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_LISTVIEWCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_COMBOEXCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_TRACKBARCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_RICHEDITCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_REBARCTRLCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_COLORCOMBOCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_UPDOWNCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_BUTTONCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_DIVIDERCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_PANELCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_TABCTRLCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_CALENDARCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_DATETIMECLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_BOXCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_PAGERCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_SHADOWCLASS, GetModuleHandle(NULL));
		UnregisterClass(DCX_VISTACLASS, GetModuleHandle(NULL));

		// Class Factory of Web Control
		if (g_pClassFactory != NULL)
			g_pClassFactory->Release();

		// Terminating OLE Support
		OleUninitialize();


		/***** TrayIcon Stuff *****/
		if (trayIcons != NULL) {
			delete trayIcons;
			trayIcons = NULL;
		}
		
		// reset the treebars font if it's been changed.
		if (mIRCLink.m_hTreeFont != NULL) {
			HFONT hfont = (HFONT)SendMessage(mIRCLink.m_hTreeView,WM_GETFONT,0,0);
			if (hfont != mIRCLink.m_hTreeFont) {
				SendMessage( mIRCLink.m_hTreeView, WM_SETFONT, (WPARAM) mIRCLink.m_hTreeFont, (LPARAM) MAKELPARAM(TRUE,0));
				DeleteObject(hfont);
			}
		}

		/***** XMenuBar Stuff *****/
		g_XMenuBar.resetMenuBar();

		/***** XPopup Stuff *****/
		SetWindowLongPtr(mIRCLink.m_mIRCHWND, GWLP_WNDPROC, (LONG_PTR) g_OldmIRCWindowProc);
		//HWND tmp_hwnd = CreateWindowEx(0,"#32768",NULL,WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandle(NULL),NULL);
		//if (tmp_hwnd != NULL) {
		//	SetClassLongPtr(tmp_hwnd,GCLP_WNDPROC,(LONG_PTR)g_OldmIRCMenusWindowProc);
		//	DestroyWindow(tmp_hwnd);
		//}

		g_XPopupMenuManager.clearMenus();

		delete g_mIRCPopupMenu;
		g_mIRCMenuBar->cleanMenu(GetMenu(mIRCLink.m_mIRCHWND));
		delete g_mIRCMenuBar;

		if (mhMenuOwner != NULL)
			DestroyWindow(mhMenuOwner);

		UnregisterClass(XPOPUPMENUCLASS, GetModuleHandle(NULL));

		FreeOSCompatibility();

		UnmapViewOfFile(mIRCLink.m_pData);
		CloseHandle(mIRCLink.m_hFileMap);

		return 1;
	}
	// keep DLL in memory
	else
		return 0;
}

/*!
* \brief DCX DLL Version Function
*/
mIRC(Version) {
	wsprintf(data,
		"DCX (XPopup) DLL %d.%d.%d %s by ClickHeRe, twig*, Ook, andy and Mpdreamz  �2006-2007",
		DLL_VERSION, DLL_SUBVERSION, DLL_BUILD, DLL_STATE);
	return 3;
}

/*!
* \brief DCX DLL is DcxDirectShow supported?
*/
static TString dxData;
mIRC(IsUsingDirectX) {
	if (mIRCLink.m_bDX9Installed) {
		lstrcpy(data, dxData.to_chr());
		return 3;
	}
	else if (DXSetup(data, 900)) {
		dxData = data;
		return 3;
	}
	ret("$false");
//#ifdef DCX_USE_DXSDK
//	DWORD dx_ver;
//	if (GetDXVersion(&dx_ver, data, 900) == S_OK) {
//		if (dx_ver < 0x00090000)
//			mIRCLink.m_bDX9Installed = false;
//		else
//			mIRCLink.m_bDX9Installed = true;
//		if (dx_ver < 1)
//			ret("$false");
//	}
//	return 3;
//#else
//	ret("$false");
//#endif // DCX_USE_DXSDK
}

/*!
* \brief DCX DLL is GDI+ supported?
*/
mIRC(IsUsingGDI) {
	ret((mIRCLink.m_bUseGDIPlus ? "$true" : "$false"));
}

/*!
* \brief Check if it's safe to unload DLL
*/
mIRC(IsUnloadSafe) {
	ret(((Dialogs.safeToCloseAll() && !g_XPopupMenuManager.isPatched()) ? "$true" : "$false"));
}

/*!
* \brief Check if windows is themed
*/
mIRC(IsThemedXP) {
	ret((dcxIsThemeActive() ? "$true" : "$false"));
}

/*!
* \brief DCX DLL Mark Function
*
* Argument \b data contains -> [NAME] [ALIAS]
*/

// Mark [NAME] [ALIAS]
mIRC(Mark) {
	TString d(data);
	d.trim();

	if (d.numtok() < 2)
		ret("D_ERROR Mark: [NAME] [ALIAS]");

	char res[40];

	// check if the alias exists
	mIRCevalEX(res, 40, "$isalias(%s)", d.gettok(2).to_chr());

	if (lstrcmp(res, "$false") == 0) {
		wsprintf(data, "D_ERROR Mark : No such alias : %s", d.gettok(2).to_chr());
		mIRCError(data);
		return 3;
	}

	// check if valid dialog
	HWND mHwnd = GetHwndFromString(d.gettok(1));

	if (IsWindow(mHwnd) == FALSE) {
		wsprintf(data, "D_ERROR Mark : Invalid Dialog Window : %s", d.gettok(1).to_chr());
		mIRCError(data);
		return 3;
	}

	if (Dialogs.getDialogByHandle(mHwnd) != NULL) {
		wsprintf(data, "D_ERROR Mark: Window Already Marked : %s", d.gettok(1).to_chr());
		mIRCError(data);
		return 3;
	}
	Dialogs.markDialog(mHwnd, d.gettok(1), d.gettok(2));
	{
		DcxDialog *pTmp = Dialogs.getDialogByHandle(mHwnd);
		if (pTmp != NULL) {
			pTmp->callAliasEx(res, "isverbose,0");

			if (lstrcmp(res, "$false") == 0)
				pTmp->SetVerbose(false);
		}
	}
	ret("D_OK Mark: Dialog Marked");
}

/*!
* \brief DCX DLL GetSystemColor Function
*
* Argument \b data contains -> [ATTRIB] (not implemented yet, [DEFAULT])
*/
// GetSystemColor [ATTRIBUTE]
mIRC(GetSystemColor) {
	TString d(data);
	d.trim();

	if (d.numtok() < 1) {
		DCXError("$!dcx(GetSysColor)","Invalid arguments");
		return 0;
	}

	int col;
	TString coltype(d.gettok(1));

	if      (coltype == "COLOR_3DDKSHADOW"		) { col = COLOR_3DDKSHADOW; }
	else if (coltype == "COLOR_3DFACE"			) { col = COLOR_3DFACE; }
	else if (coltype == "COLOR_3DHIGHLIGHT"	) { col = COLOR_3DHIGHLIGHT; }
	else if (coltype == "COLOR_3DHILIGHT"		) { col = COLOR_3DHILIGHT; }
	else if (coltype == "COLOR_3DLIGHT"			) { col = COLOR_3DLIGHT; }
	else if (coltype == "COLOR_3DSHADOW"		) { col = COLOR_3DSHADOW; }
	else if (coltype == "COLOR_ACTIVEBORDER"	) { col = COLOR_ACTIVEBORDER; }
	else if (coltype == "COLOR_ACTIVECAPTION"	) { col = COLOR_ACTIVECAPTION; }
	else if (coltype == "COLOR_APPWORKSPACE"	) { col = COLOR_APPWORKSPACE; }
	else if (coltype == "COLOR_BACKGROUND"		) { col = COLOR_BACKGROUND; }
	else if (coltype == "COLOR_BTNFACE"			) { col = COLOR_BTNFACE; }
	else if (coltype == "COLOR_BTNHIGHLIGHT"	) { col = COLOR_BTNHIGHLIGHT; }
	else if (coltype == "COLOR_BTNSHADOW"		) { col = COLOR_BTNSHADOW; }
	else if (coltype == "COLOR_BTNTEXT"			) { col = COLOR_BTNTEXT; }
	else if (coltype == "COLOR_CAPTIONTEXT"	) { col = COLOR_CAPTIONTEXT; }
	else if (coltype == "COLOR_DESKTOP"			) { col = COLOR_DESKTOP; }
	else if (coltype == "COLOR_GRADIENTACTIVECAPTION") { col = COLOR_GRADIENTACTIVECAPTION; }
	else if (coltype == "COLOR_GRADIENTINACTIVECAPTION") { col = COLOR_GRADIENTINACTIVECAPTION; }
	else if (coltype == "COLOR_GRAYTEXT"		) { col = COLOR_GRAYTEXT; }
	else if (coltype == "COLOR_HIGHLIGHT"		) { col = COLOR_HIGHLIGHT; }
	else if (coltype == "COLOR_HIGHLIGHTTEXT"	) { col = COLOR_HIGHLIGHTTEXT; }
	else if (coltype == "COLOR_HOTLIGHT"		) { col = COLOR_HOTLIGHT; }
	else if (coltype == "COLOR_INACTIVEBORDER") { col = COLOR_INACTIVEBORDER; }
	else if (coltype == "COLOR_INACTIVECAPTION") { col = COLOR_INACTIVECAPTION; }
	else if (coltype == "COLOR_INACTIVECAPTIONTEXT") { col = COLOR_INACTIVECAPTIONTEXT; }
	else if (coltype == "COLOR_INFOBK"			) { col = COLOR_INFOBK; }
	else if (coltype == "COLOR_INFOTEXT"		) { col = COLOR_INFOTEXT; }
	else if (coltype == "COLOR_MENU"				) { col = COLOR_MENU; }
	else if (coltype == "COLOR_MENUHILIGHT"	) { col = COLOR_MENUHILIGHT; }
	else if (coltype == "COLOR_MENUBAR"			) { col = COLOR_MENUBAR; }
	else if (coltype == "COLOR_MENUTEXT"		) { col = COLOR_MENUTEXT; }
	else if (coltype == "COLOR_SCROLLBAR"		) { col = COLOR_SCROLLBAR; }
	else if (coltype == "COLOR_WINDOW"			) { col = COLOR_WINDOW; }
	else if (coltype == "COLOR_WINDOWFRAME"	) { col = COLOR_WINDOWFRAME; }
	else if (coltype == "COLOR_WINDOWTEXT"		) { col = COLOR_WINDOWTEXT; }
	else
		ret("D_ERROR GetSystemColor: Invalid parameter specified");

	// max of 8 digits, 9 for null terminator
	wsprintf(data, "%d", GetSysColor(col));
	return 3;
}


/*!
* \brief DCX DLL /xdid Function
*
* mIRC /xdid -switch dialog ID (options) interface
*
* Argument \b data contains -> [NAME] [ID] [SWITCH] (OPTIONS)
*/
mIRC(xdid) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 3) {
		DCXError("/xdid","Invalid arguments");
		return 0;
	}

	DcxDialog * p_Dialog = Dialogs.getDialogByName(d.gettok( 1 ));

	if (p_Dialog == NULL) {
		TString error;
		error.sprintf("Unknown dialog \"%s\": see Mark command", d.gettok( 1 ).to_chr());
		DCXError("/xdid",error.to_chr());
		return 0;
	}

	TString IDs(d.gettok( 2 )), d2;
	DcxControl * p_Control;
	int n = IDs.numtok(TSCOMMA);

	// Multiple IDs id,id,id,id-id,id-id
	if (n > 1) {
		for (int i = 1; i <= n; i++) {
			TString tsID(IDs.gettok(i, TSCOMMA));
			UINT id_start = 0, id_end = 0;
			if (tsID.numtok("-") == 2) {
				id_start = tsID.gettok(1, "-").to_int();
				id_end = tsID.gettok(2, "-").to_int();
			}
			else
				id_start = id_end = tsID.to_int();

			for (UINT id = id_start; id <= id_end; id++) {
				p_Control = p_Dialog->getControlByID(id + mIRC_ID_OFFSET);

				if (p_Control == NULL) {
					p_Dialog->showErrorEx(NULL,d.gettok( 3 ).to_chr(), "(xdid) Invalid ID : %ld (dialog : %s)", id, d.gettok( 1 ).to_chr());
					return 0;
				}

				d2.sprintf("%s %ld %s",d.gettok( 1 ).to_chr(), id, d.gettok(3, -1).to_chr());

				p_Control->parseCommandRequest(d2);
			}
		}
	}
	//Single ID or single id-id
	else {
		UINT id_start = 0, id_end = 0;
		if (IDs.numtok("-") == 2) {
			id_start = IDs.gettok(1, "-").to_int();
			id_end = IDs.gettok(2, "-").to_int();
		}
		else
			id_start = id_end = IDs.to_int();

		for (UINT id = id_start; id <= id_end; id++) {
			p_Control = p_Dialog->getControlByID(id + mIRC_ID_OFFSET);

			if (p_Control == NULL) {
				p_Dialog->showErrorEx(NULL,d.gettok( 3 ).to_chr(), "(xdid) Invalid ID : %ld (dialog : %s)", id, d.gettok( 1 ).to_chr());
				return 0;
			}

			p_Control->parseCommandRequest(d);
		}
	}

	return 3;
}

/*!
* \brief DCX DLL $xdid Function
*
* mIRC $xdid(dialog, ID, options).prop interface
*
* Argument \b data contains -> [NAME] [ID] [PROP] (OPTIONS)
*/
mIRC(_xdid) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 3) {
		DCXError("$!xdid()","Invalid arguments");
		return 0;
	}

	DcxDialog * p_Dialog = Dialogs.getDialogByName(d.gettok( 1 ));

	if (p_Dialog == NULL) {
		TString error;
		error.sprintf("Unknown dialog \"%s\": see Mark command", d.gettok( 1 ).to_chr());
		DCXError("$!xdid()",error.to_chr());
		return 0;
	}

	DcxControl * p_Control = p_Dialog->getControlByID((UINT) d.gettok( 2 ).to_int() + mIRC_ID_OFFSET);

	if (p_Control == NULL) {
		p_Dialog->showErrorEx(d.gettok( 3 ).to_chr(), NULL, "Invalid ID : %s (dialog %s)", d.gettok( 2 ).to_chr(), d.gettok( 1 ).to_chr());
		return 0;
	}

	p_Control->parseInfoRequest(d, data);
	return 3;
}


/*!
* \brief DCX DLL $dcx(GetTaskbarPos) Function
*
* $dcx(GetTaskbarPos) interface
*
*/
mIRC(GetTaskbarPos) {
	HWND hTaskbar = FindWindow("Shell_TrayWnd", NULL);

	if (hTaskbar) {
		RECT rc;

		GetWindowRect(hTaskbar, &rc);
		wsprintf(data, "%d %d %d %d", rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
		return 3;
	}

	ret("D_ERROR GetTaskbarPos: could not find taskbar");
}


/*!
* \brief DCX DLL /xdialog Function
*
* mIRC /xdialog -switch dialog (options) interface
*
* Argument \b data contains -> [NAME] [SWITCH] (OPTIONS)
*/
mIRC(xdialog) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 2) {
		TString error;
		error.sprintf("Invalid arguments ( dialog %s)", d.gettok( 1 ).to_chr());
		DCXError("/xdialog",error.to_chr());
		return 0;
	}

	DcxDialog * p_Dialog = Dialogs.getDialogByName(d.gettok( 1 ));

	if (p_Dialog == NULL) {
		TString error;
		error.sprintf("Unknown dialog \"%s\": see Mark command", d.gettok( 1 ).to_chr());
		DCXError("/xdialog",error.to_chr());
		return 0;
	}

	p_Dialog->parseCommandRequest(d);
	return 1;
}

/*!
* \brief DCX DLL $xdialog Function
*
* mIRC $xdialog(dialog, options).prop interface
*
* Argument \b data contains -> [NAME] [SWITCH] (OPTIONS)
*/
mIRC(_xdialog) {
	TString d(data);
	d.trim();

	// reset mIRC data
	data[0] = 0;

	if (d.numtok( ) < 2) {
		TString error;
		error.sprintf("Invalid arguments ( dialog %s)", d.gettok( 1 ).to_chr());
		DCXError("$!xdialog()",error.to_chr());
		return 0;
	}

	DcxDialog *p_Dialog = Dialogs.getDialogByName(d.gettok( 1 ));

	if (p_Dialog == NULL) {
		if (d.gettok( 2 ) != "ismarked") {
			TString error;
			error.sprintf("Unknown dialog \"%s\": see Mark command", d.gettok( 1 ).to_chr());
			DCXError("$!xdialog()",error.to_chr());
			return 0;
		}
		else
			ret("$false");
	}

	p_Dialog->parseInfoRequest(d, data);
	return 3;
}

/***** XPopup Stuff *****/
/*!
* \brief blah
*
* blah
*/
LRESULT CALLBACK mIRCSubClassWinProc(HWND mHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SIZE:
			{
				mIRCSignalDCX(dcxSignal.xdock, "size mIRC %d %d %d", mHwnd, LOWORD(lParam), HIWORD(lParam));
			}
			break;

//		case WM_SYSCOMMAND:
//			{
//#define SC_SHOWSYSMENU 0xF093
//				if ((wParam & 0xFFF0) == SC_SHOWSYSMENU) {
//					GetSystemMenu(mHwnd,TRUE);
//				}
//			}
//			break;

		//case WM_CONTEXTMENU:
		//case WM_INITMENU:
		case WM_INITMENUPOPUP:
		{
			HMENU menu = (HMENU) wParam;
			HMENU currentMenubar = GetMenu(mIRCLink.m_mIRCHWND);
			bool switchMenu = (g_mIRCScriptMenu != NULL) &&                  // The mIRC scriptpopup menu has been wrapped,
				              (menu == g_mIRCScriptMenu->getMenuHandle()) && // The menu the same as the one just shown,
							  (currentMenubar != g_OriginalMenuBar) &&       // The menubar is our generated menubar,
							  (g_OriginalMenuBar != NULL);                   // And ensure it has been generated.

			if (HIWORD(lParam) == FALSE) {
				if (switchMenu)
					SetMenu(mIRCLink.m_mIRCHWND, g_OriginalMenuBar);

				// let mIRC populate the menus dynamically
				LRESULT lRes = CallWindowProc(g_OldmIRCWindowProc, mHwnd, uMsg, wParam, lParam);

				if (switchMenu)
					SetMenu(mIRCLink.m_mIRCHWND, currentMenubar);

				if (isMenuBarMenu(GetMenu(mHwnd), menu)) {
					isMenuBar = TRUE;

					if (bIsActiveMircMenubarPopup == TRUE) {
						HWND hActive = (HWND)SendMessage(mIRCLink.m_hMDI, WM_MDIGETACTIVE, NULL, NULL);
						if (!IsZoomed(hActive) || GetSystemMenu(hActive,FALSE) != menu)
							g_mIRCMenuBar->convertMenu(menu, TRUE);
					}
				}
				else {
					isMenuBar = FALSE;

					if (bIsActiveMircPopup == TRUE)
						g_mIRCPopupMenu->convertMenu(menu, FALSE);
				}

				isSysMenu = FALSE;
				return lRes;
			}
			else
				isSysMenu = TRUE;
			break;
		}

		case WM_COMMAND:
		{
			// Check if the message came from the menubar
			if ((HIWORD(wParam) == 0) && (isMenuBar) && g_XMenuBar.hasCallback()) {
				// If the user requested the message to be halted ...
				if (g_XMenuBar.parseCallback(LOWORD(wParam))) {
					// Stop parsing and prevent default action.
					return 0L;
				}
			}

			break;
		}

		case WM_EXITMENULOOP:
		{
			if ((isMenuBar == FALSE) && (bIsActiveMircPopup == TRUE))
				g_mIRCPopupMenu->clearAllMenuItems();

			break;
		}

		case WM_UNINITMENUPOPUP:
		{
			if ((isMenuBar == TRUE) && (isSysMenu == FALSE) && (bIsActiveMircMenubarPopup == TRUE))
				g_mIRCMenuBar->deleteAllItemData((HMENU) wParam);

			break;
		}

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;

			if (lpmis->CtlType == ODT_MENU) {
				XPopupMenuItem *p_Item = (XPopupMenuItem*) lpmis->itemData;

				if (p_Item != NULL) {
					SIZE size = p_Item->getItemSize(mHwnd);

					lpmis->itemWidth = size.cx;
					lpmis->itemHeight = size.cy;
					return TRUE; 
				}
			}

			break;
		}

		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;

			if (lpdis->CtlType == ODT_MENU) {
				XPopupMenuItem *p_Item = (XPopupMenuItem*) lpdis->itemData;

				if (p_Item != NULL) {
					p_Item->DrawItem(lpdis);
					return TRUE; 
				}
			}

			break;
		}

		case WM_CLOSE:
		{
			SetWindowLongPtr(mIRCLink.m_mIRCHWND, GWLP_WNDPROC, (LONG_PTR) g_OldmIRCWindowProc);
			PostMessage(mHwnd, uMsg, 0, 0);
			return 0L;
			break;
		}
		// ghost drag stuff
		case WM_ENTERSIZEMOVE:
			{
				if (mIRCLink.m_bDoGhostDrag < 255 && SetLayeredWindowAttributesUx != NULL) {
					long style = GetWindowExStyle(mIRCLink.m_mIRCHWND);
					// Set WS_EX_LAYERED on this window
					if (!(style & WS_EX_LAYERED))
						SetWindowLong(mIRCLink.m_mIRCHWND, GWL_EXSTYLE, style | WS_EX_LAYERED);
					// Make this window 75 alpha
					SetLayeredWindowAttributesUx(mIRCLink.m_mIRCHWND, 0, (BYTE)mIRCLink.m_bDoGhostDrag, LWA_ALPHA);
					mIRCLink.m_bGhosted = true;
				}
			}
			break;

		case WM_EXITSIZEMOVE:
		{
			// turn off ghosting.
			if (mIRCLink.m_bGhosted && SetLayeredWindowAttributesUx != NULL) {
				// Make this window solid
				SetLayeredWindowAttributesUx(mIRCLink.m_mIRCHWND, 0, 255, LWA_ALPHA);
				mIRCLink.m_bGhosted = false;
			}
			break;
		}

		case WM_DWMCOMPOSITIONCHANGED:
			{
				if (DwmIsCompositionEnabledUx != NULL)
					DwmIsCompositionEnabledUx(&mIRCLink.m_bAero);
			}
			break;

		default:
			break;
	}

	return CallWindowProc(g_OldmIRCWindowProc, mHwnd, uMsg, wParam, lParam);
}


BOOL isMenuBarMenu(const HMENU hMenu, const HMENU hMatch) {
	HMENU hTemp;
	int i = 0, n = GetMenuItemCount(hMenu);

	while (i < n) {
		if ((hTemp = GetSubMenu(hMenu, i)) != NULL) {
			if (hTemp == hMatch)
				return TRUE;

			if (isMenuBarMenu(hTemp, hMatch) == TRUE)
				return TRUE;
		}

		++i;
	}

	return FALSE;
}

/*!
* \brief XPopup DLL /xpop Function
*
* mIRC /xpop -switch menu path [TAB] optional data
*
* Argument \b data contains -> [MENU] [SWITCH] [PATH] [TAB] [OPTION]
*/
mIRC(xpop) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 3) {
		DCXError("/xpop","Invalid arguments");
		return 0;
	}

	if ((d.gettok(1) == "mirc") || (d.gettok(1) == "mircbar")) {
		DCXError("/xpop","Invalid menu name : mirc or mircbar menus don't have access to this feature.");
		return 0;
	}

	XPopupMenu *p_Menu = g_XPopupMenuManager.getMenuByName(d.gettok(1), FALSE);

	if (p_Menu == NULL) {
		TString error;
		error.sprintf("Unknown menu \"%s\": see /xpopup -c command", d.gettok( 1 ).to_chr());
		DCXError("/xpop",error.to_chr());
		return 0;
	}

	p_Menu->parseXPopCommand(d);
	return 1;
}

/*!
* \brief XPopup DLL $xpop Function
*
* mIRC $xpop(menu, path, options).prop interface
*
* Argument \b data contains -> [MENU] [PROP] [PATH] [TAB] [OPTION]
*/
mIRC(_xpop) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 3) {
		DCXError("$!xpop()","Invalid arguments");
		return 0;
	}

	if ((d.gettok( 1 ) == "mirc") || (d.gettok( 1 ) == "mircbar")) {
		DCXError("$!xpop()","Invalid menu name : mirc or mircbar menus don't have access to this feature.");
		return 0;
	}

	XPopupMenu *p_Menu = g_XPopupMenuManager.getMenuByName(d.gettok(1), FALSE);

	if (p_Menu == NULL) {
		TString error;
		error.sprintf("Unknown menu \"%s\": see /xpopup -c command", d.gettok( 1 ).to_chr());
		DCXError("$!xpop()",error.to_chr());
		return 0;
	}

	p_Menu->parseXPopIdentifier(d, data);
	return 3;
}

/*!
* \brief XPopup DLL /xpopup Function
*
* mIRC /xpopup -switch menu (options)
*
* Argument \b data contains -> [MENU] [SWITCH] (OPTIONS)
*/
mIRC(xpopup) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 2) {
		DCXError("/xpopup","Invalid arguments");
		return 0;
	}

	g_XPopupMenuManager.parseXPopupCommand(d);
	return 1;
}

/*!
* \brief XPopup DLL $xpopup Function
*
* mIRC $xpopup(menu, options).prop interface
*
* Argument \b data contains -> [MENU] [PROP] (OPTIONS)
*/
mIRC(_xpopup) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 2) {
		DCXError("$!xpopup()","Invalid arguments");
		return 0;
	}

	g_XPopupMenuManager.parseXPopupIdentifier(d, data);
	return 3;
}

/*!
* \brief XMenuBar DLL /xmenubar Function
*
* mIRC /xmenubar -switch (options)
*/
mIRC(xmenubar) {
	TString d(data);
	d.trim();

	data[0] = 0;
	g_XMenuBar.parseXMenuBarCommand(d);

	return 1;
}

/*!
* \brief XMenuBar DLL $xmenubar Function
*
* mIRC $xmenubar(options).prop interface
*/
mIRC(_xmenubar) {
	TString d(data);
	d.trim();

	data[0] = 0;
	g_XMenuBar.parseXMenuBarInfo(d, data);

	return 3;
}

/*!
* \brief XPopup DLL /mpopup Function
*
* mIRC /mpopup menuname [0|1]
*
* Argument \b data contains -> [MENU] [0|1]
*/
mIRC(mpopup) {
	TString d(data);
	d.trim();

	data[0] = 0;

	if (d.numtok( ) < 2) {
		DCXError("/mpopup","Invalid arguments");
		return 0;
	}

	if (d.gettok( 1 ) == "mirc") {
		if (d.gettok( 2 ) == "1")
			bIsActiveMircPopup = TRUE;
		else
			bIsActiveMircPopup = FALSE;
	}
	else if (d.gettok( 1 ) == "mircbar") {
		if (d.gettok( 2 ) == "1")
			bIsActiveMircMenubarPopup = TRUE;
		else {
			bIsActiveMircMenubarPopup = FALSE;
			g_mIRCMenuBar->cleanMenu(GetMenu(mIRCLink.m_mIRCHWND));
		}
	}
	return 3;
}
// /dcx xSignal [1|0] (+FLAGS)
mIRC(xSignal) {
	TString d(data);
	TString flags;
	bool state;

	d.trim();

	// flags specified
	if (d.numtok() > 1)
		flags = d.gettok(2);
	// if no flags specified, set all states
	else
		flags = "+dst";

	// determine state
	if (d.to_num() > 0)
		state = true;
	else
		state = false;

	// start from 1 because we expect it to be the '+' symbol
	for (int i = 1; i < (int) flags.len(); i++) {
		switch (flags[i]){
			case 'd':
				dcxSignal.xdock = state;
				break;

			case 's':
				dcxSignal.xstatusbar = state;
				break;

			case 't':
				dcxSignal.xtray = state;
				break;

			default:
				TString flag;
				flag.sprintf("Unknown flag '%c' specified.", flags[i]);

				DCXError("/dcx xSignal", flag.to_chr());
		}
	}

	data[0] = 0;
	return 1;
}

// /dcx WindowProps [HWND] [+FLAGS] (ARGS)
mIRC(WindowProps) {
	TString input(data);
	int numtok = input.numtok( );

	if (numtok < 2) {
		DCXError("/dcx WindowProps", "Insuffient parameters");
		return 0;
	}

	HWND hwnd = (HWND) input.gettok(1).to_int();

	if (!IsWindow(hwnd)) {
		DCXError("/dcx WindowProps", "Invalid window");
		return 0;
	}

	TString flags(input.gettok( 2 ));
	flags.trim();

	if ((flags[0] != '+') || (flags.len() < 2)) {
		DCXError("/dcx WindowProps","No Flags Found");
		return 0;
	}

	if ((flags.find('T', 0) == 0) && (flags.find('i', 0) == 0) && (flags.find('t', 0) == 0) && (flags.find('r', 0) == 0)) {
		DCXError("/dcx WindowProps","Unknown Flags");
		return 0;
	}

	// set hwnd NoTheme
	// +T
	if (flags.find('T', 0)) {
		if (XPPlus) {
			if (dcxSetWindowTheme(hwnd,L" ",L" ") != S_OK)
				DCXError("/dcx WindowProps", "Unable to set theme");
		}
	}
	// set hwnd's title icon
	// +i [INDEX] [FILENAME]
	if (flags.find('i', 0) && numtok > 3) {
		int index = input.gettok( 3 ).to_int();
		TString filename(input.gettok(1,TSTAB).gettok(4, -1));
		filename.trim();

		if (!ChangeHwndIcon(hwnd,flags,index,filename))
			return 0;
	}
	// set hwnd title text
	// +t [TEXT]
	if (flags.find('t', 0)) { 
		TString txt;
		
		if (flags.find('i', 0)) {
			if (input.numtok(TSTAB) > 1)
				txt = input.gettok(2,-1,TSTAB);
		}
		else if (numtok > 2) {
				txt = input.gettok(3, -1);
		}
		txt.trim();

		SetWindowText(hwnd, txt.to_chr());
	}
	// RMB click hwnd at pos.
	// +r [X] [Y]
	if (flags.find('r', 0)) {
		UINT x = (UINT)input.gettok( 3 ).to_num();
		UINT y = (UINT)input.gettok( 4 ).to_num();
		LPARAM parm = MAKELONG(x,y);
		SendMessage(hwnd,WM_RBUTTONDOWN,MK_RBUTTON,parm);
		PostMessage(hwnd,WM_RBUTTONUP,MK_RBUTTON,parm); // MUST be a PostMessage or the dll hangs untill the menu is closed.
	}

	return 1;
}

// $dcx(ActiveWindow, property)
mIRC(ActiveWindow) {
	if (GetWindowInfoUx == NULL) {
		DCXError("$!dcx(ActiveWindow)", "Function Unsupported By Current OS");
		return 0;
	}

	TString input(data);
	input.trim();

	data[0] = 0;

	int numtok = input.numtok();

	if (numtok < 1) {
		DCXError("$!dcx(ActiveWindow)", "Insufficient parameters");
		return 0;
	}

	HWND hwnd = GetForegroundWindow();

	if (!IsWindow(hwnd)) {
		DCXError("$!dcx(ActiveWindow)", "Unable to determine active window");
		return 0;
	}

	TString prop(input.gettok(1));
	WINDOWINFO wi;

	ZeroMemory(&wi, sizeof(WINDOWINFO));
	GetWindowInfoUx(hwnd, &wi);

	if (prop == "hwnd")         // handle
		wsprintf(data, "%d", hwnd);
	else if (prop == "x")       // left
		wsprintf(data, "%d", wi.rcWindow.left);
	else if (prop == "y")       // top
		wsprintf(data, "%d", wi.rcWindow.top);
	else if (prop == "w")       // width
		wsprintf(data, "%d", wi.rcWindow.right - wi.rcWindow.left);
	else if (prop == "h")       // height
		wsprintf(data, "%d", wi.rcWindow.bottom - wi.rcWindow.top);
	else if (prop == "caption") // title text
		GetWindowText(hwnd, data, 900);
	else {                      // otherwise
		DCXError("$!dcx(ActiveWindow)", "Invalid parameters");
		return 0;
	}

	return 3;
}

mIRC(GhostDrag) {
	TString input(data);
	input.trim();

	if (input == "")
	{
		DCXError("/dcx GhostDrag", "Invalid parameters");
		return 0;
	}

	// [0-255] enable or (255 == disable) ghost drag for main mIRC window.
	int alpha = input.gettok(1).to_int();

	if ((alpha > -1) && (alpha < 256))
		mIRCLink.m_bDoGhostDrag = alpha;
	else {
		DCXError("/dcx GhostDrag", "Invalid alpha value");
		return 0;
	}

	return 1;
}
