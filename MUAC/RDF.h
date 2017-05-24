#pragma once
#include <vector>
#include <string>
#include <regex>
#include "EuroScopePlugIn.h"
#include "HttpHelper.h"

using namespace std;
using namespace EuroScopePlugIn;

string LoadRDFCallsign(string url, string channel);

class RDF
{
public:
	static string CurrentlyTransmitting;
	static string LastTransmitting;

	static bool Enabled;

	static int RandomizedOffsetDirection;
	static int RandomizedOffsetDistance;
};

