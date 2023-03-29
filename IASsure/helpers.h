#pragma once

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <Windows.h>

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

	inline void ltrim(std::string& s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));
	}

	inline void rtrim(std::string& s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), s.end());
	}

	inline void trim(std::string& s)
	{
		ltrim(s);
		rtrim(s);
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

	extern "C" IMAGE_DOS_HEADER __ImageBase;

	inline std::string getPluginDirectory()
	{
		char buf[MAX_PATH] = { 0 };
		GetModuleFileName(HINSTANCE(&__ImageBase), buf, MAX_PATH);

		std::string::size_type pos = std::string(buf).find_last_of("\\/");

		return std::string(buf).substr(0, pos);
	}

	inline COLORREF* parseRGBString(const std::string& s)
	{
		std::vector<std::string> rgbValues = split(s, ',');
		if (rgbValues.size() != 3) {
			return nullptr;
		}

		trim(rgbValues[0]);
		trim(rgbValues[1]);
		trim(rgbValues[2]);
		
		int r, g, b;
		try {
			r = std::stoi(rgbValues[0]);
			g = std::stoi(rgbValues[1]);
			b = std::stoi(rgbValues[2]);
		}
		catch (std::exception) {
			return nullptr;
		}

		if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
			return nullptr;
		}

		return new COLORREF(RGB(r, g, b));
	}

	inline std::string toLowercase(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {return std::tolower(c); });
		return str;
	}
}