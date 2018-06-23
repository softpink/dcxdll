/*!
 * \file dcxcheck.cpp
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
#include "Classes/mirc/dcxcheck.h"
#include "Classes/dcxdialog.h"
#include "Dcx.h"



/*!
 * \brief Constructor
 *
 * \param ID Control ID
 * \param p_Dialog Parent DcxDialog Object
 * \param mParentHwnd Parent Window Handle
 * \param rc Window Rectangle
 * \param styles Window Style Tokenized List
 */

DcxCheck::DcxCheck(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles)
	: DcxControl(ID, p_Dialog)
{
	const auto[bNoTheme, Styles, ExStyles] = parseControlStyles(styles);

	m_Hwnd = dcxCreateWindow(
		ExStyles,
		DCX_CHECKCLASS,
		Styles | WindowStyle::Child,
		rc,
		mParentHwnd,
		ID,
		this);

	if (!IsWindow(m_Hwnd))
		throw Dcx::dcxException("Unable To Create Window");

	if (bNoTheme)
		Dcx::UXModule.dcxSetWindowTheme(m_Hwnd, L" ", L" ");

	this->setNoThemed( (bNoTheme != FALSE) );

	if (styles.istok(TEXT("tooltips")))
	{
		if (!IsWindow(p_Dialog->getToolTip()))
			throw Dcx::dcxException("Unable to Initialize Tooltips");

		setToolTipHWND(p_Dialog->getToolTip());
		AddToolTipToolInfo(getToolTipHWND(), m_Hwnd);
	}

	this->setControlFont(GetStockFont(DEFAULT_GUI_FONT), FALSE);
}

/*!
 * \brief blah
 *
 * blah
 */

DcxCheck::~DcxCheck( )
{
}


void DcxCheck::toXml(TiXmlElement *const xml) const
{
	__super::toXml(xml);

	const TString wtext(TGetWindowText(m_Hwnd));
	xml->SetAttribute("caption", wtext.c_str());
	xml->SetAttribute("styles", getStyles().c_str());
}

TiXmlElement * DcxCheck::toXml(void) const
{
	auto xml = std::make_unique<TiXmlElement>("control");
	toXml(xml.get());
	return xml.release();
}

const TString DcxCheck::getStyles(void) const
{
	auto styles(__super::getStyles());
	const auto Styles = dcxGetWindowStyle(m_Hwnd);

	if (dcx_testflag(Styles, BS_RIGHT))
		styles.addtok(TEXT("rjustify"));
	if (dcx_testflag(Styles, BS_CENTER))
		styles.addtok(TEXT("center"));
	if (dcx_testflag(Styles, BS_LEFT))
		styles.addtok(TEXT("ljustify"));
	if (dcx_testflag(Styles, BS_RIGHTBUTTON))
		styles.addtok(TEXT("right"));
	if (dcx_testflag(Styles, BS_PUSHLIKE))
		styles.addtok(TEXT("pushlike"));
	if (dcx_testflag(Styles, BS_AUTO3STATE))
		styles.addtok(TEXT("3state"));
	return styles;
}

/*!
 * \brief blah
 *
 * blah
 */

//void DcxCheck::parseControlStyles( const TString & styles, LONG * Styles, LONG * ExStyles, BOOL * bNoTheme )
//{
//	*Styles |= BS_AUTOCHECKBOX;
//
//	for (const auto &tsStyle: styles)
//	{
//#if DCX_USE_HASHING
//		switch (std::hash<TString>{}(tsStyle.to_chr()))
//		{
//			case L"rjustify"_hash:
//				*Styles |= BS_RIGHT;
//				break;
//			case L"center"_hash:
//				*Styles |= BS_CENTER;
//				break;
//			case L"ljustify"_hash:
//				*Styles |= BS_LEFT;
//				break;
//			case L"right"_hash:
//				*Styles |= BS_RIGHTBUTTON;
//				break;
//			case L"pushlike"_hash:
//				*Styles |= BS_PUSHLIKE;
//				break;
//			case L"3state"_hash: {
//				*Styles &= ~BS_AUTOCHECKBOX;
//				*Styles |= BS_AUTO3STATE;
//			default:
//				break;
//			}
//		}
//#else
//		if ( tsStyle == TEXT("rjustify") )
//			*Styles |= BS_RIGHT;
//		else if ( tsStyle == TEXT("center") )
//			*Styles |= BS_CENTER;
//		else if ( tsStyle == TEXT("ljustify") )
//			*Styles |= BS_LEFT;
//		else if ( tsStyle == TEXT("right") )
//			*Styles |= BS_RIGHTBUTTON;
//		else if ( tsStyle == TEXT("pushlike") )
//			*Styles |= BS_PUSHLIKE;
//		else if ( tsStyle == TEXT("3state") ) {
//			*Styles &= ~BS_AUTOCHECKBOX;
//			*Styles |= BS_AUTO3STATE;
//		}
//#endif
//	}
//
//	parseGeneralControlStyles( styles, Styles, ExStyles, bNoTheme );
//}

std::tuple<NoTheme, WindowStyle, WindowExStyle> DcxCheck::parseControlStyles(const TString & tsStyles)
{
	WindowStyle Styles(WindowStyle::None);
	WindowExStyle ExStyles(WindowExStyle::None);

	Styles |= BS_AUTOCHECKBOX;

	for (const auto &tsStyle : tsStyles)
	{
		switch (std::hash<TString>{}(tsStyle.to_chr()))
		{
		case L"rjustify"_hash:
			Styles |= BS_RIGHT;
			break;
		case L"center"_hash:
			Styles |= BS_CENTER;
			break;
		case L"ljustify"_hash:
			Styles |= BS_LEFT;
			break;
		case L"right"_hash:
			Styles |= BS_RIGHTBUTTON;
			break;
		case L"pushlike"_hash:
			Styles |= BS_PUSHLIKE;
			break;
		case L"3state"_hash:
		{
			Styles &= gsl::narrow_cast<DWORD>(~BS_AUTOCHECKBOX);
			Styles |= BS_AUTO3STATE;
		}
		default:
			break;
		}
	}

	return parseGeneralControlStyles(tsStyles, Styles, ExStyles);
}

/*!
 * \brief $xdid Parsing Function
 *
 * \param input [NAME] [ID] [PROP] (OPTIONS)
 * \param szReturnValue mIRC Data Container
 *
 * \return > void
 */

void DcxCheck::parseInfoRequest( const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const
{
	switch (std::hash<TString>{}(input.getfirsttok(3)))
	{
		// [NAME] [ID] [PROP]
	case L"text"_hash:
		GetWindowText(m_Hwnd, szReturnValue, MIRC_BUFFER_SIZE_CCH);
		break;
		// [NAME] [ID] [PROP]
	case L"state"_hash:
	{
		TCHAR p = TEXT('0');

		if (const auto iCheck = Button_GetCheck(m_Hwnd); dcx_testflag(iCheck, BST_INDETERMINATE))
			p = TEXT('2');
		else if (dcx_testflag(iCheck, BST_CHECKED))
			p = TEXT('1');

		szReturnValue[0] = p;
		szReturnValue[1] = 0;
	}
	break;
	default:
		parseGlobalInfoRequest(input, szReturnValue);
		break;
	}
}

/*!
 * \brief blah
 *
 * blah
 */

void DcxCheck::parseCommandRequest(const TString & input)
{
	//xdid -c [NAME] [ID] [SWITCH]
	if (const XSwitchFlags flags(input.getfirsttok(3)); flags[TEXT('c')])
	{
		// xdid -cu
		if (flags[TEXT('u')])
			Button_SetCheck(m_Hwnd, BST_INDETERMINATE);
		else
			Button_SetCheck(m_Hwnd, BST_CHECKED);
	}
	//xdid -t [NAME] [ID] [SWITCH] ItemText
	else if (flags[TEXT('t')])
	{
		SetWindowText(m_Hwnd, input.getlasttoks().trim().to_chr());	// tok 4, -1
	}
	//xdid -u [NAME] [ID] [SWITCH]
	else if (flags[TEXT('u')])
	{
		Button_SetCheck(m_Hwnd, BST_UNCHECKED);
	}
	else
		this->parseGlobalCommandRequest(input, flags);
}

/*!
 * \brief blah
 *
 * blah
 */
LRESULT DcxCheck::ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed)
{
	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (HIWORD(wParam))
		{
			// catch this so we can use $xdid(checkbox).state in sclick callback
		case BN_CLICKED:
		{
			if (dcx_testflag(this->getParentDialog()->getEventMask(), DCX_EVENT_CLICK))
			{
				//execAliasEx(TEXT("sclick,%u"), getUserID());

				// /.timer repetitions delay alias dialog event id
				//mIRCLinker::execex(TEXT("/.timer 1 0 %s %s sclick %u"),
				//	this->getParentDialog()->getAliasName().to_chr(),
				//	this->getParentDialog()->getName().to_chr(),
				//	getUserID());
				mIRCLinker::exec(TEXT("/.timer 1 0 % % sclick %"),
					this->getParentDialog()->getAliasName(),
					this->getParentDialog()->getName(),
					getUserID());
			}

			break;
		}
		}

		break;
	}
	}

	return 0L;
}

LRESULT DcxCheck::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed)
{
	switch (uMsg)
	{
	case WM_LBUTTONUP:
	{
		if (dcx_testflag(this->getParentDialog()->getEventMask(), DCX_EVENT_CLICK))
			execAliasEx(TEXT("lbup,%u"), getUserID());
	}
	break;

	case WM_ERASEBKGND:
	{
		bParsed = TRUE;
		return TRUE;
	}
	break;

	case WM_PRINTCLIENT:
	{
		this->DrawClientArea((HDC)wParam, uMsg, lParam);
		bParsed = TRUE;
	}
	break;
	case WM_PAINT:
	{
		bParsed = TRUE;
		if (GetUpdateRect(m_Hwnd, nullptr, FALSE))
		{
			PAINTSTRUCT ps{};
			auto hdc = BeginPaint(m_Hwnd, &ps);
			Auto(EndPaint(m_Hwnd, &ps));

			this->DrawClientArea(hdc, uMsg, lParam);
		}
		else {
			auto hdc = GetDC(m_Hwnd);
			//auto hdc = GetDCEx(m_Hwnd, nullptr, DCX_CLIPCHILDREN|DCX_CLIPSIBLINGS); // <- Ook: needs testing
			Auto(ReleaseDC(m_Hwnd, hdc));

			this->DrawClientArea(hdc, uMsg, lParam);
		}
	}
	break;

	case WM_DESTROY:
	{
		delete this;
		bParsed = TRUE;
	}
	break;

	default:
		return this->CommonMessage(uMsg, wParam, lParam, bParsed);
		break;
	}

	return 0L;
}

void DcxCheck::DrawClientArea(HDC hdc, const UINT uMsg, LPARAM lParam)
{
	// Setup alpha blend if any.
	const auto ai = SetupAlphaBlend(&hdc);
	Auto(FinishAlphaBlend(ai));

	if (!this->IsThemed() || !Dcx::UXModule.dcxIsThemeActive())
	{
		// get controls client area
		if (RECT rcClient{}; GetClientRect(m_Hwnd, &rcClient))
		{
			if (const auto clr = getBackColor(); clr != CLR_INVALID)
				SetBkColor(hdc, clr);

			if (const auto clr = getTextColor(); clr != CLR_INVALID)
				SetTextColor(hdc, clr);

			const auto bWasTransp = this->isExStyle(WindowExStyle::Transparent);

			// fill background.
			if (bWasTransp)
			{
				if (!this->IsAlphaBlend())
					this->DrawParentsBackground(hdc, &rcClient);
			}
			else
				DcxControl::DrawCtrlBackground(hdc, this, &rcClient);

			// This is a workaround to allow our background to be seen under the control.
			if (!bWasTransp)
				this->addExStyle(WindowExStyle::Transparent);

			CallDefaultClassProc(uMsg, (WPARAM)hdc, lParam);

			if (!bWasTransp)
				this->removeExStyle(WindowExStyle::Transparent);
		}
	}
	else
		CallDefaultClassProc(WM_PRINTCLIENT, (WPARAM)hdc, PRF_NONCLIENT | PRF_CLIENT | PRF_CHILDREN);
}

WNDPROC DcxCheck::m_hDefaultClassProc = nullptr;

LRESULT DcxCheck::CallDefaultClassProc(const UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (m_hDefaultClassProc != nullptr)
		return CallWindowProc(m_hDefaultClassProc, this->m_Hwnd, uMsg, wParam, lParam);

	return DefWindowProc(this->m_Hwnd, uMsg, wParam, lParam);
}
