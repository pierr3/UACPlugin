#pragma once
#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class TagItem
{
public:
	const enum TagColourTypes { StateColor, DimOnSituation, Information, Event };

	TagItem() {};
	~TagItem() {};

	static TagItem CreatePassive(string text, TagColourTypes ColourType = TagColourTypes::StateColor) {
		TagItem i; i.Text = text; i.ColourType = ColourType; i.TagType = text;
		return i;
	};

	wstring TextToWString() {
		return wstring(Text.begin(), Text.end());
	}

	string Text;
	TagColourTypes ColourType;
	string TagType;
};