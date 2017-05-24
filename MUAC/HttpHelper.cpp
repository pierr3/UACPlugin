#include "stdafx.h"
#include "HttpHelper.h"

string LoadHttpString(string url)
{
	const string AGENT{ "EuroScopeMUAC/" + PLUGIN_VERSION };
	HINTERNET connect = InternetOpen(AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (connect) {
		HINTERNET OpenAddress = InternetOpenUrl(connect, url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
		if (OpenAddress) {
			char DataReceived[256];
			DWORD NumberOfBytesRead{ 0 };
			std::string answer;
			while (InternetReadFile(OpenAddress, DataReceived, 256, &NumberOfBytesRead) && NumberOfBytesRead)
				answer.append(DataReceived, NumberOfBytesRead);

			InternetCloseHandle(OpenAddress);
			InternetCloseHandle(connect);
			return answer;
		}
		else {
			InternetCloseHandle(connect);
		}
	}

	return "";
}
