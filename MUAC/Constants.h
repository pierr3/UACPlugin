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
const string PREFIX_BLUE_COLOR = "_}";
const string PREFIX_ORANGE_COLOR = "_-";
const string PREFIX_GREY_COLOR = "_]";
const string PREFIX_YELLOW_UNDERLINE = "_[";
const string PREFIX_BACKSTEP = "_'";
const string PREFIX_FORESTEP = "_|";

const int SCREEN_TAG = 0;
const int SCREEN_AC_SYMBOL = 1;
const int SCREEN_AC_APP_ARROW = 5;

const int SCREEN_SEP_TOOL = 20;
const int SCREEN_QDM_TOOL = 21;

const int SCREEN_BACKGROUND = 31;

const int DRAWING_AC_SQUARE_SYMBOL_SIZE = 4;
const int DRAWING_AC_SQUARE_TRAIL_SIZE = 1;
const int DRAWING_AC_PRIMARY_DIAMOND_SIZE = 4;

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

const int BUTTON_MTCD = 410;
const int BUTTON_FIM = 411;

const int BUTTON_QDM = 421;
const int BUTTON_TOPDOWN = 422;

const int BUTTON_MODE_A = 423;
const int BUTTON_LABEL_V = 424;

const int BUTTON_PRIMARY_TARGETS_ON = 425;

const int BUTTON_VFR_ON = 426;

const int BUTTON_VELOTH = 427;
const int BUTTON_VEL1 = 428;
const int BUTTON_VEL2 = 429;
const int BUTTON_VEL4 = 430;
const int BUTTON_VEL8 = 431;

const int BUTTON_DOTS = 432;

const int BUTTON_FIN = 433;


const int FIM_WINDOW = 501;

const int FIM_STAR = 502;
const int FIM_RWY = 503;
const int FIM_SCRATCHPAD = 504;
const int FIM_CLOSE = 505;

const int MTCD_WINDOW = 551;
const int MTCD_WINDOW_BUTTONS = 552;

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
const int SCREEN_TAG_RWY = 712;
const int SCREEN_TAG_ADES = 713;
const int SCREEN_TAG_GSPEED = 714;
const int SCREEN_TAG_STAR = 715;

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

const string SAVE_VFR_FILTER = "UACVFRFilter";

const enum AircraftStates {
	NOT_CONCERNED, NOTIFIED, CONCERNED, TRANSFERRED, ASSUMED
};
