#include "stdafx.h"
#include "RadarScreen.h"

POINT MousePoint = { 0, 0 };
string DetailedTag = "";

RadarScreen::RadarScreen()
{
	srand((unsigned int)time(NULL));

	// Initialize the Menu Bar
	MenuButtons = MenuBar::MakeButtonData();
	LoadAllData();
}

RadarScreen::~RadarScreen()
{
}

void RadarScreen::LoadAllData()
{
	ButtonsPressed[BUTTON_VEL1] = true;
	ButtonsPressed[BUTTON_FILTER_ON] = true;
	ButtonsPressed[BUTTON_VELOTH] = true;
	ButtonsPressed[BUTTON_PRIMARY_TARGETS_ON] = true;
	ButtonsPressed[BUTTON_DOTS] = true;

	LoadFilterButtonsData();
}

void RadarScreen::LoadFilterButtonsData()
{
	MenuButtons[BUTTON_FILTER_HARD_LOW] = to_string(RadarFilters.Hard_Low / 100) + string(" =");
	MenuButtons[BUTTON_FILTER_SOFT_LOW] = to_string(RadarFilters.Soft_Low / 100) + string(" =");
	MenuButtons[BUTTON_FILTER_SOFT_HIGH] = to_string(RadarFilters.Soft_High / 100) + string(" =");
	MenuButtons[BUTTON_FILTER_HARD_HIGH] = to_string(RadarFilters.Hard_High / 100) + string(" =");
}

void RadarScreen::OnRefresh(HDC hDC, int Phase)
{
	if (Phase != REFRESH_PHASE_AFTER_TAGS && Phase != REFRESH_PHASE_BEFORE_TAGS)
		return;
	
	// Mouse Pointer
	POINT p;
	if (GetCursorPos(&p)) {
		if (ScreenToClient(GetActiveWindow(), &p)) {
			MousePoint = p;
		}
	}

	// Setup gdi renderer
	CDC dc;
	dc.Attach(hDC);

	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {

		int saveTool = dc.SaveDC();

		// Sep tools

		CPen SepToolColorPen(PS_SOLID, 1, Colours::OrangeTool.ToCOLORREF());
		dc.SelectObject(&SepToolColorPen);
		dc.SetTextColor(Colours::OrangeTool.ToCOLORREF());

		CFont* pOldFont = dc.GetCurrentFont();
		LOGFONT logFont;
		pOldFont->GetLogFont(&logFont);
		logFont.lfHeight = FONT_SIZE;
		CFont TagNormalFont;
		TagNormalFont.CreateFontIndirect(&logFont);

		dc.SelectObject(&TagNormalFont);

		// 
		// First is active tool
		//

		if (AcquiringSepTool != "") {
			CPosition ActivePosition = GetPlugIn()->RadarTargetSelect(AcquiringSepTool.c_str()).GetPosition().GetPosition();
			
			
			if (GetPlugIn()->RadarTargetSelect(AcquiringSepTool.c_str()).GetPosition().IsValid()) {
				POINT ActivePositionPoint = ConvertCoordFromPositionToPixel(ActivePosition);

				dc.MoveTo(ActivePositionPoint);
				dc.LineTo(MousePoint);
				string distanceText = to_string(ActivePosition.DistanceTo(ConvertCoordFromPixelToPosition(MousePoint)));
				size_t decimal_pos = distanceText.find(".");
				distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

				string headingText = to_string(ActivePosition.DirectionTo(ConvertCoordFromPixelToPosition(MousePoint)));
				decimal_pos = headingText.find(".");
				distanceText += " " + headingText.substr(0, decimal_pos + 2) + "°";

				POINT TextPositon = { MousePoint.x + 10, MousePoint.y };
				dc.TextOutA(TextPositon.x, TextPositon.y, distanceText.c_str());
			}

			RequestRefresh();
		}

		// 
		// Other tools
		//
		for (auto kv : SepToolPairs) {
			CRadarTarget FirstTarget = GetPlugIn()->RadarTargetSelect(kv.first.c_str());
			CRadarTarget SecondTarget = GetPlugIn()->RadarTargetSelect(kv.second.c_str());

			if (FirstTarget.IsValid() && SecondTarget.IsValid()) {
				CPosition FirstTargetPos = FirstTarget.GetPosition().GetPosition();
				CPosition SecondTargetPos = SecondTarget.GetPosition().GetPosition();

				POINT FirstPos = ConvertCoordFromPositionToPixel(FirstTargetPos);
				POINT SecondPos = ConvertCoordFromPositionToPixel(SecondTargetPos);

				dc.MoveTo(FirstPos);
				dc.LineTo(SecondPos);

				// First the distance tool

				string distanceText = to_string(FirstTargetPos.DistanceTo(SecondTargetPos));
				size_t decimal_pos = distanceText.find(".");
				distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

				string headingText = to_string(FirstTargetPos.DirectionTo(SecondTargetPos));
				decimal_pos = headingText.find(".");
				distanceText += " " + headingText.substr(0, decimal_pos + 2) + "°";

				POINT MidPointDistance = { (int)((FirstPos.x + SecondPos.x) / 2), (int)((FirstPos.y + SecondPos.y) / 2) };

				// We have to calculate the text angle 
				int lineheading = (int)fmod(FirstTargetPos.DirectionTo(SecondTargetPos) - 90, 360);
				int angle = (int)fmod(lineheading + 90.0f, 360.0f);
				if (FirstTargetPos.DirectionTo(SecondTargetPos) >= 180) {
					angle = (int)fmod(lineheading - 90, 360);
				}

				POINT TextPositon;
				TextPositon.x = long(MidPointDistance.x + float(20 * cos(DegToRad(angle))));
				TextPositon.y = long(MidPointDistance.y + float(20 * sin(DegToRad(angle))));

				dc.TextOutA(TextPositon.x, TextPositon.y, distanceText.c_str());

				CSize Measure = dc.GetTextExtent(distanceText.c_str());

				CRect AreaRemoveTool = { TextPositon.x, TextPositon.y, TextPositon.x + Measure.cx, TextPositon.y + Measure.cy };

				CPosition Prediction;
				CPosition PredictionOther;
				double lastDistance = FirstTargetPos.DistanceTo(SecondTargetPos);
				for (int i = 10; i < 60 * 45; i += 10) {
					Prediction = Extrapolate(FirstTargetPos, FirstTarget.GetTrackHeading(),
						FirstTarget.GetPosition().GetReportedGS()*0.514444*i);
					PredictionOther = Extrapolate(SecondTargetPos, SecondTarget.GetTrackHeading(),
						SecondTarget.GetPosition().GetReportedGS()*0.514444*i);

					// The distance started to increase, we passed the minimum point
					if (Prediction.DistanceTo(PredictionOther) > lastDistance) {
						if (lastDistance == FirstTargetPos.DistanceTo(SecondTargetPos))
							break;

						CPosition velocity = Extrapolate(Prediction, FirstTarget.GetTrackHeading(),
							FirstTarget.GetPosition().GetReportedGS()*0.514444*MenuBar::GetVelValueButtonPressed(ButtonsPressed));
						DrawHourGlassWithLeader(&dc, ConvertCoordFromPositionToPixel(Prediction),
							ConvertCoordFromPositionToPixel(velocity));

						velocity = Extrapolate(PredictionOther, SecondTarget.GetTrackHeading(),
							SecondTarget.GetPosition().GetReportedGS()*0.514444*MenuBar::GetVelValueButtonPressed(ButtonsPressed));
						DrawHourGlassWithLeader(&dc, ConvertCoordFromPositionToPixel(PredictionOther),
							ConvertCoordFromPositionToPixel(velocity));

						distanceText = to_string(Prediction.DistanceTo(PredictionOther));
						decimal_pos = distanceText.find(".");
						distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

						distanceText += " " + to_string((int)i / 60) + "'" + to_string((int)i % 60) + '"';

						dc.TextOutA(TextPositon.x, TextPositon.y + Measure.cy, distanceText.c_str());
						AreaRemoveTool.right = max(AreaRemoveTool.right, TextPositon.x + Measure.cx);
						AreaRemoveTool.bottom = TextPositon.y + Measure.cy * 2;

						break;
					}
						
					lastDistance = Prediction.DistanceTo(PredictionOther);
				}

				POINT ClipFrom, ClipTo;
				if (LiangBarsky(AreaRemoveTool, MidPointDistance, AreaRemoveTool.CenterPoint(), ClipFrom, ClipTo)) {
					CPen DashedPen(PS_DOT, 1, Colours::OrangeTool.ToCOLORREF());
					dc.SelectObject(&DashedPen);

					dc.MoveTo(MidPointDistance);
					dc.LineTo(ClipFrom);

					dc.SelectObject(&SepToolColorPen);
				}

				AddScreenObject(SCREEN_SEP_TOOL, string(kv.first + "," + kv.second).c_str(), AreaRemoveTool, false, "");
			}
		}

		dc.RestoreDC(saveTool);
	}

	for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
		radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
	{
		CFlightPlan CorrelatedFlightPlan = radarTarget.GetCorrelatedFlightPlan();
		CFlightPlan CheatFlightPlan = GetPlugIn()->FlightPlanSelect(radarTarget.GetCallsign());
		bool isCorrelated = CorrelatedFlightPlan.IsValid();
		int Altitude = radarTarget.GetPosition().GetFlightLevel();

		Tag::TagStates AcState = Tag::TagStates::NotConcerned;

		bool IsSoft = false;

		/// 
		/// Filtering
		///
		if (ButtonsPressed[BUTTON_FILTER_ON]) {

			// If not in between the hard filters, we don't show it.
			if (Altitude <= RadarFilters.Hard_Low)
				continue;

			if (Altitude >= RadarFilters.Hard_High)
				continue;

			// If Primary
			if (!radarTarget.GetPosition().GetTransponderC() && !radarTarget.GetPosition().GetTransponderI()
				&& !ButtonsPressed[BUTTON_PRIMARY_TARGETS_ON])
				continue;

			// If VFR
			if (radarTarget.GetPosition().GetSquawk() == "7000" && !ButtonsPressed[BUTTON_VFR_ON])
				continue;

			if (isCorrelated && CorrelatedFlightPlan.GetFlightPlanData().GetPlanType() == "V" && !ButtonsPressed[BUTTON_VFR_ON])
				continue;

			if (Altitude <= RadarFilters.Soft_Low || Altitude >= RadarFilters.Soft_High)
				IsSoft = true;
		}
			
		bool isDetailed = DetailedTag == radarTarget.GetCallsign();
		POINT radarTargetPoint = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

		// Determining the tag state
		if (isCorrelated) {
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_NOTIFIED)
				AcState = Tag::TagStates::InSequence;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_COORDINATED)
				AcState = Tag::TagStates::Next;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED)
				AcState = Tag::TagStates::TransferredToMe;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED)
				AcState = Tag::TagStates::TransferredFromMe;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_ASSUMED)
				AcState = Tag::TagStates::Assumed;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_REDUNDANT)
				AcState = Tag::TagStates::Redundant;
		}
		else {
			if (CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED || 
				CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED ||
				CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_ASSUMED ||
				CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_REDUNDANT)
				AcState = Tag::TagStates::Redundant;
		}

#pragma region BeforeTags

		if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
			CRect r = AcSymbols::DrawSquareAndTrail(&dc, AcState, this, radarTarget, ButtonsPressed[BUTTON_DOTS], IsSoft, isDetailed);
			AddScreenObject(SCREEN_AC_SYMBOL, radarTarget.GetCallsign(), r, false, "");
			AcSymbols::DrawSpeedVector(&dc, AcState, this, radarTarget, IsSoft, MenuBar::GetVelValueButtonPressed(ButtonsPressed));
		}

#pragma endregion

#pragma region AfterTags

		if (Phase == REFRESH_PHASE_AFTER_TAGS) {
			// Draw the tag

			Tag t = Tag(AcState, isDetailed, IsSoft, this, radarTarget, CheatFlightPlan);

			// Getting the tag center
			if (TagOffsets.find(radarTarget.GetCallsign()) == TagOffsets.end())
				TagOffsets[radarTarget.GetCallsign()] = { 25, -50 };

			map<int, CRect> DetailedTagData;

			RECT r = TagRenderer::Render(&dc, MousePoint, TagOffsets[radarTarget.GetCallsign()], radarTargetPoint, t, isDetailed, &DetailedTagData);

			RECT SymbolArea = { radarTargetPoint.x - DRAWING_AC_SQUARE_SYMBOL_SIZE, radarTargetPoint.y - DRAWING_AC_SQUARE_SYMBOL_SIZE,
				radarTargetPoint.x + DRAWING_AC_SQUARE_SYMBOL_SIZE, radarTargetPoint.y + DRAWING_AC_SQUARE_SYMBOL_SIZE };

			// We check if the tag is still detailed
			if ((!IsInRect(MousePoint, r) && !IsInRect(MousePoint, SymbolArea)) && DetailedTag == radarTarget.GetCallsign())
				DetailedTag = "";

			// We add the screen rect
			AddScreenObject(SCREEN_TAG, radarTarget.GetCallsign(), r, true, "");

			// If detailed we add the screen objects 
			if (isDetailed) {
				for (auto kv : DetailedTagData) {
					AddScreenObject(kv.first, radarTarget.GetCallsign(), kv.second, false, "");
				}
			}
		}

#pragma endregion
	}

	if (Phase == REFRESH_PHASE_AFTER_TAGS) {
		// Update certain menubar values first
		CPosition TopLeft, BottomRight;
		GetDisplayArea(&TopLeft, &BottomRight);
		int range = (int)TopLeft.DistanceTo(BottomRight);
		range = ((range + 10 / 2) / 10) * 10;
		MenuButtons[BUTTON_RANGE] = to_string(range) + " =";
		
		// Menubar
		MenuBar::DrawMenuBar(&dc, this, { RadarArea.left, RadarArea.top + 1 }, MousePoint, MenuButtons, ButtonsPressed);
	}

	// Releasing the handle
	dc.Detach();
}

void RadarScreen::OnMoveScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, bool Released)
{
	MousePoint = Pt;
	if (ObjectType == SCREEN_TAG) {
		POINT AcPosPix = ConvertCoordFromPositionToPixel(GetPlugIn()->RadarTargetSelect(sObjectId).GetPosition().GetPosition());
		POINT CustomTag = { Area.left - AcPosPix.x, Area.top - AcPosPix.y };
		TagOffsets[sObjectId] = CustomTag;
	}

	RequestRefresh();
}

void RadarScreen::OnOverScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area)
{
	if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_AC_SYMBOL) {
		DetailedTag = sObjectId;
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
	}
	
	MousePoint = Pt;
	RequestRefresh();
}

void RadarScreen::OnClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button)
{
	// Handle button menu click
	if (ObjectType >= BUTTON_HIDEMENU && ObjectType < BUTTON_DOTS && Button == BUTTON_LEFT) {
		if (ObjectType >= BUTTON_VEL1 && ObjectType <= BUTTON_VEL8 && !ButtonsPressed[ObjectType])
			ButtonsPressed = MenuBar::ResetAllVelButtons(ButtonsPressed);

		if (ButtonsPressed.find(ObjectType) == ButtonsPressed.end())
			ButtonsPressed[ObjectType] = true;
		else
			ButtonsPressed[ObjectType] = !ButtonsPressed[ObjectType];

		if (ObjectType >= BUTTON_FILTER_HARD_LOW && ObjectType <= BUTTON_FILTER_HARD_HIGH) {
			ButtonsPressed[ObjectType] = false;

			int Current = RadarFilters.Hard_High;
			if (ObjectType == BUTTON_FILTER_HARD_LOW)
				Current = RadarFilters.Hard_Low;
			if (ObjectType == BUTTON_FILTER_SOFT_LOW)
				Current = RadarFilters.Soft_Low;
			if (ObjectType == BUTTON_FILTER_SOFT_HIGH)
				Current = RadarFilters.Soft_High;

			// Show the lists
			GetPlugIn()->OpenPopupList(Area, "Filter", 1);
			FillInAltitudeList(GetPlugIn(), ObjectType, Current);
		}

		if (ObjectType == BUTTON_DECREASE_RANGE || ObjectType == BUTTON_INCREASE_RANGE) {
			ButtonsPressed[ObjectType] = false;

			CPosition TopLeft, BottomRight;
			GetDisplayArea(&TopLeft, &BottomRight);
			int range = (int)TopLeft.DistanceTo(BottomRight);
			range = ((range + 10 / 2) / 10) * 10;

			int bearing = (int)TopLeft.DirectionTo(BottomRight);

			if (ObjectType == BUTTON_DECREASE_RANGE)
				range -= 10;

			if (ObjectType == BUTTON_INCREASE_RANGE)
				range += 10;

			range = max(0, range);

			CPosition RightUp = Extrapolate(TopLeft, bearing, range * 1852);

			SetDisplayArea(TopLeft, RightUp);
		}
	}

	if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_AC_SYMBOL || ObjectType >= SCREEN_TAG_CALLSIGN) {
		if (AcquiringSepTool != "" && AcquiringSepTool != sObjectId) {
			SepToolPairs.insert(pair<string, string>(AcquiringSepTool, sObjectId));
			AcquiringSepTool = "";

			return;
		}
	}

	if (ObjectType == SCREEN_SEP_TOOL) {
		vector<string> s = split(sObjectId, ',');
		pair<string, string> toRemove = pair<string, string>(s.front(), s.back());

		typedef multimap<string, string>::iterator iterator;
		std::pair<iterator, iterator> iterpair = SepToolPairs.equal_range(toRemove.first);

		iterator it = iterpair.first;
		for (; it != iterpair.second; ++it) {
			if (it->second == toRemove.second) {
				it = SepToolPairs.erase(it);
				break;
			}
		}
	}

	// Tag clicks
	if (ObjectType >= SCREEN_TAG_CALLSIGN) {
		GetPlugIn()->SetASELAircraft(GetPlugIn()->FlightPlanSelect(sObjectId));
		
		CFlightPlan fp = GetPlugIn()->FlightPlanSelectASEL();

		int FunctionId = EuroScopePlugIn::TAG_ITEM_FUNCTION_NO;

		if (ObjectType == SCREEN_TAG_CALLSIGN) {
			if (Button == BUTTON_LEFT)
				FunctionId = TAG_ITEM_FUNCTION_HANDOFF_POPUP_MENU;
			if (Button == BUTTON_RIGHT)
				FunctionId = TAG_ITEM_FUNCTION_COMMUNICATION_POPUP;
		}

		if (ObjectType == SCREEN_TAG_SECTOR)
			FunctionId = TAG_ITEM_FUNCTION_ASSIGNED_NEXT_CONTROLLER;

		if (ObjectType == SCREEN_TAG_ROUTE)
			FunctionId = TAG_ITEM_FUNCTION_TOGGLE_PREDICTION_DRAW;

		if (ObjectType == SCREEN_TAG_CFL)
			FunctionId = TAG_ITEM_FUNCTION_TEMP_ALTITUDE_POPUP;

		if (ObjectType == SCREEN_TAG_HORIZ) {
			if (Button == BUTTON_LEFT)
				FunctionId = TAG_ITEM_FUNCTION_NEXT_ROUTE_POINTS_POPUP;
			if (Button == BUTTON_RIGHT)
				FunctionId = TAG_ITEM_FUNCTION_ASSIGNED_HEADING_POPUP;
		}
		
		if (ObjectType == SCREEN_TAG_RFL)
			FunctionId = TAG_ITEM_FUNCTION_RFL_POPUP;

		if (ObjectType == SCREEN_TAG_XFL) {
			if (fp.GetTrackingControllerIsMe()) {
				FunctionId = TAG_ITEM_FUNCTION_COPX_ALTITUDE;
			}
			else {
				FunctionId = TAG_ITEM_FUNCTION_COPN_ALTITUDE;
			}
		}

		if (ObjectType == SCREEN_TAG_COP) {
			if (fp.GetTrackingControllerIsMe()) {
				FunctionId = TAG_ITEM_FUNCTION_COPX_NAME;
			}
			else {
				FunctionId = TAG_ITEM_FUNCTION_COPN_NAME;
			}
		}

		if (ObjectType == SCREEN_TAG_SEP) {
			FunctionId = TAG_ITEM_FUNCTION_NO;

			if (AcquiringSepTool == sObjectId)
				AcquiringSepTool = "";
			else
				AcquiringSepTool = sObjectId;

		}

		StartTagFunction(sObjectId, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL, 
			FunctionId, Pt, Area);
	}

	MousePoint = Pt;
	RequestRefresh();
}

void RadarScreen::OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area)
{
	if (FunctionId == FUNC_FILTER_HARD_LOW) {
		RadarFilters.Hard_Low = atoi(sItemString) * 100;
		LoadFilterButtonsData();
	}
		
	if (FunctionId == FUNC_FILTER_SOFT_LOW) {
		RadarFilters.Soft_Low = atoi(sItemString) * 100;
		LoadFilterButtonsData();
	}

	if (FunctionId == FUNC_FILTER_SOFT_HIGH) {
		RadarFilters.Soft_High = atoi(sItemString) * 100;
		LoadFilterButtonsData();
	}
		
	if (FunctionId == FUNC_FILTER_HARD_HIGH) {
		RadarFilters.Hard_High = atoi(sItemString) * 100;
		LoadFilterButtonsData();
	}
}

void RadarScreen::OnDoubleClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button)
{
	if (ObjectType == SCREEN_TAG_CALLSIGN && Button == BUTTON_LEFT) {
		StartTagFunction(sObjectId, NULL, TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL, TAG_ITEM_FUNCTION_TAKE_HANDOFF, Pt, Area);
	}
}
