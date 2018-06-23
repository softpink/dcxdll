/*!
 * \file dcxdivider.h
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

#ifndef _DCXDIVIDER_H_
#define _DCXDIVIDER_H_

#include "defines.h"
#include "Classes/dcxcontrol.h"
#include "Classes/custom/divider.h"

class DcxDialog;

/*!
 * \brief blah
 *
 * blah
 */

class DcxDivider
	: public DcxControl
{
public:
	DcxDivider() = delete;
	DcxDivider(const DcxDivider &) = delete;
	DcxDivider &operator =(const DcxDivider &) = delete;	// No assignments!
	DcxDivider(DcxDivider &&) = delete;
	DcxDivider &operator =(DcxDivider &&) = delete;

	DcxDivider(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles );
	~DcxDivider( );

	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) final;
	LRESULT ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) noexcept final;

	//void parseInfoRequest(const TString & input, PTCHAR szReturnValue) const final;
	void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const final;
	void parseCommandRequest(const TString & input) final;
	//void parseControlStyles(const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) final;
	std::tuple<NoTheme, WindowStyle, WindowExStyle> parseControlStyles(const TString & tsStyles) final;

	LRESULT setPane( const UINT iPaneId, const LPDVPANEINFO lpdvpi ) noexcept;
	LRESULT setDivPos( const UINT iDivPos ) noexcept;

	inline const TString getType() const final { return TEXT("divider"); };
	inline const DcxControlTypes getControlType() const noexcept final { return DcxControlTypes::DIVIDER; }

	const TString getStyles(void) const final;
	void toXml(TiXmlElement *const xml) const final;
	TiXmlElement * toXml(void) const final;

	static WNDPROC m_hDefaultClassProc;	//!< Default window procedure
	LRESULT CallDefaultClassProc(const UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept final;

protected:
	HWND m_leftPane{ nullptr };
	HWND m_rightPane{ nullptr };
};

#endif // _DCXDIVIDER_H_
