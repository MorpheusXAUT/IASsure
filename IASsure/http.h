#pragma once

#include <string>
#include <windows.h>
#include <wininet.h>

#include "constants.h"
#include "helpers.h"

namespace IASsure {
	namespace HTTP {
		std::string get(const std::string& url);
		std::string get(const char* const url);

		std::pair<std::string, std::string> getServerNameAndPath(const std::string& url);
		[[noreturn]] void throwLastError(const std::string& functionName);

		const std::string USER_AGENT = "IASsure/" + std::string(PLUGIN_VERSION);
	}
}