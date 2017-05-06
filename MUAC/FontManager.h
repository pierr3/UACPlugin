#pragma once
#include "stdafx.h"
#include "Constants.h"


using namespace std;
using namespace EuroScopePlugIn;


class FontManager
{
public:
	static void SelectStandardFont(CDC* dc) {
		CFont* pOldFont = dc->GetCurrentFont();
		LOGFONT logFont;
		pOldFont->GetLogFont(&logFont);
		logFont.lfHeight = FONT_SIZE;
		CFont font;
		font.CreateFontIndirect(&logFont);

		dc->SelectObject(&font);
	};

	static void SelectBoldBigFont(CDC* dc) {
		CFont* pOldFont = dc->GetCurrentFont();
		LOGFONT logFont;
		pOldFont->GetLogFont(&logFont);
		logFont.lfHeight += (long)(logFont.lfHeight*0.15);
		logFont.lfWeight = 700;
		CFont font;
		font.CreateFontIndirect(&logFont);

		dc->SelectObject(&font);
	};
};