#pragma once
#include "stdafx.h"
#include "Colours.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace Gdiplus;
using namespace EuroScopePlugIn;

class AcSymbols
{
public:
	static CRect DrawSquareAndTrail(CDC* dc, Tag::TagStates State, CRadarScreen* radar, CRadarTarget radarTarget, bool drawTrail, bool isSoft, bool isDetailed) {
		int save = dc->SaveDC();
		
		COLORREF SymbolColor = Colours::AircraftDarkGrey.ToCOLORREF();
		COLORREF TrailColor = Colours::AircraftDarkGrey.ToCOLORREF();

		if (!isSoft) {
			TrailColor = Colours::AircraftLightGrey.ToCOLORREF();
			SymbolColor = Colours::AircraftLightGrey.ToCOLORREF();
		}

		if (State == Tag::TagStates::Assumed) {
			TrailColor = Colours::AircraftGreen.ToCOLORREF();
			SymbolColor = Colours::AircraftGreen.ToCOLORREF();
		}

		if (State == Tag::TagStates::Next || State == Tag::TagStates::TransferredToMe)
			SymbolColor = Colours::AircraftGreen.ToCOLORREF();

		if (State == Tag::TagStates::Redundant)
			SymbolColor = Colours::AircraftBlue.ToCOLORREF();

		CPen TrailPen(PS_SOLID, 1, TrailColor);
		CPen SymbolPen(PS_SOLID, 1, SymbolColor);

		dc->SelectObject(&TrailPen);
		dc->SelectStockObject(NULL_BRUSH);

		// History Trail
		CRadarTargetPositionData TrailPosition = radarTarget.GetPreviousPosition(radarTarget.GetPreviousPosition(radarTarget.GetPosition()));
		for (int i = 0; i < 5; i++) {
			if (!drawTrail)
				break;

			POINT historyTrailPoint = radar->ConvertCoordFromPositionToPixel(TrailPosition.GetPosition());

			CRect Area(historyTrailPoint.x - DRAWING_AC_SQUARE_TRAIL_SIZE, historyTrailPoint.y - DRAWING_AC_SQUARE_TRAIL_SIZE,
				historyTrailPoint.x + DRAWING_AC_SQUARE_TRAIL_SIZE,  historyTrailPoint.y + DRAWING_AC_SQUARE_TRAIL_SIZE);
			Area.NormalizeRect();
			dc->Rectangle(Area);

			// We skip one position for the other
			TrailPosition = radarTarget.GetPreviousPosition(radarTarget.GetPreviousPosition(TrailPosition));
		}

		dc->SelectObject(&SymbolPen);

		// Symbol itself
		POINT radarTargetPoint = radar->ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

		int Size = DRAWING_AC_SQUARE_SYMBOL_SIZE;

		if (isDetailed)
			Size += (int)(Size*0.25);

		CRect Area(radarTargetPoint.x - Size, radarTargetPoint.y - Size,
			radarTargetPoint.x + Size, radarTargetPoint.y + Size);
		Area.NormalizeRect();
		dc->Rectangle(Area);

		// Pixel in center
		//dc->SetPixel(Area.CenterPoint(), SymbolColor);

		dc->RestoreDC(save);

		return Area;
	}

	static void DrawPrimaryTrailAndDiamong(CDC* dc, CRadarScreen* radar, CRadarTarget radarTarget, bool drawTrail) {
		int save = dc->SaveDC();

		COLORREF TrailColor = Colours::AircraftLightGrey.ToCOLORREF();
		COLORREF SymbolColor = Colours::AircraftLightGrey.ToCOLORREF();

		CPen TrailPen(PS_SOLID, 1, TrailColor);
		CPen SymbolPen(PS_SOLID, 1, SymbolColor);

		dc->SelectObject(&TrailPen);
		dc->SelectStockObject(NULL_BRUSH);

		

		// History Trail
		CRadarTargetPositionData TrailPosition = radarTarget.GetPreviousPosition(radarTarget.GetPreviousPosition(radarTarget.GetPosition()));
		for (int i = 0; i < 5; i++) {
			if (!drawTrail)
				break;

			POINT historyTrailPoint = radar->ConvertCoordFromPositionToPixel(TrailPosition.GetPosition());

			CRect Area(historyTrailPoint.x - DRAWING_AC_SQUARE_TRAIL_SIZE, historyTrailPoint.y - DRAWING_AC_SQUARE_TRAIL_SIZE,
				historyTrailPoint.x + DRAWING_AC_SQUARE_TRAIL_SIZE, historyTrailPoint.y + DRAWING_AC_SQUARE_TRAIL_SIZE);
			Area.NormalizeRect();
			dc->Rectangle(Area);

			// We skip one position for the other
			TrailPosition = radarTarget.GetPreviousPosition(radarTarget.GetPreviousPosition(TrailPosition));
		}

		dc->SelectObject(&SymbolPen);

		// Symbol itself
		POINT radarTargetPoint = radar->ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

		int DiamondSize = DRAWING_AC_PRIMARY_DIAMOND_SIZE;

		dc->MoveTo(radarTargetPoint.x, radarTargetPoint.y - DiamondSize);
		dc->LineTo(radarTargetPoint.x - DiamondSize, radarTargetPoint.y);
		dc->LineTo(radarTargetPoint.x, radarTargetPoint.y + DiamondSize);
		dc->LineTo(radarTargetPoint.x + DiamondSize, radarTargetPoint.y);
		dc->LineTo(radarTargetPoint.x, radarTargetPoint.y - DiamondSize);

		dc->RestoreDC(save);
	}

	static void DrawSpeedVector(CDC* dc, Tag::TagStates State, CRadarScreen* radar, CRadarTarget radarTarget, bool isPrimary, bool isSoft, int Seconds) {
		int save = dc->SaveDC();
		
		COLORREF VectorColor = Colours::AircraftDarkGrey.ToCOLORREF();

		if (!isSoft)
			VectorColor = Colours::AircraftLightGrey.ToCOLORREF();

		if (State == Tag::TagStates::Assumed || State == Tag::TagStates::Next || State == Tag::TagStates::TransferredToMe)
			VectorColor = Colours::AircraftGreen.ToCOLORREF();

		CPen GreenPen(PS_SOLID, 1, VectorColor);
		dc->SelectObject(&GreenPen);

		POINT AcPoint = radar->ConvertCoordFromPositionToPixel(radarTarget.GetPosition().GetPosition());

		double d = double(radarTarget.GetPosition().GetReportedGS()*0.514444) * (Seconds);
		CPosition PredictedEnd = Extrapolate(radarTarget.GetPosition().GetPosition(), radarTarget.GetTrackHeading(), d);
		POINT PredictedEndPoint = radar->ConvertCoordFromPositionToPixel(PredictedEnd);

		int bound = DRAWING_AC_SQUARE_SYMBOL_SIZE + DRAWING_PADDING;

		if (isPrimary)
			bound = DRAWING_AC_PRIMARY_DIAMOND_SIZE + DRAWING_PADDING;

		CRect Area(AcPoint.x - bound, AcPoint.y - bound,
			AcPoint.x + bound, AcPoint.y + bound);

		POINT ClipFrom, ClipTo;
		if (LiangBarsky(Area, AcPoint, PredictedEndPoint, ClipFrom, ClipTo)) {
			dc->MoveTo(ClipTo.x, ClipTo.y);
			dc->LineTo(PredictedEndPoint.x, PredictedEndPoint.y);
		}

		dc->RestoreDC(save);
	}
	
};
