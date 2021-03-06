/*!
 * \file dcxtab.cpp
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
#include "defines.h"
#include "Classes/dcxtab.h"
#include "Classes/dcxdialog.h"

/*!
 * \brief Constructor
 *
 * \param ID Control ID
 * \param p_Dialog Parent DcxDialog Object
 * \param mParentHwnd Parent Window Handle
 * \param rc Window Rectangle
 * \param styles Window Style Tokenized List
 */

DcxTab::DcxTab( UINT ID, DcxDialog * p_Dialog, HWND mParentHwnd, RECT * rc, const TString & styles ) 
: DcxControl(ID, p_Dialog)
, m_bClosable(false)
, m_bGradient(false)
{
	LONG Styles = 0, ExStyles = 0;
	BOOL bNoTheme = FALSE;
	this->parseControlStyles( styles, &Styles, &ExStyles, &bNoTheme );

	this->m_Hwnd = CreateWindowEx(	
		ExStyles | WS_EX_CONTROLPARENT, 
		DCX_TABCTRLCLASS, 
		NULL,
		WS_CHILD | Styles, 
		rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
		mParentHwnd,
		(HMENU) ID,
		GetModuleHandle(NULL), 
		NULL);

	if (!IsWindow(this->m_Hwnd))
		throw TEXT("Unable To Create Window");

	if ( bNoTheme )
		Dcx::UXModule.dcxSetWindowTheme( this->m_Hwnd , L" ", L" " );

	/*
	HWND hHwndTip = TabCtrl_GetToolTips( this->m_Hwnd );
	if ( IsWindow( hHwndTip ) ) {

	TOOLINFO ti;
	ZeroMemory( &ti, sizeof( TOOLINFO ) );
	ti.cbSize = sizeof( TOOLINFO );
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = mParentHwnd;
	ti.uId = (UINT) this->m_Hwnd;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage( hHwndTip, TTM_ADDTOOL, (WPARAM) 0, (LPARAM) &ti );
	}
	*/
	//if (p_Dialog->getToolTip() != NULL) {
	//	if (styles.istok(TEXT("tooltips"))) {
	//		this->m_ToolTipHWND = p_Dialog->getToolTip();
	//		TabCtrl_SetToolTips(this->m_Hwnd,this->m_ToolTipHWND);
	//		//AddToolTipToolInfo(this->m_ToolTipHWND, this->m_Hwnd);
	//	}
	//}

	this->setControlFont( GetStockFont( DEFAULT_GUI_FONT ), FALSE );
	this->registreDefaultWindowProc( );
	SetProp( this->m_Hwnd, TEXT("dcx_cthis"), (HANDLE) this );
}

/*!
 * \brief blah
 *
 * blah
 */

DcxTab::~DcxTab( ) {

	ImageList_Destroy( this->getImageList( ) );

	const int nItems = TabCtrl_GetItemCount( this->m_Hwnd );
	for (int n = 0; n < nItems; n++ )
		this->deleteLParamInfo( n );

	this->unregistreDefaultWindowProc( );
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxTab::parseControlStyles( const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme ) {

	//*ExStyles = WS_EX_CONTROLPARENT;

	for (TString tsStyle(styles.getfirsttok( 1 )); tsStyle != TEXT(""); tsStyle = styles.getnexttok( ))
	{
		if ( tsStyle == TEXT("vertical") )
			*Styles |= TCS_VERTICAL;
		else if ( tsStyle == TEXT("bottom") )
			*Styles |= TCS_BOTTOM;
		else if ( tsStyle == TEXT("right") )
			*Styles |= TCS_RIGHT;
		else if ( tsStyle == TEXT("fixedwidth") )
			*Styles |= TCS_FIXEDWIDTH;
		else if ( tsStyle == TEXT("buttons") )
			*Styles |= TCS_BUTTONS;
		else if ( tsStyle == TEXT("flat") )
			*Styles |= TCS_FLATBUTTONS;
		else if ( tsStyle == TEXT("hot") )
			*Styles |= TCS_HOTTRACK;
		else if ( tsStyle == TEXT("multiline") )
			*Styles |= TCS_MULTILINE;
		else if ( tsStyle == TEXT("rightjustify") )
			*Styles |= TCS_RIGHTJUSTIFY;
		else if ( tsStyle == TEXT("scrollopposite") )
			*Styles |= TCS_SCROLLOPPOSITE;
		//else if ( tsStyle == TEXT("tooltips") )
		//  *Styles |= TCS_TOOLTIPS;
		else if ( tsStyle == TEXT("flatseps") )
			*ExStyles |= TCS_EX_FLATSEPARATORS;
		else if ( tsStyle == TEXT("closable")) {
			this->m_bClosable = true;
			*Styles |= TCS_OWNERDRAWFIXED;
		}
		else if ( tsStyle == TEXT("gradient") )
			this->m_bGradient = true;
	}
	this->parseGeneralControlStyles( styles, Styles, ExStyles, bNoTheme );
}

/*!
 * \brief $xdid Parsing Function
 *
 * \param input [NAME] [ID] [PROP] (OPTIONS)
 * \param szReturnValue mIRC Data Container
 *
 * \return > void
 */

void DcxTab::parseInfoRequest( const TString & input, TCHAR * szReturnValue ) const {

	const UINT numtok = input.numtok( );
	const TString prop(input.getfirsttok( 3 ));

	if ( prop == TEXT("text") && numtok > 3 ) {

		const int nItem = input.getnexttok( ).to_int( ) - 1;	// tok 4

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {

			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );

			tci.mask = TCIF_TEXT;
			tci.pszText = szReturnValue;
			tci.cchTextMax = MIRC_BUFFER_SIZE_CCH;

			TabCtrl_GetItem( this->m_Hwnd, nItem, &tci );
			return;
		}
	}
	else if ( prop == TEXT("num") ) {

		wnsprintf( szReturnValue, MIRC_BUFFER_SIZE_CCH, TEXT("%d"), TabCtrl_GetItemCount( this->m_Hwnd ) );
		return;
	}
	// [NAME] [ID] [PROP] [N]
	else if ( prop == TEXT("icon") && numtok > 3 ) {

		const int iTab = input.getnexttok( ).to_int( ) - 1;		// tok 4

		if ( iTab > -1 && iTab < TabCtrl_GetItemCount( this->m_Hwnd ) ) {

			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );

			tci.mask = TCIF_IMAGE;

			TabCtrl_GetItem( this->m_Hwnd, iTab, &tci );

			wnsprintf( szReturnValue, MIRC_BUFFER_SIZE_CCH, TEXT("%d"), tci.iImage + 1 );
			return;
		}
	}
	else if ( prop == TEXT("sel") ) {

		const int nItem = TabCtrl_GetCurSel( this->m_Hwnd );

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {

			wnsprintf( szReturnValue, MIRC_BUFFER_SIZE_CCH, TEXT("%d"), nItem + 1 );
			return;
		}
	}
	else if ( prop == TEXT("seltext") ) {

		const int nItem = TabCtrl_GetCurSel( this->m_Hwnd );

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {

			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );

			tci.mask = TCIF_TEXT;
			tci.pszText = szReturnValue;
			tci.cchTextMax = MIRC_BUFFER_SIZE_CCH;

			TabCtrl_GetItem( this->m_Hwnd, nItem, &tci );
			return;
		}
	}
	else if ( prop == TEXT("childid") && numtok > 3 ) {

		const int nItem = input.getnexttok( ).to_int( ) - 1;	// tok 4

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {

			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );

			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem( this->m_Hwnd, nItem, &tci );

			LPDCXTCITEM lpdtci = (LPDCXTCITEM) tci.lParam;

			DcxControl * c = this->m_pParentDialog->getControlByHWND( lpdtci->mChildHwnd );
			if ( c != NULL ) 
				wnsprintf( szReturnValue, MIRC_BUFFER_SIZE_CCH, TEXT("%d"), c->getUserID( ) );

			return;
		}
	}
	// [NAME] [ID] [PROP]
	else if (prop == TEXT("mouseitem")) {
		TCHITTESTINFO tchi;

		tchi.flags = TCHT_ONITEM;
		GetCursorPos(&tchi.pt);
		MapWindowPoints(NULL, this->m_Hwnd, &tchi.pt, 1);

		const int tab = TabCtrl_HitTest(this->m_Hwnd, &tchi);

		wnsprintf(szReturnValue, MIRC_BUFFER_SIZE_CCH, TEXT("%d"), tab +1);
		return;
	}


	else if ( this->parseGlobalInfoRequest( input, szReturnValue ) )
		return;

	szReturnValue[0] = 0;
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxTab::parseCommandRequest( const TString & input ) {
	const XSwitchFlags flags(input.getfirsttok( 3 ));
	const UINT numtok = input.numtok( );

	// xdid -r [NAME] [ID] [SWITCH]
	if (flags[TEXT('r')]) {
		TCITEM tci;
		const int nItems = TabCtrl_GetItemCount(this->m_Hwnd);

		for (int n = 0; n < nItems; n++)
		{
			ZeroMemory(&tci, sizeof(TCITEM));

			tci.mask = TCIF_PARAM;

			if (TabCtrl_GetItem(this->m_Hwnd, n, &tci)) {
				LPDCXTCITEM lpdtci = (LPDCXTCITEM) tci.lParam;

				if (lpdtci != NULL && lpdtci->mChildHwnd != NULL && IsWindow(lpdtci->mChildHwnd)) {
					DestroyWindow(lpdtci->mChildHwnd);
					delete lpdtci;
				}
			}
		}

		TabCtrl_DeleteAllItems(this->m_Hwnd);
	}

	// xdid -a [NAME] [ID] [SWITCH] [N] [ICON] [TEXT][TAB][ID] [CONTROL] [X] [Y] [W] [H] (OPTIONS)[TAB](TOOLTIP)
	if ( flags[TEXT('a')] && numtok > 4 ) {
		TCITEM tci;
		ZeroMemory( &tci, sizeof( TCITEM ) );
		tci.mask = TCIF_IMAGE | TCIF_PARAM;

		const TString data(input.getfirsttok( 1, TSTAB ).trim());

		TString control_data;
		TString tooltip;
		const UINT nToks = input.numtok( TSTAB );

		if ( nToks > 1 ) {
			control_data = input.getnexttok( TSTAB ).trim();	// tok 2

			if ( nToks > 2 )
				tooltip = input.gettok( 3, -1, TSTAB ).trim();
		}

		int nIndex = data.getfirsttok( 4 ).to_int( ) - 1;

		if ( nIndex == -1 )
			nIndex += TabCtrl_GetItemCount( this->m_Hwnd ) + 1;

		tci.iImage = data.getnexttok( ).to_int( ) - 1;	// tok 5

		// Extra params
		LPDCXTCITEM lpdtci = new DCXTCITEM;

		if (lpdtci == NULL) {
			this->showError(NULL, TEXT("-a"), TEXT("Unable to Allocate Memory"));
			return;
		}

		lpdtci->tsTipText = tooltip;
		tci.lParam = (LPARAM) lpdtci;

		// Itemtext
		TString itemtext;
		if ( data.numtok( ) > 5 ) {
			itemtext = data.gettok( 6, -1 );
			tci.mask |= TCIF_TEXT;

			if (this->m_bClosable)
				itemtext += TEXT("   ");

			tci.pszText = itemtext.to_chr( );
		}

		if ( control_data.numtok( ) > 5 ) {
			UINT ID = mIRC_ID_OFFSET + (UINT)control_data.gettok( 1 ).to_int( );

			if ( ID > mIRC_ID_OFFSET - 1 && 
				!IsWindow( GetDlgItem( this->m_pParentDialog->getHwnd( ), ID ) ) && 
				this->m_pParentDialog->getControlByID( ID ) == NULL ) 
			{
				try {
					DcxControl * p_Control = DcxControl::controlFactory(this->m_pParentDialog,ID,control_data,2,
						CTLF_ALLOW_TREEVIEW |
						CTLF_ALLOW_LISTVIEW |
						CTLF_ALLOW_RICHEDIT |
						CTLF_ALLOW_DIVIDER |
						CTLF_ALLOW_PANEL |
						CTLF_ALLOW_TAB |
						CTLF_ALLOW_REBAR |
						CTLF_ALLOW_WEBCTRL |
						CTLF_ALLOW_EDIT |
						CTLF_ALLOW_IMAGE |
						CTLF_ALLOW_LIST
						,this->m_Hwnd);

					if ( p_Control != NULL ) {
						lpdtci->mChildHwnd = p_Control->getHwnd( );
						this->m_pParentDialog->addControl( p_Control );
					}
				}
				catch ( TCHAR *err ) {
					this->showErrorEx(NULL, TEXT("-a"), TEXT("Unable To Create Control %d (%s)"), ID - mIRC_ID_OFFSET, err);
				}
			}
			else
				this->showErrorEx(NULL, TEXT("-a"), TEXT("Control with ID \"%d\" already exists"), ID - mIRC_ID_OFFSET );
		}

		TabCtrl_InsertItem( this->m_Hwnd, nIndex, &tci );
		this->activateSelectedTab( );
	}
	// xdid -c [NAME] [ID] [SWITCH] [N]
	else if ( flags[TEXT('c')] && numtok > 3 ) {
		const int nItem = input.getnexttok( ).to_int( ) - 1;	// tok 4

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {
			TabCtrl_SetCurSel( this->m_Hwnd, nItem );
			this->activateSelectedTab( );
		}
	}
	// xdid -d [NAME] [ID] [SWITCH] [N]
	else if ( flags[TEXT('d')] && numtok > 3 ) {
		const int nItem = input.getnexttok( ).to_int( ) - 1;	// tok 4

		// if a valid item to delete
		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {
			const int curSel = TabCtrl_GetCurSel(this->m_Hwnd);
			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );

			tci.mask = TCIF_PARAM;

			if (TabCtrl_GetItem(this->m_Hwnd, nItem, &tci)) {
				LPDCXTCITEM lpdtci = (LPDCXTCITEM) tci.lParam;

				if ( lpdtci != NULL && lpdtci->mChildHwnd != NULL && IsWindow( lpdtci->mChildHwnd ) ) {
					DestroyWindow( lpdtci->mChildHwnd );
					delete lpdtci;
				}
			}

			TabCtrl_DeleteItem( this->m_Hwnd, nItem );

			// select the next tab item if its the current one
			if (curSel == nItem) {
				if (nItem < TabCtrl_GetItemCount(this->m_Hwnd))
					TabCtrl_SetCurSel(this->m_Hwnd, nItem);
				else
					TabCtrl_SetCurSel(this->m_Hwnd, nItem -1);

				this->activateSelectedTab( );
			}
		}
	}
	// xdid -l [NAME] [ID] [SWITCH] [N] [ICON]
	else if ( flags[TEXT('l')] && numtok > 4 ) {
		const int nItem = input.getnexttok( ).to_int( ) - 1;	// tok 4
		const int nIcon = input.getnexttok( ).to_int( ) - 1;	// tok 5

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {
			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );
			tci.mask = TCIF_IMAGE;
			tci.iImage = nIcon;

			TabCtrl_SetItem( this->m_Hwnd, nItem, &tci );
		}
	}
	// xdid -m [NAME] [ID] [SWITCH] [X] [Y]
	else if ( flags[TEXT('m')] && numtok > 4 ) {

		const int X = input.getnexttok( ).to_int( );	// tok 4
		const int Y = input.getnexttok( ).to_int( );	// tok 5

		TabCtrl_SetItemSize( this->m_Hwnd, X, Y );
	}
	// This it to avoid an invalid flag message.
	// xdid -r [NAME] [ID] [SWITCH]
	else if ( flags[TEXT('r')] ) {
	}
	// xdid -t [NAME] [ID] [SWITCH] [N] (text)
	else if ( flags[TEXT('t')] && numtok > 3 ) {

		const int nItem = input.getnexttok( ).to_int( ) - 1;	// tok 4

		if ( nItem > -1 && nItem < TabCtrl_GetItemCount( this->m_Hwnd ) ) {

			TString itemtext;

			TCITEM tci;
			ZeroMemory( &tci, sizeof( TCITEM ) );
			tci.mask = TCIF_TEXT;

			if ( numtok > 4 )
				itemtext = input.gettok( 5, -1 ).trim();

			tci.pszText = itemtext.to_chr( );

			TabCtrl_SetItem( this->m_Hwnd, nItem, &tci );
		}
	}

	// xdid -v [DNAME] [ID] [SWITCH] [N] [POS]
	else if (flags[TEXT('v')] && numtok > 4) {
		int nItem = input.getnexttok( ).to_int();		// tok 4
		const int pos = input.getnexttok( ).to_int();	// tok 5
		BOOL adjustDelete = FALSE;

		if (nItem == pos)
			return;
		else if ((nItem < 1) || (nItem > TabCtrl_GetItemCount(this->m_Hwnd)))
			return;
		else if ((pos < 1) || (pos > TabCtrl_GetItemCount(this->m_Hwnd)))
			return;

		// does the nItem index get shifted after we insert
		if (nItem > pos)
			adjustDelete = TRUE;

		// decrement coz of 0-index
		nItem--;

		// get the item we're moving
		TCHAR* text = new TCHAR[MIRC_BUFFER_SIZE_CCH];
		TCITEM tci;
		ZeroMemory(&tci, sizeof(TCITEM));

		tci.pszText = text;
		tci.cchTextMax = MIRC_BUFFER_SIZE_CCH;
		tci.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_TEXT | TCIF_STATE;

		TabCtrl_GetItem(this->m_Hwnd, nItem, &tci);

		// insert it into the new position
		TabCtrl_InsertItem(this->m_Hwnd, pos, &tci);

		// remove the old tab item
		TabCtrl_DeleteItem(this->m_Hwnd, (adjustDelete ? nItem +1 : nItem));
		delete [] text; // delete text buffer allocated above.
	}

	// xdid -w [NAME] [ID] [SWITCH] [FLAGS] [INDEX] [FILENAME]
	else if (flags[TEXT('w')] && numtok > 5) {
		const TString flag(input.getnexttok( ));		// tok 4
		const int index = input.getnexttok( ).to_int();	// tok 5
		TString filename(input.gettok(6, -1));

		HIMAGELIST himl = this->getImageList();

		if (himl == NULL) {
			himl = this->createImageList();

			if (himl != NULL)
				this->setImageList(himl);
		}
		if (himl == NULL) {
			this->showError(NULL,TEXT("-w"),TEXT("Unable to get Image List"));
			return;
		}

		HICON icon = dcxLoadIcon(index, filename, false, flag);

		if (icon != NULL) {
			ImageList_AddIcon(himl, icon);
			DestroyIcon(icon);
		}
	}
	// xdid -y [NAME] [ID] [SWITCH] [+FLAGS]
	else if ( flags[TEXT('y')] ) {

		ImageList_Destroy( this->getImageList( ) );
	}
	else
		this->parseGlobalCommandRequest( input, flags );
}

/*!
 * \brief blah
 *
 * blah
 */

HIMAGELIST DcxTab::getImageList(  ) {

  return TabCtrl_GetImageList( this->m_Hwnd );
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxTab::setImageList( HIMAGELIST himl ) {

  TabCtrl_SetImageList( this->m_Hwnd, himl );
}

/*!
 * \brief blah
 *
 * blah
 */

HIMAGELIST DcxTab::createImageList( ) {

  return ImageList_Create( 16, 16, ILC_COLOR32|ILC_MASK, 1, 0 );
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxTab::deleteLParamInfo( const int nItem ) {

  TCITEM tci;
  ZeroMemory( &tci, sizeof( TCITEM ) );

  tci.mask = TCIF_PARAM;

  if ( TabCtrl_GetItem( this->m_Hwnd, nItem, &tci ) ) {

    LPDCXTCITEM lpdtci = (LPDCXTCITEM) tci.lParam;

    if ( lpdtci != NULL )
      delete lpdtci;
  }
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxTab::activateSelectedTab( ) {

	int nTab = TabCtrl_GetItemCount( this->m_Hwnd );
	const int nSel = TabCtrl_GetCurSel( this->m_Hwnd );

	if ( nTab > 0 ) {

		RECT tabrect, rc;
		GetWindowRect( this->m_Hwnd, &tabrect );
		TabCtrl_AdjustRect( this->m_Hwnd, FALSE, &tabrect );
		GetWindowRect( this->m_Hwnd, &rc );

		OffsetRect( &tabrect, -rc.left, -rc.top );

		/*
		TCHAR data[500];
		wnsprintf( data, 500, TEXT("WRECT %d %d %d %d - ARECT %d %d %d %d"), 
		rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
		tabrect.left, tabrect.top, tabrect.right-tabrect.left, tabrect.bottom-tabrect.top );

		mIRCError( data );
		*/

		TCITEM tci;
		ZeroMemory( &tci, sizeof( TCITEM ) );
		tci.mask = TCIF_PARAM;

		HDWP hdwp = BeginDeferWindowPos( 0 );
		while ( nTab-- > 0 ) {

			TabCtrl_GetItem( this->m_Hwnd, nTab, &tci );
			LPDCXTCITEM lpdtci = (LPDCXTCITEM) tci.lParam;

			if ( lpdtci->mChildHwnd != NULL && IsWindow( lpdtci->mChildHwnd ) ) {

				if ( nTab == nSel ) {

					hdwp = DeferWindowPos( hdwp, lpdtci->mChildHwnd, NULL, 
						tabrect.left, tabrect.top, tabrect.right-tabrect.left, tabrect.bottom-tabrect.top, 
						SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOOWNERZORDER );
				}
				else {
					hdwp = DeferWindowPos( hdwp, lpdtci->mChildHwnd, NULL, 0, 0, 0, 0, 
						SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOOWNERZORDER );
				}
			}
		}
		EndDeferWindowPos( hdwp );
	}
}

void DcxTab::getTab(const int index, LPTCITEM tcItem) const {
	TabCtrl_GetItem(this->m_Hwnd, index, tcItem);
}

int DcxTab::getTabCount() const {
	return TabCtrl_GetItemCount(this->m_Hwnd);
}

void DcxTab::GetCloseButtonRect(const RECT& rcItem, RECT& rcCloseButton)
{
	// ----------
	//rcCloseButton.top = rcItem.top + 2;
	//rcCloseButton.bottom = rcCloseButton.top + (m_iiCloseButton.rcImage.bottom - m_iiCloseButton.rcImage.top);
	//rcCloseButton.right = rcItem.right - 2;
	//rcCloseButton.left = rcCloseButton.right - (m_iiCloseButton.rcImage.right - m_iiCloseButton.rcImage.left);
	// ----------
	rcCloseButton.top = rcItem.top + 2;
	rcCloseButton.bottom = rcCloseButton.top + (16);
	rcCloseButton.right = rcItem.right - 2;
	rcCloseButton.left = rcCloseButton.right - (16);
	// ----------
}

TString DcxTab::getStyles(void) const {
	TString styles(__super::getStyles());
	const DWORD Styles = GetWindowStyle(this->m_Hwnd);
	const DWORD ExStyles = GetWindowExStyle(this->m_Hwnd);

	if (Styles & TCS_VERTICAL)
		styles.addtok(TEXT("vertical"));
	if (Styles & TCS_BOTTOM)
		styles.addtok(TEXT("bottom"));
	if (Styles & TCS_RIGHT)
		styles.addtok(TEXT("right"));
	if (Styles & TCS_FIXEDWIDTH)
		styles.addtok(TEXT("fixedwidth"));
	if (Styles & TCS_RIGHT)
		styles.addtok(TEXT("buttons"));
	if (Styles & TCS_BUTTONS)
		styles.addtok(TEXT("flat"));
	if (Styles & TCS_FLATBUTTONS)
		styles.addtok(TEXT("flat"));
	if (Styles & TCS_HOTTRACK)
		styles.addtok(TEXT("hot"));
	if (Styles & TCS_MULTILINE)
		styles.addtok(TEXT("multiline"));
	if (Styles & TCS_RIGHTJUSTIFY)
		styles.addtok(TEXT("rightjustify"));
	if (Styles & TCS_SCROLLOPPOSITE)
		styles.addtok(TEXT("scrollopposite"));
	if (ExStyles & TCS_EX_FLATSEPARATORS)
		styles.addtok(TEXT("flatseps"));
	if (this->m_bClosable)
		styles.addtok(TEXT("closable"));
	if (this->m_bGradient)
		styles.addtok(TEXT("gradient"));
	return styles;
}

void DcxTab::toXml(TiXmlElement * xml) const {
	__super::toXml(xml);
	const int count = this->getTabCount();
	TCHAR buf[MIRC_BUFFER_SIZE_CCH];
	TCITEM tci;
	for (int i = 0; i < count; i++) {
		tci.cchTextMax = MIRC_BUFFER_SIZE_CCH -1;
		tci.pszText = buf;
		tci.mask |= TCIF_TEXT;
		if(TabCtrl_GetItem(this->m_Hwnd, i, &tci)) {
			LPDCXTCITEM lpdtci = (LPDCXTCITEM) tci.lParam;
			DcxControl * ctrl = this->m_pParentDialog->getControlByHWND(lpdtci->mChildHwnd);
			if (ctrl != NULL) {
				TiXmlElement * ctrlxml = ctrl->toXml();
				// we need to remove hidden style here
				TString styles(ctrlxml->Attribute("styles"));
				if (styles.len() > 0) {
					styles.remtok(TEXT("hidden"), 1); 
					if (styles.len() > 0) ctrlxml->SetAttribute("styles", styles.c_str());
					else ctrlxml->RemoveAttribute("styles");
				}
				if (tci.mask & TCIF_TEXT) ctrlxml->SetAttribute("caption", TString(tci.pszText).c_str());
				xml->LinkEndChild(ctrlxml);
			}
		}
	}
}

/*!
 * \brief blah
 *
 * blah
 */
LRESULT DcxTab::ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bParsed)
{
	switch (uMsg) {
		case WM_NOTIFY : 
			{
				LPNMHDR hdr = (LPNMHDR) lParam;

				if (!hdr)
					break;

				switch (hdr->code) {
					case NM_RCLICK:
					{
						if (this->m_pParentDialog->getEventMask() & DCX_EVENT_CLICK) {
							TCHITTESTINFO tchi;

							tchi.flags = TCHT_ONITEM;
							GetCursorPos(&tchi.pt);
							MapWindowPoints(NULL, this->m_Hwnd, &tchi.pt, 1);

							const int tab = TabCtrl_HitTest(this->m_Hwnd, &tchi);
							TabCtrl_GetCurSel(this->m_Hwnd);

							if (tab != -1)
								this->execAliasEx(TEXT("%s,%d,%d"), TEXT("rclick"), this->getUserID(), tab +1);
						}

						bParsed = TRUE;
						break;
					}

					case NM_CLICK:
						{
							if (this->m_pParentDialog->getEventMask() & DCX_EVENT_CLICK) {
								const int tab = TabCtrl_GetCurFocus(this->m_Hwnd);
								//int tab = TabCtrl_GetCurSel(this->m_Hwnd);

								if (tab != -1) {
									if (this->m_bClosable) {
										RECT rcCloseButton, rc;
										POINT pt;
										GetCursorPos(&pt);
										MapWindowPoints(NULL,this->m_Hwnd, &pt, 1);
										TabCtrl_GetItemRect(this->m_Hwnd, tab, &rc);
										GetCloseButtonRect(rc, rcCloseButton);

										if (PtInRect(&rcCloseButton, pt)) {
											this->execAliasEx(TEXT("%s,%d,%d"), TEXT("closetab"), this->getUserID(), tab +1);
											break;
										}
									}
									this->execAliasEx(TEXT("%s,%d,%d"), TEXT("sclick"), this->getUserID(), tab +1);
								}
							}
						}
					// fall through.
					case TCN_SELCHANGE:
						{
							this->activateSelectedTab();
							bParsed = TRUE;
						}
						break;
				}
				break;
			}

		// Original source based on code from eMule 0.47 source code available at http://www.emule-project.net
		case WM_DRAWITEM:
			{
				if (!m_bClosable)
					break;

				DRAWITEMSTRUCT *idata = (DRAWITEMSTRUCT *)lParam;

				if ((idata == NULL) || (!IsWindow(idata->hwndItem)))
					break;

				//DcxControl *c_this = (DcxControl *) GetProp(idata->hwndItem, TEXT("dcx_cthis"));

				//if (c_this == NULL)
				//	break;

				RECT rect;
				const int nTabIndex = idata->itemID;

				if (nTabIndex < 0)
					break;

				CopyRect(&rect, &idata->rcItem);

				// if themes are active use them.
				// call default WndProc(), DrawThemeParentBackgroundUx() is only temporary
				DcxControl::DrawCtrlBackground(idata->hDC, this, &rect);
				//DrawThemeParentBackgroundUx(this->m_Hwnd, idata->hDC, &rect);
				//CopyRect(&rect, &idata->rcItem);
				if (this->m_bGradient) {
					if (this->m_clrBackText == -1)
						// Gives a nice silver/gray gradient
						XPopupMenuItem::DrawGradient(idata->hDC, &rect, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNFACE), TRUE);
					else
						XPopupMenuItem::DrawGradient(idata->hDC, &rect, GetSysColor(COLOR_BTNHIGHLIGHT), this->m_clrBackText, TRUE);
				}
				rect.left += 1+ GetSystemMetrics(SM_CXEDGE); // move in past border.

				// TODO: (twig) Ook can u take a look at this plz? string stuff isnt my forte
				TCHAR szLabel[MIRC_BUFFER_SIZE_CCH];
				TC_ITEM tci;

				tci.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_STATE;
				tci.pszText = szLabel;
				tci.cchTextMax = MIRC_BUFFER_SIZE_CCH;
				tci.dwStateMask = TCIS_HIGHLIGHTED;

				if (!TabCtrl_GetItem(this->getHwnd(), nTabIndex, &tci)) {
					this->showError(NULL, TEXT("DcxTab Fatal Error"), TEXT("Invalid item"));
					break;
				}

				const TString label(szLabel);

				// fill the rect so it appears to TEXT("merge") with the tab page content
				//if (!dcxIsThemeActive())
				//FillRect(idata->hDC, &rect, GetSysColorBrush(COLOR_BTNFACE));

				// set transparent so text background isnt annoying
				const int iOldBkMode = SetBkMode(idata->hDC, TRANSPARENT);

				// Draw icon on left side if the item has an icon
				if (tci.iImage != -1) {
					if (ImageList_DrawEx(this->getImageList(), tci.iImage, idata->hDC, rect.left, rect.top, 0, 0, CLR_NONE, CLR_NONE, ILD_TRANSPARENT))
					{
						IMAGEINFO ii;
						if (ImageList_GetImageInfo(this->getImageList(), tci.iImage, &ii))
							rect.left += (ii.rcImage.right - ii.rcImage.left);
					}
				}
				// Draw TEXT('Close button') at right side
				if (m_bClosable) {
					RECT rcCloseButton;
					GetCloseButtonRect(rect, rcCloseButton);
					// Draw systems close button ? or do you want a custom close button?
					DrawFrameControl(idata->hDC, &rcCloseButton, DFC_CAPTION, DFCS_CAPTIONCLOSE | DFCS_FLAT | DFCS_TRANSPARENT);
					//MoveToEx( idata->hDC, rcCloseButton.left, rcCloseButton.top, NULL );
					//LineTo( idata->hDC, rcCloseButton.right, rcCloseButton.bottom );
					//MoveToEx( idata->hDC, rcCloseButton.right, rcCloseButton.top, NULL );
					//LineTo( idata->hDC, rcCloseButton.left, rcCloseButton.bottom );

					rect.right = rcCloseButton.left - 2;
				}
				COLORREF crOldColor;

				if (tci.dwState & TCIS_HIGHLIGHTED)
					crOldColor = SetTextColor(idata->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));

				rect.top += 1+ GetSystemMetrics(SM_CYEDGE); //4;
				//DrawText(idata->hDC, label.to_chr(), label.len(), &rect, DT_SINGLELINE | DT_TOP | DT_NOPREFIX);
				// allow mirc formatted text.
				//mIRC_DrawText(idata->hDC, label, &rect, DT_SINGLELINE | DT_TOP | DT_NOPREFIX, false, this->m_bUseUTF8);
				//if (!this->m_bCtrlCodeText) {
				//	if (this->m_bShadowText)
				//		dcxDrawShadowText(idata->hDC, label.to_wchr(this->m_bUseUTF8), label.wlen(),&rect, DT_WORD_ELLIPSIS | DT_LEFT | DT_TOP | DT_SINGLELINE, GetTextColor(idata->hDC), 0, 5, 5);
				//	else
				//		DrawTextW( idata->hDC, label.to_wchr(this->m_bUseUTF8), label.wlen( ), &rect, DT_WORD_ELLIPSIS | DT_LEFT | DT_TOP | DT_SINGLELINE );
				//}
				//else
				//	mIRC_DrawText( idata->hDC, label, &rect, DT_WORD_ELLIPSIS | DT_LEFT | DT_TOP | DT_SINGLELINE, this->m_bShadowText, this->m_bUseUTF8);
				this->ctrlDrawText(idata->hDC, label, &rect, DT_WORD_ELLIPSIS | DT_LEFT | DT_TOP | DT_SINGLELINE);

				if (tci.dwState & TCIS_HIGHLIGHTED)
					SetTextColor(idata->hDC, crOldColor);

				SetBkMode(idata->hDC, iOldBkMode);
				break;
			}
	}

	return 0L;
}

LRESULT DcxTab::PostMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed )
{
	LRESULT lRes = 0L;
	switch( uMsg ) {

		case WM_CONTEXTMENU:
		case WM_LBUTTONUP:
			break;

		case WM_NOTIFY : 
			{
				LPNMHDR hdr = (LPNMHDR) lParam;

				if (!hdr)
					break;

				//if (hdr->hwndFrom == this->m_ToolTipHWND) {
				//	switch(hdr->code) {
				//	case TTN_GETDISPINFO:
				//		{
				//			LPNMTTDISPINFO di = (LPNMTTDISPINFO)lParam;
				//			di->lpszText = this->m_tsToolTip.to_chr();
				//			di->hinst = NULL;
				//			bParsed = TRUE;
				//		}
				//		break;
				//	case TTN_LINKCLICK:
				//		{
				//			bParsed = TRUE;
				//			this->execAliasEx(TEXT("%s,%d"), TEXT("tooltiplink"), this->getUserID());
				//		}
				//		break;
				//	default:
				//		break;
				//	}
				//}
				if (IsWindow(hdr->hwndFrom)) {
					DcxControl *c_this = (DcxControl *) GetProp(hdr->hwndFrom,TEXT("dcx_cthis"));
					if (c_this != NULL)
						lRes = c_this->ParentMessage(uMsg, wParam, lParam, bParsed);
				}
			}
			break;

		case WM_HSCROLL: 
		case WM_VSCROLL: 
		case WM_COMMAND:
			{
				if (IsWindow((HWND) lParam)) {
					DcxControl *c_this = (DcxControl *) GetProp((HWND) lParam,TEXT("dcx_cthis"));
					if (c_this != NULL)
						lRes = c_this->ParentMessage(uMsg, wParam, lParam, bParsed);
				}
			}
			break;

		case WM_DELETEITEM:
			{
				DELETEITEMSTRUCT *idata = (DELETEITEMSTRUCT *)lParam;
				if ((idata != NULL) && (IsWindow(idata->hwndItem))) {
					DcxControl *c_this = (DcxControl *) GetProp(idata->hwndItem,TEXT("dcx_cthis"));
					if (c_this != NULL)
						lRes = c_this->ParentMessage(uMsg, wParam, lParam, bParsed);
				}
			}
			break;

		case WM_MEASUREITEM:
			{
				HWND cHwnd = GetDlgItem(this->m_Hwnd, wParam);
				if (IsWindow(cHwnd)) {
					DcxControl *c_this = (DcxControl *) GetProp(cHwnd,TEXT("dcx_cthis"));
					if (c_this != NULL)
						lRes = c_this->ParentMessage(uMsg, wParam, lParam, bParsed);
				}
			}
			break;

		case WM_SIZE:
			{
				this->activateSelectedTab( );
				if (this->m_pParentDialog->getEventMask() & DCX_EVENT_SIZE)
					this->execAliasEx(TEXT("%s,%d"), TEXT("sizing"), this->getUserID( ) );
			}
			break;

		case WM_ERASEBKGND:
			{
				if (this->isExStyle(WS_EX_TRANSPARENT))
					this->DrawParentsBackground((HDC)wParam);
				else
					DcxControl::DrawCtrlBackground((HDC) wParam,this);
				bParsed = TRUE;
				return TRUE;
			}
			break;

		case WM_PAINT:
			{
				if (!this->m_bAlphaBlend)
					break;
				PAINTSTRUCT ps;
				HDC hdc;

				hdc = BeginPaint( this->m_Hwnd, &ps );

				bParsed = TRUE;

				// Setup alpha blend if any.
				LPALPHAINFO ai = this->SetupAlphaBlend(&hdc);

				lRes = CallWindowProc( this->m_DefaultWindowProc, this->m_Hwnd, uMsg, (WPARAM) hdc, lParam );

				this->FinishAlphaBlend(ai);

				EndPaint( this->m_Hwnd, &ps );
			}
			break;

			//case WM_CLOSE:
			//	{
			//		if (GetKeyState(VK_ESCAPE) != 0) // don't allow the window to close if escape is pressed. Needs looking into for a better method.
			//			bParsed = TRUE;
			//	}
			//	break;
		case WM_DESTROY:
			{
				delete this;
				bParsed = TRUE;
			}
			break;

		default:
			lRes = this->CommonMessage( uMsg, wParam, lParam, bParsed);
			break;
	}

	return lRes;
}
