#pragma once
#include <string>
#include <vector>
#include "HttpHelper.h"
#include "Constants.h"

using namespace std;

class CPDLCHandler
{
public:
	static inline void PollOnlineStations() {
		string s = LoadHttpString("http://www.hoppie.nl/acars/system/connect.html?logon=&from=ALL-CALLSIGNS&to=ALL-CALLSIGNS&type=ping&packet=ALL-CALLSIGNS");

		if (s.size() > 0) {
			CPDLCHandler::OnlineCPDLCStations.clear();

			s.erase(0, 4);
			s.pop_back();

			
		}
	}

	static vector<string> OnlineCPDLCStations;

};

