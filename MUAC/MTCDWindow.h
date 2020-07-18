#pragma once
#include "stdafx.h"
#include "MUAC.h"
#include "Constants.h"
#include "Colours.h"
#include "MenuBar.h"
#include "FontManager.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;
using namespace EuroScopePlugIn;


class CMTCDWindow
{

private:
private:
	POINT TopLeftPosition;
	CSize WindowSize;
	bool Display = true;
	bool Released = true;

	string lengthString = "AAA0000 AAA0000 00 00 00.0 00.0 ^";

	int PaddingSides = 3;
	int PaddingTops = 5;
public:
	CMTCDWindow(POINT TopLeft) {
		TopLeftPosition = TopLeft;
	};

	void Move(CRect Area, bool isReleased) {
		Released = isReleased;
		TopLeftPosition = { Area.left, Area.top };
	};

	POINT GetTopLeftPosition() {
		return TopLeftPosition;
	};

	CRect Render(CDC* dc, CRadarScreen* instance, POINT mousePt, CMTCD* mtcd, multimap<string, string> veraTools, bool display) {
		Display = display;

		if (!Display)
			return {0, 0, 0, 0};

		int saveDc = dc->SaveDC();

		FontManager::SelectStandardFont(dc);

		// We need to count how many we show in the vera list
		int veraLn = 0;
		map<pair<string, string>, VERA::VERADataStruct> veraPrecalcData;
		for (auto &kv : veraTools) {
			if (mtcd->IsPairInMtcd(kv.first, kv.second))
				continue;

			CRadarTarget FirstTarget = instance->GetPlugIn()->RadarTargetSelect(kv.first.c_str());
			CRadarTarget SecondTarget = instance->GetPlugIn()->RadarTargetSelect(kv.second.c_str());

			VERA::VERADataStruct vera = VERA::Calculate(FirstTarget, SecondTarget);

			if (vera.minDistanceSeconds == -1)
				continue;

			veraPrecalcData.insert(std::pair<pair<string, string>, VERA::VERADataStruct>(kv, vera));

			veraLn++;
		}

		// Always at least one line for topbar
		int lineNumbers = veraLn + mtcd->Alerts.size() + 1;

		// We need to calculate the size of the menu
		CSize TextSize = dc->GetTextExtent(lengthString.c_str());
		WindowSize.cx = TextSize.cx + PaddingSides * 2;
		WindowSize.cy = TextSize.cy * lineNumbers + PaddingTops * 2;

		int LineHeight = (int)TextSize.cy + PaddingTops;

		// Whole window
		CRect Window(TopLeftPosition.x, TopLeftPosition.y, TopLeftPosition.x + WindowSize.cx, TopLeftPosition.y + WindowSize.cy);

		if (Released)
			dc->FillSolidRect(Window, Colours::MenuButtonBottom.ToCOLORREF());

		// Black outline
		CRect WinOutline = Window;
		WinOutline.InflateRect(1, 1);
		WinOutline.NormalizeRect();

		CPen BlackPen(PS_SOLID, 1, RGB(0, 0, 0));
		CPen GreyPen(PS_SOLID, 1, Colours::AircraftLightGrey.ToCOLORREF());
		if (IsInRect(mousePt, Window) || !Released)
			dc->SelectObject(&GreyPen);
		else
			dc->SelectObject(&BlackPen);

		dc->SelectStockObject(NULL_BRUSH);
		dc->Rectangle(WinOutline);

		instance->AddScreenObject(MTCD_WINDOW, "", Window, true, "");

		if (!Released)
			return Window;

		// Topbar
		CRect TopBar(TopLeftPosition.x, TopLeftPosition.y, TopLeftPosition.x + WindowSize.cx, TopLeftPosition.y + LineHeight);
		dc->FillSolidRect(TopBar, Colours::DarkBlueMenu.ToCOLORREF());

		dc->SetTextColor(Colours::GreyTextMenu.ToCOLORREF());

		//
		// Topbar text
		//

		dc->SetTextAlign(TA_LEFT | TA_BASELINE);

		dc->TextOutA(TopLeftPosition.x + 3, TopBar.bottom - PaddingSides, "            MTCD   VERA");

		// Close button, first one for size, second one for render
		dc->SetTextAlign(TA_LEFT | TA_TOP);
		int LeftButtonOffset = TopBar.right - dc->GetTextExtent("X").cx * 2 - 3;
		CRect ButtonRect = MenuBar::DrawMenuBarButton(dc, { LeftButtonOffset, TopLeftPosition.y }, "X", mousePt, false);

		instance->AddScreenObject(MTCD_WINDOW_BUTTONS, "CLOSE", ButtonRect, false, "Close MTCD Window");
	
		dc->SetTextColor(Colours::AircraftBlue.ToCOLORREF());

		// MTCD List first
		int TopOffset = TopBar.bottom + 2;
		for (auto &mtcdAlert : mtcd->Alerts) {
			string veraString = "    DIV";

			if (mtcdAlert.vera.minDistanceSeconds != -1) {
				string veraTime = padWithZeros(2, (int)(mtcdAlert.vera.minDistanceSeconds / 60))
					+ "." + to_string(mtcdAlert.vera.minDistanceSeconds % 60).substr(0, 1);

				string veraDistance = to_string(mtcdAlert.vera.minDistanceNm);
				size_t decimal_pos = veraDistance.find(".");
				veraDistance = veraDistance.substr(0, decimal_pos + 2);
				if (mtcdAlert.vera.minDistanceNm < 10)
					veraDistance = "0" + veraDistance;

				veraString = veraTime + " " + veraDistance;
			}
			
			string line = mtcdAlert.sourceCallsign + " " + mtcdAlert.conflictCallsign + " " 
				+ padWithZeros(2, mtcdAlert.minDistanceMin) + " " + padWithZeros(2, mtcdAlert.minDistanceNm) + " " 
				+ veraString;
		
			CSize extent = dc->GetTextExtent(line.c_str());

			dc->TextOutA(TopBar.left + 3, TopOffset, line.c_str());
			

			TopOffset += extent.cy;
		}

		CPen WhitePen(PS_SOLID, 1, Colours::AircraftLightGrey.ToCOLORREF());
		dc->SelectObject(&WhitePen);

		dc->MoveTo(TopBar.left + 2, TopOffset);
		dc->LineTo(TopBar.right - 2, TopOffset);

		// VERA List
		for (auto &kv : veraTools) {
			if (mtcd->IsPairInMtcd(kv.first, kv.second))
				continue;

			VERA::VERADataStruct vera = veraPrecalcData[kv];

			if (vera.minDistanceSeconds == -1)
				continue;

			string veraTime = padWithZeros(2, (int)(vera.minDistanceSeconds / 60))
				+ "." + to_string(vera.minDistanceSeconds % 60).substr(0, 1);

			string veraDistance = to_string(vera.minDistanceNm);
			size_t decimal_pos = veraDistance.find(".");
			veraDistance = veraDistance.substr(0, decimal_pos + 2);
			if (vera.minDistanceNm < 10)
				veraDistance = "0" + veraDistance;

			string line = kv.first + " " + kv.second + " 00 00 " + veraTime + " " + veraDistance;
			CSize extent = dc->GetTextExtent(string(line + " ").c_str());

			dc->TextOutA(TopBar.left + 3, TopOffset, line.c_str());
			
			// Push to mtcd button
			CSize singleExtent = dc->GetTextExtent("^");
			dc->TextOutA(TopBar.left + 3 + extent.cx, TopOffset, "^");

			CRect pushRect = { TopBar.left + 3 + extent.cx, TopOffset,
				TopBar.left + 3 + extent.cx + singleExtent.cx, TopOffset + singleExtent.cy };

			CPen YellowPen(PS_SOLID, 1, Colours::YellowHighlight.ToCOLORREF());

			if (IsInRect(mousePt, pushRect)) {
				dc->SelectStockObject(NULL_BRUSH);
				dc->SelectObject(&YellowPen);
				dc->Rectangle(pushRect);
			}

			TopOffset += extent.cy;
		}

		dc->RestoreDC(saveDc);

		return Window;
	};
};