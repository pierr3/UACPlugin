#pragma once
#include <vector>
#include <map>
#include <string>
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
	TagItem HorizontalClearItem = TagItem::CreatePassive("Horizontal");
	// Current or next sector
	TagItem SectorItem = TagItem::CreatePassive("Sector");

	TagItem ReportedGS = TagItem::CreatePassive("ReportedGS");
	TagItem VerticalRate = TagItem::CreatePassive("VerticalRate");

	TagItem RouteDisplay = TagItem::CreatePassive("R");
	TagItem SepItem = TagItem::CreatePassive("V");

	TagItem BlankItem = TagItem::CreatePassive(" ");

	// Tag definitions
	const vector<vector<TagItem>> MinimizedTag = { { AltitudeItem, TendencyItem } };
	const vector<vector<TagItem>> StandardTag = { { CallsignItem }, { AltitudeItem, TendencyItem, CFLItem }, { XFLItem,  } };
	const vector<vector<TagItem>> MagnifiedTag = { { CallsignItem, SectorItem },
	{ AltitudeItem, TendencyItem, CFLItem, HorizontalClearItem },
	{ XFLItem, COPItem, RFLItem },
	{ ReportedGS, VerticalRate } };

	// Tag Object

	Tag(TagStates State, bool IsMagnified, bool IsSoft, CRadarTarget RadarTarget) {

		this->IsMagnified = IsMagnified;
		this->IsSoft = IsSoft;

		map<string, string> TagReplacementMap = GenerateTag(RadarTarget);

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
	map<string, string> GenerateTag(CRadarTarget RadarTarget) {
		map<string, string> TagReplacementMap;
		TagReplacementMap.insert(pair<string, string>("Callsign", RadarTarget.GetCallsign()));
		TagReplacementMap.insert(pair<string, string>("Squawk", RadarTarget.GetPosition().GetSquawk()));
		TagReplacementMap.insert(pair<string, string>("Altitude", to_string((int)RadarTarget.GetPosition().GetFlightLevel() / 100)));
		TagReplacementMap.insert(pair<string, string>("CFL", to_string((int)RadarTarget.GetCorrelatedFlightPlan().GetClearedAltitude() / 100)));
		TagReplacementMap.insert(pair<string, string>("ReportedGS", string("G") + to_string(RadarTarget.GetPosition().GetReportedGS())));

		string tendency = "-";
		int delta_fl = RadarTarget.GetPosition().GetFlightLevel() -
			RadarTarget.GetPreviousPosition(RadarTarget.GetPosition()).GetFlightLevel();
		if (abs(delta_fl) >= 50)
			tendency = delta_fl < 0 ? "|" : "^";

		TagReplacementMap.insert(pair<string, string>("Tendency", tendency));

		return TagReplacementMap;
	}
};

