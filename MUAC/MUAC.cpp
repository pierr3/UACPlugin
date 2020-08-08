#include "stdafx.h"
#include "MUAC.h"

future<string> fRDFString;

MUAC::MUAC():CPlugIn(COMPATIBILITY_CODE, PLUGIN_NAME.c_str(), PLUGIN_VERSION.c_str(), PLUGIN_AUTHOR.c_str(), PLUGIN_COPY.c_str()) {

	srand((unsigned int)time(nullptr));
	this->RegisterPlugin();

	DisplayUserMessage("Message", "MUAC PlugIn", string("Version " + PLUGIN_VERSION + " loaded").c_str(), false, false, false, false, false);

	char DllPathFile[_MAX_PATH];
	string DllPath;
	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	DllPath = DllPathFile;
	DllPath.resize(DllPath.size() - strlen("MUAC.dll"));

	string FilePath = DllPath + "\\ICAO_Airlines.txt";
	if (file_exist(FilePath)) {
		CCallsignLookup::Lookup = new CCallsignLookup(FilePath);
		CCallsignLookup::Available = true;
	}
	else {
		CCallsignLookup::Available = false;
		DisplayUserMessage("Message", "MUAC PlugIn", string("Warning: Could not load callsigns, they will be unavailable").c_str(), 
			true, true, false, false, true);
	}

}

MUAC::~MUAC() {}

CRadarScreen * MUAC::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	if (!strcmp(sDisplayName, MUAC_RADAR_SCREEN_VIEW))
		return new RadarScreen();

	return nullptr;
}

void MUAC::OnTimer(int Counter)
{
	if (Counter % 5 == 0) {

	}
}

void MUAC::RegisterPlugin() {
	RegisterDisplayType(MUAC_RADAR_SCREEN_VIEW, false, true, true, true);
}
