#pragma once

#include <numbers>

namespace IASsure {
	double haversine(const double lat1, const double long1, const double lat2, const double long2);
	double degToRad(const double degrees);

	constexpr double EARTH_MEAN_RADIUS_METERS = 6371008.7714;
}