#pragma once
#include "stdafx.h"
#include <vector>
#include "Constants.h"
#include "Colours.h"
#include "TagItem.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;

class TagRenderer {
public:
	static RECT Render(CDC* dc, POINT MousePt, POINT TagOffset, POINT AcPosition, Tag tag, bool isDetailed) {
		
		vector<vector<TagItem>> Definition = tag.Definition;

		int save = dc->SaveDC();
		
		CFont* pOldFont = dc->GetCurrentFont();
		LOGFONT logFont;
		pOldFont->GetLogFont(&logFont);
		logFont.lfHeight = FONT_SIZE;
		CFont TagNormalFont;
		TagNormalFont.CreateFontIndirect(&logFont);
		
		dc->SelectObject(&TagNormalFont);

		int leftOffset = 0;
		int topOffset = 0;

		int TagWidth = 0;

		POINT TagTopLeft = { AcPosition.x + TagOffset.x, AcPosition.y + TagOffset.y };

		bool NeedPrimaryAreaSet = true;
		CRect PrimaryArea;

		// Used for the rest
		COLORREF PrimaryColor = Colours::AircraftDarkGrey.ToCOLORREF();

		// Used for callsign
		COLORREF SecondaryColor = Colours::AircraftDarkGrey.ToCOLORREF();

		if (!tag.IsSoft) {
			PrimaryColor = Colours::AircraftLightGrey.ToCOLORREF();
			SecondaryColor = Colours::AircraftLightGrey.ToCOLORREF();
		}

		if (tag.TagState == Tag::TagStates::Redundant) {
			SecondaryColor = Colours::AircraftBlue.ToCOLORREF();
		}

		if (tag.TagState == Tag::TagStates::Assumed) {
			PrimaryColor = Colours::AircraftGreen.ToCOLORREF();
			SecondaryColor = Colours::AircraftGreen.ToCOLORREF();
		}

		if (tag.TagState == Tag::TagStates::Next || tag.TagState == Tag::TagStates::TransferredToMe || 
			tag.TagState == Tag::TagStates::InSequence) {
			SecondaryColor = Colours::AircraftGreen.ToCOLORREF();
		}

		if (tag.TagState == Tag::TagStates::TransferredFromMe) {
			PrimaryColor = Colours::AircraftGreen.ToCOLORREF();
		}

		dc->SetTextColor(PrimaryColor);

		if (isDetailed) {
			CFont *pOldFont = dc->GetCurrentFont();

			LOGFONT logFont;
			pOldFont->GetLogFont(&logFont);
			logFont.lfHeight += (long)(logFont.lfHeight*0.25);

			CFont newFont;
			newFont.CreateFontIndirect(&logFont);

			dc->SelectObject(&newFont);
		}

		for (auto TagLine : Definition) {
			// For each item
			CSize MeasureRect = { 0, 0 };
			for (auto TagItem : TagLine) {

				if (TagItem.TagType == "Callsign") {
					dc->SetTextColor(SecondaryColor);
				}
					
				MeasureRect = dc->GetTextExtent(TagItem.Text.c_str());
				
				dc->TextOutA(TagTopLeft.x + leftOffset, TagTopLeft.y + topOffset, TagItem.Text.c_str());

				CRect TextBox(TagTopLeft.x + leftOffset, TagTopLeft.y + topOffset, 
					TagTopLeft.x + leftOffset + MeasureRect.cx, TagTopLeft.y + topOffset + MeasureRect.cy);

				// Here we also dispaly the rectangle if the mouse cursor is in it
				if (IsInRect(MousePt, TextBox) && isDetailed) {
					CPen YellowPen(PS_SOLID, 1, Colours::YellowWarning.ToCOLORREF());
					dc->SelectObject(&YellowPen);
					dc->SelectStockObject(NULL_BRUSH);
					dc->Rectangle(TextBox);
				}

				if (TagItem.TagType == "Callsign")
					dc->SetTextColor(PrimaryColor);

				if (NeedPrimaryAreaSet && TagItem.Text != " ") {
					PrimaryArea = TextBox;
					NeedPrimaryAreaSet = false;
				}

				
				leftOffset += (int)MeasureRect.cx;

				// We don't need a blank space if it's one of the empty items
				if (TagItem.Text != " ")
					leftOffset += 5;
			}
			topOffset += (int)MeasureRect.cy;

			// Store the greatest width for the tag
			if (leftOffset > TagWidth)
				TagWidth = leftOffset;

			leftOffset = 0;
		}

		// Rendering the leaderline

		RECT Area = { TagTopLeft.x , TagTopLeft.y, TagTopLeft.x + TagWidth, TagTopLeft.y + topOffset };

		POINT Center = { (Area.left + Area.right) /2, (Area.top + Area.bottom) / 2 };

		// First we calculate the area around the tag
		POINT toDraw1, toDraw2;
		if (LiangBarsky(Area, AcPosition, PrimaryArea.CenterPoint(), toDraw1, toDraw2)) {
			// Then we substract the symbol as well
			POINT TagPoint = toDraw1;

			int Size = DRAWING_AC_SQUARE_SYMBOL_SIZE + DRAWING_PADDING;

			CRect SymbolArea(AcPosition.x - Size, AcPosition.y - Size,
				AcPosition.x + Size, AcPosition.y + Size);

			POINT toDraw4, toDraw5;
			if (LiangBarsky(SymbolArea, AcPosition, TagPoint, toDraw4, toDraw5)) {

				COLORREF leaderLineColor = tag.IsSoft ? Colours::AircraftDarkGrey.ToCOLORREF() : Colours::AircraftLightGrey.ToCOLORREF();

				CPen leaderLinePen(PS_SOLID, 1, leaderLineColor);
				dc->SelectObject(&leaderLinePen);
				dc->MoveTo(toDraw5.x, toDraw5.y);
				dc->LineTo(TagPoint.x, TagPoint.y);
			}
		}
		
		dc->RestoreDC(save);

		return Area;
	}
};