/*!
 * \file dcxipaddress.h
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

#ifndef _DCXIPADDRESS_H_
#define _DCXIPADDRESS_H_

#include "defines.h"
#include "Classes/dcxcontrol.h"

class DcxDialog;

/*!
 * \brief blah
 *
 * blah
 */

class DcxIpAddress
	: public DcxControl
{
public:
	DcxIpAddress() = delete;
	DcxIpAddress(const DcxIpAddress &) = delete;
	DcxIpAddress &operator =(const DcxIpAddress &) = delete;	// No assignments!
	DcxIpAddress(DcxIpAddress &&) = delete;
	DcxIpAddress &operator =(DcxIpAddress &&) = delete;

	DcxIpAddress(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles );
	~DcxIpAddress( );

	LRESULT PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) final;
	LRESULT ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed) final;

	//void parseInfoRequest(const TString & input, PTCHAR szReturnValue) const final;
	void parseInfoRequest(const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const final;
	void parseCommandRequest(const TString & input) final;
	//void parseControlStyles(const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme) final;
	std::tuple<NoTheme, WindowStyle, WindowExStyle> parseControlStyles(const TString & tsStyles) final;

	LRESULT setRange( const int nField, const BYTE iMin, const BYTE iMax ) noexcept;
	LRESULT setFocus( const int nField ) noexcept;
	LRESULT setAddress( const DWORD dwIpAddress ) noexcept;
	LRESULT getAddress( const LPDWORD lpdwIpAddress ) const noexcept;
	LRESULT clearAddress( ) noexcept;

	template <typename T>
	void AddressToString(T *strBuffer, size_t iSize) const noexcept
	{
		DWORD ip;
		this->getAddress(&ip);

		if constexpr(std::is_same_v<char, T>)
		{
			_ts_snprintf(strBuffer, iSize, "%u.%u.%u.%u",
				FIRST_IPADDRESS(ip),
				SECOND_IPADDRESS(ip),
				THIRD_IPADDRESS(ip),
				FOURTH_IPADDRESS(ip));
		}
		else {
			_ts_snprintf(strBuffer, iSize, L"%u.%u.%u.%u",
				FIRST_IPADDRESS(ip),
				SECOND_IPADDRESS(ip),
				THIRD_IPADDRESS(ip),
				FOURTH_IPADDRESS(ip));
		}
	}

	inline const TString getType() const final { return TEXT("ipaddress"); };
	inline const DcxControlTypes getControlType() const noexcept final { return DcxControlTypes::IPADDRESS; }

	void toXml(TiXmlElement *const xml) const final;
	TiXmlElement * toXml(void) const final;

	static WNDPROC m_hDefaultClassProc;	//!< Default window procedure
	LRESULT CallDefaultClassProc(const UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept final;

};

#endif // _DCXIPADDRESS_H_
