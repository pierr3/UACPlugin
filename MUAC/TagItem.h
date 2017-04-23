#pragma once
#include <string>
#include "EuroScopePlugIn.h"

using namespace std;
using namespace EuroScopePlugIn;

class TagItem
{
public:
	const enum TagColourTypes { StateColor, Information, Highlight };

	TagItem() {};
	~TagItem() {};

	static TagItem CreatePassive(string text, int ClickId = 0, TagColourTypes ColourType = TagColourTypes::StateColor) {
		TagItem i; i.Text = text; i.ColourType = ColourType; i.TagType = text; i.ClickId = ClickId;
		return i;
	};

	wstring TextToWString() {
		return wstring(Text.begin(), Text.end());
	}

	string Text;
	TagColourTypes ColourType;
	string TagType;
	int ClickId;
};