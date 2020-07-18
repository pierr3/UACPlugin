#pragma once
#include "stdafx.h"
#include "Helper.h"
#include <vector>
#include <map>
#include "EuroScopePlugIn.h"
#include <string>


using namespace std;
using namespace EuroScopePlugIn;

class AntiOverlap
{
private:
	static int CalculateCost(CRadarScreen* instance, 
		map<string, CRect> TagAreas, map<string, POINT> TagOffsets, CRect Tile, int vvTime, CRadarTarget radarTrack) {
		
		POINT AcPoint = instance->ConvertCoordFromPositionToPixel(radarTrack.GetPosition().GetPosition());

		int overlapCost = 0;

		//
		// Overlap cost
		//

		// If the AC is in the tile, then maximum cost
		if (IsInRect(AcPoint, Tile))
			overlapCost += 99999;

		// If the leader line of the ac is in the tag
		double d = double(radarTrack.GetPosition().GetReportedGS()*0.514444) * 8*60;
		POINT VvPoint = instance->ConvertCoordFromPositionToPixel(Extrapolate(radarTrack.GetPosition().GetPosition(), radarTrack.GetTrackHeading(), d));

		POINT t1, t2;
		if (LiangBarsky(Tile, AcPoint, VvPoint, t1, t2))
			overlapCost += 40;


		for (auto TagAreaskv: TagAreas)
		{
			if (TagAreaskv.first == radarTrack.GetCallsign())
				continue;

			CRect work;
			CRadarTarget testTarget = instance->GetPlugIn()->RadarTargetSelect(TagAreaskv.first.c_str());
			POINT TestPoint = instance->ConvertCoordFromPositionToPixel(testTarget.GetPosition().GetPosition());

			
			if (work.IntersectRect(TagAreaskv.second, Tile))
				overlapCost += 30;

			if (IsInRect(TestPoint, Tile))
				overlapCost += 40;

			// Leader line
			POINT endPointLeader = { TestPoint.x + TagOffsets[TagAreaskv.first.c_str()].x, TestPoint.y + TagOffsets[TagAreaskv.first.c_str()].y };
			POINT t1, t2;
			if (LiangBarsky(Tile, TestPoint, endPointLeader, t1, t2))
				overlapCost += 20;

			// TODO: Add speed vector line

		}

		//
		// Angle cost
		//
		double angle1 = atan2(VvPoint.y - AcPoint.y, VvPoint.x - AcPoint.x);
		double angle2 = atan2(Tile.top - AcPoint.y, Tile.left - AcPoint.x);

		int angleLeaderAc = (int)(angle1-angle2);
		int angleCost = (int)((fmod(angleLeaderAc, 180) - 135));

		//
		// Distance cost
		//

		int xDelta = Tile.left - TagAreas[radarTrack.GetCallsign()].left;
		int yDelta = Tile.top - TagAreas[radarTrack.GetCallsign()].top;
		int distanceCost = (int)sqrt(xDelta*xDelta + yDelta*yDelta);

		return 100* overlapCost + 10 * angleCost + 2 * distanceCost;
	};

	static bool IsConflicting(CRadarScreen* instance, CRect Area, map<string, CRect> TagAreas) {
		bool IsConflicting = false;
		// We check for conflicts
		for (auto kv : TagAreas) {
			if (kv.second == Area)
				continue;

			//
			// Filter situatiosn where tags are more than 400px apart for performance
			//
			if (abs(kv.second.top - Area.top) > 400)
				continue;

			if (abs(kv.second.right - Area.right) > 400)
				continue;

			

			POINT acPt = instance->ConvertCoordFromPositionToPixel(instance->GetPlugIn()->RadarTargetSelect(kv.first.c_str()).GetPosition().GetPosition());

			// If tags intersect
			CRect h;
			if (h.IntersectRect(Area, kv.second)) {
				IsConflicting = true;
				break;
			}

			// If symbols overlap
			if (IsInRect(acPt, Area)) {
				IsConflicting = true;
				break;
			}
		}

		return IsConflicting;
	}

public:
	static map<int, CRect> BuildGrid(POINT AcPoint, int TileWidth, int TileHeight) {
		map<int, CRect> out;

		POINT WorkingPoint = { AcPoint.x - (TileWidth + TileWidth/2), AcPoint.y - (TileHeight + TileHeight/2) };
		for (int i = 0; i < 9; i++) {

			POINT TopLeftWorking = { WorkingPoint.x + i*TileWidth, WorkingPoint.y};
			out[i] = { TopLeftWorking.x, TopLeftWorking.y, TopLeftWorking.x + TileWidth, TopLeftWorking.y + TileHeight };
			
			if (i == 2 || i == 5) {
				WorkingPoint.x -= TileWidth*3;
				WorkingPoint.y += TileHeight;
			}
				
		}

		return out;
	};

	static map<int, int> CalculateGridCost(CRadarScreen* instance, map<string, CRect> TagAreas, map<string, POINT> TagOffsets, 
		map<int, CRect> Grid, int vvTime, CRadarTarget radarTrack) {
		
		map<int, int> out;

		for (auto kv : Grid)
			out[kv.first] = CalculateCost(instance, TagAreas, TagOffsets, kv.second, vvTime, radarTrack);

		return out;
	}

	static POINT Execute(CRadarScreen* instance, map<string, CRect> TagAreas, map<string, POINT> TagOffsets, int vvTime, CRadarTarget radarTarget) {
		POINT AcPoint = instance->ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());
		
		if (IsConflicting(instance, TagAreas[radarTarget.GetCallsign()], TagAreas)) {
			int TileWidth = TagAreas[radarTarget.GetCallsign()].Size().cx + (int)(TagAreas[radarTarget.GetCallsign()].Size().cx*0.20);
			int TileHeight = TagAreas[radarTarget.GetCallsign()].Size().cy + (int)(TagAreas[radarTarget.GetCallsign()].Size().cy*0.05);

			map<int, CRect> Grid = BuildGrid(AcPoint, TileWidth, TileHeight);
			map<int, int> Cost = CalculateGridCost(instance, TagAreas, TagOffsets, Grid, vvTime, radarTarget);

			int minCost = -1;
			int minKey = -1;
			for (auto kv : Cost) {
				if (kv.second < minCost || minCost == -1) {
					minCost = kv.second;
					minKey = kv.first;
				}
			}

			return { Grid[minKey].left - AcPoint.x, Grid[minKey].top - AcPoint.y };
		}
		else {
			return TagOffsets[radarTarget.GetCallsign()];
		}
	};

};