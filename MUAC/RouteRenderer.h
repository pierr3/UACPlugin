#pragma once
#include <vector>
#include <map>
#include <string>
#include "Helper.h"
#include "Constants.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class RouteRenderer
{
public:
	static void Render(CDC* dc, CRadarScreen* instance, CRadarTarget rt, CFlightPlan CorrFp) {
		int saveRouteDc = dc->SaveDC();
		
		POINT AcPosPix = instance->ConvertCoordFromPositionToPixel(rt.GetPosition().GetPosition());
		if (CorrFp.IsValid())
		{
			CFlightPlanExtractedRoute exR = CorrFp.GetExtractedRoute();
			int index = exR.GetPointsAssignedIndex();
			if (index < 0)
				index = exR.GetPointsCalculatedIndex();

			// We do a quick loop to remove any -1 minute estimates
			for (int i = index; i < exR.GetPointsNumber(); i++) {
				index = i;

				if (exR.GetPointDistanceInMinutes(i) != -1)
					break;
			}

			FontManager::SelectStandardFont(dc);

			for (int i = index; i < exR.GetPointsNumber(); i++)
			{
				POINT exRPos = instance->ConvertCoordFromPositionToPixel(exR.GetPointPosition(i));

				Color routeColor = Colours::AirwayBlue;

				if (exR.GetPointAirwayClassification(i) == AIRWAY_CLASS_DIRECTION_ERROR ||
					exR.GetPointAirwayClassification(i) == AIRWAY_CLASS_UNCONNECTED) {
					routeColor = Colours::RedWarning;
				}
				
				CPen routePen(PS_SOLID, 1, routeColor.ToCOLORREF());
				CPen* oldPen = dc->SelectObject(&routePen);


				// Draw the line

				if (i == index)
					dc->MoveTo(AcPosPix);
				else
					dc->MoveTo(instance->ConvertCoordFromPositionToPixel(exR.GetPointPosition(i - 1)));

				dc->LineTo(exRPos);
				
				string pointName = exR.GetPointName(i);
				string pointTime = getUtcTimePlusMinutes(exR.GetPointDistanceInMinutes(i), "%H:%M");
				string pointAltitude = padWithZeros(5, exR.GetPointCalculatedProfileAltitude(i)).substr(0, 3);

				string FinalString = pointName + "|" + pointTime + " " + pointAltitude;
				
				dc->SetTextColor(Colours::AircraftLightGrey.ToCOLORREF());
				dc->SetTextAlign(TA_CENTER | TA_BASELINE);

				DrawTextMultiline(dc, FinalString, exRPos);
			
				dc->SelectObject(&oldPen);

			}
		}

		dc->RestoreDC(saveRouteDc);
	};
};