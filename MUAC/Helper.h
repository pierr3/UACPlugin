#pragma once
#include "stdafx.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "Colours.h"
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

static void FillInAltitudeList(CPlugIn* Plugin, int FunctionId, int Current) {
	Plugin->AddPopupListElement(" 999 ", "", FunctionId, Current == 99999);
	for(int i = 410; i >= 0; i -= 10)
		Plugin->AddPopupListElement(string(string(" ") + to_string(i) + string(" ")).c_str(), "", FunctionId, Current/100 == i);
};

// Radians

inline static double DegToRad(double x)
{
	return x / 180.0 * M_PI;
};

inline static double RadToDeg(double x)
{
	return x / M_PI * 180.0;
};

// Utils

inline static bool IsInRect(POINT pt, CRect rect) {
	if (pt.x >= rect.left + 1 && pt.x <= rect.right - 1 && pt.y >= rect.top + 1 && pt.y <= rect.bottom - 1)
		return true;
	return false;
}

static void DrawHourGlassWithLeader(CDC * dc, POINT Position, POINT PositionOfLeader) {
	int save = dc->SaveDC();

	CPen SepToolColorPen(PS_SOLID, 1, Colours::OrangeTool.ToCOLORREF());
	dc->SelectObject(&SepToolColorPen);

	int Distance = 8;

	dc->MoveTo(Position);
	dc->LineTo(Position.x - Distance / 2, Position.y - Distance);
	dc->LineTo(Position.x + Distance / 2, Position.y - Distance);
	dc->LineTo(Position);
	dc->LineTo(Position.x - Distance / 2, Position.y + Distance);
	dc->LineTo(Position.x + Distance / 2, Position.y + Distance);
	dc->LineTo(Position);
	dc->LineTo(PositionOfLeader);

	dc->RestoreDC(save);
}

// Liang-Barsky function by Daniel White @ http://www.skytopia.com/project/articles/compsci/clipping.html
// This function inputs 8 numbers, and outputs 4 new numbers (plus a boolean value to say whether the clipped line is drawn at all).
//
static bool LiangBarsky(RECT Area, POINT fromSrc, POINT toSrc, POINT &ClipFrom, POINT &ClipTo)         // The output values, so declare these outside.
{

	double edgeLeft, edgeRight, edgeBottom, edgeTop, x0src, y0src, x1src, y1src;

	edgeLeft = Area.left;
	edgeRight = Area.right;
	edgeBottom = Area.top;
	edgeTop = Area.bottom;

	x0src = fromSrc.x;
	y0src = fromSrc.y;
	x1src = toSrc.x;
	y1src = toSrc.y;

	double t0 = 0.0;    double t1 = 1.0;
	double xdelta = x1src - x0src;
	double ydelta = y1src - y0src;
	double p = 0, q = 0, r;

	for (int edge = 0; edge<4; edge++) {   // Traverse through left, right, bottom, top edges.
		if (edge == 0) { p = -xdelta;    q = -(edgeLeft - x0src); }
		if (edge == 1) { p = xdelta;     q = (edgeRight - x0src); }
		if (edge == 2) { p = -ydelta;    q = -(edgeBottom - y0src); }
		if (edge == 3) { p = ydelta;     q = (edgeTop - y0src); }
		r = q / p;
		if (p == 0 && q<0) return false;   // Don't draw line at all. (parallel line outside)

		if (p<0) {
			if (r>t1) return false;         // Don't draw line at all.
			else if (r>t0) t0 = r;            // Line is clipped!
		}
		else if (p>0) {
			if (r<t0) return false;      // Don't draw line at all.
			else if (r<t1) t1 = r;         // Line is clipped!
		}
	}

	ClipFrom.x = long(x0src + t0*xdelta);
	ClipFrom.y = long(y0src + t0*ydelta);
	ClipTo.x = long(x0src + t1*xdelta);
	ClipTo.y = long(y0src + t1*ydelta);

	return true;        // (clipped) line is drawn
};

static CPosition Extrapolate(CPosition init, double angle, double meters)
{
	CPosition newPos;

	double d = (meters*0.00053996) / 60 * M_PI / 180;
	double trk = DegToRad(angle);
	double lat0 = DegToRad(init.m_Latitude);
	double lon0 = DegToRad(init.m_Longitude);

	double lat = asin(sin(lat0) * cos(d) + cos(lat0) * sin(d) * cos(trk));
	double lon = cos(lat) == 0 ? lon0 : fmod(lon0 + asin(sin(trk) * sin(d) / cos(lat)) + M_PI, 2 * M_PI) - M_PI;

	newPos.m_Latitude = RadToDeg(lat);
	newPos.m_Longitude = RadToDeg(lon);

	return newPos;
};


// From http://www.edwilliams.org/avform.htm#Intersection
static bool IntersectRadials(CPosition firstPoint, double firstBearing, CPosition secondPoint, double secondBearing, CPosition* Intersect) {
	double lat1 = DegToRad(firstPoint.m_Latitude);
	double lon1 = DegToRad(firstPoint.m_Longitude);
	double lat2 = DegToRad(secondPoint.m_Latitude);
	double lon2 = DegToRad(secondPoint.m_Longitude);

	double crs13 = DegToRad(firstBearing);
	double crs23 = DegToRad(secondBearing);

	double dst12 = 2 * asin(sqrt((sin((lat1 - lat2) / 2.0))*(sin((lat1 - lat2) / 2.0)) +
		cos(lat1)*cos(lat2)*(sin((lon1 - lon2) / 2.0)*sin((lon1 - lon2) / 2.0))));
		
	double crs12, crs21;
	if (sin(lon2 - lon1) < 0) {
		crs12 = acos((sin(lat2) - sin(lat1)*cos(dst12)) / (sin(dst12)*cos(lat1)));
		crs21 = 2.0*M_PI - acos((sin(lat1) - sin(lat2)*cos(dst12)) / (sin(dst12)*cos(lat2)));
	}
	else {
		crs12 = 2.0*M_PI - acos((sin(lat2) - sin(lat1)*cos(dst12)) / (sin(dst12)*cos(lat1)));
		crs21 = acos((sin(lat1) - sin(lat2)*cos(dst12)) / (sin(dst12)*cos(lat2)));
	}
		

	double ang1 = fmod(crs13 - crs12 + M_PI, 2.0*M_PI) - M_PI;
	double ang2 = fmod(crs21 - crs23 + M_PI, 2.0*M_PI) - M_PI;

	if (sin(ang1) == 0 && sin(ang2) == 0) {
		return false;
	}
	else if ((sin(ang1)*sin(ang2))<0) {
		return false;
	}
	else {
		ang1 = abs(ang1);
		ang2 = abs(ang2);
		double ang3 = acos(-cos(ang1)*cos(ang2) + sin(ang1)*sin(ang2)*cos(dst12));
		double dst13 = atan2(cos(ang2) + cos(ang1)*cos(ang3), sin(dst12)*sin(ang1)*sin(ang2));
		double lat3 = asin(sin(lat1)*cos(dst13) + cos(lat1)*sin(dst13)*cos(crs13));
		double dlon = atan2(cos(dst13) - sin(lat1)*sin(lat3), sin(crs13)*sin(dst13)*cos(lat1));
		double lon3 = fmod(lon1 - dlon + M_PI, 2 * M_PI) - M_PI;

		CPosition p;
		p.m_Latitude = RadToDeg(lat3);
		p.m_Longitude = RadToDeg(lon3);
		*Intersect = p;
		return true;
	}
}

inline static string padWithZeros(int padding, int s)
{
	stringstream ss;
	ss << setfill('0') << setw(padding) << s;
	return ss.str();
};

inline static int RoundTo(int n, int multiplier) {
	return (n + multiplier/2) / multiplier * multiplier;
};

inline static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
};
inline static std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
};