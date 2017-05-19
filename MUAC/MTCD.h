#pragma once
#include "VERA.h"
#include "STCA.h"
#include <EuroScopePlugIn.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
using namespace EuroScopePlugIn;


class CMTCD
{
public:
	int mtcd_distance = 7;
	int mtcd_height = 950;
	int mtcd_disable_level = 10500;
	int max_extrapolate_time = 30;

	int start_time = CSTCA::time_to_extrapolate/60;

	struct MtcdAlertStruct {
		string sourceCallsign, conflictCallsign;
		int minDistanceMin, minDistanceNm;
		VERA::VERADataStruct vera;
	};

	vector<MtcdAlertStruct> Alerts;

	void OnRefresh(CPlugIn * pl) {
		Alerts.clear();

		CFlightPlan fp;
		for (fp = pl->FlightPlanSelectFirst();
			fp.IsValid();
			fp = pl->FlightPlanSelectNext(fp))
		{
			CFlightPlanPositionPredictions PosPred = fp.GetPositionPredictions();

			if (PosPred.GetPointsNumber() <= start_time)
				continue;

			// if not following route
			if (fp.GetRAMFlag())
				continue;

			// is on a heading
			if (fp.GetControllerAssignedData().GetAssignedHeading() != 0)
				continue;

			// MTCD Only works for assumed flights and incoming flights
			if (!fp.GetTrackingControllerIsMe() && fp.GetFPState() != FLIGHT_PLAN_STATE_COORDINATED)
				continue;

			// We scan up to x
			int toExtract = min(max_extrapolate_time, PosPred.GetPointsNumber());

			// For each minute left in the route, we check for conflicts
			for (int i = start_time; i < toExtract; i++) {
				
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
					CFlightPlanPositionPredictions PosPredConflicting = conflicting.GetPositionPredictions();

					if (conflicting.GetCallsign() == fp.GetCallsign())
						continue;

					if (PosPredConflicting.GetPointsNumber() <= start_time)
						continue;

					// if not following route
					if (conflicting.GetRAMFlag())
						continue;

					// is on a heading
					if (conflicting.GetControllerAssignedData().GetAssignedHeading() != 0)
						continue;

					if (PosPredConflicting.GetPointsNumber() < i)
						continue;

					if (IsPairInMtcd(fp.GetCallsign(), conflicting.GetCallsign()))
						continue;

					CPosition conflictingPos = PosPredConflicting.GetPosition(i);
					
					// We have an MTCD conflicting on distance, lets save the position with the altitude for probe tool
					if (Pos.DistanceTo(conflictingPos) < mtcd_distance) {
						
						// We have a true mtcd conflict
						if (abs(PosPred.GetAltitude(i) - PosPredConflicting.GetAltitude(i)) < mtcd_height) {
							MtcdAlertStruct alert;
							alert.sourceCallsign = fp.GetCallsign();
							alert.conflictCallsign = conflicting.GetCallsign();
							alert.minDistanceMin = i;
							alert.minDistanceNm = (int)Pos.DistanceTo(conflictingPos);
							alert.vera = VERA::Calculate(pl->RadarTargetSelect(fp.GetCallsign()), pl->RadarTargetSelect(conflicting.GetCallsign()));

							Alerts.push_back(alert);
						}

					}

				}

			}
		}
	};

	bool IsPairInMtcd(string callsign1, string callsign2) {
		for (auto &data : Alerts) {
			if (data.sourceCallsign == callsign1 && data.conflictCallsign == callsign2)
				return true;
			if (data.sourceCallsign == callsign2 && data.conflictCallsign == callsign1)
				return true;
		}

		return false;
	};

	bool IsMTCD(string callsign) {
		for (auto &data : Alerts) {
			if (data.sourceCallsign == callsign || data.conflictCallsign == callsign)
				return true;
		}

		return false;
	};
};