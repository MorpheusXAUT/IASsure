#include "http.h"

std::string IASsure::HTTP::get(const std::string url)
{
    return IASsure::HTTP::get(url.c_str());
}

std::string IASsure::HTTP::get(const char* const url)
{
    HINTERNET connect = InternetOpen(IASsure::HTTP::USER_AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!connect) {
        throw std::exception("InternetOpen failed");
    }

    HINTERNET address = InternetOpenUrl(connect, url, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
    if (!address) {
        InternetCloseHandle(connect);
        throw std::exception("InternetOpenUrl failed");
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
