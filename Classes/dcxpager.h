/*
	* Pager Control
*/

#ifndef _DCXPAGER_H_
#define _DCXPAGER_H_

#include "defines.h"
#include "Classes/dcxcontrol.h"
#include "Classes/layout/layoutmanager.h"

class DcxDialog;

/*!
 * \brief blah
 *
 * blah
 */

class DcxPager
	: public DcxControl
{
public:
	DcxPager() = delete;
	DcxPager(const DcxPager &) = delete;
	DcxPager &operator =(const DcxPager &) = delete;	// No assignments!
	DcxPager(DcxPager &&) = delete;
	DcxPager &operator =(DcxPager &&) = delete;

	DcxPager(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles );
	~DcxPager( );

	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) final;
	LRESULT ParentMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed ) noexcept final;

	//void parseInfoRequest(const TString & input, PTCHAR szReturnValue) const final;
	void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const final;
	void parseCommandRequest( const TString & input ) final;
	//void parseControlStyles(const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) final;
	std::tuple<NoTheme, WindowStyle, WindowExStyle> parseControlStyles(const TString & tsStyles) final;

	inline const TString getType() const final { return TEXT("pager"); };
	inline const DcxControlTypes getControlType() const noexcept final { return DcxControlTypes::PAGER; }

	const TString getStyles(void) const final;
	TiXmlElement * toXml(void) const final;
	void toXml(TiXmlElement *const xml) const final;

	static WNDPROC m_hDefaultClassProc;	//!< Default window procedure
	LRESULT CallDefaultClassProc(const UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept final;

	void setChild(const HWND child) noexcept;
	void setBkColor(const COLORREF c) noexcept;
	void setBorderSize(const int bSize) noexcept;
	void setButtonSize(const int bSize) noexcept;
	void reCalcSize(void) const noexcept;

protected:
	HWND m_ChildHWND{ nullptr };
};

#endif // _DCXPAGER_H_
