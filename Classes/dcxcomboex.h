/*!
 * \file dcxcomboex.h
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

#ifndef _DCXCOMBOEX_H_
#define _DCXCOMBOEX_H_

#include "defines.h"
#include "Classes/dcxcontrol.h"
#include "Classes\custom\ListHelper.h"

class DcxDialog;

//#define CBEXSEARCH_W 0x01 //!< ComboEx WildCard Search
//#define CBEXSEARCH_R 0x02 //!< ComboEx Regex Search

/*!
 * \brief blah
 *
 * blah
 */

struct DCXCOMBOEXEDIT {

  WNDPROC OldProc; //!< Subclassed Window Procedure of Combo
  HWND cHwnd;      //!< Parent ComboEx Handle
  HWND pHwnd;      //!< Dialog Handle

  DCXCOMBOEXEDIT() : OldProc(nullptr), cHwnd(nullptr), pHwnd(nullptr) {}
};
using LPDCXCOMBOEXEDIT = DCXCOMBOEXEDIT *;

struct DCXCBITEM {
	TString tsMark;		// Marked text
};
using LPDCXCBITEM = DCXCBITEM *;

/*!
 * \brief blah
 *
 * blah
 */

class DcxComboEx
	: public DcxControl
	, public DcxListHelper
{
public:
	DcxComboEx() = delete;
	DcxComboEx(const DcxComboEx &) = delete;
	DcxComboEx &operator =(const DcxComboEx &) = delete;	// No assignments!

	DcxComboEx(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles );
	virtual ~DcxComboEx( );

	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) override;
	LRESULT ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) override;

	//void parseInfoRequest(const TString & input, PTCHAR szReturnValue) const override;
	void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const override;
	void parseCommandRequest(const TString & input) override;
	//void parseControlStyles(const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) override;
	std::tuple<NoTheme, WindowStyle, WindowExStyle> parseControlStyles(const TString & tsStyles) override;

	HIMAGELIST getImageList( ) const;
	void setImageList( HIMAGELIST himl );
	//static HIMAGELIST createImageList( );

	bool matchItemText( const int nItem, const TString &search, const DcxSearchTypes &SearchType ) const;

	LRESULT insertItem( const PCOMBOBOXEXITEM lpcCBItem );
	LRESULT getItem( PCOMBOBOXEXITEM lpcCBItem ) const;
	HWND getEditControl( ) const noexcept;
	LRESULT deleteItem( const int iIndex );
	LRESULT setCurSel( const int iIndex );
	LRESULT getCurSel( ) const;
	LRESULT getLBText( const int iIndex, LPSTR lps );
	LRESULT resetContent( );
	LRESULT getCount( ) const;
	LRESULT limitText( const int iLimit );

	//static void getItemRange(const TString &tsItems, const int nItemCnt, int *iStart, int *iEnd);
	//static std::pair<int, int> getItemRange(const TString &tsItems, const int nItemCnt);
	static LRESULT CALLBACK ComboExEditProc( HWND mHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	inline const TString getType() const override { return TEXT("comboex"); };
	inline const DcxControlTypes getControlType() const noexcept override { return DcxControlTypes::COMBOEX; }

	void toXml(TiXmlElement *const xml) const override;
	TiXmlElement * toXml(void) const override;
	const TString getStyles(void) const override;

protected:

	HWND				m_EditHwnd;	//!< Combo's Edit Control Handle
	HWND				m_hComboHwnd;	//!< Combo's handle
	TString				m_tsSelected;
private:
	DCXCOMBOEXEDIT		m_exEdit;
};

#endif // _DCXCOMBOEX_H_
