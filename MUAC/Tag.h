#pragma once
#include <vector>
#include <map>
#include <string>
#include "Helper.h"
#include "TagItem.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class Tag
{
public:

	// General things

	const enum TagStates { NotConcerned, InSequence, Next, TransferredToMe, Assumed, TransferredFromMe, Redundant };

	// TagItems Definition
	TagItem CallsignItem = TagItem::CreatePassive("Callsign");
	TagItem SquawkItem = TagItem::CreatePassive("Squawk");
	TagItem AltitudeItem = TagItem::CreatePassive("Altitude");
	TagItem TendencyItem = TagItem::CreatePassive("Tendency");

	// CFL
	TagItem CFLItem = TagItem::CreatePassive("CFL");
	// Combination of NFL and XFL, depending on the state
	TagItem XFLItem = TagItem::CreatePassive("XFL");
	// RFL
	TagItem RFLItem = TagItem::CreatePassive("RFL");
	// COPX/COPN Point Item
	TagItem COPItem = TagItem::CreatePassive("COP");
	// Lateral clearance (Direct/HDG)
	TagItem HorizontalClearItem = TagItem::CreatePassive("HDG");
	// Current or next sector
	TagItem SectorItem = TagItem::CreatePassive("Sector");

	TagItem ReportedGS = TagItem::CreatePassive("ReportedGS");
	TagItem VerticalRate = TagItem::CreatePassive("VerticalRate");

	TagItem RouteDisplay = TagItem::CreatePassive("R");
	TagItem SepItem = TagItem::CreatePassive("V");

	TagItem BlankItem = TagItem::CreatePassive(" ");

	// Tag definitions
	const vector<vector<TagItem>> MinimizedTag = { { AltitudeItem, TendencyItem } };
	const vector<vector<TagItem>> StandardTag = { { CallsignItem }, { AltitudeItem, TendencyItem, CFLItem }, { XFLItem  } };
	const vector<vector<TagItem>> MagnifiedTag = { 
	{ SepItem, CallsignItem, SectorItem },
	{ RouteDisplay, AltitudeItem, TendencyItem, CFLItem, HorizontalClearItem },
	{ BlankItem, XFLItem, COPItem, RFLItem },
	{ BlankItem, ReportedGS, VerticalRate } };

	// Tag Object

	Tag(TagStates State, bool IsMagnified, bool IsSoft, CRadarScreen* radarScreen, CRadarTarget RadarTarget, CFlightPlan FlightPlan) {

		this->IsMagnified = IsMagnified;
		this->IsSoft = IsSoft;

		map<string, string> TagReplacementMap = GenerateTag(radarScreen, IsMagnified, RadarTarget, FlightPlan);

		if (State == TagStates::NotConcerned && IsSoft)
			Definition = MinimizedTag;

		if (State == TagStates::NotConcerned && !IsSoft)
			Definition = StandardTag;

		if (State == TagStates::InSequence && IsSoft)
			Definition = MinimizedTag;

		if (State == TagStates::InSequence && !IsSoft)
			Definition = StandardTag;

		if (State == TagStates::Next || State == TagStates::TransferredToMe 
			|| State == TagStates::Assumed || State == TagStates::TransferredFromMe || State == TagStates::Redundant)
			Definition = StandardTag;

		if (IsMagnified)
			Definition = MagnifiedTag;

		TagState = State;

		// Replacing the tag

		// For each line
		for (auto& TagLine : Definition) {
			// For each item
			for (auto& TagItem : TagLine) {
				// We know replace the item
				for (auto& kv : TagReplacementMap) {
					if (TagItem.Text == kv.first) {
						TagItem.Text = kv.second;
						break;
					}
				}
			}
		}

	};
	~Tag() {};

	vector<vector<TagItem>> Definition;
	TagStates TagState;
	bool IsMagnified;
	bool IsSoft;

protected:
	map<string, string> GenerateTag(CRadarScreen* radarScreen, bool isMagnified, CRadarTarget RadarTarget, CFlightPlan FlightPlan) {

		map<string, string> TagReplacementMap;

		if (!RadarTarget.IsValid())
			return TagReplacementMap;

		TagReplacementMap.insert(pair<string, string>("Callsign", RadarTarget.GetCallsign()));
		TagReplacementMap.insert(pair<string, string>("Squawk", RadarTarget.GetPosition().GetSquawk()));
		// Alt
		string alt = to_string((int)RoundTo(RadarTarget.GetPosition().GetFlightLevel(), 100) / 100);
		
		if (RadarTarget.GetPosition().GetFlightLevel() <= radarScreen->GetPlugIn()->GetTransitionAltitude())
			alt = "A" + to_string((int)RoundTo(RadarTarget.GetPosition().GetPressureAltitude(), 100) / 100);

		TagReplacementMap.insert(pair<string, string>("Altitude", alt));
		
		TagReplacementMap.insert(pair<string, string>("ReportedGS", string("G") + to_string(RadarTarget.GetPosition().GetReportedGS())));

		TagReplacementMap.insert(pair<string, string>("R", " "));
		TagReplacementMap.insert(pair<string, string>("V", " "));

		string VerticalRate = "00";
		CRadarTargetPositionData pos = RadarTarget.GetPosition();
		CRadarTargetPositionData oldpos = RadarTarget.GetPreviousPosition(pos);
		if (pos.IsValid() && oldpos.IsValid()) {
			int deltaalt = pos.GetFlightLevel() - oldpos.GetFlightLevel();
			int deltaT = oldpos.GetReceivedTime() - pos.GetReceivedTime();

			if (deltaT > 0) {
				float vz = abs(deltaalt) * (60.0f / deltaT);

				// If the rate is too much
				if ((int)abs(vz) >= 9999) {
					VerticalRate = "^99";
					if (deltaalt < 0)
						VerticalRate = "|99";
						
				}
				else if (abs(vz) >= 100 && abs(deltaalt) >= 20) {
					string rate = padWithZeros(2, (int)abs(vz / 100));
					VerticalRate = "^" + rate;

					if (deltaalt < 0)
						VerticalRate = "|" + rate;
				}
			}
		}

		TagReplacementMap.insert(pair<string, string>("VerticalRate", VerticalRate));
		
		if (FlightPlan.IsValid()) {
			
			// RFL
			string RFL = to_string((int)FlightPlan.GetControllerAssignedData().GetFinalAltitude() / 100);

			if (FlightPlan.GetControllerAssignedData().GetFinalAltitude() == 0)
				RFL = to_string((int)FlightPlan.GetFlightPlanData().GetFinalAltitude() / 100);

			TagReplacementMap.insert(pair<string, string>("RFL", RFL));


			// CFL
			string CFL = to_string((int)FlightPlan.GetClearedAltitude() / 100);

			if (FlightPlan.GetClearedAltitude() == 0)
				CFL = RFL;

			// If not detailed and reached alt, then nothing to show
			if (!isMagnified && abs(RadarTarget.GetPosition().GetFlightLevel()- FlightPlan.GetClearedAltitude()) < 100) {
				CFL = " ";
			}

			TagReplacementMap.insert(pair<string, string>("CFL", CFL));
			
			// Sector
			string SectorId = "--";
			if (FlightPlan.GetTrackingControllerIsMe()) {
				string controllerCallsign = FlightPlan.GetCoordinatedNextController();
				if (controllerCallsign.length()  > 0) {
					SectorId = radarScreen->GetPlugIn()->ControllerSelect(controllerCallsign.c_str()).GetPositionId();
				}
			}
			else {
				string controllerCallsign = FlightPlan.GetTrackingControllerId();
				if (controllerCallsign.length() > 0) {
					SectorId = controllerCallsign;
				}
			}

			TagReplacementMap.insert(pair<string, string>("Sector", SectorId));

			string Horizontal = FlightPlan.GetControllerAssignedData().GetDirectToPointName();
			if (Horizontal.empty() && FlightPlan.GetControllerAssignedData().GetAssignedHeading() != 0) {
				Horizontal = "H" + padWithZeros(3, FlightPlan.GetControllerAssignedData().GetAssignedHeading());
			}
			else {
				Horizontal = "HDG";
			}

			TagReplacementMap.insert(pair<string, string>("HDG", Horizontal));
		}

		string tendency = "-";
		int delta_fl = RadarTarget.GetPosition().GetFlightLevel() -
			RadarTarget.GetPreviousPosition(RadarTarget.GetPosition()).GetFlightLevel();
		if (abs(delta_fl) >= 50)
			tendency = delta_fl < 0 ? "|" : "^";

		TagReplacementMap.insert(pair<string, string>("Tendency", tendency));

		return TagReplacementMap;
	}
};

