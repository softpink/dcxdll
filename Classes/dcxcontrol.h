/*!
 * \file dcxcontrol.h
 * \brief blah
 *
 * blah
 *
 * \author David Legault ( clickhere at scriptsdb dot org )
 * \version 1.0
 *
 * \b Revisions
 *
 * � ScriptsDB.org - 2006
 */

#ifndef _DCXCONTROL_H_
#define _DCXCONTROL_H_

#define DCC_TEXTCOLOR     0x01 //!< Control Text Color
#define DCC_TEXTBKGCOLOR  0x02 //!< Control Text Background Color
#define DCC_BKGCOLOR      0x04 //!< Control Background Color
#define DCC_BORDERCOLOR   0x08 //!< Control Border Color

#define DCCS_FROMRESSOURCE 0x01 //!< Cursor from ressource
#define DCCS_FROMFILE      0x02 //!< Cursor from File

#include "../defines.h"
#include "dcxwindow.h"

#define CTLF_ALLOW_PBAR				0x000000001
#define CTLF_ALLOW_TRACKBAR			0x000000002
#define CTLF_ALLOW_COMBOEX			0x000000004
#define CTLF_ALLOW_COLORCOMBO		0x000000008
#define CTLF_ALLOW_STATUSBAR		0x000000010
#define CTLF_ALLOW_TOOLBAR			0x000000020
#define CTLF_ALLOW_TREEVIEW			0x000000040
#define CTLF_ALLOW_LISTVIEW			0x000000080
#define CTLF_ALLOW_REBAR			0x000000100
#define CTLF_ALLOW_BUTTON			0x000000200
#define CTLF_ALLOW_RICHEDIT			0x000000400
#define CTLF_ALLOW_EDIT				0x000000800
#define CTLF_ALLOW_UPDOWN			0x000001000
#define CTLF_ALLOW_IPADDRESS		0x000002000
#define CTLF_ALLOW_WEBCTRL			0x000004000
#define CTLF_ALLOW_CALANDER			0x000008000
#define CTLF_ALLOW_DIVIDER			0x000010000
#define CTLF_ALLOW_PANEL			0x000020000
#define CTLF_ALLOW_TAB				0x000040000
#define CTLF_ALLOW_LINE				0x000080000
#define CTLF_ALLOW_BOX				0x000100000
#define CTLF_ALLOW_RADIO			0x000200000
#define CTLF_ALLOW_CHECK			0x000400000
#define CTLF_ALLOW_TEXT				0x000800000
#define CTLF_ALLOW_SCROLL			0x001000000
#define CTLF_ALLOW_LIST				0x002000000
#define CTLF_ALLOW_LINK				0x004000000
#define CTLF_ALLOW_IMAGE			0x008000000
#define CTLF_ALLOW_PAGER			0x010000000
#define CTLF_ALLOW_DOCK				0x020000000 // allows @Window and Dialog docking
#define CTLF_ALLOW_DATETIME			0x040000000
#define CTLF_ALLOW_STACKER			0x080000000
#define CTLF_ALLOW_DIRECTSHOW		0x100000000

class DcxDialog;

typedef struct {
	HDC ai_hdc;
	HDC *ai_hdcBuffer;
	HDC ai_Oldhdc;
	HBITMAP ai_bitmap;
	HBITMAP ai_bkg;
	HBITMAP ai_oldBM;
	RECT ai_rcClient;
	RECT ai_rcWin;
#ifdef DCX_USE_WINSDK
	HPAINTBUFFER ai_Buffer;
#endif
} ALPHAINFO, *LPALPHAINFO;

/*!
 * \brief blah
 *
 * blah
 */

class DcxControl : public DcxWindow {

public:

  DcxControl( const UINT mID, DcxDialog * p_Dialog );
  virtual ~DcxControl( );

  virtual void parseInfoRequest( TString & input, char * szReturnValue ) = 0;
  virtual void parseCommandRequest( TString & input ) = 0;
  virtual void parseControlStyles( TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme ) = 0;

  static void parseGeneralControlStyles( const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme );

  BOOL callAliasEx( char * szReturn, const char * szFormat, ... );

  UINT getUserID( ) const;

  static LRESULT CALLBACK WindowProc( HWND mHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

  virtual LRESULT PostMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed ) = 0;
  virtual LRESULT ParentMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed ) = 0;

  LRESULT setFont( const HFONT hFont, const BOOL fRedraw );
  LRESULT getFont( ) const;

  void setControlFont( const HFONT hFont, const BOOL fRedraw );

  LRESULT setRedraw( const BOOL fView );

  HBRUSH getBackClrBrush( ) const;
  COLORREF getBackColor( ) const;
  COLORREF getTextColor( ) const;

  static DcxControl * controlFactory( DcxDialog * p_Dialog, const UINT mID, const TString & input, int offset, const UINT64 mask = -1, HWND hParent = NULL);

  virtual TString getType( ) = 0;

  inline void incRef( ) { ++this->m_iRefCount; };
  inline void decRef( ) { --this->m_iRefCount; };
  inline UINT getRefCount( ) const { return this->m_iRefCount; };
	//DcxControl *getParentCtrl() const { return this->m_pParentCtrl; };
	void updateParentCtrl(void); //!< updates controls host control pointers, MUST be called before these pointers are used.
	static void DrawCtrlBackground(const HDC hdc, const DcxControl *p_this, const LPRECT rwnd);
	void DrawParentsBackground(const HDC hdc, const LPRECT rcBounds = NULL, const HWND dHwnd = NULL);
	LPALPHAINFO SetupAlphaBlend(HDC *hdc, const bool DoubleBuffer = false);
	void FinishAlphaBlend(LPALPHAINFO ai);
	static HBITMAP resizeBitmap(HBITMAP srcBM, const LPRECT rc);
	void showError(const char *prop, const char *cmd, const char *err);
	void showErrorEx(const char *prop, const char *cmd, const char *fmt, ...);

protected:

  DcxDialog * m_pParentDialog; //!< Parent DcxDialog object

  WNDPROC m_DefaultWindowProc;  //!< Default window procedure

  HFONT m_hFont; //!< Control Font

  TString m_tsMark; //!< Mark Information (see /xdid -M)

  COLORREF m_clrText;     //!< Font color
  COLORREF m_clrBackText; //!< Font Back Color (not supported)
  HBRUSH m_hBackBrush;    //!< Background control color
	HBRUSH m_hBorderBrush;	//!< Controls Border Colour.
	HBITMAP m_bitmapBg;			//!< Background bitmap
	COLORREF m_colTransparentBg;
	COLORREF m_clrBackground;	//!< Background Colour. (used to make m_hBackBrush)

  UINT m_iRefCount;

  HCURSOR m_hCursor;  //!< Cursor Handle
  BOOL m_bCursorFromFile; //!< Cursor comes from a file?

	HWND m_ToolTipHWND; //!< Tooltip window (if any)
	TString m_tsToolTip; //!< This controls tooltip text (if any).
	DWORD m_dEventMask;
	bool m_bAlphaBlend;	//!< Control is alpha blended.
	int m_iAlphaLevel; //!< The amount the control is alpha blended.
	//DcxControl *m_pParentCtrl;
	HWND m_pParentHWND;
	bool m_bInPrint;
	bool m_bShadowText; //!< Text is drawn with a shadow.
	bool m_bCtrlCodeText; //!< mIRC's ctrl codes are used to change the text's appearance.
	bool m_bNoTheme; //!< Control isn't themed.
  /* ***** */

  void parseGlobalCommandRequest( const TString & input, XSwitchFlags & flags );
  BOOL parseGlobalInfoRequest( const TString & input, char * szReturnValue );

  static UINT parseColorFlags( TString & flags );
  static UINT parseCursorFlags( TString & flags );
  static LPSTR parseCursorType( TString & cursor );

  void registreDefaultWindowProc( );
  void unregistreDefaultWindowProc( );

  static void parseBorderStyles( TString & flags, LONG * Styles, LONG * ExStyles );
  LRESULT CommonMessage( const UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed );
	static void InvalidateParentRect(HWND hwnd);
	void DrawControl(HDC hDC, HWND hwnd);
};

#endif // _DCXCONTROL_H_
