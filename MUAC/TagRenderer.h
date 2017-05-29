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
		map<int, CRect> LineAreas;

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

		int i = 0;
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
					leftOffset -= MeasureRect.cx;

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

				// See if we don't have an empty tag, if not we add it to the detection list
				//if (TagItem.Text != " ") {
					if (LineAreas.find(i) != LineAreas.end()) {
						LineAreas[i].right = TextBox.right;
					}
					else {
						LineAreas.insert(pair<int, CRect>(i, TextBox));
					}
				//}

			}
			topOffset += (int)MeasureRect.cy;

			// Store the greatest width for the tag
			if (leftOffset > TagWidth)
				TagWidth = leftOffset;

			leftOffset = 0;

			i++;
		}

		// Rendering the leaderline

		RECT Area = { TagTopLeft.x , TagTopLeft.y, TagTopLeft.x + TagWidth, TagTopLeft.y + topOffset };
		POINT Center = { (Area.left + Area.right) /2, (Area.top + Area.bottom) / 2 };
		int Size = DRAWING_AC_SQUARE_SYMBOL_SIZE + DRAWING_PADDING;
		CRect SymbolArea(AcPosition.x - Size, AcPosition.y - Size, AcPosition.x + Size, AcPosition.y + Size);

		COLORREF leaderLineColor = tag.IsSoft ? Colours::AircraftDarkGrey.ToCOLORREF() : Colours::AircraftLightGrey.ToCOLORREF();

		if (isStca)
			leaderLineColor = Colours::YellowHighlight.ToCOLORREF();

		CPen leaderLinePen(PS_SOLID, 1, leaderLineColor);

		CPen leaderLineYellow(PS_SOLID, 1, Colours::YellowHighlight.ToCOLORREF());
		CPen leaderLineRed(PS_SOLID, 1, Colours::RedWarning.ToCOLORREF());

		dc->SelectStockObject(NULL_BRUSH);

		// we use a vector to store the points we're gonna draw
		vector<pair<POINT, POINT>> clippedLeaderLine;

		// First we get the starting point next to the Target
		POINT liangOrigin, liangEnd, firstPoint;
		if (LiangBarsky(SymbolArea, AcPosition, Center, liangOrigin, liangEnd)) {
			firstPoint = liangEnd;
			// Add any points that are in between
			for (auto lineArea : LineAreas) {

				liangOrigin = { 0, 0 }; liangEnd = { 0, 0 };
				if (LiangBarsky(lineArea.second, firstPoint, Center, liangOrigin, liangEnd)) {
					clippedLeaderLine.push_back(pair<POINT, POINT>(liangOrigin, liangEnd));
					
					dc->SelectObject(leaderLineRed);
					DrawCross(dc, liangOrigin, 10);
					DrawCross(dc, liangEnd, 10);
					dc->SelectObject(leaderLineYellow);
				}
				else {
					dc->SelectObject(leaderLineRed);
				}

				dc->Rectangle(lineArea.second);

			}

			dc->SelectObject(leaderLineRed);
			bool star = false;
			int color = 0;

			dc->MoveTo(firstPoint);
			for (auto pt : clippedLeaderLine) {
				dc->LineTo(pt.first);
				dc->MoveTo(pt.second);
				color++;
				if (color >= 3) {
					dc->SelectObject(leaderLineYellow);
					color = 0;
				}
				else {
					dc->SelectObject(leaderLineRed);
				}
			}
			dc->MoveTo(Center);
		}

		// We now draw the leaderline, only if even number

	
		dc->RestoreDC(save);

		return Area;
	}
};