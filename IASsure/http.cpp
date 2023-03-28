#include "http.h"

std::string IASsure::HTTP::get(const char* const url)
{
    return IASsure::HTTP::get(std::string(url));
}

std::string IASsure::HTTP::get(const std::string& url)
{
    auto [serverName, path] = IASsure::HTTP::getServerNameAndPath(url);
    HINTERNET hInternet = nullptr, hConnect = nullptr, hRequest = nullptr;
    std::string resp;

    try {
        hInternet = InternetOpenA(IASsure::HTTP::USER_AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
        if (hInternet == nullptr) {
            IASsure::HTTP::throwLastError("InternetOpenA");
        }

        hConnect = InternetConnectA(hInternet, serverName.c_str(), INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect == nullptr) {
            IASsure::HTTP::throwLastError("InternetConnectA");
        }

        hRequest = HttpOpenRequestA(hConnect, "GET", path.c_str(), nullptr, nullptr, nullptr, INTERNET_FLAG_SECURE | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
        if (hRequest == nullptr) {
            IASsure::HTTP::throwLastError("HttpOpenRequestA");
        }

        BOOL success = HttpSendRequestA(hRequest, nullptr, 0, nullptr, 0);
        if (!success) {
            IASsure::HTTP::throwLastError("HttpSendRequestA");
        }

        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        success = HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusCodeSize, nullptr);
        if (!success) {
            IASsure::HTTP::throwLastError("HttpQueryInfoA");
        }

        if (statusCode != HTTP_STATUS_OK) {
            std::ostringstream msg;
            msg << "Received non-OK HTTP status code: " << statusCode;
            throw std::runtime_error(msg.str());
        }

        std::ostringstream response;
        char buffer[1024]{};
        DWORD bytesRead{ 0 };
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
            response.write(buffer, bytesRead);
        }

        resp = response.str();
    }
    catch (...) {
        if (hRequest != nullptr) {
            InternetCloseHandle(hRequest);
        }
        if (hConnect != nullptr) {
            InternetCloseHandle(hConnect);
        }
        if (hInternet != nullptr) {
            InternetCloseHandle(hInternet);
        }

        throw;
    }

    IASsure::trim(resp);

    return resp;
}

std::pair<std::string, std::string> IASsure::HTTP::getServerNameAndPath(const std::string& url)
{
    std::string serverName;
    std::string path;

    // Find the position of the start of the server name in the URL
    size_t start = url.find("://");
    if (start == std::string::npos) {
        throw std::invalid_argument("Invalid URL: no protocol specified.");
    }
    start += 3; // Skip over "://"

    // Find the position of the end of the server name in the URL
    size_t end = url.find('/', start);
    if (end == std::string::npos) {
        // No path was found, so the entire URL is the server name
        serverName = url.substr(start);
        path = "/";
    }
    else {
        // Split the URL into server name and path
        serverName = url.substr(start, end - start);
        path = url.substr(end);
    }

    return { serverName, path };
}

[[noreturn]] void IASsure::HTTP::throwLastError(const std::string& functionName)
{
    DWORD errorCode = GetLastError();
    LPSTR errorText = nullptr;

    std::ostringstream msg;
    msg << "Call";
    if (!functionName.empty()) {
        msg << " to " << functionName;
    }
    msg << " failed with error code " << errorCode;

    DWORD formatResult = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorText, 0, nullptr);
    if (formatResult && errorText != nullptr) {
        msg << ": " << errorText;
        LocalFree(errorText);
    }

    throw std::runtime_error(msg.str());
}