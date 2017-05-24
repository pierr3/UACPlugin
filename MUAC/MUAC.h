#pragma once
#include <string>
#include <future>
#include <thread>
#include "EuroScopePlugIn.h"
#include "Constants.h"
#include "CallsignLookup.h"
#include "RadarScreen.h"
#include "RDF.h"

using namespace std;
using namespace EuroScopePlugIn;

class MUAC : CPlugIn
{
public:
	MUAC();
	virtual ~MUAC();

	virtual void OnTimer(int Counter);

	virtual void OnVoiceReceiveStarted(CGrountToAirChannel Channel);
	virtual void OnVoiceReceiveEnded(CGrountToAirChannel Channel);

	CRadarScreen * OnRadarScreenCreated(const char * sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

protected:
	void RegisterPlugin();

private:

};