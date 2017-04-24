#pragma once
#include "stdafx.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

static void FillInAltitudeList(CPlugIn* Plugin, int FunctionId, int Current) {
	Plugin->AddPopupListElement(" 999 ", "", FunctionId, Current == 99999);
	for(int i = 410; i >= 0; i -= 10)
		Plugin->AddPopupListElement(string(string(" ") + to_string(i) + string(" ")).c_str(), "", FunctionId, Current/100 == i);
};

inline static string padWithZeros(int padding, int s)
{
	stringstream ss;
	ss << setfill('0') << setw(padding) << s;
	return ss.str();
};

inline static int RoundTo(int n, int multiplier) {
	return (n + multiplier/2) / multiplier * multiplier;
};

inline static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
};
inline static std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
};