#pragma once
#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

const string PLUGIN_NAME = "MUAC PlugIn";
const string PLUGIN_VERSION = "312E30";
const string PLUGIN_AUTHOR = "github.com/pierr3";
const string PLUGIN_COPY = "GPL v3";

#define MUAC_RADAR_SCREEN_VIEW "MUAC Radar Screen"

const string PREFIX_PURPLE_COLOR = "_/";

const int SCREEN_TAG = 0;
const int SCREEN_AC_SYMBOL = 1;

const int SCREEN_SEP_TOOL = 20;

const int DRAWING_AC_SQUARE_SYMBOL_SIZE = 4;
const int DRAWING_AC_SQUARE_TRAIL_SIZE = 1;

const int DRAWING_PADDING = 3;

const long FONT_SIZE = 15;

// Menu Buttons
const int BUTTON_HIDEMENU = 401;
const int BUTTON_DECREASE_RANGE = 402;
const int BUTTON_RANGE = 403;
const int BUTTON_INCREASE_RANGE = 404;

const int BUTTON_FILTER_ON = 405;
const int BUTTON_FILTER_HARD_LOW = 406;
const int BUTTON_FILTER_SOFT_LOW = 407;
const int BUTTON_FILTER_SOFT_HIGH = 408;
const int BUTTON_FILTER_HARD_HIGH = 409;

const int BUTTON_MODE_A = 410;
const int BUTTON_LABEL_V = 411;

const int BUTTON_PRIMARY_TARGETS_ON = 412;

const int BUTTON_VFR_ON = 413;

const int BUTTON_VELOTH = 414;
const int BUTTON_VEL1 = 415;
const int BUTTON_VEL2 = 416;
const int BUTTON_VEL4 = 417;
const int BUTTON_VEL8 = 418;

const int BUTTON_DOTS = 419;

// Clickable tag items

const int SCREEN_TAG_CALLSIGN = 701;
const int SCREEN_TAG_SECTOR = 702;
const int SCREEN_TAG_ROUTE = 703;
const int SCREEN_TAG_CFL = 704;
const int SCREEN_TAG_HORIZ = 705;
const int SCREEN_TAG_RFL = 706;
const int SCREEN_TAG_XFL = 707;
const int SCREEN_TAG_COP = 708;
const int SCREEN_TAG_SEP = 709;

// Functions
const int FUNC_FILTER_HARD_LOW = BUTTON_FILTER_HARD_LOW;
const int FUNC_FILTER_SOFT_LOW = BUTTON_FILTER_SOFT_LOW;
const int FUNC_FILTER_SOFT_HIGH = BUTTON_FILTER_SOFT_HIGH;
const int FUNC_FILTER_HARD_HIGH = BUTTON_FILTER_HARD_HIGH;

 

const enum AircraftStates {
	NOT_CONCERNED, NOTIFIED, CONCERNED, TRANSFERRED, ASSUMED
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

static CPosition BetterHarversine(CPosition init, double angle, double meters)
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