/*!
* \file dcxrichedit.cpp
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

#include "dcxrichedit.h"
#include "dcxdialog.h"
#include "Custom/RichEditThemed.h"

/*!
* \brief Constructor
*
* \param ID Control ID
* \param p_Dialog Parent DcxDialog Object
* \param mParentHwnd Parent Window Handle
* \param rc Window Rectangle
* \param styles Window Style Tokenized List
*/
DcxRichEdit::DcxRichEdit(UINT ID, DcxDialog *p_Dialog, HWND mParentHwnd, RECT *rc, TString &styles)
: DcxControl(ID, p_Dialog)
, m_iFontSize(10 * 20)
, m_bFontBold(FALSE)
, m_bFontItalic(FALSE)
, m_bFontUnderline(FALSE)
, m_bFontStrikeout(FALSE)
, m_byteCharset(DEFAULT_CHARSET)
//, m_tsFontFaceName("")
//, m_tsText("")
{
	LONG Styles = 0, ExStyles = 0;
	BOOL bNoTheme = FALSE;
	this->parseControlStyles(styles, &Styles, &ExStyles, &bNoTheme);

	this->m_Hwnd = CreateWindowEx(
		ExStyles | WS_EX_CLIENTEDGE,
		DCX_RICHEDITCLASS,
		NULL,
		WS_CHILD | Styles,
		rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
		mParentHwnd,
		(HMENU) ID,
		GetModuleHandle(NULL),
		NULL);

	if (bNoTheme)
		dcxSetWindowTheme(this->m_Hwnd , L" ", L" ");
	else {
		CRichEditThemed::Attach(this->m_Hwnd);
	}

	this->m_clrBackText = GetSysColor(COLOR_WINDOW);
	this->m_clrText = GetSysColor(COLOR_WINDOWTEXT);

	this->loadmIRCPalette();
	this->setContentsFont();

	SendMessage(this->m_Hwnd, EM_SETEVENTMASK, NULL, (LPARAM) (ENM_SELCHANGE | ENM_CHANGE));

	if (p_Dialog->getToolTip() != NULL) {
		if (styles.istok("tooltips")) {
			this->m_ToolTipHWND = p_Dialog->getToolTip();
			AddToolTipToolInfo(this->m_ToolTipHWND, this->m_Hwnd);
		}
	}

	this->registreDefaultWindowProc();
	SetProp(this->m_Hwnd, "dcx_cthis", (HANDLE) this);
}

/*!
* \brief blah
*
* blah
*/
DcxRichEdit::~DcxRichEdit() {
	this->unregistreDefaultWindowProc();
}

/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::parseControlStyles(TString &styles, LONG *Styles, LONG *ExStyles, BOOL *bNoTheme) {
	//*Styles |= ES_READONLY;
	//ES_NOHIDESEL
	unsigned int i = 1, numtok = styles.numtok( );

	while (i <= numtok) {
		if (styles.gettok( i ) == "multi")
			*Styles |= ES_MULTILINE | ES_WANTRETURN;
		else if (styles.gettok( i ) == "readonly")
			*Styles |= ES_READONLY;
		else if (styles.gettok( i ) == "center")
			*Styles |= ES_CENTER;
		else if (styles.gettok( i ) == "right")
			*Styles |= ES_RIGHT;
		else if (styles.gettok( i ) == "autohs")
			*Styles |= ES_AUTOHSCROLL;
		else if (styles.gettok( i ) == "autovs")
			*Styles |= ES_AUTOVSCROLL;
		else if (styles.gettok( i ) == "vsbar")
			*Styles |= WS_VSCROLL;
		else if (styles.gettok( i ) == "hsbar")
			*Styles |= WS_HSCROLL;
		else if (styles.gettok( i ) == "disablescroll")
			*Styles |= ES_DISABLENOSCROLL;
		//else if ( styles.gettok( i ) == "alpha" )
		//	this->m_bAlphaBlend = true;

		i++;
	}

	this->parseGeneralControlStyles(styles, Styles, ExStyles, bNoTheme);
}

/*!
* \brief $xdid Parsing Function
*
* \param input [NAME] [ID] [PROP] (OPTIONS)
* \param szReturnValue mIRC Data Container
*
* \return > void
*/
void DcxRichEdit::parseInfoRequest(TString &input, char *szReturnValue) {
	int numtok = input.numtok( );
	TString prop(input.gettok( 3 ));

	// [NAME] [ID] [PROP] [N]
	if (prop == "text") {
		// determine the line number
		int line = 0;

		if (numtok > 3)
			line = input.gettok( 4 ).to_int() -1;

		// get index of first character in line
		int offset = SendMessage(this->m_Hwnd, EM_LINEINDEX, (WPARAM) line, NULL);
		// get length of the line we want to copy
		int len = SendMessage(this->m_Hwnd, EM_LINELENGTH, (WPARAM) offset, NULL);
		// create and fill the buffer
		char *p = new char[len];
		*(LPWORD) p = (WORD)len;
		int res = SendMessage(this->m_Hwnd, EM_GETLINE, (WPARAM) line, (LPARAM) p);

		// terminate the string at the right position
		p[res] = '\0';

		// copy to result
		lstrcpyn(szReturnValue, p, 900);
		delete [] p;
		return;
	}
	// [NAME] [ID] [PROP]
	else if (prop == "num") {
		if (this->isStyle(ES_MULTILINE))
			wsprintf(szReturnValue, "%d", (int)SendMessage(this->m_Hwnd, EM_GETLINECOUNT, 0, 0L));
			//wsprintf(szReturnValue, "%d", this->m_tsText.numtok("\r\n"));
		else
			lstrcpy(szReturnValue,"1");	// single line control so always 1 line.
		return;
	}
	// [NAME] [ID] [PROP]
	else if (prop == "caretpos") {
		DWORD dwAbsoluteStartSelPos = 0;

		// caret startsel position
		SendMessage(this->m_Hwnd, EM_GETSEL, (WPARAM) &dwAbsoluteStartSelPos, NULL);

		if (this->isStyle(ES_MULTILINE)) {
			int iLinePos = 0;
			int iAbsoluteCharPos = 0;

			// current line
			iLinePos = SendMessage(this->m_Hwnd, EM_LINEFROMCHAR, -1, NULL);
			// line offset
			iAbsoluteCharPos = (int) SendMessage(this->m_Hwnd, EM_LINEINDEX, -1, NULL);

			wsprintf(szReturnValue, "%d %d", iLinePos +1, dwAbsoluteStartSelPos - iAbsoluteCharPos);
		}
		else {
			// return selstart
			wsprintf(szReturnValue, "%d %d", 1, dwAbsoluteStartSelPos);
		}

		return;
	}
	else if (prop == "selstart") {
		CHARRANGE c;

		SendMessage(this->m_Hwnd, EM_EXGETSEL, NULL, (LPARAM) &c);
		wsprintf(szReturnValue, "%d", c.cpMin);
		return;
	}
	else if (prop == "selend") {
		CHARRANGE c;

		SendMessage(this->m_Hwnd, EM_EXGETSEL, NULL, (LPARAM) &c);
		wsprintf(szReturnValue, "%d", c.cpMax);
		return;
	}
	else if (prop == "sel") {
		CHARRANGE c;

		SendMessage(this->m_Hwnd, EM_EXGETSEL, NULL, (LPARAM) &c);
		wsprintf(szReturnValue, "%d %d", c.cpMin, c.cpMax);
		return;
	}
	else if (prop == "seltext") {
		CHARRANGE c;

		SendMessage(this->m_Hwnd, EM_EXGETSEL, NULL, (LPARAM) &c);
		char *buffer = new char[c.cpMax - c.cpMin];

		SendMessage(this->m_Hwnd, EM_GETSELTEXT, NULL, (LPARAM) buffer);
		lstrcpyn(szReturnValue, buffer, 900);
		delete [] buffer;
		return;
	}
	else if (this->parseGlobalInfoRequest(input, szReturnValue))
		return;

	szReturnValue[0] = 0;
}

/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::parseCommandRequest(TString &input) {
	XSwitchFlags flags;
	ZeroMemory((void*) &flags, sizeof(XSwitchFlags));
	this->parseSwitchFlags(input.gettok( 3 ), &flags);

	int numtok = input.numtok( );

	// xdid -r [NAME] [ID] [SWITCH]
	if (flags.switch_flags[17]) {
		this->m_tsText = "";
		this->clearContents();
	}

	// xdid -a [NAME] [ID] [SWITCH] [TEXT]
	if (flags.switch_flags[0] && numtok > 3) {
		this->m_tsText += input.gettok(4, -1);
		this->parseContents(TRUE);
		//DCXSTREAM ds;
		//EDITSTREAM es = {0};
		//es.dwCookie = (DWORD_PTR)&ds;
		//SendMessage(this->m_Hwnd,EM_STREAMIN
	}
	// xdid -c [NAME] [ID] [SWITCH]
	else if (flags.switch_flags[2] && numtok > 2) {
		CopyToClipboard(this->m_Hwnd, this->m_tsText);
	}
	// xdid -d [NAME] [ID] [SWITCH] [N]
	else if (flags.switch_flags[3] && numtok > 3) {
		if (this->isStyle(ES_MULTILINE)) {
			int nLine = input.gettok( 4 ).to_int();
			this->m_tsText.deltok(nLine, "\r\n");
		}

		this->parseContents(TRUE);
	}
	// special richedit interception for font change
	// xdid -f [NAME] [ID] [SWITCH] [+FLAGS] [CHARSET] [SIZE] [FONTNAME]
	else if (flags.switch_flags[5] && numtok > 3) {
		UINT iFontFlags = parseFontFlags(input.gettok( 4 ));

		if (iFontFlags & DCF_DEFAULT) {
			this->m_clrBackText = GetSysColor(COLOR_WINDOW);
			this->m_clrText = GetSysColor(COLOR_WINDOWTEXT);
			this->m_iFontSize = 10 * 20;
			this->m_bFontBold = FALSE;
			this->m_bFontItalic = FALSE;
			this->m_bFontUnderline = FALSE;
			this->m_bFontStrikeout = FALSE;
			this->m_byteCharset = DEFAULT_CHARSET;
			this->setContentsFont();
		}
		else if (numtok > 5) {
			this->setRedraw(FALSE);

			this->m_byteCharset = (BYTE)parseFontCharSet(input.gettok( 5 ));
			this->m_iFontSize = 20 * input.gettok( 6 ).to_int();
			this->m_tsFontFaceName = input.gettok(7, -1);
			this->m_tsFontFaceName.trim();

			//HDC hdc = GetDC( NULL );
			//chrf.yHeight = 20 * -MulDiv( fSize, GetDeviceCaps( hdc, LOGPIXELSY ), 72 );
			//ReleaseDC( NULL, hdc );

			if (iFontFlags & DCF_BOLD)
				this->m_bFontBold = TRUE;
			else
				this->m_bFontBold = FALSE;

			if (iFontFlags & DCF_ITALIC)
				this->m_bFontItalic = TRUE;
			else
				this->m_bFontItalic = FALSE;

			if (iFontFlags & DCF_STRIKEOUT)
				this->m_bFontStrikeout = TRUE;
			else
				this->m_bFontStrikeout = FALSE;

			if (iFontFlags & DCF_UNDERLINE)
				this->m_bFontUnderline = TRUE;
			else
				this->m_bFontUnderline = FALSE;

			//this->setContentsFont();

			this->parseContents(TRUE);
		}
	}
	// xdid -i [NAME] [ID] [SWITCH] [N] [TEXT]
	else if (flags.switch_flags[8] && numtok > 4) {
		if (this->isStyle(ES_MULTILINE)) {
			int nLine = input.gettok( 4 ).to_int();
			this->m_tsText.instok(input.gettok(5, -1).to_chr(), nLine, "\r\n");
		}
		else {
			this->m_tsText = input.gettok(4, -1);
		}

		this->parseContents(TRUE);
	}
	// xdid -k [NAME] [ID] [SWITCH] [COLOR]
	else if (flags.switch_flags[10] && numtok > 3) {
		COLORREF clrColor = (COLORREF)input.gettok( 4 ).to_num();

		if (clrColor == -1)
			SendMessage(this->m_Hwnd, EM_SETBKGNDCOLOR, (WPARAM) 1, (LPARAM) GetSysColor(COLOR_WINDOWTEXT));
		else
			SendMessage(this->m_Hwnd, EM_SETBKGNDCOLOR, (WPARAM) 0, (LPARAM) clrColor);

		this->m_clrBackText = clrColor;
		this->redrawWindow();
	}
	// xdid -l [NAME] [ID] [SWITCH] [N] [COLOR]
	else if (flags.switch_flags[11] && numtok > 4) {
		int nColor = input.gettok( 4 ).to_int() -1;

		if (nColor > -1 && nColor < 16) {
			this->m_aColorPalette[nColor] = (COLORREF)input.gettok( 5 ).to_num();
			this->parseContents(TRUE);
		}
	}
	// xdid -m [NAME] [ID] [SWITCH]
	else if (flags.switch_flags[12]) {
		this->loadmIRCPalette();
		this->parseContents(TRUE);
	}
	// xdid -n [NAME] [ID] [SWITCH] [BOOL]
	else if (flags.switch_flags[13] && numtok > 3) {
		int b = input.gettok( 4 ).to_int();

		this->setAutoUrlDetect(b ? TRUE : FALSE);
	}
	// xdid -o [NAME] [ID] [SWITCH] [N] [TEXT]
	else if (flags.switch_flags[14] && numtok > 4) {
		if (this->isStyle(ES_MULTILINE)) {
			int nLine = input.gettok( 4 ).to_int();
			this->m_tsText.puttok(input.gettok(5, -1).to_chr(), nLine, "\r\n");
		}
		else
			this->m_tsText = input.gettok(4, -1);

		this->parseContents(TRUE);
	}
	// xdid -P [NAME] [ID]
	else if (flags.switch_cap_flags[15] && numtok > 1) {
		SendMessage(this->getHwnd(),WM_PASTE,NULL,NULL);
	}
	// xdid -q [NAME] [ID] [SWITCH] [COLOR1] ... [COLOR16]
	else if (flags.switch_flags[16] && numtok > 3) {
		int i = 0, len = input.gettok(4, -1).numtok( );

		while (i < len && i < 16) {
			this->m_aColorPalette[i] = (COLORREF)input.gettok( 4 + i ).to_num();
			i++;
		}

		this->parseContents(TRUE);
	}
	// xdid -r [NAME] [ID] [SWITCH]
	else if (flags.switch_flags[17]) {
	}
	// xdid -t [NAME] [ID] [SWITCH] [FILENAME]
	else if (flags.switch_flags[19] && numtok > 3) { // TODO: replace all this with an EM_STREAMIN message & callback
		char *contents = readFile(input.gettok(4, -1).to_chr());

		if (contents != NULL) {
			this->m_tsText = contents;
			delete [] contents;
			DWORD mask = this->m_dEventMask;
			this->m_dEventMask = 0;
			this->parseContents(TRUE);
			this->m_dEventMask = mask;
		}
	}
	// xdid -u [NAME] [ID] [SWITCH] [FILENAME]
	else if (flags.switch_flags[20] && numtok > 3) {
		FILE *file = fopen(input.gettok(4, -1).to_chr(), "wb");

		if (file != NULL) {
			fwrite(this->m_tsText.to_chr(), sizeof(char), this->m_tsText.len(), file);
			fflush(file);
			fclose(file);
		}
	}
	// xdid -S [NAME] [ID] [SWITCH] [START] [END]
	else if (flags.switch_cap_flags[18] && numtok > 3) {
		CHARRANGE c;

		c.cpMin = input.gettok( 4 ).to_int();

		if (numtok > 4)
			c.cpMax = input.gettok( 5 ).to_int();
		else
			c.cpMax = c.cpMin;

		SendMessage(this->m_Hwnd, EM_EXSETSEL, NULL, (LPARAM) &c);

		//DWORD dwAbsoluteStartSelPos = 0;
		//// caret startsel position
		//SendMessage(this->m_Hwnd, EM_GETSEL, (WPARAM) &dwAbsoluteStartSelPos, NULL);

		//if (this->isStyle(ES_MULTILINE)) {
		//	int iLinePos = 0;

		//	// current line
		//	iLinePos = SendMessage(this->m_Hwnd, EM_LINEFROMCHAR, -1, NULL);
		//	POINT pt = {0};
		//	pt.y = this->m_iFontSize * iLinePos;
		//	SendMessage(this->m_Hwnd, EM_SETSCROLLPOS,NULL,(LPARAM)&pt);
		//}
	}
	// xdid -Z [NAME] [ID] [SWITCH] [NUMERATOR] [DENOMINATOR]
	else if (flags.switch_cap_flags[25] && numtok > 4) {
		int num = input.gettok( 4 ).to_int();
		int den = input.gettok( 5 ).to_int();

		if (!SendMessage(this->m_Hwnd, EM_SETZOOM, (WPARAM) num, (LPARAM) den))
			this->showError(NULL, "-Z", "Richedit zooming error");
			//DCXError("/xdid -Z","Richedit zooming error");
	}
	else
		this->parseGlobalCommandRequest(input, flags);
}

/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::loadmIRCPalette() {
	char res[512];
	static const char com[] = "$color(0) $color(1) $color(2) $color(3) $color(4) $color(5) $color(6) $color(7) $color(8) $color(9) $color(10) $color(11) $color(12) $color(13) $color(14) $color(15)";
	mIRCeval(com, res);

	TString colors(res);
	int i = 0, len = colors.numtok( );

	while (i < len) {
		this->m_aColorPalette[i] = (COLORREF)colors.gettok( i +1 ).to_num();
		i++;
	}
}

/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::setContentsFont() {
	CHARFORMAT2 chrf;
	ZeroMemory(&chrf, sizeof(CHARFORMAT2));

	chrf.cbSize = sizeof(CHARFORMAT2);
	chrf.dwMask = CFM_BACKCOLOR | CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_CHARSET;
	chrf.yHeight = this->m_iFontSize;
	chrf.crTextColor = this->m_clrText;
	chrf.crBackColor = this->m_clrBackText;
	chrf.bCharSet = this->m_byteCharset;

	if (this->m_bFontBold)
		chrf.dwEffects |= CFE_BOLD;

	if (this->m_bFontItalic)
		chrf.dwEffects |= CFE_ITALIC;

	if (this->m_bFontStrikeout)
		chrf.dwEffects |= CFE_STRIKEOUT;

	if (this->m_bFontUnderline)
		chrf.dwEffects |= CFE_UNDERLINE;

	if (this->m_tsFontFaceName != "") {
		lstrcpyn(chrf.szFaceName, this->m_tsFontFaceName.to_chr(), 31);
		chrf.szFaceName[31] = 0;
	}

	this->hideSelection(TRUE);
	this->setSel(0, -1);
	this->setCharFormat(SCF_SELECTION, &chrf);
	this->setSel(0, 0);
	this->hideSelection(FALSE);
}

/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::clearContents() {
	this->hideSelection(TRUE);
	this->setSel(0, -1);
	this->replaceSel(FALSE, "");
	this->setSel(0, 0);
	this->hideSelection(FALSE);

	//this->setContentsFont( );
}
/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::parseContents(const BOOL fNewLine) { // old function
	this->m_bIgnoreInput = TRUE;
	this->setRedraw(FALSE);
	this->clearContents();

	char *text = this->m_tsText.to_chr();

	bool uline = false;
	bool bline = false;
	//int mcolor = -1;
	//int bkgcolor = -1;
	COLORREF mcolor = 0;
	COLORREF bkgcolor = 0;

	bool bmcolor = true;
	bool bbkgcolor = false;

	char cbuf[2];
	char colbuf[3];
	cbuf[1] = '\0';
	colbuf[2] = '\0';

	int i = 0;
	while (text[i] != '\0') {
		cbuf[0] = text[i];

		// if current char is a control code
		if (text[i] == 2 || text[i] == 3 || text[i] == 15 || text[i] == 22 || text[i] == 31) {
			// CTRL+B Parsing
			if (text[i] == 2)
				bline = !(bline);

			// CTRL+K Parsing
			else if (text[i] == 3) {
				if (text[i +1] >= '0' && text[i +1] <= '9') {
					colbuf[0] = text[i +1];
					colbuf[1] = '\0';
					++i;

					if (text[i +1] >= '0' && text[i +1] <= '9') {
						colbuf[1] = text[i +1];
						i++;
					}

					// color code number
					bmcolor = true;
					mcolor = this->m_aColorPalette[this->unfoldColor(colbuf)];

					// maybe a background color
					if (text[i+1] == ',') {
						++i;

						if (text[i +1] >= '0' && text[i +1] <= '9') {
							colbuf[0] = text[i +1];
							colbuf[1] = '\0';
							i++;

							if (text[i +1] >= '0' && text[i +1] <= '9') {
								colbuf[1] = text[i +1];
								++i;
							}

							// color code number
							bbkgcolor = true;
							bkgcolor = this->m_aColorPalette[this->unfoldColor(colbuf)];
						}
					}
				}
				else {
					bmcolor = false;
					bbkgcolor = false;
				}
			}
			// CTRL+O Parsing
			else if (text[i] == 15) {
				bline = false;
				uline = false;
				mcolor = -1;
				bkgcolor = -1;
			}
			// CTRL+R Parsing
			else if (text[i] == 22) {
				bline = false;
				uline = false;
			}
			// CTRL+U Parsing
			else if (text[i] == 31) {
				uline = !(uline);
			}
		}
		// regular char
		else if (fNewLine && text[i] == '\r' && text[i +1] == '\n') {
			bline = false;
			uline = false;
			mcolor = -1;
			bkgcolor = -1;

			//cbuf[0] = text[i];
			//this->insertText( cbuf, bline, uline, bmcolor, mcolor, bbkgcolor, bkgcolor, 0 );
			++i;
			cbuf[0] = text[i];
			this->insertText(cbuf, bline, uline, bmcolor, mcolor, bbkgcolor, bkgcolor, 0);
		}
		else {
			this->insertText(cbuf, bline, uline, bmcolor, mcolor, bbkgcolor, bkgcolor, 0);
		}

		++i;
	}

	this->setSel(0, 0);
	this->setRedraw(TRUE);
	this->redrawWindow();

	this->m_bIgnoreInput = FALSE;
}

/*!
* \brief blah
*
* blah
*/
int DcxRichEdit::unfoldColor(const char *color) {
	int nColor = atoi(color);

	while (nColor > 15) {
		nColor -=16;
	}

	return nColor;
}

/*!
* \brief blah
*
* blah
*/
void DcxRichEdit::insertText(char *text, bool bline, bool uline, bool bcolor, COLORREF color, bool bbkgcolor, COLORREF bkgcolor, int reverse) {
	// get total length
	int len = GetWindowTextLength(this->m_Hwnd);
	// get line char number from end char pos
	int line = Edit_LineIndex(this->m_Hwnd, Edit_LineFromChar(this->m_Hwnd, len -1));
	// get line length
	int linelen = Edit_LineLength(this->m_Hwnd, line);
	// total length of insert point
	len = line + linelen;
	this->hideSelection(TRUE);
	this->setSel(len, -1);
	this->replaceSel(FALSE, text);
	this->setSel(-1, 0);
	this->hideSelection(FALSE);
	CHARFORMAT2 chrf;
	ZeroMemory(&chrf, sizeof(CHARFORMAT2));
	chrf.cbSize = sizeof(CHARFORMAT2);
	chrf.dwMask = CFM_BACKCOLOR | CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_CHARSET;
	chrf.yHeight = this->m_iFontSize;
	chrf.crTextColor = this->m_clrText;
	chrf.crBackColor = this->m_clrBackText;
	chrf.bCharSet = this->m_byteCharset;

	if (this->m_bFontBold)
		chrf.dwEffects |= CFE_BOLD;

	if (this->m_bFontItalic)
		chrf.dwEffects |= CFE_ITALIC;

	if (this->m_bFontStrikeout)
		chrf.dwEffects |= CFE_STRIKEOUT;

	if (this->m_bFontUnderline)
		chrf.dwEffects |= CFE_UNDERLINE;

	if (this->m_tsFontFaceName != "") {
		lstrcpyn(chrf.szFaceName, this->m_tsFontFaceName.to_chr(), 31);
		chrf.szFaceName[31] = 0;
	}

	if (bcolor == true)
		chrf.crTextColor = color;
	if (bbkgcolor == true)
		chrf.crBackColor = bkgcolor;
	if (bline == true)
		chrf.dwEffects |= CFE_BOLD;
	if (uline == true)
		chrf.dwEffects |= CFE_UNDERLINE;

	this->hideSelection(TRUE);
	this->setSel(len, -1);
	this->setCharFormat(SCF_SELECTION, &chrf);
	this->setSel(0, 0);
	this->hideSelection(FALSE);
}

/*!
* \brief blah
*
* blah
*/
LRESULT DcxRichEdit::setAutoUrlDetect(const BOOL iEnable) {
	return SendMessage(this->m_Hwnd, EM_AUTOURLDETECT, (WPARAM) iEnable, (LPARAM) 0);
}

/*!
* \brief blah
*
* blah
*/
LRESULT DcxRichEdit::hideSelection(const BOOL iHide) {
	return SendMessage(this->m_Hwnd, EM_HIDESELECTION, (WPARAM) iHide, (LPARAM) 0);
}

/*!
* \brief blah
*
* blah
*/
LRESULT DcxRichEdit::setSel(const int iStart, const int iEnd) {
	return SendMessage(this->m_Hwnd, EM_SETSEL, (WPARAM) iStart, (LPARAM) iEnd);
}

/*!
* \brief blah
*
* blah
*/
LRESULT DcxRichEdit::replaceSel(const BOOL bUndo, LPCTSTR lpstr) {
	return SendMessage(this->m_Hwnd, EM_REPLACESEL, (WPARAM) bUndo, (LPARAM) lpstr);
}

/*!
* \brief blah
*
* blah
*/
LRESULT DcxRichEdit::getCharFormat(const UINT iType, CHARFORMAT2 *cfm) const {
	return SendMessage(this->m_Hwnd, EM_GETCHARFORMAT, (WPARAM) iType, (LPARAM) cfm);
}

/*!
* \brief blah
*
* blah
*/

LRESULT DcxRichEdit::setCharFormat(const UINT iType, CHARFORMAT2 *cfm) {
	return SendMessage(this->m_Hwnd, EM_SETCHARFORMAT, (WPARAM) iType, (LPARAM) cfm);
}

/*!
* \brief blah
*
* blah
*/
LRESULT DcxRichEdit::ParentMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bParsed) {
	switch (uMsg) {
		case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR) lParam;

			if (!hdr)
				break;

			switch(hdr->code) {
				case EN_LINK:
				{
					if (this->m_pParentDialog->getEventMask() & DCX_EVENT_CLICK) {
						ENLINK* enl = (ENLINK*) lParam;

						if ((enl->msg == WM_LBUTTONDOWN) ||
							(enl->msg == WM_LBUTTONDBLCLK) ||
							(enl->msg == WM_RBUTTONDOWN))
						{
							TEXTRANGE tr;
							TString event;

							// get information about link text
							ZeroMemory(&tr, sizeof(TEXTRANGE));
							tr.chrg = enl->chrg;
							char *str = new char[enl->chrg.cpMax - enl->chrg.cpMin];

							tr.lpstrText = str;
							SendMessage(this->m_Hwnd, EM_GETTEXTRANGE, NULL, (LPARAM) &tr);

							if (enl->msg == WM_LBUTTONDOWN)
								event = "sclick";
							else if (enl->msg == WM_LBUTTONDBLCLK)
								event = "dclick";
							else if (enl->msg == WM_RBUTTONDOWN)
								event = "rclick";

							this->callAliasEx(NULL, "%s,%d,%s,%s", "link", this->getUserID(), event.to_chr(), tr.lpstrText);
							delete [] str;
						}
					}
					break;
				} // EN_LINK
				//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/richedit/richeditcontrols/richeditcontrolreference/richeditmessages/em_gettextrange.asp
				case EN_SELCHANGE:
				{
					if (this->m_bIgnoreInput)
						break;

					if (this->m_pParentDialog->getEventMask() & DCX_EVENT_EDIT) {
						SELCHANGE* sel = (SELCHANGE*) lParam;

						if (sel->seltyp != SEL_EMPTY) {
							TEXTRANGE tr;

							// get information about selected text
							ZeroMemory(&tr, sizeof(TEXTRANGE));
							tr.chrg = sel->chrg;
							char *str = new char[sel->chrg.cpMax - sel->chrg.cpMin +1];
							tr.lpstrText = str;
							SendMessage(this->m_Hwnd, EM_GETTEXTRANGE, NULL, (LPARAM) &tr);

							this->callAliasEx(NULL, "%s,%d,%d,%d,%s", "selchange", this->getUserID(), sel->chrg.cpMin, sel->chrg.cpMax, tr.lpstrText);
							delete [] str;
						}
					}
					break;
				} // EN_SELCHANGE
			} // SWITCH 

			break;
		} // WM_NOTIFY
	}

	return 0L;
}

LRESULT DcxRichEdit::PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bParsed) {
	switch(uMsg) {

		case WM_KEYDOWN:
		{
			if (this->m_pParentDialog->getEventMask() & DCX_EVENT_EDIT) {
				if (wParam == VK_RETURN) {
					this->callAliasEx(NULL, "%s,%d", "return", this->getUserID());
				}

				if (lParam & 0x40000000) // ignore repeats
					break;
				this->callAliasEx(NULL, "%s,%d,%d:", "keydown", this->getUserID(), wParam);
			}
			break;
		}
		case WM_KEYUP:
		{
			if (this->m_pParentDialog->getEventMask() & DCX_EVENT_EDIT)
				this->callAliasEx(NULL, "%s,%d,%d", "keyup", this->getUserID(), wParam);
			break;
		}
		//case WM_PAINT:
		//	{
		//		if (!this->m_bAlphaBlend)
		//			break;
		//		PAINTSTRUCT ps;
		//		HDC hdc;

		//		hdc = BeginPaint( this->m_Hwnd, &ps );

		//		LRESULT res = 0L;
		//		bParsed = TRUE;

		//		// Setup alpha blend if any.
		//		LPALPHAINFO ai = this->SetupAlphaBlend(&hdc);

		//		// fill background.
		//		//DcxControl::DrawCtrlBackground(hdc,this,&ps.rcPaint);

		//		res = CallWindowProc( this->m_DefaultWindowProc, this->m_Hwnd, uMsg, (WPARAM) hdc, lParam );
		//		//res = CallWindowProc( this->m_DefaultWindowProc, this->m_Hwnd, WM_PRINT, (WPARAM) hdc, (LPARAM) (PRF_CLIENT|PRF_NONCLIENT|PRF_CHILDREN|PRF_OWNED) );

		//		this->FinishAlphaBlend(ai);

		//		EndPaint( this->m_Hwnd, &ps );
		//		return res;
		//	}
		//	break;
		case WM_DESTROY:
		{
			delete this;
			bParsed = TRUE;
			break;
		}
		default:
			return this->CommonMessage( uMsg, wParam, lParam, bParsed);
			break;
	}

	return 0L;
}
