#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "constants.h"

namespace IASsure {
	inline std::vector<std::string> split(const std::string& s, char delim = ' ')
	{
		std::istringstream ss(s);
		std::string item;
		std::vector<std::string> res;

		while (std::getline(ss, item, delim)) {
			res.push_back(item);
		}

		return res;
	}

	inline int roundToNearest(int num, int multiple) {
		return ((num + multiple / 2) / multiple) * multiple;
	}

	inline int roundToNearest(double num, int multiple) {
		return roundToNearest((int)num, multiple);
	}
}