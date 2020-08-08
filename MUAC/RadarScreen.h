#pragma once

#include <algorithm>
#include "Colours.h"
#include "Tag.h"
#include "FIMWindow.h"
#include "MTCDWindow.h"
#include "TagRenderer.h"
#include "Constants.h"
#include "AcSymbols.h"
#include "MenuBar.h"
#include "Helper.h"
#include "STCA.h"
#include "MTCD.h"
#include "VERA.h"
#include "AntiOverlap.h"
#include "RouteRenderer.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class RadarScreen : public CRadarScreen
{
public:
	RadarScreen();
	virtual ~RadarScreen();

	void LoadAllData();
	void LoadFilterButtonsData();

	void OnRefresh(HDC hDC, int Phase);
	void OnMoveScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, bool Released);
	void OnOverScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area);
	void OnFlightPlanControllerAssignedDataUpdate(CFlightPlan FlightPlan, int DataType);
	void OnClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button);
	void OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area);
	void OnDoubleClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button);
	void OnAsrContentToBeSaved(void);
	void OnAsrContentLoaded(bool Loaded);

	inline void OnAsrContentToBeClosed(void)
	{
		delete this;
	};

private:
	map<string, POINT> TagOffsets;
	map<int, string> MenuButtons;
	map<int, bool> ButtonsPressed;
	multimap<string, string> SepToolPairs;
	map<int, pair<CPosition, CPosition>> VariableQDMs;
	int CurrentQDMId = 1;
	POINT VariableQDMAcquisition = { 0, 0 };
	map<string, CRect> TagAreas;
	map<string, CRect> SoftTagAreas;
	map <string, clock_t> RecentlyAutoMovedTags;
	vector<string> RouteBeingShown;
	vector<string> ExtendedAppVector;
	POINT MousePoint = { 0, 0 };
	CSTCA * StcaInstance;
	CMTCD * MtcdInstance;
	CMTCDWindow* MTCDWindow;
	CFIMWindow* FIMWindow;
	string DetailedTag = "";
	string mouseOverTag = "";
	clock_t mouseOverTagTimer;
	CRect mouseOverArea;
	clock_t OneSecondTimer;
	clock_t HalfSecondTimer;
	bool Blink = false;

	string AcquiringSepTool = "";
	string FixedQDMTool = "";
	string RouteDisplayMouseOver = "";
	CRect RouteDisplayMouseOverArea;

	struct RadarFiltersStruct {
		int Hard_Low = 0;
		int Soft_Low = 20000;
		int Soft_High = 99999;
		int Hard_High = 99999;
	} RadarFilters;

};

