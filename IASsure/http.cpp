#include "http.h"

std::string IASsure::HTTP::get(const char* const url)
{
    return IASsure::HTTP::get(std::string(url));
}

std::string IASsure::HTTP::get(const std::string& url)
{
    auto [serverName, port, flags, path] = IASsure::HTTP::parseURL(url);
    HINTERNET hInternet = nullptr, hConnect = nullptr, hRequest = nullptr;
    std::string resp;

    try {
        hInternet = InternetOpenA(IASsure::HTTP::USER_AGENT.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
        if (hInternet == nullptr) {
            IASsure::HTTP::throwLastError("InternetOpenA");
        }

        hConnect = InternetConnectA(hInternet, serverName.c_str(), port, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect == nullptr) {
            IASsure::HTTP::throwLastError("InternetConnectA");
        }

        DWORD timeout = 10000; // 10s
        BOOL success = InternetSetOption(hConnect, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        if (!success) {
            IASsure::HTTP::throwLastError("InternetSetOption");
        }

        hRequest = HttpOpenRequestA(hConnect, "GET", path.c_str(), nullptr, nullptr, nullptr, flags, 0);
        if (hRequest == nullptr) {
            IASsure::HTTP::throwLastError("HttpOpenRequestA");
        }

        success = HttpSendRequestA(hRequest, nullptr, 0, nullptr, 0);
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

std::tuple<std::string, INTERNET_PORT, DWORD, std::string> IASsure::HTTP::parseURL(std::string url)
{
    std::string protocol = "http", hostname, path = "/";
    INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
    DWORD flags = INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;

    // find protocol
    int protocolEnd = url.find("://");
    if (protocolEnd != std::string::npos) {
        protocol = toLowercase(url.substr(0, protocolEnd));
        if (protocol == "https") {
            port = INTERNET_DEFAULT_HTTPS_PORT;
            flags |= INTERNET_FLAG_SECURE;
        } else if (protocol != "http") {
            throw std::invalid_argument("Invalid protocol: " + protocol);
        }
        url = url.substr(protocolEnd + 3); // skip "://"
    }

    // find port
    int portStart = url.find(":");
    int portEnd = url.find("/");
    if (portStart != std::string::npos && portEnd != std::string::npos && portEnd > portStart) {
        port = stoi(url.substr(portStart + 1, portEnd - portStart - 1));
        url = url.substr(0, portStart) + url.substr(portEnd); // remove the port from the URL
    }

    // find hostname
    int hostnameEnd = url.find("/");
    if (hostnameEnd != std::string::npos) {
        hostname = url.substr(0, hostnameEnd);
        url = url.substr(hostnameEnd); // keep the rest as path
        if (url.length() > 0) {
            path = url;
        }
    }
    else {
        hostname = url;
    }

    // return the results as a tuple
    return make_tuple(hostname, port, flags, path);
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
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
        GetModuleHandle("wininet.dll"), errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorText, 0, nullptr);
    if (formatResult && errorText != nullptr) {
        msg << ": " << errorText;
        LocalFree(errorText);
    }

    throw std::runtime_error(msg.str());
}