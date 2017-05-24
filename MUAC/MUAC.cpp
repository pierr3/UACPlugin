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

void MUAC::OnTimer(int Counter)
{
	if (RDF::Enabled) {
		if (fRDFString.valid() && fRDFString.wait_for(0ms) == future_status::ready) {
			
			// Save who is transmitting
			RDF::LastTransmitting = RDF::CurrentlyTransmitting;
			RDF::CurrentlyTransmitting = fRDFString.get();

			// Generate a random offset to simulate errors
			RDF::RandomizedOffsetDirection = RandomInt(0, 360);
			RDF::RandomizedOffsetDistance = RandomInt(200, 5500);

			fRDFString = future<string>();
		}
	}
}

void MUAC::OnVoiceReceiveStarted(CGrountToAirChannel Channel)
{
	if (RDF::Enabled) {
		// Query the RDF for currently transmitting station
		string url = "http://" + string(Channel.GetVoiceServer()) + ":18009/?opts=-R-D";
		fRDFString = async(LoadRDFCallsign, url.c_str(), string(Channel.GetVoiceChannel()));
	}
	
}

void MUAC::OnVoiceReceiveEnded(CGrountToAirChannel Channel)
{
	if (RDF::Enabled) {
		// Clear up the RDF
		if (RDF::CurrentlyTransmitting != "")
			RDF::LastTransmitting = RDF::CurrentlyTransmitting;
		
		RDF::CurrentlyTransmitting = "";
		fRDFString = future<string>();
	}
}

CRadarScreen * MUAC::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	if (!strcmp(sDisplayName, MUAC_RADAR_SCREEN_VIEW))
		return new RadarScreen();

	return nullptr;
}

void MUAC::RegisterPlugin() {
	RegisterDisplayType(MUAC_RADAR_SCREEN_VIEW, false, true, true, true);
}
