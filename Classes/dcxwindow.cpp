/*!
 * \file dcxwindow.cpp
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
#include "dcxwindow.h"

/*!
 * \brief Constructor
 *
 * \param mHwnd Window Handle
 * \param mID Window ID
 */

DcxWindow::DcxWindow( const HWND mHwnd, const UINT mID )
	: m_Hwnd( mHwnd ), m_ID( mID )
{
	if (IDC_map.empty()) {
		IDC_map[TEXT("appstarting")] = IDC_APPSTARTING;
		IDC_map[TEXT("arrow")] = IDC_ARROW;
		IDC_map[TEXT("cross")] = IDC_CROSS;
		IDC_map[TEXT("hand")] = IDC_HAND;
		IDC_map[TEXT("help")] = IDC_HELP;
		IDC_map[TEXT("ibeam")] = IDC_IBEAM;
		IDC_map[TEXT("no")] = IDC_NO;
		IDC_map[TEXT("sizeall")] = IDC_SIZEALL;
		IDC_map[TEXT("sizenesw")] = IDC_SIZENESW;
		IDC_map[TEXT("sizens")] = IDC_SIZENS;
		IDC_map[TEXT("sizenwse")] = IDC_SIZENWSE;
		IDC_map[TEXT("sizewe")] = IDC_SIZEWE;
		IDC_map[TEXT("uparrow")] = IDC_UPARROW;
		IDC_map[TEXT("wait")] = IDC_WAIT;
	}
}

/*!
 * \brief Constructor
 *
 * \param mID Window ID
 */

DcxWindow::DcxWindow( const UINT mID )
: DcxWindow(nullptr, mID)
{
}

/*!
 * \brief Destructor
 *
 * Destructor
 */

DcxWindow::~DcxWindow( ) {


}

/*!
 * \brief blah
 *
 * blah
 */

bool DcxWindow::isStyle( const LONG Styles ) const noexcept {

	//if ( GetWindowStyle( this->m_Hwnd ) & Styles )
	//	return TRUE;
	//
	//return FALSE;
	return dcx_testflag(GetWindowStyle(this->m_Hwnd), (DWORD)Styles);	// this makes sure ALL flags match not just some.
}

/*!
 * \brief blah
 *
 * blah
 */

LONG DcxWindow::removeStyle( const LONG Styles ) {

	auto winStyles = GetWindowStyle(this->m_Hwnd);
	return SetWindowLongPtr( this->m_Hwnd, GWL_STYLE, winStyles &= ~Styles );
}

/*!
 * \brief blah
 *
 * blah
 */

LONG DcxWindow::addStyle( const LONG Styles ) {

	auto winStyles = GetWindowStyle(this->m_Hwnd);
	return SetWindowLongPtr( this->m_Hwnd, GWL_STYLE, winStyles |= Styles );
}

/*!
 * \brief blah
 *
 * blah
 */

LONG DcxWindow::setStyle( const LONG Styles ) {

	return SetWindowLongPtr( this->m_Hwnd, GWL_STYLE, Styles );
}

/*!
 * \brief blah
 *
 * blah
 */

bool DcxWindow::isExStyle( const LONG Styles ) const noexcept {

	//if ( GetWindowExStyle( this->m_Hwnd ) & Styles )
	//	return TRUE;
	//
	//return FALSE;
	return dcx_testflag(GetWindowExStyle(this->m_Hwnd), (DWORD)Styles);
}

/*!
 * \brief blah
 *
 * blah
 */

LONG DcxWindow::removeExStyle( const LONG Styles ) {

	auto winStyles = GetWindowExStyle(this->m_Hwnd);
	return SetWindowLongPtr( this->m_Hwnd, GWL_EXSTYLE, winStyles &= ~Styles );
}

/*!
 * \brief blah
 *
 * blah
 */

LONG DcxWindow::addExStyle( const LONG Styles ) {

	auto winStyles = GetWindowExStyle(this->m_Hwnd);
	return SetWindowLongPtr( this->m_Hwnd, GWL_EXSTYLE, winStyles |= Styles );
}

/*!
 * \brief blah
 *
 * blah
 */

LONG DcxWindow::setExStyle( const LONG Styles ) {

	return SetWindowLongPtr( this->m_Hwnd, GWL_EXSTYLE, Styles );
}

/*!
 * \brief blah
 *
 * blah
 */

const UINT &DcxWindow::getID( ) const noexcept {

	return this->m_ID;
}

/*!
 * \brief blah
 *
 * blah
 */

const HWND &DcxWindow::getHwnd( ) const noexcept {

	return this->m_Hwnd;
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxWindow::redrawWindow( ) {

	RedrawWindow( this->m_Hwnd, nullptr, nullptr, RDW_INTERNALPAINT|RDW_ALLCHILDREN|RDW_INVALIDATE|RDW_ERASE/*|RDW_FRAME|RDW_UPDATENOW*/ );
}

/*
 * Redraw Window using a buffered method to avoid flicker of children etc..
 * Vista Style has issues with this, as it relies on the WM_PAINT, anyway, its not needed for Vista style.
 * Composited windows already have buffered draw.
*/
// TODO: this changes the border style of children upon resize of dialog.
void DcxWindow::redrawBufferedWindow( ) {

	if (this->isExStyle(WS_EX_COMPOSITED)) {
		this->redrawWindow();
		return;
	}

	auto hdc = GetWindowDC(this->m_Hwnd);

	if (hdc == nullptr)
		return;
	Auto(ReleaseDC(this->m_Hwnd, hdc));

	RECT rc;
	if (GetWindowRect(this->m_Hwnd, &rc))
	{
#if DCX_USE_WRAPPERS
		Dcx::dcxHDCBuffer hBuffer(hdc, &rc);

		SendMessage(this->m_Hwnd, WM_PRINT, (WPARAM)(HDC)hBuffer, PRF_NONCLIENT | PRF_CLIENT | PRF_CHILDREN | PRF_CHECKVISIBLE | PRF_ERASEBKGND);

		BitBlt(hdc, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), hBuffer, 0, 0, SRCCOPY);
#else
		auto hBuffer = CreateHDCBuffer(hdc, &rc);

		if (hBuffer != nullptr) {
			Auto(DeleteHDCBuffer(hBuffer));

			SendMessage(this->m_Hwnd, WM_PRINT, (WPARAM)*hBuffer, PRF_NONCLIENT | PRF_CLIENT | PRF_CHILDREN | PRF_CHECKVISIBLE | PRF_ERASEBKGND);

			BitBlt(hdc, 0, 0, (rc.right - rc.left), (rc.bottom - rc.top), *hBuffer, 0, 0, SRCCOPY);
		}
#endif
	}
	ValidateRect(this->m_Hwnd, nullptr);
}

/*!
 * \brief blah
 *
 * blah
 */
std::map<TString, PTCHAR> DcxWindow::IDC_map;

PTCHAR DcxWindow::parseCursorType( const TString & cursor )
{
	auto got = IDC_map.find(cursor);

	if (got != IDC_map.end())
		return got->second;
	return nullptr;
}

/*!
 * \brief blah
 *
 * blah
 */

UINT DcxWindow::parseCursorFlags(const TString &flags) {
	UINT iFlags = 0;
	const XSwitchFlags xflags(flags);

	// no +sign, missing params
	if (!xflags[TEXT('+')])
		return iFlags;

	if (xflags[TEXT('f')])
		iFlags |= DCCS_FROMFILE;
	if (xflags[TEXT('r')])
		iFlags |= DCCS_FROMRESSOURCE;

	return iFlags;
}
