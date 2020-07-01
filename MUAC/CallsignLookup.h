#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>

using namespace std;

class CCallsignLookup
{
private:
	std::map<string, string> callsigns;

public:
	CCallsignLookup(string fileName);
	string getCallsign(string airlineCode);

	static bool Available;
	static CCallsignLookup* Lookup;

	~CCallsignLookup();
};