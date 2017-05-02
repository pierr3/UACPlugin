#pragma once
#include <EuroScopePlugIn.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace EuroScopePlugIn;


class CSTCA
{
public:
	CSTCA();
	virtual ~CSTCA();

	vector<string> Alerts;

	void OnRefresh(CPlugIn * pl);
	bool IsSTCA(string cs);
};

