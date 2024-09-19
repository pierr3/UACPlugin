#include "stdafx.h"
#include "RadarScreen.h"

//ULONG_PTR m_gdiplusToken;

RadarScreen::RadarScreen()
{
	// Initialize GDI+
	//GdiplusStartupInput gdiplusStartupInput;
	//GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

	// Initialize the Menu Bar
	MenuButtons = MenuBar::MakeButtonData();
	StcaInstance = new CSTCA();
	MtcdInstance = new CMTCD();

	MTCDWindow = new CMTCDWindow({ 500, 200 });
	FIMWindow = new CFIMWindow({500, 500});

	OneSecondTimer = clock();
	HalfSecondTimer = clock();
	LoadAllData();
}

RadarScreen::~RadarScreen()
{
	//GdiplusShutdown(m_gdiplusToken);
}

void RadarScreen::LoadAllData()
{
	ButtonsPressed[BUTTON_VEL1] = true;
	ButtonsPressed[BUTTON_FILTER_ON] = true;
	ButtonsPressed[BUTTON_VELOTH] = true;
	ButtonsPressed[BUTTON_PRIMARY_TARGETS_ON] = true;
	ButtonsPressed[BUTTON_DOTS] = true;
	ButtonsPressed[BUTTON_MTCD] = true;
	ButtonsPressed[BUTTON_FIM] = true;

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

	// Creating the gdi+ graphics
	Graphics graphics(hDC);
	graphics.SetPageUnit(UnitPixel);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	CRect RadarArea(GetRadarArea());
	RadarArea.top = RadarArea.top - 1;
	RadarArea.bottom = GetChatArea().bottom;

	if (Phase == REFRESH_PHASE_BEFORE_TAGS) {

		// One second actions
		double t = (double)(clock() - OneSecondTimer) / ((double)CLOCKS_PER_SEC);
		if (t >= 1) {
			StcaInstance->OnRefresh(GetPlugIn());
			MtcdInstance->OnRefresh(GetPlugIn());
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



		FontManager::SelectStandardFont(&dc);

		// 
		// Starting with tools
		//

		CPen QDMToolPen(PS_SOLID, 1, Colours::BlueTool.ToCOLORREF());
		CPen SepToolColorPen(PS_SOLID, 1, Colours::OrangeTool.ToCOLORREF());

		//
		// Active VERA
		// 

		if (AcquiringSepTool != "") {
			dc.SelectObject(&SepToolColorPen);
			dc.SetTextColor(Colours::OrangeTool.ToCOLORREF());

			CPosition ActivePosition = GetPlugIn()->RadarTargetSelect(AcquiringSepTool.c_str()).GetPosition().GetPosition();
			
			if (GetPlugIn()->RadarTargetSelect(AcquiringSepTool.c_str()).GetPosition().IsValid()) {
				POINT ActivePositionPoint = ConvertCoordFromPositionToPixel(ActivePosition);

				dc.MoveTo(ActivePositionPoint);
				dc.LineTo(MousePoint);

				string headingText = padWithZeros(3, (int)ActivePosition.DirectionTo(ConvertCoordFromPixelToPosition(MousePoint)));
				
				string distanceText = to_string(ActivePosition.DistanceTo(ConvertCoordFromPixelToPosition(MousePoint)));
				size_t decimal_pos = distanceText.find(".");
				distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

				POINT TextPositon = { MousePoint.x + 15, MousePoint.y };
				dc.TextOutA(TextPositon.x, TextPositon.y, string(headingText + "/" + distanceText).c_str());

				CRect ellipsePointer = { MousePoint.x - 4, MousePoint.y - 4, MousePoint.x + 4, MousePoint.y + 4 };
				graphics.DrawEllipse(&Pen(Color::White), RectToGdiplus(ellipsePointer));
				ellipsePointer.InflateRect(4, 4);
				graphics.DrawEllipse(&Pen(Color::White), RectToGdiplus(ellipsePointer));
			}

			RequestRefresh();
		}

		//
		// Fixed QDM
		//

		if (FixedQDMTool != "") {
			dc.SelectObject(&QDMToolPen);
			dc.SetTextColor(Colours::BlueTool.ToCOLORREF());

			CPosition ActivePosition = GetPlugIn()->RadarTargetSelect(FixedQDMTool.c_str()).GetPosition().GetPosition();

			if (GetPlugIn()->RadarTargetSelect(FixedQDMTool.c_str()).GetPosition().IsValid()) {
				POINT ActivePositionPoint = ConvertCoordFromPositionToPixel(ActivePosition);

				dc.MoveTo(ActivePositionPoint);
				dc.LineTo(MousePoint);

				string headingText = padWithZeros(3, (int)ActivePosition.DirectionTo(ConvertCoordFromPixelToPosition(MousePoint)));

				string distanceText = to_string(ActivePosition.DistanceTo(ConvertCoordFromPixelToPosition(MousePoint)));
				size_t decimal_pos = distanceText.find(".");
				distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

				POINT TextPositon = { MousePoint.x + 15, MousePoint.y };
				dc.TextOutA(TextPositon.x, TextPositon.y, string(headingText + "/" + distanceText).c_str());
			}

			RequestRefresh();
		}

		//
		// Variable QDM
		//

		if (ButtonsPressed[BUTTON_QDM]) {
			dc.SelectObject(&QDMToolPen);
			dc.SetTextColor(Colours::BlueTool.ToCOLORREF());

			//
			// Add clickable background area
			//
			CRect R(GetRadarArea());
			R.top += 50;
			R.bottom = GetChatArea().top;

			R.NormalizeRect();
			AddScreenObject(SCREEN_BACKGROUND, "", R, false, "");

			if (VariableQDMAcquisition.x != 0 && VariableQDMAcquisition.y != 0) {
				CPosition SelectedFirstPosition = ConvertCoordFromPixelToPosition(VariableQDMAcquisition);

				dc.MoveTo(VariableQDMAcquisition);
				dc.LineTo(MousePoint);

				string headingText = padWithZeros(3, (int)SelectedFirstPosition.DirectionTo(ConvertCoordFromPixelToPosition(MousePoint)));

				string distanceText = to_string(SelectedFirstPosition.DistanceTo(ConvertCoordFromPixelToPosition(MousePoint)));
				size_t decimal_pos = distanceText.find(".");
				distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

				POINT TextPositon = { MousePoint.x + 15, MousePoint.y };
				dc.TextOutA(TextPositon.x, TextPositon.y, string(headingText + "/" + distanceText).c_str());
			}

			RequestRefresh();
		}


		// 
		// Existing VERA
		//
		
		dc.SelectObject(&SepToolColorPen);
		dc.SetTextColor(Colours::OrangeTool.ToCOLORREF());

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

				string headingText = padWithZeros(3, (int)FirstTargetPos.DirectionTo(SecondTargetPos));

				string distanceText = to_string(FirstTargetPos.DistanceTo(SecondTargetPos));
				size_t decimal_pos = distanceText.find(".");
				distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

				distanceText = headingText + "/" + distanceText;

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

				VERA::VERADataStruct vera = VERA::Calculate(FirstTarget, SecondTarget);

				if (vera.minDistanceNm != -1) {
					CPosition velocity = Extrapolate(vera.predictedFirstPos, FirstTarget.GetTrackHeading(),
						FirstTarget.GetPosition().GetReportedGS()*0.514444*MenuBar::GetVelValueButtonPressed(ButtonsPressed));
					DrawHourGlassWithLeader(&dc, ConvertCoordFromPositionToPixel(vera.predictedFirstPos),
						ConvertCoordFromPositionToPixel(velocity));

					velocity = Extrapolate(vera.predictedSecondPos, SecondTarget.GetTrackHeading(),
						SecondTarget.GetPosition().GetReportedGS()*0.514444*MenuBar::GetVelValueButtonPressed(ButtonsPressed));
					DrawHourGlassWithLeader(&dc, ConvertCoordFromPositionToPixel(vera.predictedSecondPos),
						ConvertCoordFromPositionToPixel(velocity));

					distanceText = to_string(vera.minDistanceNm);
					decimal_pos = distanceText.find(".");
					distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

					distanceText = to_string((int)vera.minDistanceSeconds / 60) + "'" + 
						to_string((int)vera.minDistanceSeconds % 60) + '"' + "/" + distanceText;
					dc.TextOutA(TextPositon.x, TextPositon.y + Measure.cy, distanceText.c_str());
				}

				AreaRemoveTool.right = max(AreaRemoveTool.right, TextPositon.x + Measure.cx);
				AreaRemoveTool.bottom = TextPositon.y + Measure.cy * 2;

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

		// 
		// Existing VariableQDM
		//
		
		dc.SelectObject(&QDMToolPen);
		dc.SetTextColor(Colours::BlueTool.ToCOLORREF());

		for (auto kv : VariableQDMs) {
			POINT FirstPositionPix = ConvertCoordFromPositionToPixel(kv.second.first);
			POINT SecondPositionPix = ConvertCoordFromPositionToPixel(kv.second.second);

			dc.MoveTo(FirstPositionPix);
			dc.LineTo(SecondPositionPix);

			string headingText = padWithZeros(3, (int)kv.second.first.DirectionTo(kv.second.second));

			string distanceText = to_string(kv.second.first.DistanceTo(kv.second.second));
			size_t decimal_pos = distanceText.find(".");
			distanceText = distanceText.substr(0, decimal_pos + 2) + "nm";

			POINT TextPositon = { SecondPositionPix.x + 15, SecondPositionPix.y };
			dc.TextOutA(TextPositon.x, TextPositon.y, string(headingText + "/" + distanceText).c_str());

			CSize Measure = dc.GetTextExtent(distanceText.c_str());
			CRect AreaRemoveTool = { TextPositon.x, TextPositon.y, TextPositon.x + Measure.cx, TextPositon.y + Measure.cy };

			AddScreenObject(SCREEN_QDM_TOOL, to_string(kv.first).c_str(), AreaRemoveTool, false, "");
		}


		//
		// Existing variable QDMs
		//


		dc.RestoreDC(saveTool);

		//
		// Here we check to see if there is a detailed tag
		//
		if (mouseOverTag.size() > 0) {

			// We check the mouse is still in the tag
			if (IsInRect(MousePoint, mouseOverArea)) {
				clock_t clock_final = clock() - mouseOverTagTimer;
				double delta_t = (double)clock_final / ((double)CLOCKS_PER_SEC);
				if (delta_t > 0.1) {
					DetailedTag = mouseOverTag;
					mouseOverTag = "";

					CRadarTarget rt = GetPlugIn()->RadarTargetSelect(DetailedTag.c_str());

					if (rt.IsValid() && rt.GetPosition().GetRadarFlags() > RADAR_POSITION_PRIMARY && rt.GetCorrelatedFlightPlan().IsValid()) {
						GetPlugIn()->SetASELAircraft(rt.GetCorrelatedFlightPlan());
					}
					else if (rt.IsValid()) {
						GetPlugIn()->SetASELAircraft(rt);
					}
				}
			}
			else {
				mouseOverTag = "";
			}

			// If we still have a mouseOverTag to handle, we refresh ASAP to determine it
			if (mouseOverTag.size() >0)
				RequestRefresh();
		}
	}

	for (CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelectFirst(); radarTarget.IsValid();
		radarTarget = GetPlugIn()->RadarTargetSelectNext(radarTarget))
	{
		CFlightPlan CorrelatedFlightPlan = radarTarget.GetCorrelatedFlightPlan();
		bool isCorrelated = CorrelatedFlightPlan.IsValid();
		int Altitude = radarTarget.GetPosition().GetFlightLevel();
		POINT radarTargetPoint = ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

		TagConfiguration::TagStates AcState = TagConfiguration::TagStates::NotConcerned;

		bool IsSoft = false;
		bool IsPrimary = !radarTarget.GetPosition().GetTransponderC() && !radarTarget.GetPosition().GetTransponderI();

		bool HideTarget = false;

		// If is below 60kts, don't show
		if (radarTarget.GetPosition().GetReportedGS() < 60)
			HideTarget = true;

		// 
		// Filtering
		//
		if (ButtonsPressed[BUTTON_FILTER_ON]) {

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
		
		// Determining the tag state
		if (isCorrelated) {
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_NOTIFIED)
				AcState = TagConfiguration::TagStates::InSequence;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_COORDINATED)
				AcState = TagConfiguration::TagStates::Next;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_TO_ME_INITIATED)
				AcState = TagConfiguration::TagStates::TransferredToMe;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_TRANSFER_FROM_ME_INITIATED)
				AcState = TagConfiguration::TagStates::TransferredFromMe;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_ASSUMED)
				AcState = TagConfiguration::TagStates::Assumed;
			if (CorrelatedFlightPlan.GetState() == FLIGHT_PLAN_STATE_REDUNDANT)
				AcState = TagConfiguration::TagStates::Redundant;
		}
		else{ 
			AcState = TagConfiguration::TagStates::Uncorrelated;
		}

		if (IsPrimary) {
			AcState = TagConfiguration::TagStates::NotConcerned;
			IsSoft = false;
		}

		// if in a state that needs to force filters
		if (AcState == TagConfiguration::TagStates::Redundant || AcState == TagConfiguration::TagStates::TransferredFromMe ||
			AcState == TagConfiguration::TagStates::TransferredToMe) {
			IsSoft = false;
			HideTarget = false;
		}

		// In topdown mode, assumed traffic can be hidden by filters to facilitate topdown control
		if (AcState == TagConfiguration::TagStates::Assumed && !ButtonsPressed[BUTTON_TOPDOWN]) {
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
				AddScreenObject(SCREEN_AC_SYMBOL, radarTarget.GetCallsign(), r, true, GetPlugIn()->FlightPlanSelect(radarTarget.GetCallsign()).GetPilotName());
			}
			else {
				CRect r = AcSymbols::DrawPrimaryTrailAndDiamong(&dc, this, radarTarget, ButtonsPressed[BUTTON_DOTS]);
				AddScreenObject(SCREEN_AC_SYMBOL, radarTarget.GetCallsign(), r, true, GetPlugIn()->FlightPlanSelect(radarTarget.GetCallsign()).GetPilotName());
			}

			if ((IsPrimary || !isCorrelated) && ButtonsPressed[BUTTON_VELOTH]) {
				AcSymbols::DrawSpeedVector(&dc, AcState, this, radarTarget, IsPrimary, IsSoft, MenuBar::GetVelValueButtonPressed(ButtonsPressed));
			}
			else if (!IsPrimary && isCorrelated) {
				AcSymbols::DrawSpeedVector(&dc, AcState, this, radarTarget, IsPrimary, IsSoft, MenuBar::GetVelValueButtonPressed(ButtonsPressed));
			}


			// If final approach help is toggled, display the vectors
			if (!IsPrimary && isCorrelated && ButtonsPressed[BUTTON_FIN]) {
				if (CorrelatedFlightPlan.GetControllerAssignedData().GetClearedAltitude() == 1) {
					bool existsInList = find(ExtendedAppVector.begin(), ExtendedAppVector.end(), string(radarTarget.GetCallsign())) != ExtendedAppVector.end();
					CRect r = AcSymbols::DrawApproachVector(&dc, this, radarTarget, existsInList ? 5 : 3);
					AddScreenObject(SCREEN_AC_APP_ARROW, radarTarget.GetCallsign(), r, true, "");
				}
			}
		}

#pragma endregion

#pragma region AfterTags

		if (Phase == REFRESH_PHASE_AFTER_TAGS) {
			// Draw the tag

			if (IsPrimary)
				continue;

			Tag t = Tag(AcState, isDetailed, IsSoft, ButtonsPressed[BUTTON_MODE_A], 
				ButtonsPressed[BUTTON_LABEL_V], ButtonsPressed[BUTTON_TOPDOWN], this, MtcdInstance, radarTarget, radarTarget.GetCorrelatedFlightPlan());

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

			// Check if route over is still drawn
			if (!IsInRect(MousePoint, RouteDisplayMouseOverArea) && RouteDisplayMouseOver == radarTarget.GetCallsign())
				RouteDisplayMouseOver = "";

			// Store the tag for tag deconfliction
			if (!isDetailed) {
				if (IsSoft)
					SoftTagAreas[radarTarget.GetCallsign()] = r;
				else
					TagAreas[radarTarget.GetCallsign()] = r;
			}
				

			// We add the screen rect
			AddScreenObject(SCREEN_TAG, radarTarget.GetCallsign(), r, true, GetPlugIn()->FlightPlanSelect(radarTarget.GetCallsign()).GetPilotName());

			// If detailed we add the screen objects 
			if (isDetailed) {
				for (auto kv : DetailedTagData) {
					const char* callsignToCallActions = radarTarget.GetCallsign();
					if (isCorrelated)
						callsignToCallActions = CorrelatedFlightPlan.GetCallsign();
					AddScreenObject(kv.first, callsignToCallActions, kv.second, false, GetPlugIn()->FlightPlanSelect(radarTarget.GetCallsign()).GetPilotName());
				}
			}

			// If the route is shown, then we display it
			if (find(RouteBeingShown.begin(), RouteBeingShown.end(), radarTarget.GetCallsign()) != RouteBeingShown.end()) {
				RouteRenderer::Render(&dc, this, radarTarget, CorrelatedFlightPlan);
			} else if (RouteDisplayMouseOver == radarTarget.GetCallsign()) {
				RouteRenderer::Render(&dc, this, radarTarget, CorrelatedFlightPlan);
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

		CRadarTarget fimRt = GetPlugIn()->RadarTargetSelectASEL();
		CFlightPlan fimFp = fimRt.GetCorrelatedFlightPlan();

		// FIM Window
		FIMWindow->Render(&dc, this, MousePoint, fimRt, fimFp, ButtonsPressed[BUTTON_FIM]);
		
		// MTCD Window
		MTCDWindow->Render(&dc, this, MousePoint, MtcdInstance, SepToolPairs, ButtonsPressed[BUTTON_MTCD]);

		// Soft Tag deconfliction
		/*for (const auto areas : SoftTagAreas)
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

			/POINT newOffset = AntiOverlap::Execute(this, SoftTagAreas, TagOffsets, MenuBar::GetVelValueButtonPressed(ButtonsPressed), rt);

			if (newOffset.x != TagOffsets[rt.GetCallsign()].x && newOffset.y != TagOffsets[rt.GetCallsign()].y) {
				TagOffsets[areas.first] = newOffset;
				SoftTagAreas[areas.first] = { newOffset.x, newOffset.y, newOffset.x + OriginalArea.Size().cx, newOffset.y + OriginalArea.Size().cy };
				RecentlyAutoMovedTags[areas.first] = clock();
			}
		}*/

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
			
			dc.RestoreDC(saveTest);*/
			// END TEST

			//POINT newOffset = AntiOverlap::Execute(this, TagAreas, TagOffsets, MenuBar::GetVelValueButtonPressed(ButtonsPressed), rt);

			/*if (newOffset.x != TagOffsets[rt.GetCallsign()].x && newOffset.y != TagOffsets[rt.GetCallsign()].y) {
				TagOffsets[areas.first] = newOffset;
				TagAreas[areas.first] = { newOffset.x, newOffset.y, newOffset.x + OriginalArea.Size().cx, newOffset.y + OriginalArea.Size().cy };
				RecentlyAutoMovedTags[areas.first] = clock();
			}*/
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
		if (!Released)
			DetailedTag = sObjectId;
	}

	if (ObjectType == SCREEN_AC_SYMBOL) {
		FixedQDMTool = sObjectId;
		if (Released)
			FixedQDMTool = "";
	}

	if (ObjectType == FIM_WINDOW) {
		FIMWindow->Move(Area, Released);
	}

	if (ObjectType == MTCD_WINDOW) {
		MTCDWindow->Move(Area, Released);
	}

	RequestRefresh();
}

void RadarScreen::OnOverScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area)
{
	if (ObjectType == SCREEN_TAG || ObjectType == SCREEN_AC_SYMBOL) {
		// We only select the aircraft after waiting for the mouse to be on the tag for a few milliseconds
		if (mouseOverTag != string(sObjectId) && sObjectId != DetailedTag) {
			mouseOverTag = sObjectId;
			mouseOverTagTimer = clock();
			mouseOverArea = Area;
		}
	}

	if (ObjectType == SCREEN_TAG_ROUTE) {
		RouteDisplayMouseOver = sObjectId;
		RouteDisplayMouseOverArea = Area;
	}
	
	MousePoint = Pt;
	RequestRefresh();
}

void RadarScreen::OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType)
{
}

void RadarScreen::OnClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button)
{
	// Handle button menu click
	if (ObjectType >= BUTTON_HIDEMENU && ObjectType <= BUTTON_FIN && Button == BUTTON_LEFT) {
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

		if (ObjectType == BUTTON_TOPDOWN) {
			ButtonsPressed[BUTTON_LABEL_V] = true;
			ButtonsPressed[BUTTON_VFR_ON] = true;
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
		CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelect(sObjectId);

		bool IsPrimary = !radarTarget.GetPosition().GetTransponderC() && !radarTarget.GetPosition().GetTransponderI();
		if (IsPrimary || !radarTarget.GetCorrelatedFlightPlan().IsValid())
			GetPlugIn()->SetASELAircraft(radarTarget);
		else
			GetPlugIn()->SetASELAircraft(radarTarget.GetCorrelatedFlightPlan());
		DetailedTag = sObjectId;
		if (AcquiringSepTool != "" && AcquiringSepTool != sObjectId) {
			SepToolPairs.insert(pair<string, string>(AcquiringSepTool, sObjectId));
			AcquiringSepTool = "";

			return;
		}
	}

	if (ObjectType == SCREEN_BACKGROUND) {
		if (Button == BUTTON_RIGHT) {
			ButtonsPressed[BUTTON_QDM] = false; // Disable QDM
			VariableQDMAcquisition = { 0, 0 };
		}
		else if (VariableQDMAcquisition.x == 0 && VariableQDMAcquisition.y == 0)
			VariableQDMAcquisition = Pt; // Add first selected QDM Point
		else {

			// Add QDM to list
			CurrentQDMId++;
			pair<CPosition, CPosition> Positions = pair<CPosition, CPosition>(ConvertCoordFromPixelToPosition(VariableQDMAcquisition), ConvertCoordFromPixelToPosition(Pt));
			VariableQDMs.insert(pair<int, pair<CPosition, CPosition>>(CurrentQDMId, Positions));
			VariableQDMAcquisition = { 0, 0 };
			ButtonsPressed[BUTTON_QDM] = false;
		}
			
	}

	if (ObjectType == SCREEN_AC_APP_ARROW) {
		if (find(ExtendedAppVector.begin(), ExtendedAppVector.end(), string(sObjectId)) != ExtendedAppVector.end())
			ExtendedAppVector.erase(find(ExtendedAppVector.begin(), ExtendedAppVector.end(), sObjectId));
		else
			ExtendedAppVector.push_back(string(sObjectId));
	}

	if (ObjectType == SCREEN_QDM_TOOL) {
		map<int, pair<CPosition, CPosition>>::iterator it = VariableQDMs.find(atoi(sObjectId));
		if (it != VariableQDMs.end())
			VariableQDMs.erase(it);
	}

	if (ObjectType == SCREEN_AC_SYMBOL) {
		DetailedTag = sObjectId;
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

	if (ObjectType == FIM_STAR || ObjectType == FIM_RWY || ObjectType == FIM_SCRATCHPAD) {
		CRadarTarget radarTarget = GetPlugIn()->RadarTargetSelect(sObjectId);

		bool IsPrimary = !radarTarget.GetPosition().GetTransponderC() && !radarTarget.GetPosition().GetTransponderI();
		if (IsPrimary || !radarTarget.GetCorrelatedFlightPlan().IsValid())
			GetPlugIn()->SetASELAircraft(radarTarget);
		else
			GetPlugIn()->SetASELAircraft(radarTarget.GetCorrelatedFlightPlan());
	}

	if (ObjectType == FIM_STAR) {
		StartTagFunction(sObjectId, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL,
			TAG_ITEM_FUNCTION_ASSIGNED_STAR, Pt, Area);
	}

	if (ObjectType == FIM_RWY) {
		StartTagFunction(sObjectId, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL,
			TAG_ITEM_FUNCTION_ASSIGNED_RUNWAY, Pt, Area);
	}

	if (ObjectType == MTCD_WINDOW_BUTTONS) {
		ButtonsPressed[BUTTON_MTCD] = false;
	}

	if (ObjectType == FIM_CLOSE) {
		ButtonsPressed[BUTTON_FIM] = false;
	}

	if (ObjectType == FIM_SCRATCHPAD) {
		StartTagFunction(sObjectId, NULL, EuroScopePlugIn::TAG_ITEM_TYPE_CALLSIGN, sObjectId, NULL,
			TAG_ITEM_FUNCTION_EDIT_SCRATCH_PAD, Pt, Area);
	}

	// Tag clicks
	if (ObjectType >= SCREEN_TAG_CALLSIGN) {
		
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
			
		if (ObjectType == SCREEN_TAG_CFL) {
			if (Button == BUTTON_LEFT)
				FunctionId = TAG_ITEM_FUNCTION_TEMP_ALTITUDE_POPUP;
			if (Button == BUTTON_RIGHT)
				FunctionId = TAG_ITEM_FUNCTION_OPEN_FP_DIALOG;
		}
			
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
			if (Button == BUTTON_LEFT) {
				if (fp.GetTrackingControllerIsMe()) {
					FunctionId = TAG_ITEM_FUNCTION_COPX_NAME;
				}
				else {
					FunctionId = TAG_ITEM_FUNCTION_COPN_NAME;
				}
			}
			else {
				FunctionId = TAG_ITEM_FUNCTION_EDIT_SCRATCH_PAD;
			}

		}

		if (ObjectType == SCREEN_TAG_SEP) {
			FunctionId = TAG_ITEM_FUNCTION_NO;

			if (AcquiringSepTool == sObjectId)
				AcquiringSepTool = "";
			else
				AcquiringSepTool = sObjectId;

		}

		if (ObjectType == SCREEN_TAG_SSR)
			FunctionId = TAG_ITEM_FUNCTION_SQUAWK_POPUP;

		if (ObjectType == SCREEN_TAG_RWY)
			FunctionId = TAG_ITEM_FUNCTION_ASSIGNED_RUNWAY;

		if (ObjectType == SCREEN_TAG_STAR)
			FunctionId = TAG_ITEM_FUNCTION_ASSIGNED_STAR;
		
		if (ObjectType == SCREEN_TAG_GSPEED || ObjectType == SCREEN_TAG_ADES)
			FunctionId = TAG_ITEM_FUNCTION_OPEN_FP_DIALOG;

		if (ObjectType == SCREEN_TAG_ASPEED) {
			if (Button == BUTTON_LEFT)
				FunctionId = TAG_ITEM_FUNCTION_ASSIGNED_SPEED_POPUP;
			if (Button == BUTTON_RIGHT)
				FunctionId = TAG_ITEM_FUNCTION_ASSIGNED_MACH_POPUP;
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

void RadarScreen::OnAsrContentToBeSaved() {
	// Saving to ASR all radar screen specific settings

	SaveDataToAsr(SAVE_MTCD_POSX.c_str(), "Position of the UAC MTCD Window", to_string(MTCDWindow->GetTopLeftPosition().x).c_str());
	SaveDataToAsr(SAVE_MTCD_POSY.c_str(), "Position of the UAC MTCD Window", to_string(MTCDWindow->GetTopLeftPosition().y).c_str());

	SaveDataToAsr(SAVE_FIM_POSX.c_str(), "Position of the UAC FIM Window", to_string(FIMWindow->GetTopLeftPosition().x).c_str());
	SaveDataToAsr(SAVE_FIM_POSY.c_str(), "Position of the UAC FIM Window", to_string(FIMWindow->GetTopLeftPosition().y).c_str());

	SaveDataToAsr(SAVE_HARD_FILTER_LOWER.c_str(), "UAC Filter settings", to_string(RadarFilters.Hard_Low).c_str());
	SaveDataToAsr(SAVE_SOFT_FILTER_LOWER.c_str(), "UAC Filter settings", to_string(RadarFilters.Soft_Low).c_str());
	SaveDataToAsr(SAVE_HARD_FILTER_UPPER.c_str(), "UAC Filter settings", to_string(RadarFilters.Hard_High).c_str()); 
	SaveDataToAsr(SAVE_SOFT_FILTER_UPPER.c_str(), "UAC Filter settings", to_string(RadarFilters.Soft_High).c_str());

	SaveDataToAsr(SAVE_VFR_FILTER.c_str(), "UAC Filter settings", ButtonsPressed[BUTTON_VFR_ON] ? "1" : "0");

	SaveDataToAsr(SAVE_VEL_TIME.c_str(), "UAC Velocity Leader Length", to_string(MenuBar::GetVelValueButtonPressed(ButtonsPressed)).c_str());
};

void RadarScreen::OnAsrContentLoaded(bool Loaded) {
	if (!Loaded)
		return;

	const char *j_value, *k_value;
	// Loading position of the MTCD window
	if ((j_value = GetDataFromAsr(SAVE_MTCD_POSX.c_str())) != NULL && (k_value = GetDataFromAsr(SAVE_MTCD_POSY.c_str())) != NULL)
		MTCDWindow->Move(CRect(atoi(j_value), atoi(k_value), 0, 0), true);

	// Loading position of the FIM window
	if ((j_value = GetDataFromAsr(SAVE_FIM_POSX.c_str())) != NULL && (k_value = GetDataFromAsr(SAVE_FIM_POSY.c_str())) != NULL)
		FIMWindow->Move(CRect(atoi(j_value), atoi(k_value), 0, 0), true);

	// Loading filter settings
	if ((j_value = GetDataFromAsr(SAVE_HARD_FILTER_LOWER.c_str())) != NULL)
		RadarFilters.Hard_Low = atoi(j_value);
	if ((j_value = GetDataFromAsr(SAVE_SOFT_FILTER_LOWER.c_str())) != NULL)
		RadarFilters.Soft_Low = atoi(j_value);
	if ((j_value = GetDataFromAsr(SAVE_HARD_FILTER_UPPER.c_str())) != NULL)
		RadarFilters.Hard_High = atoi(j_value);
	if ((j_value = GetDataFromAsr(SAVE_SOFT_FILTER_UPPER.c_str())) != NULL)
		RadarFilters.Soft_High = atoi(j_value);

	// Displayed the loaded values 
	LoadFilterButtonsData();

	// Loading vel value
	if ((j_value = GetDataFromAsr(SAVE_VEL_TIME.c_str())) != NULL)
		ButtonsPressed = MenuBar::LoadVelValueToButtons(atoi(j_value), ButtonsPressed);

	// Loading VFR filter
	if ((j_value = GetDataFromAsr(SAVE_VFR_FILTER.c_str())) != NULL)
		ButtonsPressed[BUTTON_VFR_ON] = (j_value == "1");
};
