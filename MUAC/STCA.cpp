#include "stdafx.h"
#include "STCA.h"
#include "Helper.h"

CSTCA::CSTCA()
{

}


CSTCA::~CSTCA()
{

}

bool CSTCA::IsSTCA(string cs)
{
	if (std::find(Alerts.begin(), Alerts.end(), cs) != Alerts.end())
	{
		return true;
	}
	return false;
};

void CSTCA::OnRefresh(CPlugIn * pl)
{
	Alerts.clear();

	CRadarTarget rt;
	for (rt = pl->RadarTargetSelectFirst();
		rt.IsValid();
		rt = pl->RadarTargetSelectNext(rt))
	{
		if (rt.GetPosition().GetRadarFlags() == EuroScopePlugIn::RADAR_POSITION_PRIMARY)
			continue;
		
		if (rt.GetPosition().GetPressureAltitude() < 5000)
			continue;
		
		if (strcmp(rt.GetPosition().GetSquawk(), "7000") == 0)
			continue;

		if (rt.GetCorrelatedFlightPlan().IsValid())
		{
			if (rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetPlanType() == "V")
				continue;
		}

		CRadarTarget conflicting;
		for (conflicting = pl->RadarTargetSelectFirst();
		conflicting.IsValid();
			conflicting = pl->RadarTargetSelectNext(conflicting))
		{
			int distance_mini = 5;
			int time_to_extrapolate = 180;

			if (rt.GetCallsign() == conflicting.GetCallsign())
				continue;

			if (rt.GetPosition().GetPressureAltitude() <= 14500 || conflicting.GetPosition().GetPressureAltitude() <= 14500)
			{
				distance_mini = 3;
				time_to_extrapolate = 60;
			}

			if (conflicting.GetPosition().GetRadarFlags() == EuroScopePlugIn::RADAR_POSITION_PRIMARY)
				continue;

			if (rt.GetPosition().GetPosition().DistanceTo(conflicting.GetPosition().GetPosition()) > 10)
				continue;

			if (conflicting.GetPosition().GetPressureAltitude() < 5000)
				continue;

			if (strcmp(rt.GetPosition().GetSquawk(), "7000") == 0)
				continue;

			if (rt.GetCorrelatedFlightPlan().IsValid())
			{
				if (rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetPlanType() == "V")
					continue;
			}

			if (rt.GetPosition().GetPosition().DistanceTo(conflicting.GetPosition().GetPosition()) < distance_mini &&
				abs(rt.GetPosition().GetPressureAltitude()-conflicting.GetPosition().GetPressureAltitude()) < 900)
			{
				if (std::find(Alerts.begin(), Alerts.end(), rt.GetCallsign()) == Alerts.end()) 
					Alerts.push_back(rt.GetCallsign());
				if (std::find(Alerts.begin(), Alerts.end(), conflicting.GetCallsign()) == Alerts.end())
					Alerts.push_back(conflicting.GetCallsign());
				continue;
			}
				
			for (int i = 10; i <= time_to_extrapolate; i += 10)
			{
				CPosition ex1 = Extrapolate(rt.GetPosition().GetPosition(), rt.GetTrackHeading(), double(rt.GetPosition().GetReportedGS()*0.514444)*(i));
				CPosition ex2 = Extrapolate(conflicting.GetPosition().GetPosition(), conflicting.GetTrackHeading(), double(conflicting.GetPosition().GetReportedGS()*0.514444)*(i));
			
				int alt1 = rt.GetPosition().GetPressureAltitude();
				int alt2 = conflicting.GetPosition().GetPressureAltitude();

				if (rt.GetPreviousPosition(rt.GetPosition()).IsValid() && 
					conflicting.GetPreviousPosition(conflicting.GetPosition()).IsValid())
				{
					int dalt1 = alt1 - rt.GetPreviousPosition(rt.GetPosition()).GetPressureAltitude();
					int dalt2 = alt2 - conflicting.GetPreviousPosition(conflicting.GetPosition()).GetPressureAltitude();

					int dt1 = rt.GetPreviousPosition(rt.GetPosition()).GetReceivedTime() - rt.GetPosition().GetReceivedTime();
					int dt2 = conflicting.GetPreviousPosition(conflicting.GetPosition()).GetReceivedTime() - conflicting.GetPosition().GetReceivedTime();
				
					int vz1 = 0;
					int vz2 = 0;

					if (dt1 > 0 && dt2 > 0)
					{
						vz1 = dalt1 * (i / dt1);
						vz2 = dalt2 * (i / dt2);
					}

					alt1 += vz1;
					alt2 += vz2;
				}

				if (ex1.DistanceTo(ex2) < distance_mini && 
					abs(alt1 - alt2) < 900)
				{
					if (std::find(Alerts.begin(), Alerts.end(), rt.GetCallsign()) == Alerts.end())
						Alerts.push_back(rt.GetCallsign());
					if (std::find(Alerts.begin(), Alerts.end(), conflicting.GetCallsign()) == Alerts.end())
						Alerts.push_back(conflicting.GetCallsign());
					break;
				}
			}


		}

	}

}