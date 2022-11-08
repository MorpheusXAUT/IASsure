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

IASsure::Weather::Weather() : hash(-1)
{
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
	this->update(nlohmann::json::parse(rawJSON));
}

void IASsure::Weather::parse(std::istream& rawJSON)
{
	this->update(nlohmann::json::parse(rawJSON));
}

void IASsure::Weather::clear()
{
	std::scoped_lock<std::shared_mutex> lock(this->mutex);
	this->points.clear();
	this->hash = 0;
}

IASsure::WeatherReferenceLevel IASsure::Weather::findClosest(double latitude, double longitude, int altitude) const
{
	if (!this->mutex.try_lock_shared()) {
		// cannot acquire read lock (weather data is being updated right now), fallback to no winds in order to not block EuroScope
		return WeatherReferenceLevel();
	}

	if (this->points.empty()) {
		this->mutex.unlock_shared();

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

	// closest reference point has been found, unlock weather map for updates as we have a copy of altitude data available locally
	this->mutex.unlock_shared();

	return closest.findClosest(altitude);
}

void IASsure::Weather::update(const nlohmann::json& j)
{
	size_t newHash = 0;
	{
		std::shared_lock<std::shared_mutex> slock(this->mutex, std::defer_lock);
		std::scoped_lock lock(slock);

		newHash = std::hash<nlohmann::json> {}(j);
		// check if hash matches currently stored data as we don't need to exclusively lock weather data if no update is required
		if (this->hash == newHash) {
			return;
		}
	}

	std::scoped_lock<std::shared_mutex> lock(this->mutex);

	this->points.clear();

	j.get_to<IASsure::Weather>(*this);
	this->hash = newHash;
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

bool IASsure::WeatherReferenceLevel::isZero()
{
	return this->temperature == 0 && this->windDirection == 0 && this->windSpeed == 0;
}