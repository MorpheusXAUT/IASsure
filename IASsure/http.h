#pragma once

#include <string>
#include <windows.h>
#include <wininet.h>

#include "constants.h"
#include "helpers.h"

namespace IASsure {
	namespace HTTP {
		std::string get(const std::string url);
		std::string get(const char* const url);

		const std::string USER_AGENT = "IASsure/" + std::string(PLUGIN_VERSION);
	}
}