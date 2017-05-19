#pragma once
#include "stdafx.h"
#include <vector>
#include "Constants.h"
#include "Colours.h"
#include "FontManager.h"
#include "TagItem.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;
using namespace Gdiplus;

class TagRenderer {
public:
	static RECT Render(CDC* dc, POINT MousePt, POINT TagOffset, POINT AcPosition, Tag tag, bool isDetailed, bool isStca, bool isMtcd, map<int, CRect>* DetailedTagClicks) {
		
		vector<vector<TagItem>> Definition = tag.Definition;

		int save = dc->SaveDC();
		
		FontManager::SelectStandardFont(dc);

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

		if (tag.TagState == Tag::TagStates::Next || 
			tag.TagState == Tag::TagStates::InSequence) {
			SecondaryColor = Colours::AircraftGreen.ToCOLORREF();
		}

		if (tag.TagState == Tag::TagStates::TransferredToMe) {
			SecondaryColor = Colours::AircraftBlue.ToCOLORREF();
		}

		if (tag.TagState == Tag::TagStates::TransferredFromMe) {
			PrimaryColor = Colours::AircraftGreen.ToCOLORREF();
		}

		dc->SetTextColor(PrimaryColor);

		if (isDetailed)
			FontManager::SelectBoldBigFont(dc);

		for (auto TagLine : Definition) {
			// For each item
			CSize MeasureRect = { 0, 0 };
			for (auto TagItem : TagLine) {

				if (TagItem.Text.length() == 0)
					continue;

				if (TagItem.ColourType == TagItem::TagColourTypes::Highlight) {
					dc->SetTextColor(SecondaryColor);
				}

				if (TagItem.ColourType == TagItem::TagColourTypes::Information) {
					dc->SetTextColor(Colours::YellowWarning.ToCOLORREF());
				}

				bool needBacktick = false;
				if (TagItem.Text.compare(0, PREFIX_BACKSTEP.length(), PREFIX_BACKSTEP) == 0) {
					needBacktick = true;
					TagItem.Text.erase(0, PREFIX_BACKSTEP.length());
				}

				if (TagItem.Text.compare(0, PREFIX_PURPLE_COLOR.length(), PREFIX_PURPLE_COLOR) == 0) {
					dc->SetTextColor(Colours::PurpleDisplay.ToCOLORREF());
					TagItem.Text.erase(0, PREFIX_PURPLE_COLOR.length());
				}

				if (TagItem.Text.compare(0, PREFIX_ORANGE_COLOR.length(), PREFIX_ORANGE_COLOR) == 0) {
					dc->SetTextColor(Colours::OrangeTool.ToCOLORREF());
					TagItem.Text.erase(0, PREFIX_ORANGE_COLOR.length());
				}
					
				MeasureRect = dc->GetTextExtent(TagItem.Text.c_str());
				
				if (needBacktick)
					leftOffset -= (MeasureRect.cx + 5);

				dc->TextOutA(TagTopLeft.x + leftOffset, TagTopLeft.y + topOffset, TagItem.Text.c_str());

				CRect TextBox(TagTopLeft.x + leftOffset, TagTopLeft.y + topOffset, 
					TagTopLeft.x + leftOffset + MeasureRect.cx, TagTopLeft.y + topOffset + MeasureRect.cy);

				// Here we also dispaly the rectangle if the mouse cursor is in it
				if (((IsInRect(MousePt, TextBox) && isDetailed) || ((isStca || isMtcd) && TagItem.TagType == "Callsign")) && TagItem.TagType != " ") {
					CPen YellowPen(PS_SOLID, 1, Colours::YellowWarning.ToCOLORREF());
					dc->SelectObject(&YellowPen);
					dc->SelectStockObject(NULL_BRUSH);
					dc->Rectangle(TextBox);
				}

				// if Detailed, then we store the area for click
				if (isDetailed && TagItem.ClickId != 0)
					DetailedTagClicks->insert(pair<int, CRect>(TagItem.ClickId, TextBox));
				
				if (NeedPrimaryAreaSet && TagItem.Text != " " && TagItem.TagType != "FPM" && TagItem.TagType != "RTEM" && TagItem.TagType != "V") {
					PrimaryArea = TextBox;
					NeedPrimaryAreaSet = false;
				}

				leftOffset += (int)MeasureRect.cx;
				dc->SetTextColor(PrimaryColor);

				// We don't need a blank space if it's one of the empty items
				if (TagItem.Text != " " && TagItem.TagType != "V")
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

				if (isStca)
					leaderLineColor = Colours::YellowHighlight.ToCOLORREF();

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