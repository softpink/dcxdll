/*!
 * \file dcxwindow.h
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

#ifndef _DCXWINDOW_H_
#define _DCXWINDOW_H_

#include <map>
#include "Classes/tinyxml/tinyxml.h"

/*!
 * \brief blah
 *
 * blah
 */
#ifdef __INTEL_COMPILER // Defined when using Intel C++ Compiler.
#pragma warning( push )
#pragma warning( disable : 2292 ) //warning #2292: destructor is declared but copy constructor and assignment operator are not
#endif

class DcxWindow {

public:
	DcxWindow() = delete;
	DcxWindow(const DcxWindow &other) = delete;
	DcxWindow &operator =(const DcxWindow &) = delete;	// No assignments!

	DcxWindow( const HWND mHwnd, const UINT mID );
	explicit DcxWindow( const UINT mID );
	virtual ~DcxWindow( );

	bool isStyle( const LONG Styles ) const noexcept;
	LONG removeStyle( const LONG Styles );
	LONG addStyle( const LONG Styles );
	LONG setStyle( const LONG Styles );
	bool isExStyle( const LONG Styles ) const noexcept;
	LONG removeExStyle( const LONG Styles );
	LONG addExStyle( const LONG Styles );
	LONG setExStyle( const LONG Styles );

	const HWND &getHwnd( ) const noexcept;
	const UINT &getID( ) const noexcept;

	void redrawWindow( );
	void redrawBufferedWindow( );
	virtual void toXml(TiXmlElement *const xml) const = 0;

	virtual void parseCommandRequest(const TString & input) = 0;
	//virtual void parseInfoRequest( const TString & input, TCHAR * szReturnValue ) const = 0;
	virtual void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const = 0;
	static PTCHAR parseCursorType(const TString & cursor);
	static UINT parseCursorFlags(const TString & flags);
	static UINT parseCursorArea(const TString &flags);
	static HIMAGELIST createImageList(bool bBigIcons = false);

	LRESULT CallDefaultProc(HWND mHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	WNDPROC m_hDefaultWindowProc; //!< Old Window Procedure
	HWND m_Hwnd;
	UINT m_ID;

	HRGN	m_hZeroRgn;

#if _MSC_VER < 1912
	static const std::map<TString, PTCHAR> IDC_map;
#else
	inline const static std::map<TString, PTCHAR> IDC_map{
		{TEXT("appstarting"), IDC_APPSTARTING},
		{TEXT("arrow"), IDC_ARROW},
		{TEXT("cross"), IDC_CROSS},
		{TEXT("hand"), IDC_HAND},
		{TEXT("help"), IDC_HELP},
		{TEXT("ibeam"), IDC_IBEAM},
		{TEXT("no"), IDC_NO},
		{TEXT("sizeall"), IDC_SIZEALL},
		{TEXT("sizenesw"), IDC_SIZENESW},
		{TEXT("sizens"), IDC_SIZENS},
		{TEXT("sizenwse"), IDC_SIZENWSE},
		{TEXT("sizewe"), IDC_SIZEWE},
		{TEXT("uparrow"), IDC_UPARROW},
		{TEXT("wait"), IDC_WAIT}
	};
#endif
};
#ifdef __INTEL_COMPILER // Defined when using Intel C++ Compiler.
#pragma warning( pop )
#endif

#endif // _DCXWINDOW_H_
