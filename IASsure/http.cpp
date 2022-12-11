#include "http.h"

std::string IASsure::HTTP::get(const std::string url)
{
    return IASsure::HTTP::get(url.c_str());
}

std::string IASsure::HTTP::get(const char* const url)
{
    HINTERNET connect = InternetOpen(IASsure::HTTP::USER_AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!connect) {
        throw std::exception(IASsure::HTTP::getLastErrorMessage("InternetOpen failed: ").c_str());
    }

    HINTERNET address = InternetOpenUrl(connect, url, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
    if (!address) {
        std::string error = IASsure::HTTP::getLastErrorMessage("InternetOpenUrl failed: ");
        InternetCloseHandle(connect);
        throw std::exception(error.c_str());
    }

    char data[256]{};
    DWORD bytesReceived{ 0 };
    std::string resp;
    while (InternetReadFile(address, data, 256, &bytesReceived) && bytesReceived) {
        resp.append(data, bytesReceived);
    }

    InternetCloseHandle(address);
    InternetCloseHandle(connect);

    IASsure::trim(resp);

    return resp;
}

std::string IASsure::HTTP::getLastErrorMessage(std::string prefix)
{
	try {
		LPVOID msgBuf = nullptr;
		DWORD dw = GetLastError();

		std::ostringstream msg;
		if (!prefix.empty()) {
			msg << prefix;
		}

		DWORD res = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
			GetModuleHandle("wininet.dll"),
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&msgBuf,
			0,
			NULL);
		if (!res) {
			msg << "FormatMessage failed with error " << GetLastError() << ", original error was " << dw;
			return msg.str();
		}

		msg << (LPCTSTR)msgBuf;

		LocalFree(msgBuf);

		return msg.str();
	}
	catch (std::exception ex) {
		std::ostringstream msg;
		if (!prefix.empty()) {
			msg << prefix;
		}
		msg << "Failed to get last error message: " << ex.what();
		return msg.str();
	}
}