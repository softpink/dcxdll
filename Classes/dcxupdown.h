/*!
 * \file dcxupdown.h
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

#ifndef _DCXUPDOWN_H_
#define _DCXUPDOWN_H_

#include "defines.h"
#include "Classes/dcxcontrol.h"

class DcxDialog;

/*!
 * \brief blah
 *
 * blah
 */

class DcxUpDown : public DcxControl {

public:
	DcxUpDown() = delete;
	DcxUpDown(const DcxUpDown &) = delete;
	DcxUpDown &operator =(const DcxUpDown &) = delete;	// No assignments!

	DcxUpDown(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles );
	virtual ~DcxUpDown( );

	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) override;
	LRESULT ParentMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed ) override;

	//void parseInfoRequest(const TString & input, PTCHAR szReturnValue) const override;
	void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const override;
	void parseCommandRequest( const TString & input ) override;
	//void parseControlStyles(const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) override;
	std::tuple<NoTheme, WindowStyle, WindowExStyle> parseControlStyles(const TString & tsStyles) override;

	LRESULT setBuddy( const HWND mHwnd );
	LRESULT setRange32( const int iLow, const int iHigh ) noexcept;
	std::pair<int,int> getRange32() const noexcept;
	LRESULT setBase( const int iBase ) noexcept;
	LRESULT getBase( ) const noexcept;
	LRESULT setPos32( const INT nPos ) noexcept;
	LRESULT getPos32( LPBOOL pfError ) const noexcept;

	const TString getStyles(void) const override;

	inline const TString getType() const override { return TEXT("updown"); };
	inline const DcxControlTypes getControlType() const noexcept override { return DcxControlTypes::UPDOWN; }

protected:

};

#endif // _DCXUPDOWN_H_
