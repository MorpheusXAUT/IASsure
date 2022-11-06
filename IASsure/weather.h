#pragma once

#include <cmath>
#include <istream>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "haversine.h"
#include "http.h"

namespace IASsure {
	class WeatherReferenceLevel {
	public:
		double temperature;
		double windSpeed;
		double windDirection;

		friend void from_json(const nlohmann::json& j, WeatherReferenceLevel& level);
	};

	class WeatherReferencePoint {
	public:
		WeatherReferenceLevel findClosest(int altitude) const;

		friend void from_json(const nlohmann::json& j, WeatherReferencePoint& points);
	private:
		double latitude;
		double longitude;
		std::map<int, WeatherReferenceLevel> levels;

		friend class Weather;
	};

	class WeatherInfo {
	public:
		friend void from_json(const nlohmann::json& j, WeatherInfo& info);
	private:
		std::string date;
		std::string datestring;
	};

	class Weather {
	public:
		Weather();
		Weather(std::string rawJSON);
		Weather(std::istream& rawJSON);

		void parse(std::string rawJSON);
		void parse(std::istream& rawJSON);
		WeatherReferenceLevel findClosest(double latitude, double longitude, int altitude) const;

		friend void from_json(const nlohmann::json& j, Weather& weather);
	private:
		WeatherInfo info;
		std::map<std::string, WeatherReferencePoint> points;
	};
}