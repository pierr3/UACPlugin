#pragma once

#include <algorithm>
#include "Colours.h"
#include "Tag.h"
#include "TagRenderer.h"
#include "Constants.h"
#include "AcSymbols.h"
#include "MenuBar.h"
#include "Helper.h"
#include "STCA.h"
#include "MTCD.h"
#include "AntiOverlap.h"
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
	void OnClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button);
	void OnFunctionCall(int FunctionId, const char * sItemString, POINT Pt, RECT Area);
	void OnDoubleClickScreenObject(int ObjectType, const char * sObjectId, POINT Pt, RECT Area, int Button);

	inline void OnAsrContentToBeClosed(void)
	{
		delete this;
	};

private:
	map<string, POINT> TagOffsets;
	map<int, string> MenuButtons;
	map<int, bool> ButtonsPressed;
	multimap<string, string> SepToolPairs;
	map<string, CRect> TagAreas;
	map<string, CRect> SoftTagAreas;
	map <string, clock_t> RecentlyAutoMovedTags;
	vector<string> RouteBeingShown;
	POINT MousePoint = { 0, 0 };
	CSTCA * StcaInstance;
	CMTCD * MtcdInstance;
	string DetailedTag = "";
	clock_t OneSecondTimer;
	clock_t HalfSecondTimer;
	bool Blink = false;

	string AcquiringSepTool = "";

	struct RadarFiltersStruct {
		int Hard_Low = 0;
		int Soft_Low = 20000;
		int Soft_High = 99999;
		int Hard_High = 99999;
	} RadarFilters;

};

