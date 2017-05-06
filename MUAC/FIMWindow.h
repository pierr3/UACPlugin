#pragma once
#include "stdafx.h"
#include "Constants.h"
#include "Colours.h"
#include "MenuBar.h"
#include "FontManager.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace EuroScopePlugIn;


class CFIMWindow
{
private:
	POINT TopLeftPosition;
	CSize WindowSize;
	bool Display = true;
	bool Released = true;

	string lengthString = "SCRATCHPAD B738 /m N0000 EHAM LEMD ECL390 MEDIL 1054 390  136.330";

	int PaddingSides = 3;
	int PaddingTops = 5;

public:
	CFIMWindow(POINT TopLeft) {
		TopLeftPosition = TopLeft;
	};

	void Move(CRect Area, bool isReleased) {
		Released = isReleased;
		TopLeftPosition = { Area.left, Area.top };
	}

	CRect Render(CDC* dc, CPlugIn* plugin, POINT mousePt, CRadarTarget radarTarget, CFlightPlan flightPlan) {
		int saveDc = dc->SaveDC();

		FontManager::SelectStandardFont(dc);

		// We need to calculate the size of the menu
		CSize TextSize = dc->GetTextExtent(lengthString.c_str());
		WindowSize.cx = TextSize.cx + PaddingSides * 2;
		WindowSize.cy = TextSize.cy*3 + PaddingTops * 2;
		
		int LineHeight = (int) WindowSize.cy / 3;

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
		if (IsInRect(mousePt, Window))
			dc->SelectObject(&GreyPen);
		else
			dc->SelectObject(&BlackPen);
		
		dc->SelectStockObject(NULL_BRUSH);
		dc->Rectangle(WinOutline);

		if (!Released)
			return Window;

		// Topbar
		CRect TopBar(TopLeftPosition.x, TopLeftPosition.y, TopLeftPosition.x + WindowSize.cx, TopLeftPosition.y + LineHeight);
		dc->FillSolidRect(TopBar, Colours::DarkBlueMenu.ToCOLORREF());

		dc->SetTextColor(Colours::GreyTextMenu.ToCOLORREF());

		//
		// Topbar text
		//
		int LeftTextOffset = 3;

		dc->SetTextAlign(TA_LEFT | TA_BASELINE);

		dc->TextOutA(TopLeftPosition.x + LeftTextOffset, TopBar.bottom - PaddingSides, "VAR");

		LeftTextOffset += dc->GetTextExtent("VARGAPGAP").cx;

		if (flightPlan.IsValid()) {
			int dcSaveBold = dc->SaveDC();
			FontManager::SelectBoldBigFont(dc);

			dc->TextOutA(TopLeftPosition.x + LeftTextOffset, TopBar.bottom - PaddingSides, flightPlan.GetCallsign());

			LeftTextOffset += dc->GetTextExtent(string(flightPlan.GetCallsign() + string("GAPGAP")).c_str()).cx;

			dc->RestoreDC(dcSaveBold);
		}

		// TODO: Add airline

		// Close button, first one for size, second one for render
		dc->SetTextAlign(TA_LEFT | TA_TOP);
		MenuBar::DrawMenuBarButton(dc, { TopBar.right - dc->GetTextExtent("X").cx*2 - 3, TopLeftPosition.y }, "X", mousePt, false);
		dc->SetTextAlign(TA_LEFT | TA_BASELINE);

		dc->SetTextColor(Colours::AircraftBlue.ToCOLORREF());

		// Second line
		CRect SecondLine(TopBar.left, TopBar.bottom, TopBar.left + WindowSize.cx, TopBar.bottom + LineHeight - 3);
		
		string SecondLineString = "";

		if (flightPlan.IsValid()) {
			if (strlen(flightPlan.GetControllerAssignedData().GetScratchPadString()) != 0) {
				SecondLineString += string(flightPlan.GetControllerAssignedData().GetScratchPadString()).substr(0, 7) + " ¦";
			}
			else {
				SecondLineString += "        ¦";
			}

			SecondLineString += string(flightPlan.GetFlightPlanData().GetAircraftFPType()).substr(0, 5) + " ";
			SecondLineString += "/" + string(1, flightPlan.GetFlightPlanData().GetAircraftWtc()) + " ";

			if (radarTarget.IsValid()) {
				SecondLineString += "N" + padWithZeros(4, radarTarget.GetGS()) + " ";
			}

			SecondLineString += string(flightPlan.GetFlightPlanData().GetOrigin()) + " " + 
				string(flightPlan.GetFlightPlanData().GetDestination()) + " ¦";

			SecondLineString += "ECL" + padWithZeros(3, flightPlan.GetFlightPlanData().GetFinalAltitude() / 100) + "¦";

			// Cop X/N
			if (flightPlan.GetTrackingControllerIsMe()) {
				string point = flightPlan.GetExitCoordinationPointName();
				if (point.length() == 0)
					point = "     ";
				string time = " +" + to_string(flightPlan.GetSectorExitMinutes()) + "' ";
				if (flightPlan.GetSectorExitMinutes() == -1)
					time = "    ";

				SecondLineString += point + time + padWithZeros(3, flightPlan.GetExitCoordinationAltitude() / 100);
			}
			else {
				string point = flightPlan.GetEntryCoordinationPointName();
				if (point.length() == 0)
					point = "     ";

				string time = " +" + to_string(flightPlan.GetSectorEntryMinutes()) + "' ";
				if (flightPlan.GetSectorEntryMinutes() == -1)
					time = "    ";

				SecondLineString += point + time + padWithZeros(3, flightPlan.GetEntryCoordinationAltitude() / 100);
			}

			SecondLineString += " ¦";

			if (flightPlan.GetTrackingControllerIsMe()) {
				if (strlen(flightPlan.GetCoordinatedNextController()) != 0 && plugin->ControllerSelect(flightPlan.GetCoordinatedNextController()).IsValid())
					SecondLineString += to_string(plugin->ControllerSelect(flightPlan.GetCoordinatedNextController()).GetPrimaryFrequency()).substr(0, 7);
			}
			else {
				if (strlen(flightPlan.GetTrackingControllerCallsign()) != 0 && plugin->ControllerSelect(flightPlan.GetTrackingControllerCallsign()).IsValid())
					SecondLineString += to_string(plugin->ControllerSelect(flightPlan.GetTrackingControllerCallsign()).GetPrimaryFrequency()).substr(0, 7);
			}
		}

		dc->TextOutA(SecondLine.left + 3, SecondLine.bottom - PaddingSides, SecondLineString.c_str());

		// Third line
		CRect ThirdLine(SecondLine.left, SecondLine.bottom, SecondLine.left + WindowSize.cx, SecondLine.bottom + LineHeight);

		string ThirdLineText = "";

		if (radarTarget.IsValid()) {
			ThirdLineText = "H" + padWithZeros(3, (int)fmod(radarTarget.GetPosition().GetReportedHeading(), 360)) + " ";
			
			if (flightPlan.IsValid()) {
				ThirdLineText += "IAS" + 
					to_string(flightPlan.GetFlightPlanData().PerformanceGetIas(radarTarget.GetPosition().GetPressureAltitude(), radarTarget.GetVerticalSpeed()));

				double mach = flightPlan.GetFlightPlanData().PerformanceGetMach(radarTarget.GetPosition().GetPressureAltitude(), radarTarget.GetVerticalSpeed()) / 100.0;

				ThirdLineText += " M" + 
					to_string(mach).substr(0, 4);
			}
			ThirdLineText += "¦GS" + padWithZeros(4, radarTarget.GetPosition().GetReportedGS()) + " ";

			if (radarTarget.GetVerticalSpeed() > 100)
				ThirdLineText += "^";
			else if (radarTarget.GetVerticalSpeed() < -100)
				ThirdLineText += "|";
			else
				ThirdLineText += "-";
			ThirdLineText += padWithZeros(4, abs(radarTarget.GetVerticalSpeed())).substr(0, 2);
		}

		dc->SetTextAlign(TA_RIGHT | TA_BASELINE);
		dc->TextOutA(ThirdLine.right - 3, ThirdLine.bottom - PaddingSides, ThirdLineText.c_str());

		dc->RestoreDC(saveDc);

		return Window;
	};
};