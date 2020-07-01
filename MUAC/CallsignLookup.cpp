#include "stdafx.h"
#include "CallsignLookup.h"

//
// CCallsignLookup Class by Even Rognlien, used with permission
//

bool CCallsignLookup::Available = false;
CCallsignLookup* CCallsignLookup::Lookup = nullptr;

CCallsignLookup::CCallsignLookup(std::string fileName) {

	ifstream myfile;

	myfile.open(fileName);

	if (myfile) {
		string line;

		while (getline(myfile, line)) {
			istringstream iss(line);
			vector<string> tokens;
			string token;

			while (std::getline(iss, token, '\t'))
				tokens.push_back(token);

			if (tokens.size() >= 3) {
				callsigns[tokens.front()] = tokens.at(2);
			}
		}
	}

	myfile.close();
}

string CCallsignLookup::getCallsign(string airlineCode) {

	if (callsigns.find(airlineCode) == callsigns.end())
		return "";

	return callsigns.find(airlineCode)->second;
}

CCallsignLookup::~CCallsignLookup()
{
}