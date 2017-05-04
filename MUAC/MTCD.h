#pragma once
#include <EuroScopePlugIn.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace EuroScopePlugIn;


class CMTCD
{
public:
	int mtcd_distance = 10;
	int mtcd_height = 950;
	int mtcd_disable_level = 14500;
	int max_extrapolate_time = 30;

	vector<string> Alerts;

	void OnRefresh(CPlugIn * pl) {
		Alerts.clear();

		CFlightPlan fp;
		for (fp = pl->FlightPlanSelectFirst();
			fp.IsValid();
			fp = pl->FlightPlanSelectNext(fp))
		{
			CFlightPlanPositionPredictions PosPred = fp.GetPositionPredictions();

			if (PosPred.GetPointsNumber() <= 5)
				continue;

			// if not following route
			if (fp.GetRAMFlag())
				continue;

			// is on a heading
			if (fp.GetControllerAssignedData().GetAssignedHeading() != 0)
				continue;

			// TODO: See if we only use it for tracked ac
			//if (fp.GetTrackingControllerIsMe()

			// We scan up to an hour
			int toExtract = max(max_extrapolate_time, PosPred.GetPointsNumber());

			// For each minute left in the route, we check for conflicts
			for (int i = 1; i < toExtract; i++) {
				
				// If is below the level, then we ignore
				if (PosPred.GetAltitude(i) < mtcd_disable_level)
					continue;

				CPosition Pos = PosPred.GetPosition(i);

				// if not, we find conflicting targets

				CFlightPlan conflicting;
				for (conflicting = pl->FlightPlanSelectFirst();
					conflicting.IsValid();
					conflicting = pl->FlightPlanSelectNext(conflicting))
				{
					CFlightPlanPositionPredictions PosPredConflicting = fp.GetPositionPredictions();

					if (PosPredConflicting.GetPointsNumber() <= 5)
						continue;

					// if not following route
					if (conflicting.GetRAMFlag())
						continue;

					// is on a heading
					if (conflicting.GetControllerAssignedData().GetAssignedHeading() != 0)
						continue;

					if (PosPredConflicting.GetPointsNumber() < i)
						continue;

					CPosition conflictingPos = PosPredConflicting.GetPosition(i);
					
					// We have an MTCD conflicting on distance, lets save the position with the altitude for probe tool
					if (Pos.DistanceTo(conflictingPos) < mtcd_distance) {
						
						// We have a true mtcd conflict
						if (abs(PosPred.GetAltitude(i) - PosPredConflicting.GetAltitude(i)) < mtcd_height) {

							if (std::find(Alerts.begin(), Alerts.end(), fp.GetCallsign()) == Alerts.end())
								Alerts.push_back(fp.GetCallsign());
							if (std::find(Alerts.begin(), Alerts.end(), conflicting.GetCallsign()) == Alerts.end())
								Alerts.push_back(conflicting.GetCallsign());

						}

					}

				}

			}
		}
	};

	bool IsMTCD(string cs) {
		if (std::find(Alerts.begin(), Alerts.end(), cs) != Alerts.end())
		{
			return true;
		}
		return false;
	};
};