#include "haversine.h"

double IASsure::haversine(const double lat1, const double long1, const double lat2, const double long2)
{
    // Taken from http://www.movable-type.co.uk/scripts/latlong.html @ 2022-11-04T20:40:00Z
    double phi1 = degToRad(lat1);
    double phi2 = degToRad(lat2);

    double deltaPhi = degToRad(lat2 - lat1);
    double deltaLambda = degToRad(long2 - long1);

    double a = sin(deltaPhi / 2) * sin(deltaPhi / 2) + cos(phi1) * cos(phi2) * sin(deltaLambda / 2) * sin(deltaLambda / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return c * EARTH_MEAN_RADIUS_METERS;
}

double IASsure::degToRad(const double degrees)
{
    return degrees * (std::numbers::pi / 180);
}
