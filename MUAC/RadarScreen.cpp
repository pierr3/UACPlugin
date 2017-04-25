#include "stdafx.h"
#include "RadarScreen.h"

RadarScreen::RadarScreen()
{
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

				//
				// TESTING
				//
				/*CPen PurplePen(PS_SOLID, 2, Colours::PurpleDisplay.ToCOLORREF());
				CPen* oldObject = (CPen*)dc.SelectObject(&PurplePen);
				COLORREF oldColor = dc.SetTextColor(Colours::PurpleDisplay.ToCOLORREF());

				double NORTH = fmod(0 - 90, 360);
				double SOUTH = fmod(180 - 90, 360);
				double EAST = fmod(90 - 90, 360);
				double WEST = fmod(270 - 90, 360);

				int LENGHT = 100;
				POINT p = { 0, 0 };

				p.x = long(MidPointDistance.x + float(LENGHT * cos(DegToRad(NORTH))));
				p.y = long(MidPointDistance.y + float(LENGHT * sin(DegToRad(NORTH))));
				dc.MoveTo(MidPointDistance);
				dc.LineTo(p);
				dc.TextOutA(p.x, p.y, "N");

				p.x = long(MidPointDistance.x + float(LENGHT * cos(DegToRad(EAST))));
				p.y = long(MidPointDistance.y + float(LENGHT * sin(DegToRad(EAST))));
				dc.MoveTo(MidPointDistance);
				dc.LineTo(p);
				dc.TextOutA(p.x, p.y, "E");

				p.x = long(MidPointDistance.x + float(LENGHT * cos(DegToRad(SOUTH))));
				p.y = long(MidPointDistance.y + float(LENGHT * sin(DegToRad(SOUTH))));
				dc.MoveTo(MidPointDistance);
				dc.LineTo(p);
				dc.TextOutA(p.x, p.y, "S");

				p.x = long(MidPointDistance.x + float(LENGHT * cos(DegToRad(WEST))));
				p.y = long(MidPointDistance.y + float(LENGHT * sin(DegToRad(WEST))));
				dc.MoveTo(MidPointDistance);
				dc.LineTo(p);
				dc.TextOutA(p.x, p.y, "W");

				dc.SetTextColor(oldColor);
				dc.SelectObject(oldObject);
				//
				//
				//
				*/

				CSize Measure = dc.GetTextExtent(distanceText.c_str());

				// We have to calculate the text angle 
				double AdjustedLineHeading = fmod(FirstTargetPos.DirectionTo(SecondTargetPos) - 90.0, 360.0);
				double NewAngle = fmod(AdjustedLineHeading + 90, 360);

				if (FirstTargetPos.DirectionTo(SecondTargetPos) > 180.0) {
					NewAngle = fmod(AdjustedLineHeading - 90, 360);
				}

				POINT TextPositon;
				TextPositon.x = long(MidPointDistance.x + float((20+Measure.cx) * cos(DegToRad(NewAngle))));
				TextPositon.y = long(MidPointDistance.y + float((20)* sin(DegToRad(NewAngle))));

				TextPositon.x -= Measure.cx / 2;

				dc.TextOutA(TextPositon.x, TextPositon.y, distanceText.c_str());

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
		bool IsPrimary = !radarTarget.GetPosition().GetTransponderC() && !radarTarget.GetPosition().GetTransponderI();

		/// 
		/// Filtering
		///
		if (ButtonsPressed[BUTTON_FILTER_ON]) {

			// If is below 60kts, don't show
			if (radarTarget.GetPosition().GetReportedGS() < 60)
				continue;

			// If not in between the hard filters, we don't show it.
			if (Altitude <= RadarFilters.Hard_Low && !IsPrimary)
				continue;

			if (Altitude >= RadarFilters.Hard_High && !IsPrimary)
				continue;

			// If Primary
			if (IsPrimary && !ButtonsPressed[BUTTON_PRIMARY_TARGETS_ON])
				continue;

			// If VFR
			if (startsWith("7000", radarTarget.GetPosition().GetSquawk()) && !ButtonsPressed[BUTTON_VFR_ON] && !IsPrimary)
				continue;

			if (isCorrelated && startsWith("V", CorrelatedFlightPlan.GetFlightPlanData().GetPlanType()) && !ButtonsPressed[BUTTON_VFR_ON] && !IsPrimary)
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
		else{ 
			if (CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED || 
				CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED ||
				CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_ASSUMED ||
				CheatFlightPlan.GetState() == FLIGHT_PLAN_STATE_REDUNDANT)
				AcState = Tag::TagStates::Redundant;
		}

		if (IsPrimary) {
			AcState = Tag::TagStates::NotConcerned;
			IsSoft = false;
		}

#pragma region BeforeTags

		if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
			if (!IsPrimary) {
				CRect r = AcSymbols::DrawSquareAndTrail(&dc, AcState, this, radarTarget, ButtonsPressed[BUTTON_DOTS], IsSoft, isDetailed);
				AddScreenObject(SCREEN_AC_SYMBOL, radarTarget.GetCallsign(), r, false, "");
			}
			else {
				AcSymbols::DrawPrimaryTrailAndDiamong(&dc, this, radarTarget, ButtonsPressed[BUTTON_DOTS]);
			}

			if ((IsPrimary || !isCorrelated) && ButtonsPressed[BUTTON_VELOTH]) {
				AcSymbols::DrawSpeedVector(&dc, AcState, this, radarTarget, IsPrimary, IsSoft, MenuBar::GetVelValueButtonPressed(ButtonsPressed));
			}
			else if (!IsPrimary && isCorrelated) {
				AcSymbols::DrawSpeedVector(&dc, AcState, this, radarTarget, IsPrimary, IsSoft, MenuBar::GetVelValueButtonPressed(ButtonsPressed));
			}
		}

#pragma endregion

#pragma region AfterTags

		if (Phase == REFRESH_PHASE_AFTER_TAGS) {
			// Draw the tag

			if (IsPrimary)
				continue;

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

			// Store the tag for tag deconfliction
			TagAreas[radarTarget.GetCallsign()] = r;

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

		// Tag deconfliction
		for (const auto areas : TagAreas)
		{
			if (areas.first == DetailedTag)
				continue;

			CRadarTarget rt = GetPlugIn()->RadarTargetSelect(areas.first.c_str());
			POINT AcPosition = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());

			int DistanceBetweenTag = (int)sqrt(pow(TagOffsets[areas.first.c_str()].x - AcPosition.x, 2) +
				pow(TagOffsets[areas.first.c_str()].y - AcPosition.y, 2));

			// If the tag has recently been automatically moved, then we don't move it
			if (RecentlyAutoMovedTags.find(areas.first) != RecentlyAutoMovedTags.end())
			{
				double t = (double)clock() - RecentlyAutoMovedTags[areas.first] / ((double)CLOCKS_PER_SEC);
				if (t >= 1)
					RecentlyAutoMovedTags.erase(areas.first);
				else
					continue;
			}

			CRect OriginalArea = areas.second;

			CRect TestArea = OriginalArea;
			int options = 0;
			bool IsConflicting = true;

			double AdjustedLineHeading = fmod(rt.GetTrackHeading() - 90.0, 360.0);
			while (IsConflicting && options <= 10) {

				POINT TopLeftTag;
				double NewAngle = 0;
				// We run through the options
				if (options == 1) {
					NewAngle = fmod(AdjustedLineHeading + 30, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 2) {
					NewAngle = fmod(AdjustedLineHeading + 60, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 3) {
					NewAngle = fmod(AdjustedLineHeading + 90, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 4) {
					NewAngle = fmod(AdjustedLineHeading + 120, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 5) {
					NewAngle = fmod(AdjustedLineHeading + 130, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 6) {
					NewAngle = fmod(AdjustedLineHeading - 30, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 7) {
					NewAngle = fmod(AdjustedLineHeading - 60, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 8) {
					NewAngle = fmod(AdjustedLineHeading - 90, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 9) {
					NewAngle = fmod(AdjustedLineHeading - 120, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}
				if (options == 10) {
					NewAngle = fmod(AdjustedLineHeading - 130, 360);
					TopLeftTag.x = long(AcPosition.x + float(40 * cos(DegToRad(NewAngle))));
					TopLeftTag.y = long(AcPosition.y + float(40 * sin(DegToRad(NewAngle))));
					TestArea = { TopLeftTag.x, TopLeftTag.y, TopLeftTag.x + areas.second.right, TopLeftTag.y + areas.second.bottom };
				}

				IsConflicting = false;
				// We check for conflicts
				for (auto kv : TagAreas) {
					if (kv.first == areas.first)
						continue;

					if (kv.first == DetailedTag)
						continue;

					CRect h;
					if (h.IntersectRect(TestArea, kv.second)) {
						IsConflicting = true;
						break;
					}
				}

				options++;
			}

			// If the tag has been moved
			if (options != 0) {
				TagOffsets[areas.first] = { TestArea.left - AcPosition.x, TestArea.top - AcPosition.y };
				TagAreas[areas.first] = TestArea;
				RecentlyAutoMovedTags[areas.first] = clock();
			}
		}
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

		if (FunctionId != TAG_ITEM_FUNCTION_NO) {
			StartTagFunction(sObjectId, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL,
				FunctionId, Pt, Area);
		}
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
