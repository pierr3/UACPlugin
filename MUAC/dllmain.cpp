// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "MUAC.h"

//---EuroScopePlugInInit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance)
{
		// create the instance
		* ppPlugInInstance = (CPlugIn*)new MUAC();
}

//---EuroScopePlugInExit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInExit(void)
{

}