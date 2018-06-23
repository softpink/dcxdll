/*!
 * \file dcximage.cpp
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
#include "Classes/mirc/dcximage.h"
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

DcxImage::DcxImage(const UINT ID, DcxDialog *const p_Dialog, const HWND mParentHwnd, const RECT *const rc, const TString & styles)
	: DcxControl(ID, p_Dialog)
{
	const auto[bNoTheme, Styles, ExStyles] = parseControlStyles(styles);

	m_Hwnd = dcxCreateWindow(
		ExStyles,
		DCX_IMAGECLASS,
		Styles | WindowStyle::Child,
		rc,
		mParentHwnd,
		ID,
		this);

	if (!IsWindow(m_Hwnd))
		throw Dcx::dcxException("Unable To Create Window");

	if (bNoTheme)
		Dcx::UXModule.dcxSetWindowTheme(m_Hwnd, L" ", L" ");

	if (p_Dialog->getToolTip() != nullptr)
	{
		if (styles.istok(TEXT("tooltips")))
		{
			this->m_ToolTipHWND = p_Dialog->getToolTip();
			if (!IsWindow(this->m_ToolTipHWND))
				throw Dcx::dcxException("Unable to get ToolTips window");

			AddToolTipToolInfo(this->m_ToolTipHWND, m_Hwnd);
		}
	}

#ifdef DCX_USE_GDIPLUS
	if (p_Dialog->isExStyle(WindowExStyle::Composited))
		this->m_bBuffer = true;
#endif

}

/*!
 * \brief blah
 *
 * blah
 */

DcxImage::~DcxImage()
{
	PreloadData();
}

/*!
 * \brief blah
 *
 * blah
 */

//void DcxImage::parseControlStyles(const TString &styles, LONG *Styles, LONG *ExStyles, BOOL *bNoTheme)
//{
//	*Styles |= SS_NOTIFY;
//
//	this->parseGeneralControlStyles(styles, Styles, ExStyles, bNoTheme);
//}

std::tuple<NoTheme, WindowStyle, WindowExStyle> DcxImage::parseControlStyles(const TString & tsStyles)
{
	WindowStyle Styles(WindowStyle::None);
	WindowExStyle ExStyles(WindowExStyle::None);

	Styles |= SS_NOTIFY;

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

void DcxImage::parseInfoRequest( const TString & input, const refString<TCHAR, MIRC_BUFFER_SIZE_CCH> &szReturnValue) const
{
	// [NAME] [ID] [PROP]
	if (const auto prop(input.getfirsttok(3)); prop == TEXT("fname"))
		szReturnValue = m_tsFilename.to_chr();
	else
		parseGlobalInfoRequest(input, szReturnValue);
}

// clears existing image and icon data and sets pointers to null
void DcxImage::PreloadData() noexcept
{
	if (this->m_hBitmap != nullptr)
	{
		DeleteBitmap(this->m_hBitmap);
		this->m_hBitmap = nullptr;
	}

	if (this->m_hIcon != nullptr)
	{
		DestroyIcon(this->m_hIcon);
		this->m_hIcon = nullptr;
	}

#ifdef DCX_USE_GDIPLUS
	this->m_pImage.reset(nullptr);
#endif
	this->m_tsFilename.clear();	// = TEXT("");
}

#ifdef DCX_USE_GDIPLUS
bool DcxImage::LoadGDIPlusImage(const TString &flags, TString &filename)
{
	if (!IsFile(filename))
		throw Dcx::dcxException(TEXT("Unable to Access File: %"), filename);

	this->m_pImage = std::make_unique<Gdiplus::Image>(filename.to_chr(),TRUE);

	// for some reason this returns `OutOfMemory` when the file doesnt exist instead of `FileNotFound`
	if (const auto status = this->m_pImage->GetLastStatus(); status != Gdiplus::Status::Ok)
	{
		PreloadData();
		throw Dcx::dcxException(TEXT("Failed to load image: %"),GetLastStatusStr(status));
	}

	const XSwitchFlags xflags(flags);

	if (xflags[TEXT('h')])
	{ // High Quality
		this->m_CQuality = Gdiplus::CompositingQualityHighQuality;
		this->m_IMode = Gdiplus::InterpolationModeHighQualityBicubic;
	}
	else
	{
		this->m_CQuality = Gdiplus::CompositingQualityDefault;
		this->m_IMode = Gdiplus::InterpolationModeDefault;
	}

	if (xflags[TEXT('b')]) // Blend Image
		this->m_CMode = Gdiplus::CompositingModeSourceOver;
	else
		this->m_CMode = Gdiplus::CompositingModeSourceCopy;

	if (xflags[TEXT('a')]) // Anti-Aliased
		this->m_SMode = Gdiplus::SmoothingModeAntiAlias;
	else
		this->m_SMode = Gdiplus::SmoothingModeDefault;

	this->m_bTileImage = xflags[TEXT('t')]; // Tile

	return true;
}
#endif

/*!
 * \brief blah
 *
 * blah
 */

void DcxImage::parseCommandRequest( const TString & input)
{
	const XSwitchFlags flags(input.getfirsttok(3));
	const auto numtok = input.numtok();

	// xdid -w [NAME] [ID] [SWITCH] [+FLAGS] [INDEX] [SIZE] [FILENAME]
	if (flags[TEXT('w')] && numtok > 6)
	{
		const auto flag(input.getnexttok());		// tok 4
		const auto index = input.getnexttok().to_int();	// tok 5
		const auto size = input.getnexttok().to_int();	// tok 6
		auto filename(input.getlasttoks().trim());	// tok 7, -1

		PreloadData();

		this->m_iIconSize = NumToIconSize(size);

		this->m_hIcon = dcxLoadIcon(index, filename, (m_iIconSize != DcxIconSizes::SmallIcon), flag);

		this->m_tsFilename = filename;

		this->m_bIsIcon = true;

		// resize window to size of icon
		RECT wnd{};

		if (!GetWindowRectParent(m_Hwnd, &wnd))
			throw Dcx::dcxException("Unable to get windows rect");

		MoveWindow(m_Hwnd, wnd.left, wnd.top, size, size, TRUE);
		this->redrawWindow();
	}
	//xdid -i [NAME] [ID] [SWITCH] [+FLAGS] [IMAGE]
	else if (flags[TEXT('i')] && numtok > 4)
	{
		const auto flag(input.getnexttok().trim());	// tok 4
		auto filename(input.getlasttoks().trim());	// tok 5, -1

		PreloadData();

		if (flag[0] != TEXT('+'))
			throw Dcx::dcxException("Invalid Flags");

#ifdef DCX_USE_GDIPLUS
		// using this method allows you to render BMP, ICON, GIF, JPEG, Exif, PNG, TIFF, WMF, and EMF (no animation)
		//if (Dcx::GDIModule.isUseable() && flag.find(TEXT('g'),0)) { // makes GDI+ the default method, bitmap is only used when GDI+ isn't supported.
		if (Dcx::GDIModule.isUseable())
		{
			if (!LoadGDIPlusImage(flag,filename))
				throw Dcx::dcxException("Unable to load Image with GDI+"); // <- should never throw this
		}
		else
			this->m_hBitmap = dcxLoadBitmap(this->m_hBitmap, filename);

		if ((this->m_hBitmap == nullptr) && (this->m_pImage == nullptr))
			throw Dcx::dcxException("Failed to load image");

		this->m_tsFilename = filename;
#else
		this->m_hBitmap = dcxLoadBitmap(this->m_hBitmap, filename);
		if (this->m_hBitmap != nullptr)
			this->m_tsFilename = filename;
#endif
		this->m_bIsIcon = false;
		//InvalidateParentRect(m_Hwnd);
		InvalidateRect(m_Hwnd, nullptr, TRUE);
	}
	// xdid -k [NAME] [ID] [SWITCH] [COLOR]
	else if (flags[TEXT('k')] && numtok > 3)
	{
		this->m_clrTransColor = input.getnexttok( ).to_<COLORREF>();	// tok 4
		this->redrawWindow();
	}
	// xdid -o [NAME] [ID] [SWITCH] [XOFFSET] [YOFFSET]
	else if (flags[TEXT('o')] && numtok > 4)
	{
		this->m_iXOffset = input++.to_int();	// tok 4
		this->m_iYOffset = input++.to_int();	// tok 5

		this->redrawWindow();
	}
	// xdid -S [NAME] [ID] [SWITCH] [1|0]
	else if (flags[TEXT('S')] && numtok > 3)
	{
		m_bResizeImage = (input.getnexttok().to_int() > 0);	// tok 4

		InvalidateRect(m_Hwnd, nullptr, TRUE);
		//UpdateWindow(m_Hwnd);
		//this->redrawWindow();
	}
	else
		this->parseGlobalCommandRequest(input, flags);
}

#ifdef DCX_USE_GDIPLUS
void DcxImage::DrawGDIImage(HDC hdc, const int x, const int y, const int w, const int h)
{
	Gdiplus::Graphics grphx( hdc );

	grphx.SetCompositingQuality(this->m_CQuality);
	grphx.SetCompositingMode(this->m_CMode);
	grphx.SetSmoothingMode(this->m_SMode);

	if (((this->m_pImage->GetWidth() == 1) || (this->m_pImage->GetHeight() == 1)) && this->m_bResizeImage)
	{
		// This fixes a GDI+ bug when resizing 1 px images
		// http://www.devnewsgroups.net/group/microsoft.public.dotnet.framework.windowsforms/topic11515.aspx
		grphx.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		grphx.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
	}
	else
		grphx.SetInterpolationMode(this->m_IMode);

	if (this->m_bTileImage)
	{
		Gdiplus::ImageAttributes imAtt;
		imAtt.SetWrapMode(Gdiplus::WrapModeTile);

		grphx.DrawImage(this->m_pImage.get(),
			Gdiplus::Rect(x + this->m_iXOffset, y + this->m_iYOffset, w, h),  // dest rect
			0, 0, w, h,       // source rect
			Gdiplus::UnitPixel,
			&imAtt);
	}
	else if (this->m_bResizeImage)
		grphx.DrawImage( this->m_pImage.get(), this->m_iXOffset, this->m_iYOffset, w, h );
	else
		grphx.DrawImage( this->m_pImage.get(), this->m_iXOffset, this->m_iYOffset);
}
#endif

void DcxImage::DrawBMPImage(HDC hdc, const int x, const int y, const int w, const int h)
{
#if DCX_USE_WRAPPERS
	Dcx::dcxHDCBitmapResource hdcbmp(hdc, m_hBitmap);

	if (BITMAP bmp{}; GetObject(m_hBitmap, sizeof(BITMAP), &bmp) != 0)
	{
		if (m_clrTransColor != CLR_INVALID)
			TransparentBlt(hdc, x, y, w, h, hdcbmp.get(), 0, 0, bmp.bmWidth, bmp.bmHeight, m_clrTransColor);
		else
			StretchBlt(hdc, x, y, w, h, hdcbmp.get(), 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}
#else
	auto hdcbmp = CreateCompatibleDC(hdc);

	if (hdcbmp == nullptr)
		return;

	Auto(DeleteDC(hdcbmp));

	if (BITMAP bmp{}; GetObject(m_hBitmap, sizeof(BITMAP), &bmp) != 0)
	{
		auto oldBitmap = SelectBitmap(hdcbmp, m_hBitmap);

		Auto(SelectBitmap(hdcbmp, oldBitmap));

		if (m_clrTransColor != CLR_INVALID)
			TransparentBlt(hdc, x, y, w, h, hdcbmp, 0, 0, bmp.bmWidth, bmp.bmHeight, m_clrTransColor);
		else
			StretchBlt(hdc, x, y, w, h, hdcbmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}
#endif
}

void DcxImage::toXml(TiXmlElement *const xml) const
{
	__super::toXml(xml);

	if (!this->m_tsFilename.empty())
		xml->SetAttribute("src", m_tsFilename.c_str());
	//xml->SetAttribute("styles", getStyles().c_str());
}

TiXmlElement * DcxImage::toXml() const
{
	auto xml = std::make_unique<TiXmlElement>("control");
	toXml(xml.get());
	return xml.release();
}

/*!
 * \brief blah
 *
 * blah
 */
LRESULT DcxImage::ParentMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed ) noexcept
{
	return 0L;
}

LRESULT DcxImage::PostMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bParsed )
{
	switch (uMsg)
	{
	case WM_ERASEBKGND:
	{
		//<#IF USER == 'hkr' COMMENT OUT, UNLESS ITS THE TIME WHEN HE WANTS THIS>
		if (this->isExStyle(WindowExStyle::Transparent))
			this->DrawParentsBackground((HDC)wParam);
		//if (this->isExStyle(WS_EX_TRANSPARENT))
		//	this->DrawParentsBackground((HDC)wParam);
		//else
		//	DcxControl::DrawCtrlBackground((HDC) wParam,this);
		//<END>
		bParsed = TRUE;
		return TRUE;
	}
	break;

	case WM_PRINTCLIENT:
	{
		this->DrawClientArea((HDC)wParam);
		bParsed = TRUE;
	}
	break;

	case WM_PAINT:
	{
		bParsed = TRUE;
		PAINTSTRUCT ps{};
		auto hdc = BeginPaint(m_Hwnd, &ps);
		Auto(EndPaint(m_Hwnd, &ps));

		this->DrawClientArea(hdc);
	}
	break;

	case WM_SIZE:
	{
		InvalidateRect(m_Hwnd, nullptr, TRUE);
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

void DcxImage::DrawClientArea(HDC hdc)
{
	RECT rect{};
	// default paint method
	if (!GetClientRect(m_Hwnd, &rect))
		return;

	const auto w = (rect.right - rect.left), h = (rect.bottom - rect.top), x = rect.left, y = rect.top;

	// Setup alpha blend if any.
	// Double Buffer required for GDI+ to look right in WS_EX_COMPOSITED
	const auto ai = SetupAlphaBlend(&hdc, m_bBuffer);
	Auto(FinishAlphaBlend(ai));

	DcxControl::DrawCtrlBackground(hdc,this,&rect);

	// draw bitmap
#ifdef DCX_USE_GDIPLUS
	if ((m_hBitmap != nullptr) && (!m_bIsIcon) && (m_pImage == nullptr))
	{
#else
	if ((m_hBitmap != nullptr) && (!m_bIsIcon))
	{
#endif
		DrawBMPImage(hdc, x, y, w, h);
	}
	// draw icon
	else if ((m_hIcon != nullptr) && (m_bIsIcon))
		DrawIconEx(hdc, 0, 0, m_hIcon, gsl::narrow_cast<int>(m_iIconSize), gsl::narrow_cast<int>(m_iIconSize), 0, m_hBackBrush, DI_NORMAL | DI_COMPAT);
#ifdef DCX_USE_GDIPLUS
	else if ((m_pImage != nullptr) && (Dcx::GDIModule.isUseable()))
		DrawGDIImage(hdc, x, y, w, h);
#endif
}

WNDPROC DcxImage::m_hDefaultClassProc = nullptr;

LRESULT DcxImage::CallDefaultClassProc(const UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (m_hDefaultClassProc != nullptr)
		return CallWindowProc(m_hDefaultClassProc, this->m_Hwnd, uMsg, wParam, lParam);

	return DefWindowProc(this->m_Hwnd, uMsg, wParam, lParam);
}
