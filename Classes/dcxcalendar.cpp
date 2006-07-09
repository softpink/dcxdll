/*!
http://www.codeguru.com/cpp/controls/controls/dateselectioncontrolsetc/article.php/c2229/
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/sysinfo/base/converting_a_time_t_value_to_a_file_time.asp

 * \file dcxcalendar.cpp
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

#include "dcxcalendar.h"
#include "dcxdialog.h"

/*!
 * \brief Constructor
 *
 * \param ID Control ID
 * \param p_Dialog Parent DcxDialog Object
 * \param rc Window Rectangle
 * \param styles Window Style Tokenized List
 */

DcxCalendar::DcxCalendar( UINT ID, DcxDialog * p_Dialog, RECT * rc, TString & styles ) 
: DcxControl( ID, p_Dialog ) 
{

  LONG Styles = 0, ExStyles = 0;
  BOOL bNoTheme = FALSE;
  this->parseControlStyles( styles, &Styles, &ExStyles, &bNoTheme );

  this->m_Hwnd = CreateWindowEx(	
    WS_EX_CLIENTEDGE, 
    DCX_CALENDARCLASS, 
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | Styles, 
    rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
    p_Dialog->getHwnd( ),
    (HMENU) ID,
    GetModuleHandle( NULL ), 
    NULL);

  if ( bNoTheme )
    SetWindowTheme( this->m_Hwnd , L" ", L" " );

  this->setControlFont( (HFONT) GetStockObject( DEFAULT_GUI_FONT ), FALSE );
  this->registreDefaultWindowProc( );
  SetProp( this->m_Hwnd, "dcx_cthis", (HANDLE) this );
}

/*!
 * \brief Constructor
 *
 * \param ID Control ID
 * \param p_Dialog Parent DcxDialog Object
 * \param mParentHwnd Parent Window Handle
 * \param rc Window Rectangle
 * \param styles Window Style Tokenized List
 */

DcxCalendar::DcxCalendar( UINT ID, DcxDialog * p_Dialog, HWND mParentHwnd, RECT * rc, TString & styles ) 
: DcxControl( ID, p_Dialog ) 
{

  LONG Styles = 0, ExStyles = 0;
  BOOL bNoTheme = FALSE;
  this->parseControlStyles( styles, &Styles, &ExStyles, &bNoTheme );

  this->m_Hwnd = CreateWindowEx(	
    WS_EX_CLIENTEDGE, 
    DCX_CALENDARCLASS, 
    NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | Styles, 
    rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
    mParentHwnd,
    (HMENU) ID,
    GetModuleHandle(NULL), 
    NULL);

  if ( bNoTheme )
    SetWindowTheme( this->m_Hwnd , L" ", L" " );

  this->setControlFont( (HFONT) GetStockObject( DEFAULT_GUI_FONT ), FALSE );
  this->registreDefaultWindowProc( );
  SetProp( this->m_Hwnd, "dcx_cthis", (HANDLE) this );
}

/*!
 * \brief blah
 *
 * blah
 */

DcxCalendar::~DcxCalendar( ) {

  this->unregistreDefaultWindowProc( );
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxCalendar::parseControlStyles(TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) {
	unsigned int i = 1, numtok = styles.numtok(" ");

	//*Styles = MCS_DAYSTATE;

	while (i <= numtok) {
		if (styles.gettok(i , " ") == "multi")
			*Styles |= MCS_MULTISELECT;
		else if (styles.gettok(i , " ") == "notoday")
			*Styles |= MCS_NOTODAY;
		else if (styles.gettok(i , " ") == "notodaycircle")
			*Styles |= MCS_NOTODAYCIRCLE;
		else if (styles.gettok(i , " ") == "weeknum")
			*Styles |= MCS_WEEKNUMBERS;

		i++;
	}

	this->parseGeneralControlStyles(styles, Styles, ExStyles, bNoTheme);
}

/*!
 * \brief $xdid Parsing Function
 *
 * \param input [NAME] [ID] [PROP] (OPTIONS)
 * \param szReturnValue mIRC Data Container
 *
 * \return > void
 */

void DcxCalendar::parseInfoRequest( TString & input, char * szReturnValue ) {

  int numtok = input.numtok( " " );

  // [NAME] [ID] [PROP]
  if ( input.gettok( 3, " " ) == "text" ) {

  }
  else if ( this->parseGlobalInfoRequest( input, szReturnValue ) ) {

    return;
  }
  
  szReturnValue[0] = 0;
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxCalendar::parseCommandRequest( TString & input ) {

  XSwitchFlags flags;
  ZeroMemory( (void*)&flags, sizeof( XSwitchFlags ) );
  this->parseSwitchFlags( &input.gettok( 3, " " ), &flags );
//MonthCal_SetMaxSelCount(this->m_Hwnd, 7);
  // set colors
  // set cur sel
  // set day state
  // set sel range
  // set today

  int numtok = input.numtok( " " );
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/monthcal/structures/nmdaystate.asp
  //xdid -t [NAME] [ID] [SWITCH]
  if ( flags.switch_flags[19] ) {
    TString text = input.gettok( 4, -1, " " );
    text.trim( );
    SetWindowText( this->m_Hwnd, text.to_chr( ) );
  }
  else
    this->parseGlobalCommandRequest( input, flags );
}

/*!
 * \brief blah
 *
 * blah
 */

LRESULT DcxCalendar::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) {
	switch (uMsg) {
		case WM_HELP: {
			char ret[256];
			this->callAliasEx( ret, "%s,%d", "help", this->getUserID( ) );
			break;
		}

		//case WM_GETDLGCODE:
		//{
		//	return DLGC_WANTARROWS;
		//	break;
		//}

		case WM_NOTIFY: {
			LPNMHDR hdr = (LPNMHDR) lParam;

			if (!hdr)
				break;

			switch(hdr->code) {
/*
				case MCS_DAYSTATE: {
#define BOLDDAY(ds,iDay) if(iDay>0 && iDay<32)\
                            (ds)|=(0x00000001<<(iDay-1))

					LPNMDAYSTATE lpNMDayState = (LPNMDAYSTATE) lParam;
					MONTHDAYSTATE mds[12];

					int iMax = lpNMDayState->cDayState;
mIRCDebug("max %d", iMax);
					for (int i = 0; i < iMax; i++) {
						mIRCDebug("%d", i);
						mds[i] = (MONTHDAYSTATE) 0;
						BOLDDAY(mds[i],15);
						//if (i == 15)
						//	mds[i] = (MONTHDAYSTATE) TRUE;
					}

						//BOLDDAY(mds[i],15);

					lpNMDayState->prgDayState = mds;
					//mIRCError("daystate");
					bParsed = TRUE;
					return FALSE;
					break;
				}
*/
				case MCN_SELCHANGE: {
					char ret[256];
					this->callAliasEx(ret, "%s,%d", "selchange", this->getUserID());
					break;
				}
				case MCN_SELECT: {
					char ret[256];

					// specific code to handle multiselect dates
					if (this->isStyle(MCS_MULTISELECT)) {
						// get the selected range
						SYSTEMTIME selrange[2];

						MonthCal_GetSelRange(this->m_Hwnd, selrange);

						// send event to callback
						this->callAliasEx(ret, "%s,%d,%d,%d", "select", this->getUserID(),
							SystemTimeToMircTime(selrange[0]),
							SystemTimeToMircTime(selrange[1]));
					}
					// code to handle single selected dates
					else {
						SYSTEMTIME st;
						MonthCal_GetCurSel(this->m_Hwnd, &st);

						// send event to callback
						this->callAliasEx(ret, "%s,%d,%d", "select", this->getUserID(), SystemTimeToMircTime(st));
					}

					break;
				}
				case NM_RELEASEDCAPTURE: {
					char ret[256];
					this->callAliasEx(ret, "%s,%d", "sclick", this->getUserID());
					break;
				}
				default: {
					//mIRCError("ELSE");
					break;
				}
			} // end switch
		}

		case WM_MOUSEMOVE: {
			this->m_pParentDialog->setMouseControl(this->getUserID());
			break;
		}

		case WM_SETFOCUS: {
			this->m_pParentDialog->setFocusControl(this->getUserID());
			break;
		}

		case WM_SETCURSOR: {
			if (LOWORD(lParam) == HTCLIENT && (HWND) wParam == this->m_Hwnd && this->m_hCursor != NULL) {
				SetCursor( this->m_hCursor );
				bParsed = TRUE;
				return TRUE;
			}

			break;
		}

		case WM_DESTROY: {
			//mIRCError( "WM_DESTROY" );
			delete this;
			bParsed = TRUE;
			break;
		}

		default:
			break;
	}

	return 0L;
}

long DcxCalendar::SystemTimeToMircTime(SYSTEMTIME st) {
	char *ret = new char[100];

	TString months[12] = {
		"January",
		"Feburary",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	};

	wsprintf(ret, "$ctime(%d %s %d)",
		st.wDay,
		months[st.wMonth -1],
		st.wYear);

	mIRCeval(ret, ret);
	return atoi(ret);
}
