#include "stdafx.h"
#include "MUAC.h"


MUAC::MUAC():CPlugIn(COMPATIBILITY_CODE, PLUGIN_NAME.c_str(), PLUGIN_VERSION.c_str(), PLUGIN_AUTHOR.c_str(), PLUGIN_COPY.c_str()) {

	srand((unsigned int)time(nullptr));
	this->RegisterPlugin();

	DisplayUserMessage("Message", "MUAC PlugIn", string("Version " + PLUGIN_VERSION + " loaded").c_str(), false, false, false, false, false);
}

MUAC::~MUAC() {}

CRadarScreen * MUAC::OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated)
{
	if (!strcmp(sDisplayName, MUAC_RADAR_SCREEN_VIEW))
		return new RadarScreen();

	return nullptr;
}

void MUAC::RegisterPlugin() {
	RegisterDisplayType(MUAC_RADAR_SCREEN_VIEW, false, true, true, true);
}
