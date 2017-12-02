/*!
 * \file divider.cpp
 * \brief Custom Win32 API Divider Control
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
#include "divider.h"

 /*!
  * \brief blah
  *
  * blah
  */

LRESULT CALLBACK DividerWndProc(HWND mHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
	{
		const auto lpdvdata = new DVCONTROLDATA;

		// this now done in constructors...
		//ZeroMemory(lpdvdata, sizeof(DVCONTROLDATA));
		//
		//lpdvdata->m_bDragging = FALSE;
		//lpdvdata->m_iLineWidth = 2;
		//lpdvdata->m_iBarPos = 100;
		//lpdvdata->m_iOldPos = 0;
		//
		//lpdvdata->m_Panes[0].cbSize = sizeof(DVPANEINFO);
		//lpdvdata->m_Panes[1].cbSize = sizeof(DVPANEINFO);

		SetProp(mHwnd, TEXT("dvc_data"), static_cast<HANDLE>(lpdvdata));

		return 0L;
	}
	break;

	case WM_SIZE:
	{
		Divider_SizeWindowContents(mHwnd, LOWORD(lParam), HIWORD(lParam));
		return 0L;
	}
	break;

	case WM_LBUTTONDOWN:
	{
		Divider_OnLButtonDown(mHwnd, uMsg, wParam, lParam);
		return 0L;
	}
	break;

	case WM_LBUTTONUP:
	{
		Divider_OnLButtonUp(mHwnd, uMsg, wParam, lParam);
		return 0L;
	}
	break;

	case WM_MOUSEMOVE:
	{
		Divider_OnMouseMove(mHwnd, uMsg, wParam, lParam);
		return 0;
	}
	break;

	case WM_SETCURSOR:
	{
		if (LOWORD(lParam) == HTCLIENT && (HWND)wParam == mHwnd)
		{
			HCURSOR hCursor = nullptr;

			if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
				hCursor = LoadCursor(nullptr, IDC_SIZEWE);
			else
				hCursor = LoadCursor(nullptr, IDC_SIZENS);
			if (GetCursor() != hCursor)
				SetCursor(hCursor);
			return TRUE;
		}
	}
	break;

	// wParam == PANE ID, lParam == pointer to a pane item
	case DV_SETPANE:
	{
		if ((wParam != DVF_PANELEFT) && (wParam != DVF_PANERIGHT))
			return FALSE;

		const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

		if (lpdvdata == nullptr)
			return FALSE;

		dcxlParam(LPDVPANEINFO, lpdvpi);

		if (lpdvpi == nullptr)
			return FALSE;

		//// Left/Top Pane
		//if (wParam == DVF_PANELEFT) {
		//
		//	// Invalid structure size
		//	if (lpdvpi->cbSize != lpdvdata->m_Panes[0].cbSize)
		//		return FALSE;
		//
		//	//MessageBox( mHwnd, TEXT("Received DV_SETPANE"), TEXT("DV_SETPANE"), MB_OK );
		//
		//	if (dcx_testflag(lpdvpi->fMask, DVPIM_CHILD)) {
		//		lpdvdata->m_Panes[0].hChild = lpdvpi->hChild;
		//	}
		//	if (lpdvpi->fMask & DVPIM_IDEAL) {
		//		lpdvdata->m_Panes[0].cxIdeal = lpdvpi->cxIdeal;
		//	}
		//	if (lpdvpi->fMask & DVPIM_MIN) {
		//		lpdvdata->m_Panes[0].cxMin = lpdvpi->cxMin;
		//	}
		//	if (lpdvpi->fMask & DVPIM_STYLE) {
		//		lpdvdata->m_Panes[0].fStyle = lpdvpi->fStyle;
		//	}
		//}
		//// Right/Bottom Pane
		//else if (wParam == DVF_PANERIGHT) {
		//
		//	// Invalid structure size
		//	if (lpdvpi->cbSize != lpdvdata->m_Panes[1].cbSize)
		//		return FALSE;
		//
		//	if (lpdvpi->fMask & DVPIM_CHILD) {
		//		lpdvdata->m_Panes[1].hChild = lpdvpi->hChild;
		//	}
		//	if (lpdvpi->fMask & DVPIM_IDEAL) {
		//		lpdvdata->m_Panes[1].cxIdeal = lpdvpi->cxIdeal;
		//	}
		//	if (lpdvpi->fMask & DVPIM_MIN) {
		//		lpdvdata->m_Panes[1].cxMin = lpdvpi->cxMin;
		//	}
		//	if (lpdvpi->fMask & DVPIM_STYLE) {
		//		lpdvdata->m_Panes[1].fStyle = lpdvpi->fStyle;
		//	}
		//}

		// Left/Top Pane by default.
		auto lpdvinfo = &lpdvdata->m_LeftTopPane;
		// Right/Bottom Pane
		if (wParam == DVF_PANERIGHT)
			lpdvinfo = &lpdvdata->m_RightBottomPane;

		// Invalid structure size
		if (lpdvpi->cbSize != lpdvinfo->cbSize)
			return FALSE;

		if (dcx_testflag(lpdvpi->fMask, DVPIM_CHILD))
			lpdvinfo->hChild = lpdvpi->hChild;

		if (dcx_testflag(lpdvpi->fMask, DVPIM_IDEAL))
			lpdvinfo->cxIdeal = lpdvpi->cxIdeal;

		if (dcx_testflag(lpdvpi->fMask, DVPIM_MIN))
			lpdvinfo->cxMin = lpdvpi->cxMin;

		if (dcx_testflag(lpdvpi->fMask, DVPIM_STYLE))
			lpdvinfo->fStyle = lpdvpi->fStyle;

		if (RECT rc{}; GetClientRect(mHwnd, &rc))
			Divider_SizeWindowContents(mHwnd, rc.right - rc.left, rc.bottom - rc.top);

		return TRUE;
	}
	break;

	case DV_GETPANE:
	{
		switch (auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data"))); wParam)
		{
		case DVF_PANELEFT:
			*((LPDVPANEINFO)lParam) = lpdvdata->m_LeftTopPane;
			break;
		case DVF_PANERIGHT:
			*((LPDVPANEINFO)lParam) = lpdvdata->m_RightBottomPane;
			break;
		}

	}
	break;

	// wParam == nullptr, lParam == POSITION VALUE
	case DV_SETDIVPOS:
	{
		const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

		RECT rc{};
		if (!GetClientRect(mHwnd, &rc))
			return FALSE;

		const UINT iPos = static_cast<UINT>(lParam);
		const UINT width = (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT) ? static_cast<UINT>(rc.right - rc.left) : static_cast<UINT>(rc.bottom - rc.top));

		if ((iPos >= lpdvdata->m_LeftTopPane.cxMin) && (iPos <= (width - lpdvdata->m_RightBottomPane.cxMin)))
		{
			lpdvdata->m_iBarPos = iPos;
			Divider_SizeWindowContents(mHwnd, rc.right - rc.left, rc.bottom - rc.top);
			return TRUE;
		}
		return FALSE;
	}
	break;

	// wParam == (OUT) BOOL isVertical?, lParam == (OUT) integer bar_position
	case DV_GETDIVPOS:
	{
		const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

		*((LPINT)lParam) = (lpdvdata->m_bDragging ? lpdvdata->m_iOldPos : static_cast<int>(lpdvdata->m_iBarPos));
		return 0L;
		//break;
	}

	case WM_DESTROY:
	{
		if (const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data"))); lpdvdata != nullptr)
		{
			// Clear Panes and delete child windows
			// Send Notifications to Parent Window About deletion of child windows

			//NMDIVIDER nmdv;
			////ZeroMemory(&nmdv, sizeof(NMDIVIDER));
			//nmdv.hdr.hwndFrom = mHwnd;
			//nmdv.hdr.idFrom = static_cast<UINT>(GetWindowLong(mHwnd, GWL_ID)); //GetWindowID()
			//nmdv.hdr.code = DVN_DELETEDPANE;
			//nmdv.fMask = DVNM_LPARAM | DVNM_PANEID;
			//nmdv.iPaneId = DVF_PANELEFT;
			//nmdv.lParam = lpdvdata->m_LeftTopPane.lParam;

			NMDIVIDER nmdv(mHwnd, DVN_DELETEDPANE, DVNM_LPARAM | DVNM_PANEID, DVF_PANELEFT, 0, lpdvdata->m_LeftTopPane.lParam);

			SendMessage(GetParent(mHwnd), WM_NOTIFY, (WPARAM)nmdv.hdr.idFrom, (LPARAM)&nmdv);

			nmdv.iPaneId = DVF_PANERIGHT;
			nmdv.lParam = lpdvdata->m_RightBottomPane.lParam;
			SendMessage(GetParent(mHwnd), WM_NOTIFY, (WPARAM)nmdv.hdr.idFrom, (LPARAM)&nmdv);

			if (IsWindow(lpdvdata->m_LeftTopPane.hChild))
				DestroyWindow(lpdvdata->m_LeftTopPane.hChild);

			if (IsWindow(lpdvdata->m_RightBottomPane.hChild))
				DestroyWindow(lpdvdata->m_RightBottomPane.hChild);

			delete lpdvdata;
		}

		RemoveProp(mHwnd, TEXT("dvc_data"));

		return 0L;
	}
	break;
	}
	return DefWindowProc(mHwnd, uMsg, wParam, lParam);
}

/*!
 * \brief blah
 *
 * blah
 */

void Divider_SizeWindowContents(HWND mHwnd, const int nWidth, const int nHeight)
{
	if (const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data"))); dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
	{
		MoveWindow(lpdvdata->m_LeftTopPane.hChild, 0, 0, static_cast<int>(lpdvdata->m_iBarPos), nHeight, TRUE);
		MoveWindow(lpdvdata->m_RightBottomPane.hChild, static_cast<int>(lpdvdata->m_iBarPos + lpdvdata->m_iLineWidth), 0,
			static_cast<int>(nWidth - lpdvdata->m_iBarPos - lpdvdata->m_iLineWidth), nHeight, TRUE);
	}
	else {
		MoveWindow(lpdvdata->m_LeftTopPane.hChild, 0, 0, nWidth, static_cast<int>(lpdvdata->m_iBarPos), TRUE);
		MoveWindow(lpdvdata->m_RightBottomPane.hChild, 0, static_cast<int>(lpdvdata->m_iBarPos + lpdvdata->m_iLineWidth),
			nWidth, static_cast<int>(nHeight - lpdvdata->m_iBarPos - lpdvdata->m_iLineWidth), TRUE);
	}
}


/*!
 * \brief blah
 *
 * blah
 */

void DrawXorBar(HDC hdc, const int x1, const int y1, const int width, const int height)
{
	// Ook: Possibly pre-allocate these...
	constexpr static const WORD _dotPatternBmp[8] =
	{
		0x00aa, 0x0055, 0x00aa, 0x0055,
		0x00aa, 0x0055, 0x00aa, 0x0055
	};

	if (const auto hbm = CreateBitmap(8, 8, 1, 1, _dotPatternBmp); hbm != nullptr)
	{
		Auto(DeleteBitmap(hbm));

		if (const auto hbr = CreatePatternBrush(hbm); hbr != nullptr)
		{
			Auto(DeleteBrush(hbr));

			SetBrushOrgEx(hdc, x1, y1, 0);
			const auto hbrushOld = SelectBrush(hdc, hbr);
			Auto(SelectBrush(hdc, hbrushOld));

			PatBlt(hdc, x1, y1, width, height, PATINVERT);
		}
	}
}

void Divider_GetChildControl(HWND mHwnd, const UINT pane, LPDVPANEINFO result)
{
	SendMessage(mHwnd, DV_GETPANE, pane, (LPARAM)result);
}

/*!
 * \brief blah
 *
 * blah
 */

void Divider_CalcBarPos(HWND mHwnd, POINT * pt, RECT * rect)
{
	const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

	if (!GetWindowRect(mHwnd, rect))
		return;

	MapWindowPoints(mHwnd, nullptr, pt, 1);

	pt->x -= rect->left;
	pt->y -= rect->top;

	OffsetRect(rect, -rect->left, -rect->top);

	if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
	{
		if (pt->x < static_cast<int>(lpdvdata->m_LeftTopPane.cxMin))
			pt->x = static_cast<int>(lpdvdata->m_LeftTopPane.cxMin);
		else if (pt->x > rect->right - static_cast<int>(lpdvdata->m_RightBottomPane.cxMin))
			pt->x = rect->right - static_cast<int>(lpdvdata->m_RightBottomPane.cxMin);

		if (pt->x < 3)
			pt->x = 2;
		else if (pt->x > rect->right - 4)
			pt->x = rect->right - 4;

	}
	else {
		if (pt->y < static_cast<int>(lpdvdata->m_LeftTopPane.cxMin))
			pt->y = static_cast<int>(lpdvdata->m_LeftTopPane.cxMin);
		else if (pt->y > rect->bottom - static_cast<int>(lpdvdata->m_RightBottomPane.cxMin))
			pt->y = rect->bottom - static_cast<int>(lpdvdata->m_RightBottomPane.cxMin);

		if (pt->y < 3)
			pt->y = 2;
		else if (pt->y > rect->bottom - 4)
			pt->y = rect->bottom - 4;
	}
}

/*!
 * \brief blah
 *
 * blah
 */

LRESULT Divider_OnLButtonDown(HWND mHwnd, const UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

	//POINT pt;
	//pt.x = (short) LOWORD( lParam );  // horizontal position of cursor 
	//pt.y = (short) HIWORD( lParam );

	POINT pt{ LOWORD(lParam), HIWORD(lParam) };

	RECT rect = { 0 };

	Divider_CalcBarPos(mHwnd, &pt, &rect);

	lpdvdata->m_bDragging = true;

	SetCapture(mHwnd);

	if (auto hdc = GetWindowDC(mHwnd); hdc != nullptr)
	{
		Auto(ReleaseDC(mHwnd, hdc));

		if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
		{
			DrawXorBar(hdc, pt.x - 2, 1, 4, rect.bottom - 2);
			lpdvdata->m_iOldPos = pt.x;
		}
		else {
			DrawXorBar(hdc, 1, pt.y - 2, rect.right - 2, 4);
			lpdvdata->m_iOldPos = pt.y;
		}
	}
	SendMessage(mHwnd, DV_CHANGEPOS, (WPARAM)DVNM_DRAG_START, (LPARAM)&pt);

	return 0L;
}

/*!
 * \brief blah
 *
 * blah
 */

LRESULT Divider_OnLButtonUp(HWND mHwnd, const UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

	//POINT pt;
	//pt.x = (short) LOWORD( lParam );  // horizontal position of cursor 
	//pt.y = (short) HIWORD( lParam );

	POINT pt{ LOWORD(lParam), HIWORD(lParam) };

	if (!lpdvdata->m_bDragging)
		return 0L;

	RECT rect = { 0 };

	Divider_CalcBarPos(mHwnd, &pt, &rect);

	if (auto hdc = GetWindowDC(mHwnd); hdc != nullptr)
	{
		Auto(ReleaseDC(mHwnd, hdc));

		if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
		{
			DrawXorBar(hdc, lpdvdata->m_iOldPos - 2, 1, 4, rect.bottom - 2);
			lpdvdata->m_iOldPos = pt.x;
		}
		else {
			DrawXorBar(hdc, 1, lpdvdata->m_iOldPos - 2, rect.right - 2, 4);
			lpdvdata->m_iOldPos = pt.y;
		}

	}
	lpdvdata->m_bDragging = false;

	//convert the divider position back to screen coords.
	if (GetWindowRect(mHwnd, &rect))
	{
		pt.x += rect.left;
		pt.y += rect.top;

		//now convert into CLIENT coordinates
		MapWindowPoints(nullptr, mHwnd, &pt, 1);
		if (GetClientRect(mHwnd, &rect))
		{
			if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
				lpdvdata->m_iBarPos = static_cast<UINT>(pt.x);
			else
				lpdvdata->m_iBarPos = static_cast<UINT>(pt.y);

			//position the child controls
			Divider_SizeWindowContents(mHwnd, rect.right, rect.bottom);
		}
	}
	ReleaseCapture();
	SendMessage(mHwnd, DV_CHANGEPOS, (WPARAM)DVNM_DRAG_END, (LPARAM)&pt);
	return 0L;
}

/*!
 * \brief blah
 *
 * blah
 */

LRESULT Divider_OnMouseMove(HWND mHwnd, const UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	const auto lpdvdata = static_cast<LPDVCONTROLDATA>(GetProp(mHwnd, TEXT("dvc_data")));

	if (!lpdvdata->m_bDragging)
		return 0L;

	RECT rect = { 0 };

	//POINT pt = { 0 };
	//pt.x = (short)LOWORD(lParam);  // horizontal position of cursor 
	//pt.y = (short)HIWORD(lParam);

	POINT pt{ LOWORD(lParam),HIWORD(lParam) };

	Divider_CalcBarPos(mHwnd, &pt, &rect);

	/*
	if( pt.y != lpdvdata->m_iOldPos && wParam & MK_LBUTTON )
	{
	hdc = GetWindowDC( mHwnd );
	*/

	//if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
	//{
	//
	//	if (pt.x != lpdvdata->m_iOldPos && dcx_testflag(wParam, MK_LBUTTON))
	//	{
	//		HDC hdc = GetWindowDC(mHwnd);
	//
	//		if (hdc != nullptr)
	//		{
	//			Auto(ReleaseDC(mHwnd, hdc));
	//
	//			DrawXorBar(hdc, lpdvdata->m_iOldPos - 2, 1, 4, rect.bottom - 2);
	//			DrawXorBar(hdc, pt.x - 2, 1, 4, rect.bottom - 2);
	//			lpdvdata->m_iOldPos = pt.x;
	//
	//		}
	//	}
	//}
	//else {
	//
	//	if (pt.y != lpdvdata->m_iOldPos && dcx_testflag(wParam, MK_LBUTTON)) {
	//
	//		HDC hdc = GetWindowDC(mHwnd);
	//
	//		if (hdc != nullptr)
	//		{
	//			Auto(ReleaseDC(mHwnd, hdc));
	//
	//			DrawXorBar(hdc, 1, lpdvdata->m_iOldPos - 2, rect.right - 2, 4);
	//			DrawXorBar(hdc, 1, pt.y - 2, rect.right - 2, 4);
	//			lpdvdata->m_iOldPos = pt.y;
	//		}
	//	}
	//}

	if (dcx_testflag(wParam, MK_LBUTTON))
	{
		if (dcx_testflag(GetWindowStyle(mHwnd), DVS_VERT))
		{
			if (pt.x != lpdvdata->m_iOldPos)
			{
				if (auto hdc = GetWindowDC(mHwnd); hdc != nullptr)
				{
					Auto(ReleaseDC(mHwnd, hdc));

					DrawXorBar(hdc, lpdvdata->m_iOldPos - 2, 1, 4, rect.bottom - 2);
					DrawXorBar(hdc, pt.x - 2, 1, 4, rect.bottom - 2);
					lpdvdata->m_iOldPos = pt.x;

				}
			}
		}
		else {

			if (pt.y != lpdvdata->m_iOldPos)
			{
				if (auto hdc = GetWindowDC(mHwnd); hdc != nullptr)
				{
					Auto(ReleaseDC(mHwnd, hdc));

					DrawXorBar(hdc, 1, lpdvdata->m_iOldPos - 2, rect.right - 2, 4);
					DrawXorBar(hdc, 1, pt.y - 2, rect.right - 2, 4);
					lpdvdata->m_iOldPos = pt.y;
				}
			}
		}
	}
	SendMessage(mHwnd, DV_CHANGEPOS, (WPARAM)DVNM_DRAG_DRAG, (LPARAM)&pt);

	return 0L;
}
