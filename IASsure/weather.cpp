#include "weather.h"

void IASsure::from_json(const nlohmann::json& j, Weather& weather)
{
	j.at("info").get_to<IASsure::WeatherInfo>(weather.info);
	j.at("data").get_to<std::map<std::string, IASsure::WeatherReferencePoint>>(weather.points);
}

void IASsure::from_json(const nlohmann::json& j, WeatherInfo& info)
{
	j.at("date").get_to(info.date);
	j.at("datestring").get_to(info.datestring);
}

void IASsure::from_json(const nlohmann::json& j, WeatherReferencePoint& point)
{
	auto& coords = j.at("coords");
	point.latitude = std::stod(coords.at("lat").get<std::string>());
	point.longitude = std::stod(coords.at("long").get<std::string>());

	for (auto it = j.at("levels").begin(); it != j.at("levels").end(); it++) {
		point.levels.insert({ std::stoi(it.key()), it.value().get<IASsure::WeatherReferenceLevel>() });
	}
}

void IASsure::from_json(const nlohmann::json& j, IASsure::WeatherReferenceLevel& level)
{
	level.temperature = std::stod(j.at("T(K)").get<std::string>());
	level.windSpeed = std::stod(j.at("windspeed").get<std::string>());
	level.windDirection = std::stod(j.at("windhdg").get<std::string>());
}

IASsure::Weather::Weather(std::string rawJSON)
{
	this->parse(rawJSON);
}

IASsure::Weather::Weather(std::istream& rawJSON)
{
	this->parse(rawJSON);
}

void IASsure::Weather::parse(std::string rawJSON)
{
	this->points.clear();
	nlohmann::json j = nlohmann::json::parse(rawJSON);
	j.get_to<IASsure::Weather>(*this);
}

void IASsure::Weather::parse(std::istream& rawJSON)
{
	this->points.clear();
	nlohmann::json j = nlohmann::json::parse(rawJSON);
	j.get_to<IASsure::Weather>(*this);
}

IASsure::WeatherReferenceLevel IASsure::Weather::findClosest(double latitude, double longitude, int altitude) const
{
	if (this->points.empty()) {
		// no reference points available, return empty reference level containing zero winds/temperature
		return IASsure::WeatherReferenceLevel();
	}

	double distance = -1;
	WeatherReferencePoint closest;

	for (auto it = this->points.begin(); it != this->points.end(); it++) {
		double d = IASsure::haversine(latitude, longitude, it->second.latitude, it->second.longitude);
		if (distance < 0 || d < distance) {
			distance = d;
			closest = it->second;
		}
	}

	return closest.findClosest(altitude);
}

IASsure::WeatherReferenceLevel IASsure::WeatherReferencePoint::findClosest(int altitude) const
{
	if (this->levels.empty()) {
		// no data available for reference point, return empty reference level containing zero winds/temperature
		return IASsure::WeatherReferenceLevel();
	}

	long fl = std::lround((double)altitude / 100.0);

	int prevDiff = -1;
	// initialise with highest level available in case the altitude is greater than highest flight level available.
	// levels map contains ordered (ascending) keys, highest available data will be last in map.
	// reverse iterator starts from the end of the map, returning last item.
	IASsure::WeatherReferenceLevel closest = this->levels.rbegin()->second;
	for (auto it = this->levels.begin(); it != this->levels.end(); it++) {
		if (fl == it->first) {
			return it->second;
		}

		int diff = std::abs(it->first - fl);
		if (prevDiff > 0 && diff > prevDiff) {
			// distances will continue increasing, no need to search further
			break;
		}

		prevDiff = diff;
		closest = it->second;
	}

	return closest;
}