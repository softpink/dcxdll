/*!
 * \file dcxlink.h
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

#ifndef _DCXLINK_H_
#define _DCXLINK_H_

#include "defines.h"
#include "Classes/dcxcontrol.h"

class DcxDialog;

/*!
 * \brief blah
 *
 * blah
 */

class DcxLink
	: public DcxControl
{
public:
	DcxLink() = delete;
	DcxLink(const DcxLink &) = delete;
	DcxLink &operator =(const DcxLink &) = delete;	// No assignments!
	DcxLink(DcxLink &&) = delete;
	DcxLink &operator =(DcxLink &&) = delete;

	DcxLink( const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles );
	~DcxLink( );

	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) final;
	LRESULT ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) noexcept final;

	//void parseInfoRequest(const TString & input, PTCHAR szReturnValue) const final;
	void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const final;
	void parseCommandRequest(const TString & input) final;
	//void parseControlStyles(const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) final;
	std::tuple<NoTheme, WindowStyle, WindowExStyle> parseControlStyles(const TString & tsStyles) final;

	inline const TString getType() const final { return TEXT("link"); };
	inline const DcxControlTypes getControlType() const noexcept final { return DcxControlTypes::LINK; }

	void toXml(TiXmlElement *const xml) const final;
	TiXmlElement * toXml(void) const final;

	LRESULT CallDefaultClassProc(const UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept final;

	static WNDPROC m_hDefaultClassProc;	//!< Default window procedure

private:

	HICON m_hIcon{ nullptr }; //!< Link Icon

	COLORREF m_aColors[4]{ RGB(0, 0, 255),RGB(255, 0, 0),RGB(0, 0, 255),RGB(128, 128, 128) }; //!< Link Colors 0 = normal, 1 = hot, 2 = visited, 3 = disabled

	bool m_bHover{ false }; //!< Hover State
	bool m_bVisited{ false }; //!< Visited State
	bool m_bReserved[2]{ false };
	BOOL m_bTracking{ FALSE }; //!< Mouse Tracking Var

	//static TString UriEncode(const TString &sSrc);
	//static TString UriDecode(const TString &sSrc);
	void DrawClientArea(HDC hdc);
};

#endif // _DCXLINK_H_
