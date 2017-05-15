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

	string lengthString = "SCRATCHPADSTR  B738 /m N0000 EHAM LEMD ECL390 MEDIL 1054 390  136.330";

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

	CRect Render(CDC* dc, CRadarScreen* instance, POINT mousePt, CRadarTarget radarTarget, CFlightPlan flightPlan) {
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
		if (IsInRect(mousePt, Window) || !Released)
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

			LeftTextOffset += dc->GetTextExtent("AFRXXXXGAPGAP").cx;

			dc->RestoreDC(dcSaveBold);

			if (CCallsignLookup::Available) {
				string callsign_code = flightPlan.GetCallsign();
				callsign_code = callsign_code.substr(0, 3);

				dc->TextOutA(TopLeftPosition.x + LeftTextOffset, TopBar.bottom - PaddingSides, 
					CCallsignLookup::Lookup->getCallsign(callsign_code).c_str());
			}
		}

		// Close button, first one for size, second one for render
		dc->SetTextAlign(TA_LEFT | TA_TOP);
		int LeftButtonOffset = TopBar.right - dc->GetTextExtent("X").cx * 2 - 3;
		CRect ButtonRect = MenuBar::DrawMenuBarButton(dc, { LeftButtonOffset, TopLeftPosition.y }, "X", mousePt, false);
		LeftButtonOffset -= ButtonRect.Size().cx*3 + 1;
		ButtonRect = MenuBar::DrawMenuBarButton(dc, { LeftButtonOffset, TopLeftPosition.y }, "CPDLC", mousePt, false);
		LeftButtonOffset -= ButtonRect.Size().cx + 1;
		ButtonRect = MenuBar::DrawMenuBarButton(dc, { LeftButtonOffset, TopLeftPosition.y }, "COORD", mousePt, false);
		dc->SetTextAlign(TA_LEFT | TA_BASELINE);

		dc->SetTextColor(Colours::AircraftBlue.ToCOLORREF());

		// Second line
		CRect SecondLine(TopBar.left, TopBar.bottom, TopBar.left + WindowSize.cx, TopBar.bottom + LineHeight - 3);

		string SecondLineString = "";

		if (flightPlan.IsValid()) {
			SecondLineString += "¦";

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
				string point = string(flightPlan.GetExitCoordinationPointName()) + " ";
				if (strlen(flightPlan.GetExitCoordinationPointName()) == 0)
					point = "      ";

				string time = "     ";
				if (flightPlan.GetSectorExitMinutes() != -1)
					time = getUtcTimePlusMinutes(flightPlan.GetSectorExitMinutes()) + " ";

				string fl = padWithZeros(3, flightPlan.GetExitCoordinationAltitude() / 100);
				if (flightPlan.GetExitCoordinationAltitude() == 0)
					fl = "   ";

				SecondLineString += point + time + fl.substr(0, 3);
			}
			else {
				string point = string(flightPlan.GetEntryCoordinationPointName()) + " ";
				if (strlen(flightPlan.GetEntryCoordinationPointName()) == 0)
					point = "      ";

				string time = "     ";
				if (flightPlan.GetSectorEntryMinutes() != -1)
					time = getUtcTimePlusMinutes(flightPlan.GetSectorEntryMinutes()) + " ";

				string fl = padWithZeros(3, flightPlan.GetEntryCoordinationAltitude() / 100);
				if (flightPlan.GetEntryCoordinationAltitude() == 0)
					fl = "   ";

				SecondLineString += point + time + fl.substr(0, 3);
			}

			SecondLineString += "¦";

			if (flightPlan.GetTrackingControllerIsMe()) {
				if (strlen(flightPlan.GetCoordinatedNextController()) != 0 && instance->GetPlugIn()->ControllerSelect(flightPlan.GetCoordinatedNextController()).IsValid())
					SecondLineString += to_string(instance->GetPlugIn()->ControllerSelect(flightPlan.GetCoordinatedNextController()).GetPrimaryFrequency()).substr(0, 7);
				else
					SecondLineString += "---.---";
			}
			else {
				if (strlen(flightPlan.GetTrackingControllerCallsign()) != 0 && instance->GetPlugIn()->ControllerSelect(flightPlan.GetTrackingControllerCallsign()).IsValid())
					SecondLineString += to_string(instance->GetPlugIn()->ControllerSelect(flightPlan.GetTrackingControllerCallsign()).GetPrimaryFrequency()).substr(0, 7);
				else
					SecondLineString += "---.---";
			}
		}

		dc->SetTextAlign(TA_RIGHT | TA_BASELINE);
		CSize SecondLineMeasure = dc->GetTextExtent(SecondLineString.c_str());
		dc->TextOutA(SecondLine.right - 3, SecondLine.bottom - PaddingSides, SecondLineString.c_str());

		if (flightPlan.IsValid()) {
			// Scratchpad
			CRect ScratchPadRect = { SecondLine.left + 3, SecondLine.top - 1,
				(SecondLine.right - 3) - SecondLineMeasure.cx, Window.bottom - 3 };

			string scratchPadString = "";
			if (strlen(flightPlan.GetControllerAssignedData().GetScratchPadString()) != 0) {
				scratchPadString += string(flightPlan.GetControllerAssignedData().GetScratchPadString());
			}

			dc->SetTextAlign(TA_LEFT | TA_TOP);
			dc->DrawText(scratchPadString.c_str(), ScratchPadRect, DT_WORDBREAK);

			instance->AddScreenObject(FIM_SCRATCHPAD, flightPlan.GetCallsign(), ScratchPadRect, false, "");
		}

		// Third line
		CRect ThirdLine(SecondLine.left, SecondLine.bottom, SecondLine.left + WindowSize.cx, SecondLine.bottom + LineHeight);

		string ThirdLineText = "";

		if (radarTarget.IsValid()) {

			if (flightPlan.IsValid()) {
				string Star = flightPlan.GetFlightPlanData().GetStarName();
				string Rwy = flightPlan.GetFlightPlanData().GetArrivalRwy();

				if (Star.length() == 0)
					Star = "       ";

				if (Rwy.length() == 0)
					Rwy = "   ";

				ThirdLineText += Star;
				ThirdLineText += " " + Rwy + "¦";
			}

			CRadarTargetPositionData pos = radarTarget.GetPosition();
			CRadarTargetPositionData oldpos = radarTarget.GetPreviousPosition(pos);
			float vz = 0.0f;
			int deltaalt = 0, deltaT = 0;
			if (pos.IsValid() && oldpos.IsValid()) {
				deltaalt = pos.GetFlightLevel() - oldpos.GetFlightLevel();
				deltaT = oldpos.GetReceivedTime() - pos.GetReceivedTime();

				if (deltaT > 0) {
					vz = abs(deltaalt) * (60.0f / deltaT);
				}
			}

			int Tendency = 0;
			if (abs(vz) >= 100 && abs(deltaalt) >= 20) {
				Tendency = 1;

				if (deltaalt < 0)
					Tendency = -1;
			}

			ThirdLineText += "H" + padWithZeros(3, (int)fmod(radarTarget.GetPosition().GetReportedHeading(), 360)) + " ";
			
			if (flightPlan.IsValid()) {
				ThirdLineText += "IAS" + 
					to_string(flightPlan.GetFlightPlanData().PerformanceGetIas(radarTarget.GetPosition().GetPressureAltitude(), Tendency));

				double mach = flightPlan.GetFlightPlanData().PerformanceGetMach(radarTarget.GetPosition().GetPressureAltitude(), Tendency) / 100.0;

				ThirdLineText += " M" + 
					to_string(mach).substr(0, 4);
			}
			ThirdLineText += "¦GS" + padWithZeros(4, radarTarget.GetPosition().GetReportedGS()) + " ";

			string VerticalRate = "00";
			
			if (deltaT > 0) {
				float vz = abs(deltaalt) * (60.0f / deltaT);

				// If the rate is too much
				if ((int)abs(vz) >= 9999) {
					VerticalRate = "^99";
					if (deltaalt < 0)
						VerticalRate = "|99";
				}
				else if (abs(vz) >= 100 && abs(deltaalt) >= 20) {
					string rate = padWithZeros(2, (int)abs(vz / 100));
					VerticalRate = "^" + rate;

					if (deltaalt < 0)
						VerticalRate = "|" + rate;
				}
				else {
					VerticalRate = " 00";
				}
			}

			ThirdLineText += VerticalRate;
		}

		dc->SetTextAlign(TA_RIGHT | TA_BASELINE);
		dc->TextOutA(ThirdLine.right - 3, ThirdLine.bottom - PaddingSides, ThirdLineText.c_str());

		// Add clickable zone
		if (flightPlan.IsValid()) {
			CSize LeftStartPointSize = dc->GetTextExtent(ThirdLineText.c_str());
			POINT LeftStartPoint = { ThirdLine.right - 3 - LeftStartPointSize.cx, ThirdLine.bottom - PaddingSides };

			string Star = flightPlan.GetFlightPlanData().GetStarName();
			string Rwy = flightPlan.GetFlightPlanData().GetArrivalRwy();

			if (Star.length() == 0)
				Star = "       ";

			if (Rwy.length() == 0)
				Rwy = "   ";

			CSize StarSize = dc->GetTextExtent(Star.c_str());
			CSize RwySize = dc->GetTextExtent(Rwy.c_str());
			CSize BlankSize = dc->GetTextExtent(" ");

			CRect StarArea = { LeftStartPoint.x, (LeftStartPoint.y + 2) - StarSize.cy, LeftStartPoint.x + StarSize.cx, LeftStartPoint.y + 4 };
			CRect RwyArea = { StarArea.left + BlankSize.cx + StarSize.cx, StarArea.top,
				StarArea.left + BlankSize.cx + RwySize.cx + StarSize.cx, StarArea.bottom };

			instance->AddScreenObject(FIM_STAR, flightPlan.GetCallsign(), StarArea, false, "");
			instance->AddScreenObject(FIM_RWY, flightPlan.GetCallsign(), RwyArea, false, "");

			CPen YellowPen(PS_SOLID, 1, Colours::YellowHighlight.ToCOLORREF());

			if (IsInRect(mousePt, StarArea)) {
				dc->SelectStockObject(NULL_BRUSH);
				dc->SelectObject(&YellowPen);
				dc->Rectangle(StarArea);
			}

			if (IsInRect(mousePt, RwyArea)) {
				dc->SelectStockObject(NULL_BRUSH);
				dc->SelectObject(&YellowPen);
				dc->Rectangle(RwyArea);
			}
		}


		dc->RestoreDC(saveDc);

		return Window;
	};
};