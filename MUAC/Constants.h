#pragma once
#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

const string PLUGIN_NAME = "MUAC PlugIn";
const string PLUGIN_VERSION = "@appveyor_build";
const string PLUGIN_AUTHOR = "github.com/pierr3";
const string PLUGIN_COPY = "GPL v3";

#define MUAC_RADAR_SCREEN_VIEW "MUAC Radar Screen"

const string PREFIX_PURPLE_COLOR = "_/";
const string PREFIX_ORANGE_COLOR = "_-";
const string PREFIX_BACKSTEP = "_'";
const string PREFIX_FORESTEP = "_|";

const int SCREEN_TAG = 0;
const int SCREEN_AC_SYMBOL = 1;

const int SCREEN_SEP_TOOL = 20;

const int DRAWING_AC_SQUARE_SYMBOL_SIZE = 4;
const int DRAWING_AC_SQUARE_TRAIL_SIZE = 1;
const int DRAWING_AC_PRIMARY_DIAMOND_SIZE = 3;

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

const int BUTTON_RDF_UNUSED = 410;
const int BUTTON_QDM = 411;
const int BUTTON_TOPDOWN_UNUSED = 412;

const int BUTTON_MODE_A = 413;
const int BUTTON_LABEL_V = 414;

const int BUTTON_PRIMARY_TARGETS_ON = 415;

const int BUTTON_VFR_ON = 416;

const int BUTTON_VELOTH = 417;
const int BUTTON_VEL1 = 418;
const int BUTTON_VEL2 = 419;
const int BUTTON_VEL4 = 420;
const int BUTTON_VEL8 = 421;

const int BUTTON_DOTS = 422;

const int FIM_WINDOW = 501;

const int FIM_STAR = 502;
const int FIM_RWY = 503;
const int FIM_SCRATCHPAD = 504;

const int MTCD_WINDOW = 551;

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
const int SCREEN_TAG_SSR = 710;
const int SCREEN_TAG_ASPEED = 711;

// Functions
const int FUNC_FILTER_HARD_LOW = BUTTON_FILTER_HARD_LOW;
const int FUNC_FILTER_SOFT_LOW = BUTTON_FILTER_SOFT_LOW;
const int FUNC_FILTER_SOFT_HIGH = BUTTON_FILTER_SOFT_HIGH;
const int FUNC_FILTER_HARD_HIGH = BUTTON_FILTER_HARD_HIGH;

// Saved Status 

const string SAVE_MTCD_POSX = "MTCDWindowPosX";
const string SAVE_MTCD_POSY = "MTCDWindowPosY";
const string SAVE_FIM_POSX = "FIMWindowPosX";
const string SAVE_FIM_POSY = "FIMWindowPosY";

const string SAVE_HARD_FILTER_LOWER = "UACFilterHardLower";
const string SAVE_SOFT_FILTER_LOWER = "UACFilterSoftLower";
const string SAVE_SOFT_FILTER_UPPER = "UACFilterSoftUpper";
const string SAVE_HARD_FILTER_UPPER = "UACFilterHardUpper";

const string SAVE_VEL_TIME = "UACVELTime";

const enum AircraftStates {
	NOT_CONCERNED, NOTIFIED, CONCERNED, TRANSFERRED, ASSUMED
};
