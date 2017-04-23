#include "stdafx.h"
#include "RadarScreen.h"

POINT MousePoint = { 0, 0 };
string DetailedTag = "";

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
			if ((radarTarget.GetPosition().GetSquawk() == "7000" ||
				(isCorrelated && CorrelatedFlightPlan.GetFlightPlanData().GetPlanType() == "V")) && !ButtonsPressed[BUTTON_VFR_ON])
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
	if (ObjectType >= 400 && Button == BUTTON_LEFT) {
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

			CPosition RightUp = BetterHarversine(TopLeft, bearing, range * 1852);

			SetDisplayArea(TopLeft, RightUp);
		}
	}

	// Tag clicks
	if (ObjectType >= 700) {
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
