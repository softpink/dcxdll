#include "Dcx.h"
#include "Classes/custom/divider.h"
#include "Classes/custom/DcxTrayIcon.h"

extern DcxTrayIcon * trayIcons;

mIRCLinker Dcx::mIRC;
TString Dcx::m_sLastError;
IClassFactory * Dcx::m_pClassFactory;
DcxGDIModule Dcx::GDIModule;
DcxUXModule Dcx::XPPlusModule;
DcxDWMModule Dcx::VistaModule;
DcxDialogCollection Dcx::Dialogs;
XPopupMenuManager Dcx::XPopups;
XMenuBar Dcx::XMenubar;
int Dcx::m_iGhostDrag = 255;
bool Dcx::m_bDX9Installed;

void Dcx::load(LOADINFO * lInfo)
{
	m_iGhostDrag = 255;
	m_bDX9Installed = false;

	// Initialize mIRCLinker
	mIRC.load(lInfo);

	// Initializing OLE Support
	debug("LoadDLL", "Initializing OLE Support...");
	OleInitialize(NULL);

	// Initializing GDI
	GDIModule.load(mIRC);

	XPPlusModule.load(mIRC);

	VistaModule.load(mIRC);

	setupOSCompatibility();

	mIRC.hookWindowProc(Dcx::mIRCSubClassWinProc);	

	XPopups.load();

	//get IClassFactory* for WebBrowser
	debug("LoadDLL", "Generating class for WebBrowser...");
	CoGetClassObject(CLSID_WebBrowser, CLSCTX_INPROC_SERVER, 0, IID_IClassFactory, (void**) &m_pClassFactory);
	//6BF52A52-394A-11D3-B153-00C04F79FAA6

	// RichEdit DLL Loading
	mIRC.debug("LoadDLL", "Generating class for RichEdit...");
	LoadLibrary("RICHED20.DLL");

	// Load Control definitions
	mIRC.debug("LoadDLL", "Loading control classes");
	INITCOMMONCONTROLSEX icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_TREEVIEW_CLASSES | ICC_BAR_CLASSES | ICC_PROGRESS_CLASS | ICC_LISTVIEW_CLASSES | 
		ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS | ICC_DATE_CLASSES | 
		ICC_TAB_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS;
	InitCommonControlsEx( &icex ); 
	// Initialise signals of diff types
	dcxSignal.xdock = false;
	dcxSignal.xstatusbar = true;
	dcxSignal.xtray = true;
}

void Dcx::unload(void)
{
	if (Dialogs.closeDialogs()) { // if unable to close dialogs stop unload.
		Dcx::error("UnloadDll", "Unable to Unload DLL from within the DLL");
		//return 0; // NB: This DOESN'T stop the unload, & mIRC will still crash.
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
	if (m_pClassFactory != NULL)
		m_pClassFactory->Release();

	// Terminating OLE Support
	OleUninitialize();


	/***** TrayIcon Stuff *****/
	if (trayIcons != NULL) {
		delete trayIcons;
		trayIcons = NULL;
	}
	
	// reset the treebars font if it's been changed.
	if (Dcx::mIRC.getTreeFont() != NULL) {
		HFONT hfont = (HFONT)SendMessage(Dcx::mIRC.getTreeview(),WM_GETFONT,0,0);
		if (hfont != Dcx::mIRC.getTreeFont()) {
			SendMessage( Dcx::mIRC.getTreeview(), WM_SETFONT, (WPARAM) Dcx::mIRC.getTreeFont(), (LPARAM) MAKELPARAM(TRUE,0));
			DeleteObject(hfont);
		}
	}

	/***** XMenuBar Stuff *****/
	XMenubar.resetMenuBar();

	XPopups.unload();

	freeOSCompatibility();
	mIRC.unload();
}

Dcx::Dcx(void)
{
}

Dcx::~Dcx(void)
{
}

void Dcx::setupOSCompatibility(void) {
	HMODULE hModule = NULL;

	hModule = GetModuleHandle("USER32.DLL");

	if (hModule != NULL) {
		// get UpdateLayeredWindow() if we can.
		UpdateLayeredWindowUx = (PFNUPDATELAYEREDWINDOW)GetProcAddress(hModule, "UpdateLayeredWindow");
		// get SetLayeredWindowAttributes() if we can.
		SetLayeredWindowAttributesUx = (PFNSETLAYEREDWINDOWATTRIBUTES)GetProcAddress(hModule, "SetLayeredWindowAttributes");
		// get GetWindowInfo()
		GetWindowInfoUx = (PFNGETWINDOWINFO)GetProcAddress(hModule, "GetWindowInfo");
		// get AnimateWindow()
		AnimateWindowUx = (PFNANIMATEWINDOW)GetProcAddress(hModule, "AnimateWindow");
		// get AnimateWindow()
		InSendMessageExUx = (PFNINSENDMESSAGEEX)GetProcAddress(hModule, "InSendMessageEx");
		// get FlashWindowEx()
		FlashWindowExUx = (PFNFLASHWINDOWEX)GetProcAddress(hModule, "FlashWindowEx");
#ifndef NDEBUG
		// debug build
		if (UpdateLayeredWindowUx && SetLayeredWindowAttributesUx && GetWindowInfoUx
			&& AnimateWindowUx && InSendMessageExUx && FlashWindowExUx) {
			mIRC.debug("LoadDLL", "Found USER32.DLL Functions");
		}
		else {
			mIRC.debug("LoadDLL", "Unable to find all USER32.DLL Functions");
		}
#else
#ifdef DCX_DEV_RELEASE
		// dev build
		if (UpdateLayeredWindowUx && SetLayeredWindowAttributesUx && GetWindowInfoUx
			&& AnimateWindowUx && InSendMessageExUx && FlashWindowExUx) {
			debug("LoadDLL", "Found USER32.DLL Functions");
		}
		else {
			debug("LoadDLL", "Unable to find all USER32.DLL Functions");
		}
#endif
#endif
	}
	mIRC.debug("LoadDLL", "Loading SHELL32.DLL...");
	hModule = GetModuleHandle("SHELL32.DLL");

	if (hModule != NULL) {
		// get PickIconDlg() if we can.
		PickIconDlgUx = (PFNPICKICONDLG)GetProcAddress(hModule, "PickIconDlg");
		mIRC.debug("LoadDLL", "Found PickIconDlg Function");
	}

	mIRC.debug("LoadDLL", "Loading COMCTL32.DLL...");
	hModule = GetModuleHandle("COMCTL32.DLL");

	if (hModule != NULL) {
		// get DrawShadowText() if we can.
		DrawShadowTextUx = (PFNDRAWSHADOWTEXT)GetProcAddress(hModule, "DrawShadowText");
		mIRC.debug("LoadDLL", "Found DrawShadowText");
	}

//#define dcxRegisterClassM(szClass, szDcxClass, CClass) { \
//	GetClassInfoEx(NULL, (TCHAR *)(szClass), &wc); \
//	wc.lpszClassName = (TCHAR *)(szDcxClass); \
//	(CClass)::m_DefaultWindowProc = wc.wc.lpfnWndProc; \
//	wc.lpfnWndProc = (CClass)::WindowProc; \
//	RegisterClassEx(&wc); \
//};

	WNDCLASSEX wc;
	ZeroMemory((void*)&wc , sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);

#define dcxRegisterClass(szClass, szDcxClass) { \
	GetClassInfoEx(NULL, (TCHAR *)(szClass), &wc); \
	wc.lpszClassName = (TCHAR *)(szDcxClass); \
	RegisterClassEx(&wc); \
};

	// Custom ProgressBar
	debug("LoadDLL", "Registering ProgressBar...");
	dcxRegisterClass(PROGRESS_CLASS, DCX_PROGRESSBARCLASS);

	// Custom TreeView
	debug("LoadDLL", "Registering TreeView...");
	dcxRegisterClass(WC_TREEVIEW, DCX_TREEVIEWCLASS);

	// Custom Toolbar
	debug("LoadDLL", "Registering ToolBar...");
	dcxRegisterClass(TOOLBARCLASSNAME, DCX_TOOLBARCLASS);

	// Custom StatusBar
	debug("LoadDLL", "Registering StatusBar...");
	dcxRegisterClass(STATUSCLASSNAME, DCX_STATUSBARCLASS);

	// Custom ListView
	debug("LoadDLL", "Registering Listview...");
	dcxRegisterClass(WC_LISTVIEW, DCX_LISTVIEWCLASS);

	// Custom ComboEx
	debug("LoadDLL", "Registering ComboEx...");
	dcxRegisterClass(WC_COMBOBOXEX, DCX_COMBOEXCLASS);

	// Custom TrackBar
	debug("LoadDLL", "Registering TrackBar...");
	dcxRegisterClass(TRACKBAR_CLASS, DCX_TRACKBARCLASS);

	// Custom RichEdit
	debug("LoadDLL", "Registering RichEdit...");
	dcxRegisterClass("RichEdit20A", DCX_RICHEDITCLASS);

	// Custom RebarCtrl
	debug("LoadDLL", "Registering ReBar...");
	dcxRegisterClass(REBARCLASSNAME, DCX_REBARCTRLCLASS);

	// Custom Color Combo
	debug("LoadDLL", "Registering ComboBox...");
	dcxRegisterClass("COMBOBOX", DCX_COLORCOMBOCLASS);

	// Custom TabCtrl
	debug("LoadDLL", "Registering Tab...");
	dcxRegisterClass(WC_TABCONTROL, DCX_TABCTRLCLASS);

	// Custom UpDown
	debug("LoadDLL", "Registering UpDown...");
	dcxRegisterClass(UPDOWN_CLASS, DCX_UPDOWNCLASS);

	// Custom IpAddress
	debug("LoadDLL", "Registering IpAddress...");
	dcxRegisterClass(WC_IPADDRESS, DCX_IPADDRESSCLASS);

	// Init Divider Control
	//InitDivider( GetModuleHandle( NULL ) );

	// Custom Divider
	debug("LoadDLL", "Registering Divider...");
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
	debug("LoadDLL", "Registering Panel...");
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
	debug("LoadDLL", "Registering Box...");
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
	debug("LoadDLL", "Registering Button...");
	dcxRegisterClass("BUTTON", DCX_BUTTONCLASS);

	// Custom Calendar
	debug("LoadDLL", "Registering Calendar...");
	dcxRegisterClass(MONTHCAL_CLASS, DCX_CALENDARCLASS);

	// Custom DateTime
	debug("LoadDLL", "Registering DateTime...");
	dcxRegisterClass(DATETIMEPICK_CLASS, DCX_DATETIMECLASS);

	// Custom Pager
	debug("LoadDLL", "Registering Pager...");
	dcxRegisterClass(WC_PAGESCROLLER, DCX_PAGERCLASS);

	// Shadow Class
	debug("LoadDLL", "Registering Shadow...");
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
	debug("LoadDLL", "Registering Vista Dialog...");
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
}

void Dcx::freeOSCompatibility(void)
{
	GDIModule.unload();
	XPPlusModule.unload();
	VistaModule.unload();
}

const char * Dcx::getLastError()
{
	return m_sLastError.to_chr();
}

IClassFactory* Dcx::getClassFactory()
{
	return m_pClassFactory;
}

int Dcx::getGhostDrag()
{
	return m_iGhostDrag;
}

bool Dcx::setGhostDrag(int newAlpha)
{
	if ((newAlpha < 0) || (newAlpha > 255)) return false;
	m_iGhostDrag = newAlpha;
	return true;
}

bool Dcx::isDX9Installed() 
{
	return m_bDX9Installed;
}

bool Dcx::isUnloadSave()
{
	return Dialogs.safeToCloseAll() && !XPopups.isPatched();
}
	
bool Dcx::initDirectX() 
{
	return initDirectX(NULL, 0);
}

bool Dcx::initDirectX(char *dxResult, int dxSize) 
{
#ifdef DCX_USE_DXSDK
	debug("DXSetup", "Checking DirectX Version...");
	DWORD dx_ver;
	if (GetDXVersion(&dx_ver, dxResult, dxSize) == S_OK) {
		if (dx_ver < 0x00090000) {
			debug("DXSetup", "Got DirectX Version: Need V9+");
			m_bDX9Installed = false;
		}
		else {
			debug("DXSetup", "Got DirectX Version: V9+ Installed");
			m_bDX9Installed = true;
		}
	}
	else {
		debug("DXSetup", "Couldn't Get DirectX Version");
		m_bDX9Installed = false;
	}
#endif
	return m_bDX9Installed;
}

/*!
* \brief Displays output text to the mIRC status window.
*/
void Dcx::debug(const char *cmd, const char *msg)
{
	mIRC.debug(cmd, msg);
}

/*!
 * \brief Sends an error message to mIRC.
 */
void Dcx::error(const char *cmd, const char *msg)
{
	m_sLastError.sprintf("D_ERROR %s (%s)", cmd, msg);
	mIRC.echo(m_sLastError.to_chr());
}

/*
 * Variable argument error message.
 */
void Dcx::errorex(const char *cmd, const char *szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
	char temp[2048];

	vsprintf(temp, szFormat, args);
	error(cmd, temp);
}

int Dcx::mark(char* data, TString & tsDName, TString & tsCallbackName)
{
	char res[40];

	// check if the alias exists
	if (!mIRC.isAlias(tsCallbackName.to_chr())) {
		errorex("Mark", "No such alias : %s", tsCallbackName.to_chr());
		ret(getLastError());
	}

	// check if valid dialog
	HWND mHwnd = GetHwndFromString(tsDName.to_chr());

	if (IsWindow(mHwnd) == FALSE) {
		errorex("Mark", "Invalid Dialog Window : %s", tsDName.to_chr());
		ret(getLastError());
	}

	if (Dialogs.getDialogByHandle(mHwnd) != NULL) {
		errorex("Mark", "Window Already Marked : %s", tsDName.to_chr());
		ret(getLastError());
	}
	Dialogs.markDialog(mHwnd, tsDName, tsCallbackName);
	{
		DcxDialog *pTmp = Dialogs.getDialogByHandle(mHwnd);
		if (pTmp != NULL) {
			pTmp->evalAliasEx(res, 40, "isverbose,0");

			if (lstrcmp(res, "$false") == 0)
				pTmp->SetVerbose(false);
		}
	}
	ret("D_OK Mark: Dialog Marked");
}

/*!
* \brief blah
*
* blah
*/
LRESULT CALLBACK Dcx::mIRCSubClassWinProc(HWND mHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SIZE:
			{
				Dcx::mIRC.signalex(dcxSignal.xdock, "size mIRC %d %d %d", mHwnd, LOWORD(lParam), HIWORD(lParam));
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
			return Dcx::XPopups.OnInitMenuPopup(mHwnd, wParam, lParam);

		case WM_COMMAND:
			return Dcx::XPopups.OnCommand(mHwnd, wParam, lParam);

		case WM_EXITMENULOOP:
			return Dcx::XPopups.OnExitMenuLoop(mHwnd, wParam, lParam);

		case WM_UNINITMENUPOPUP:
			return Dcx::XPopups.OnUninitMenuPopup(mHwnd, wParam, lParam);

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
			Dcx::mIRC.resetWindowProc();
			PostMessage(mHwnd, uMsg, 0, 0);
			return 0L;
			break;
		}
		// ghost drag stuff
		case WM_ENTERSIZEMOVE:
			{
				if (Dcx::getGhostDrag() < 255 && SetLayeredWindowAttributesUx != NULL) {
					long style = GetWindowExStyle(Dcx::mIRC.getHWND());
					// Set WS_EX_LAYERED on this window
					if (!(style & WS_EX_LAYERED))
						SetWindowLong(Dcx::mIRC.getHWND(), GWL_EXSTYLE, style | WS_EX_LAYERED);
					// Make this window 75 alpha
					SetLayeredWindowAttributesUx(Dcx::mIRC.getHWND(), 0, (BYTE)Dcx::getGhostDrag(), LWA_ALPHA);
					SetProp(Dcx::mIRC.getHWND(), "dcx_ghosted", (HANDLE)1);
				}
			}
			break;

		case WM_EXITSIZEMOVE:
		{
			// turn off ghosting.
			if (GetProp(Dcx::mIRC.getHWND(), "dcx_ghosted") != NULL && SetLayeredWindowAttributesUx != NULL) {
				// Make this window solid
				SetLayeredWindowAttributesUx(Dcx::mIRC.getHWND(), 0, 255, LWA_ALPHA);
				RemoveProp(Dcx::mIRC.getHWND(), "dcx_ghosted");
			}
			break;
		}

		case WM_DWMCOMPOSITIONCHANGED:
			{
				if (DwmIsCompositionEnabledUx != NULL)
					Dcx::VistaModule.refreshComposite();
			}
			break;

		default:
			break;
	}

	return Dcx::mIRC.callDefaultWindowProc(mHwnd, uMsg, wParam, lParam);
}