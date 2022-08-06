#pragma once

#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

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

	inline long roundToNearest(long num, long multiple)
	{
		if (multiple == 0) {
			throw std::domain_error("multiple is zero");
		}
		return ((num + multiple / 2) / multiple) * multiple;
	}

	inline long roundToNearest(double num, long multiple)
	{
		return roundToNearest(std::lround(num), multiple);
	}
}