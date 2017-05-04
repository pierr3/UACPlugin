#include "stdafx.h"
#include "RadarScreen.h"

RadarScreen::RadarScreen()
{
	// Initialize the Menu Bar
	MenuButtons = MenuBar::MakeButtonData();
	StcaInstance = new CSTCA();
	MtcdInstance = new CMTCD();

	OneSecondTimer = clock();
	HalfSecondTimer = clock();
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

		// One second actions
		double t = (double)(clock() - OneSecondTimer) / ((double)CLOCKS_PER_SEC);
		if (t >= 1) {
			StcaInstance->OnRefresh(GetPlugIn());
			//MtcdInstance->OnRefresh(GetPlugIn());
			OneSecondTimer = clock();
		}

		t = (double)(clock() - HalfSecondTimer) / ((double)CLOCKS_PER_SEC);
		if (t >= 0.5) {
			Blink = !Blink;
			HalfSecondTimer = clock();
		}
		//

		int saveTool = dc.SaveDC();

		TagAreas.clear();
		SoftTagAreas.clear();

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

		bool HideTarget = false;
		/// 
		/// Filtering
		///
		if (ButtonsPressed[BUTTON_FILTER_ON]) {

			// If is below 60kts, don't show
			if (radarTarget.GetPosition().GetReportedGS() < 60)
				HideTarget = true;

			// If not in between the hard filters, we don't show it.
			if (Altitude <= RadarFilters.Hard_Low && !IsPrimary)
				HideTarget = true;

			if (Altitude >= RadarFilters.Hard_High && !IsPrimary)
				HideTarget = true;

			// If Primary
			if (IsPrimary && !ButtonsPressed[BUTTON_PRIMARY_TARGETS_ON])
				HideTarget = true;

			// If VFR
			if (startsWith("7000", radarTarget.GetPosition().GetSquawk()) && !ButtonsPressed[BUTTON_VFR_ON] && !IsPrimary)
				HideTarget = true;

			if (isCorrelated && startsWith("V", CorrelatedFlightPlan.GetFlightPlanData().GetPlanType()) && !ButtonsPressed[BUTTON_VFR_ON] && !IsPrimary)
				HideTarget = true;

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

		// if in a state that needs to force filters
		if (AcState == Tag::TagStates::Redundant || AcState == Tag::TagStates::TransferredFromMe || 
			AcState == Tag::TagStates::TransferredToMe || AcState == Tag::TagStates::Assumed) {
			IsSoft = false;
			HideTarget = false;
		}

		//
		// Final decision
		//
		if (HideTarget)
			continue;

#pragma region BeforeTags

		if (Phase == REFRESH_PHASE_BEFORE_TAGS) {
			if (!IsPrimary) {

				CRect r = AcSymbols::DrawSquareAndTrail(&dc, AcState, this, radarTarget, ButtonsPressed[BUTTON_DOTS], 
					IsSoft, StcaInstance->IsSTCA(radarTarget.GetCallsign()), Blink, isDetailed);
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

			Tag t = Tag(AcState, isDetailed, IsSoft, ButtonsPressed[BUTTON_MODE_A], 
				ButtonsPressed[BUTTON_LABEL_V], this, radarTarget, CheatFlightPlan);

			// Getting the tag center
			if (TagOffsets.find(radarTarget.GetCallsign()) == TagOffsets.end())
				TagOffsets[radarTarget.GetCallsign()] = { 25, -50 };

			map<int, CRect> DetailedTagData;

			RECT r = TagRenderer::Render(&dc, MousePoint, TagOffsets[radarTarget.GetCallsign()], radarTargetPoint, 
				t, isDetailed, StcaInstance->IsSTCA(radarTarget.GetCallsign()), MtcdInstance->IsMTCD(radarTarget.GetCallsign()),
				&DetailedTagData);

			RECT SymbolArea = { radarTargetPoint.x - DRAWING_AC_SQUARE_SYMBOL_SIZE, radarTargetPoint.y - DRAWING_AC_SQUARE_SYMBOL_SIZE,
				radarTargetPoint.x + DRAWING_AC_SQUARE_SYMBOL_SIZE, radarTargetPoint.y + DRAWING_AC_SQUARE_SYMBOL_SIZE };

			// We check if the tag is still detailed
			if ((!IsInRect(MousePoint, r) && !IsInRect(MousePoint, SymbolArea)) && DetailedTag == radarTarget.GetCallsign())
				DetailedTag = "";

			// Store the tag for tag deconfliction
			if (!isDetailed) {
				if (IsSoft)
					SoftTagAreas[radarTarget.GetCallsign()] = r;
				else
					TagAreas[radarTarget.GetCallsign()] = r;
			}
				

			// We add the screen rect
			AddScreenObject(SCREEN_TAG, radarTarget.GetCallsign(), r, true, "");

			// If detailed we add the screen objects 
			if (isDetailed) {
				for (auto kv : DetailedTagData) {
					AddScreenObject(kv.first, radarTarget.GetCallsign(), kv.second, false, "");
				}
			}

			// If the route is shown, then we display it
			if (find(RouteBeingShown.begin(), RouteBeingShown.end(), radarTarget.GetCallsign()) != RouteBeingShown.end()) {
				int SaveRoute = dc.SaveDC();
				
				CFlightPlanExtractedRoute exR = CheatFlightPlan.GetExtractedRoute();
				int index = exR.GetPointsAssignedIndex();
				if (index < 0)
					index = exR.GetPointsCalculatedIndex();

				bool isLastTextOnLeftSide = false;

				for (int i = index; i < exR.GetPointsNumber(); i++)
				{
					
					POINT exRPos = ConvertCoordFromPositionToPixel(exR.GetPointPosition(i));

					if (exR.GetPointDistanceInMinutes(i) == -1) {
						continue;
					}

					Color routeColor = Colours::AirwayColors;

					if (exR.GetPointAirwayClassification(i) == AIRWAY_CLASS_DIRECTION_ERROR ||
						exR.GetPointAirwayClassification(i) == AIRWAY_CLASS_UNCONNECTED) {
						routeColor = Gdiplus::Color::Red;
					}

					CPen routePen(PS_SOLID, 1, routeColor.ToCOLORREF());
					CPen* oldPen = dc.SelectObject(&routePen);
					dc.SetTextColor(routeColor.ToCOLORREF());

					bool isTextOnLeftSide = false;

					if (i == index)
					{
						dc.MoveTo(radarTargetPoint);
						dc.LineTo(exRPos);
						if (radarTarget.GetPosition().GetPosition().DistanceTo(exR.GetPointPosition(i)) < 10)
						{
							if (!isLastTextOnLeftSide)
							{
								isTextOnLeftSide = true;
								isLastTextOnLeftSide = true;
							}
						}
					}
					else
					{
						dc.MoveTo(ConvertCoordFromPositionToPixel(exR.GetPointPosition(i - 1)));
						dc.LineTo(exRPos);

						if (exR.GetPointPosition(i).DistanceTo(exR.GetPointPosition(i - 1)) < 10)
						{
							if (!isLastTextOnLeftSide)
							{
								isTextOnLeftSide = true;
								isLastTextOnLeftSide = true;
							}
						}
					}

					if (!isTextOnLeftSide)
						isLastTextOnLeftSide = false;

					// Selecting top or left right

					bool isTextTop = false;
					if (i == index)
					{
						int br = (int)radarTarget.GetPosition().GetPosition().DirectionTo(exR.GetPointPosition(i));
						if ((br > 70 && br < 110) || (br > 250 && br < 290))
						{
							isTextTop = true;
						}
					}
					else
					{
						int br = (int)exR.GetPointPosition(i - 1).DirectionTo(exR.GetPointPosition(i));
						if ((br > 70 && br < 110) || (br > 250 && br < 290))
						{
							isTextTop = true;
						}
					}

					dc.MoveTo(exRPos.x, exRPos.y - 3);
					dc.LineTo(exRPos.x - 3, exRPos.y + 3);
					dc.LineTo(exRPos.x + 3, exRPos.y + 3);
					dc.LineTo(exRPos.x, exRPos.y - 3);
					string actions = CheatFlightPlan.GetCallsign();
					actions += ",";
					actions += std::to_string(i);

					string pointName = exR.GetPointName(i);
					if (pointName == string(CheatFlightPlan.GetControllerAssignedData().GetDirectToPointName()))
					{
						pointName = "->" + pointName;
					}

					string temp = "+" + to_string(exR.GetPointDistanceInMinutes(i));
					temp += "' ";

					// Converting the profile altitude to FL/Altitude

					int fl = exR.GetPointCalculatedProfileAltitude(i);
					if (fl <= GetPlugIn()->GetTransitionAltitude()) {
						fl = exR.GetPointCalculatedProfileAltitude(i);
						temp += padWithZeros(std::to_string(GetPlugIn()->GetTransitionAltitude()).size(), fl);
					}
					else
					{
						temp += "FL" + padWithZeros(5, fl).substr(0, 3);
					}

					if (isTextOnLeftSide)
					{
						if (isTextTop)
						{
							dc.SetTextAlign(TA_LEFT | TA_BASELINE);
							dc.TextOutA(exRPos.x, exRPos.y - 25, pointName.c_str());
							dc.TextOutA(exRPos.x, exRPos.y - 10, temp.c_str());
							dc.MoveTo(exRPos);
							dc.LineTo({ exRPos.x, exRPos.y - 10 });
						}
						else
						{
							dc.SetTextAlign(TA_RIGHT | TA_BASELINE);
							dc.TextOutA(exRPos.x - 15, exRPos.y, pointName.c_str());
							dc.TextOutA(exRPos.x - 15, exRPos.y + 10, temp.c_str());
							dc.MoveTo(exRPos);
							dc.LineTo({ exRPos.x - 15, exRPos.y });
						}

					}
					else
					{
						if (isTextTop)
						{
							dc.SetTextAlign(TA_LEFT | TA_TOP);
							dc.TextOutA(exRPos.x, exRPos.y + 10, pointName.c_str());
							dc.TextOutA(exRPos.x, exRPos.y + 25, temp.c_str());
							dc.MoveTo(exRPos);
							dc.LineTo({ exRPos.x, exRPos.y + 10 });
						}
						else
						{
							dc.SetTextAlign(TA_BASELINE | TA_LEFT);
							dc.TextOutA(exRPos.x + 5, exRPos.y + 10, pointName.c_str());
							dc.TextOutA(exRPos.x + 5, exRPos.y + 25, temp.c_str());
							dc.MoveTo(exRPos);
							dc.LineTo({ exRPos.x + 5, exRPos.y });
						}

					}

					dc.SelectObject(&oldPen);

				}

				dc.RestoreDC(SaveRoute);
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

		// Soft Tag deconfliction
		for (const auto areas : SoftTagAreas)
		{
			if (areas.first == DetailedTag)
				continue;

			CRadarTarget rt = GetPlugIn()->RadarTargetSelect(areas.first.c_str());
			POINT AcPosition = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());

			// If the tag has recently been automatically moved, then we don't move it
			if (RecentlyAutoMovedTags.find(areas.first) != RecentlyAutoMovedTags.end())
			{
				double t = (double)(clock() - RecentlyAutoMovedTags[areas.first]) / ((double)CLOCKS_PER_SEC);
				if (t >= 4)
					RecentlyAutoMovedTags.erase(areas.first);
				else
					continue;
			}

			CRect OriginalArea = areas.second;

			POINT newOffset = AntiOverlap::Execute(this, SoftTagAreas, TagOffsets, MenuBar::GetVelValueButtonPressed(ButtonsPressed), rt);

			if (newOffset.x != TagOffsets[rt.GetCallsign()].x && newOffset.y != TagOffsets[rt.GetCallsign()].y) {
				TagOffsets[areas.first] = newOffset;
				SoftTagAreas[areas.first] = { newOffset.x, newOffset.y, newOffset.x + OriginalArea.Size().cx, newOffset.y + OriginalArea.Size().cy };
				RecentlyAutoMovedTags[areas.first] = clock();
			}
		}

		// Tag deconfliction
		for (const auto areas : TagAreas)
		{
			if (areas.first == DetailedTag)
				continue;

			CRadarTarget rt = GetPlugIn()->RadarTargetSelect(areas.first.c_str());
			POINT AcPosition = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());

			// If the tag has recently been automatically moved, then we don't move it
			if (RecentlyAutoMovedTags.find(areas.first) != RecentlyAutoMovedTags.end())
			{
				double t = (double)(clock() - RecentlyAutoMovedTags[areas.first]) / ((double)CLOCKS_PER_SEC);
				if (t >= 4)
					RecentlyAutoMovedTags.erase(areas.first);
				else
					continue;
			}

			CRect OriginalArea = areas.second;
			//
			// TEST
			//
			
			/*int saveTest = dc.SaveDC();

			CPen PurplePen(PS_SOLID, 1, Colours::PurpleDisplay.ToCOLORREF());
			dc.SelectObject(&PurplePen);
			dc.SelectStockObject(NULL_BRUSH);
			dc.SetTextColor(Colours::PurpleDisplay.ToCOLORREF());
			
			int TileWidth = OriginalArea.Size().cx + (int)(OriginalArea.Size().cx*0.20);
			int TileHeight = OriginalArea.Size().cy + (int)(OriginalArea.Size().cy*0.05);

			map<int, CRect> testingGrid = AntiOverlap::BuildGrid(AcPosition, TileWidth, TileHeight);
			map<int, int> gridCost = AntiOverlap::CalculateGridCost(this, TagAreas, TagOffsets, testingGrid, MenuBar::GetVelValueButtonPressed(ButtonsPressed), rt);

			for (auto testGridkv : testingGrid) {
				dc.Rectangle(testGridkv.second);
				string str = to_string(testGridkv.first);
				str += " " +to_string(gridCost[testGridkv.first]);

				dc.TextOutA(testGridkv.second.left, testGridkv.second.top, str.c_str());
			}
			

			dc.RestoreDC(saveTest); */
			// END TEST

			POINT newOffset = AntiOverlap::Execute(this, TagAreas, TagOffsets, MenuBar::GetVelValueButtonPressed(ButtonsPressed), rt);

			if (newOffset.x != TagOffsets[rt.GetCallsign()].x && newOffset.y != TagOffsets[rt.GetCallsign()].y) {
				TagOffsets[areas.first] = newOffset;
				TagAreas[areas.first] = { newOffset.x, newOffset.y, newOffset.x + OriginalArea.Size().cx, newOffset.y + OriginalArea.Size().cy };
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
	if (ObjectType >= BUTTON_HIDEMENU && ObjectType <= BUTTON_DOTS && Button == BUTTON_LEFT) {
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

	if (ObjectType == SCREEN_AC_SYMBOL) {
		CRadarTarget rt = GetPlugIn()->RadarTargetSelect(sObjectId);
		POINT AcPosition = ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());

		int DistanceBetweenTag = (int)sqrt(pow(TagOffsets[sObjectId].x, 2) +
			pow(TagOffsets[sObjectId].y, 2));

		double angle = RadToDeg(atan2(TagOffsets[sObjectId].y, TagOffsets[sObjectId].x));

		if (Button == BUTTON_LEFT)
			angle = fmod(angle - 30, 360);
		if (Button == BUTTON_RIGHT)
			angle = fmod(angle + 30, 360);

		POINT TopLeftTag;
		TopLeftTag.x = long(AcPosition.x + float(DistanceBetweenTag * cos(DegToRad(angle))));
		TopLeftTag.y = long(AcPosition.y + float(DistanceBetweenTag * sin(DegToRad(angle))));

		TagOffsets[sObjectId] = { TopLeftTag.x-AcPosition.x, TopLeftTag.y - AcPosition.y };
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

		if (ObjectType == SCREEN_TAG_ROUTE) {
			if (find(RouteBeingShown.begin(), RouteBeingShown.end(), sObjectId) != RouteBeingShown.end()) {
				RouteBeingShown.erase(find(RouteBeingShown.begin(), RouteBeingShown.end(), sObjectId));
			}
			else {
				RouteBeingShown.push_back(sObjectId);
			}
		}
			

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

		if (ObjectType == SCREEN_TAG_WARNING)
			FunctionId = TAG_ITEM_FUNCTION_SQUAWK_POPUP;

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
