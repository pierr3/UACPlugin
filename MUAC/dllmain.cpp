// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "EuroScopePlugIn.h"
#include "MUAC.h"

using namespace Gdiplus;

ULONG_PTR m_gdiplusToken;

//---EuroScopePlugInInit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance)
{
	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

	// create the instance
	* ppPlugInInstance = (CPlugIn*)new MUAC();
}

//---EuroScopePlugInExit-----------------------------------------------

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	GdiplusShutdown(m_gdiplusToken);
}