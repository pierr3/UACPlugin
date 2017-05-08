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
				if (exR.GetPointDistanceInMinutes(i) != -1)
					break;

				index = i;
			}

			bool isLastTextOnLeftSide = false;

			FontManager::SelectBoldBigFont(dc);

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

				bool isTextOnLeftSide = false;

				if (i == index)
				{
					dc->MoveTo(AcPosPix);
					dc->LineTo(exRPos);
					if (rt.GetPosition().GetPosition().DistanceTo(exR.GetPointPosition(i)) < 10)
					{
						if (!isLastTextOnLeftSide)
						{
							isTextOnLeftSide = true;
							isLastTextOnLeftSide = true;
						}
					}
				}
				else
				{
					dc->MoveTo(instance->ConvertCoordFromPositionToPixel(exR.GetPointPosition(i - 1)));
					dc->LineTo(exRPos);

					if (exR.GetPointPosition(i).DistanceTo(exR.GetPointPosition(i - 1)) < 10)
					{
						if (!isLastTextOnLeftSide)
						{
							isTextOnLeftSide = true;
							isLastTextOnLeftSide = true;
						}
					}
				}

				if (!isTextOnLeftSide)
					isLastTextOnLeftSide = false;

				// Selecting top or left right

				bool isTextTop = false;
				if (i == index)
				{
					int br = (int)rt.GetPosition().GetPosition().DirectionTo(exR.GetPointPosition(i));
					if ((br > 70 && br < 110) || (br > 250 && br < 290))
					{
						isTextTop = true;
					}
				}
				else
				{
					int br = (int)exR.GetPointPosition(i - 1).DirectionTo(exR.GetPointPosition(i));
					if ((br > 70 && br < 110) || (br > 250 && br < 290))
					{
						isTextTop = true;
					}
				}

				dc->SelectObject(&routePen);
				dc->SelectStockObject(NULL_BRUSH);
				dc->Ellipse(exRPos.x - 3 , exRPos.y - 3, exRPos.x + 3 , exRPos.y + 3);
				string actions = CorrFp.GetCallsign();
				actions += ",";
				actions += std::to_string(i);

				string pointName = exR.GetPointName(i);
				if (pointName == string(CorrFp.GetControllerAssignedData().GetDirectToPointName()))
				{
					pointName = "->" + pointName;
				}

				string temp = getUtcTimePlusMinutes(exR.GetPointDistanceInMinutes(i), "%H:%M");
				temp += " ";

				// Converting the profile altitude to FL/Altitude

				int fl = exR.GetPointCalculatedProfileAltitude(i);
				if (fl <= instance->GetPlugIn()->GetTransitionAltitude()) {
					fl = exR.GetPointCalculatedProfileAltitude(i);
					temp += padWithZeros(std::to_string(instance->GetPlugIn()->GetTransitionAltitude()).size(), fl);
				}
				else
				{
					temp += "FL" + padWithZeros(5, fl).substr(0, 3);
				}

				POINT FirstLineText, SecondLineText, LinePoint;

				if (isTextOnLeftSide)
				{
					if (isTextTop)
					{
						dc->SetTextAlign(TA_BOTTOM | TA_LEFT);
						FirstLineText = { exRPos.x, exRPos.y - 25 };
						SecondLineText = { exRPos.x, exRPos.y - 10 };
						LinePoint = { exRPos.x, exRPos.y - 10 };
					}
					else
					{
						dc->SetTextAlign(TA_BASELINE | TA_RIGHT);
						FirstLineText = { exRPos.x - 15, exRPos.y };
						SecondLineText = { exRPos.x - 15, exRPos.y + 10 };
						LinePoint = { exRPos.x - 15, exRPos.y };
					}

				}
				else
				{
					if (isTextTop)
					{
						dc->SetTextAlign(TA_TOP | TA_LEFT);
						FirstLineText = { exRPos.x, exRPos.y + 10 };
						SecondLineText = { exRPos.x, exRPos.y + 25 };
						LinePoint = { exRPos.x, exRPos.y + 10 };
					}
					else
					{
						dc->SetTextAlign(TA_BASELINE | TA_LEFT);
						FirstLineText = { exRPos.x + 15, exRPos.y };
						SecondLineText = { exRPos.x + 15, exRPos.y + 10 };
						LinePoint = { exRPos.x + 15, exRPos.y };
						
					}

				}

				
				dc->SetTextColor(routeColor.ToCOLORREF());
				dc->TextOutA(FirstLineText.x, FirstLineText.y, pointName.c_str());
				dc->TextOutA(SecondLineText.x, SecondLineText.y, temp.c_str());
				dc->MoveTo(exRPos);
				dc->LineTo(LinePoint);

				dc->SelectObject(&oldPen);

			}
		}

		dc->RestoreDC(saveRouteDc);
	};
};