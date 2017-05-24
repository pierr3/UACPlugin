#include "stdafx.h"
#include "RDF.h"

bool RDF::Enabled = false;
string RDF::CurrentlyTransmitting = "";
string RDF::LastTransmitting = "";
int RDF::RandomizedOffsetDirection = 0;
int RDF::RandomizedOffsetDistance = 0;

string LoadRDFCallsign(string url, string channel)
{
	string httpstr = LoadHttpString(url);

	regex rgx("(channel \"/" + channel + "Currently transmitting: )([A-z0-9]{1,10})");
	smatch matches;

	if (regex_search(httpstr, matches, rgx)) {
		return matches[2];
	}
	else {
		return "nomatch";
	}
}

