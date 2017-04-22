#pragma once
#include "stdafx.h"
#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

static void FillInAltitudeList(CPlugIn* Plugin, int FunctionId, int Current) {
	Plugin->AddPopupListElement(" 999 ", "", FunctionId, Current == 99999);
	for(int i = 410; i >= 0; i -= 10)
		Plugin->AddPopupListElement(string(string(" ") + to_string(i) + string(" ")).c_str(), "", FunctionId, Current/100 == i);
};