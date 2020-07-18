#pragma once
#include "stdafx.h"
#include "Constants.h"
#include "Colours.h"
#include <vector>
#include <map>
#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class MenuBar
{
private:
	const static int ButtonPaddingSides = 5;
	const static int ButtonPaddingTop = 2;

public:
	static map<int, string> MakeButtonData() {
		map<int, string> data;
		data[BUTTON_HIDEMENU] = "X";
		data[BUTTON_DECREASE_RANGE] = "-";
		data[BUTTON_RANGE] = "120 =";
		data[BUTTON_INCREASE_RANGE] = "+";
		data[BUTTON_FILTER_ON] = "FILTER ON";
		data[BUTTON_FILTER_HARD_LOW] = "0 =";
		data[BUTTON_FILTER_SOFT_LOW] = "200 =";
		data[BUTTON_FILTER_SOFT_HIGH] = "999 =";
		data[BUTTON_FILTER_HARD_HIGH] = "999 =";
		data[BUTTON_MTCD] = "MTCD";
		data[BUTTON_FIM] = "VAR FIM";
		data[BUTTON_QDM] = "QDM";
		data[BUTTON_TOPDOWN] = "T";
		data[BUTTON_MODE_A] = "A";
		data[BUTTON_LABEL_V] = "V";
		data[BUTTON_PRIMARY_TARGETS_ON] = "PR";
		data[BUTTON_VFR_ON] = "VFR";
		data[BUTTON_VELOTH] = "VEL OTH";
		data[BUTTON_VEL1] = "1";
		data[BUTTON_VEL2] = "2";
		data[BUTTON_VEL4] = "4";
		data[BUTTON_VEL8] = "8";
		data[BUTTON_DOTS] = "DOTS";
		data[BUTTON_FIN] = "FIN";
		return data;
	}

	static void DrawMenuBar(CDC* dc, CRadarScreen* radarScreen, POINT TopLeft, POINT MousePt, map<int, string> ButtonData, map<int, bool> PressedData) {
		int LeftOffset = TopLeft.x;
		CRect r;
		for (auto kv : ButtonData) {
			r = DrawMenuBarButton(dc, { LeftOffset, TopLeft.y }, kv.second, MousePt, PressedData[kv.first]);
			LeftOffset += r.Width();
			radarScreen->AddScreenObject(kv.first, "", r, false, "");

			// If the menu is hidden, we stop at the first button
			if (PressedData[BUTTON_HIDEMENU])
				break;
		}
	};

	static CRect DrawMenuBarButton(CDC* dc, POINT TopLeft, string Text, POINT MousePt, bool Pressed) {
		CBrush ButtonBackground(Colours::DarkBlueMenu.ToCOLORREF());
		CBrush ButtonPressedBackground(Colours::LightBlueMenu.ToCOLORREF());
		CPen YellowPen(PS_SOLID, 1, Colours::YellowHighlight.ToCOLORREF());

		// We need to calculate the size of the button according to the text fitting
		CSize TextSize = dc->GetTextExtent(Text.c_str());
		
		int Width = TextSize.cx + ButtonPaddingSides * 2;
		int Height = TextSize.cy + ButtonPaddingTop * 2;

		CRect Button(TopLeft.x, TopLeft.y, TopLeft.x + Width, TopLeft.y + Height);

		if (!Pressed)
			dc->FillSolidRect(Button, Colours::DarkBlueMenu.ToCOLORREF());
		else
			dc->FillSolidRect(Button, Colours::LightBlueMenu.ToCOLORREF());

		dc->Draw3dRect(TopLeft.x, TopLeft.y, Width, Height, Colours::MenuButtonTop.ToCOLORREF(), Colours::MenuButtonBottom.ToCOLORREF());
	
		if (IsInRect(MousePt, Button)) {
			dc->SelectStockObject(NULL_BRUSH);
			dc->SelectObject(&YellowPen);
			dc->Rectangle(Button);
		}

		// Text Draw
		if (!Pressed)
			dc->SetTextColor(Colours::GreyTextMenu.ToCOLORREF());
		else
			dc->SetTextColor(Colours::DarkBlueMenu.ToCOLORREF());

		dc->TextOutA(TopLeft.x + ButtonPaddingSides, TopLeft.y + ButtonPaddingTop, Text.c_str());

		return Button;
	};

	static map<int, bool> ResetAllVelButtons(map<int, bool> Data) {
		Data[BUTTON_VEL1] = false; Data[BUTTON_VEL2] = false; Data[BUTTON_VEL4] = false; Data[BUTTON_VEL8] = false;
		return Data;
	}

	static int GetVelValueButtonPressed(map<int, bool> Data) {
		if (Data[BUTTON_VEL1])
			return 60;
		if (Data[BUTTON_VEL2])
			return 60*2;
		if (Data[BUTTON_VEL4])
			return 60*4;
		if (Data[BUTTON_VEL8])
			return 60*8;
		return 0;
	}

	static map<int, bool> LoadVelValueToButtons(int value, map<int, bool> Data) {
		Data = ResetAllVelButtons(Data);
		if (value == 60)
			Data[BUTTON_VEL1] = true;
		if (value == 60*2)
			Data[BUTTON_VEL2] = true;
		if (value == 60*4)
			Data[BUTTON_VEL4] = true;
		if (value == 60*8)
			Data[BUTTON_VEL8] = true;

		return Data;
	}
};