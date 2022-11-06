#include "calculations.h"

double IASsure::calculateTAS(double hdg, double gs, ::IASsure::WeatherReferenceLevel lvl)
{
	if (gs <= 0) {
		throw std::domain_error("ground speed outside of supported range");
	}

	double c = std::cos(IASsure::degToRad(lvl.windDirection - hdg));
	double windComponent = c * lvl.windSpeed;
	return gs + windComponent;
}

double IASsure::calculateTemperature(double alt)
{
	double altM = (double)alt * METER_PER_FEET;

	if (alt > ALTITUDE_HIGH_UPPER_LIMIT_FEET) {
		throw std::domain_error("altitude outside of supported range");
	} else if (alt < ALTITUDE_LOW_UPPER_LIMIT_FEET) {
		return STANDARD_TEMPERATURE_LOW + TEMPERATURE_LAPSE_RATE_LOW * altM;
	}

	return STANDARD_TEMPERATURE_HIGH + TEMPERATURE_LAPSE_RATE_HIGH * (altM - ALTITUDE_LOW_UPPER_LIMIT);
}

double IASsure::calculateStaticPressure(double alt)
{
	double altM = (double)alt * METER_PER_FEET;

	if (alt > ALTITUDE_HIGH_UPPER_LIMIT_FEET) {
		throw std::domain_error("altitude outside of supported range");
	} else if (alt < ALTITUDE_LOW_UPPER_LIMIT_FEET) {
		double beta = GRAVITATIONAL_ACCELERATION_SEA_LEVEL / (SPECIFIC_GAS_CONSTANT_DRY_AIR * TEMPERATURE_LAPSE_RATE_LOW);
		return STATIC_PRESSURE_LOW * std::pow(1 + ((TEMPERATURE_LAPSE_RATE_LOW * altM) / STANDARD_TEMPERATURE_LOW), -1 * beta);
	}
	
	double hs = (SPECIFIC_GAS_CONSTANT_DRY_AIR * STANDARD_TEMPERATURE_HIGH) / GRAVITATIONAL_ACCELERATION_SEA_LEVEL;
	return STATIC_PRESSURE_HIGH * std::exp(-1 * ((altM - ALTITUDE_LOW_UPPER_LIMIT) / hs));
}

double IASsure::calculateSpeedOfSound(double temp)
{
	return std::sqrt(HEAT_CAPACITY_RATIO_AIR * SPECIFIC_GAS_CONSTANT_DRY_AIR * temp);
}

double IASsure::calculateDynamicPressure(double ps, double temp, double tas)
{
	if (tas <= 0) {
		throw std::domain_error("true air speed outside of supported range");
	}

	double a = std::sqrt(HEAT_CAPACITY_RATIO_AIR * SPECIFIC_GAS_CONSTANT_DRY_AIR * temp);

	double tmp1 = (HEAT_CAPACITY_RATIO_AIR - 1) / 2;
	double tmp2 = std::pow((tas * METERS_PER_SECOND_PER_KNOT)/ a, 2);
	double tmp3 = std::pow((tmp1 * tmp2) + 1, HEAT_CAPACITY_RATIO_AIR / (HEAT_CAPACITY_RATIO_AIR - 1));

	double qc = ps * (tmp3 - 1);

	return qc;
}

double IASsure::calculateCAS(double alt, double hdg, double gs, ::IASsure::WeatherReferenceLevel lvl)
{
	double tas = calculateTAS(hdg, gs, lvl);

	// adapted from http://walter.bislins.ch/blog/index.asp?page=Fluggeschwindigkeiten%2C+IAS%2C+TAS%2C+EAS%2C+CAS%2C+Mach @ 2022-07-31T20:23:00Z
	double ps = calculateStaticPressure(alt);
	double temp = calculateTemperature(alt);
	double qc = calculateDynamicPressure(ps, temp, tas);

	double tmp1 = 2 / (HEAT_CAPACITY_RATIO_AIR - 1);
	double tmp2 = std::pow((qc / ATMOSPHERIC_PRESSURE_SEA_LEVEL) + 1, (HEAT_CAPACITY_RATIO_AIR - 1) / HEAT_CAPACITY_RATIO_AIR);
	double tmp3 = tmp1 * (tmp2 - 1);

	double cas = SPEED_OF_SOUND_LOW * std::sqrt(tmp3) * KNOTS_PER_METER_PER_SECOND;

	return cas;
}

double IASsure::calculateMach(double alt, double hdg, double gs, ::IASsure::WeatherReferenceLevel lvl)
{
	double tas = calculateTAS(hdg, gs, lvl);

	if (tas <= 0) {
		throw std::domain_error("true air speed outside of supported range");
	}

	// adapted from http://walter.bislins.ch/blog/index.asp?page=Fluggeschwindigkeiten%2C+IAS%2C+TAS%2C+EAS%2C+CAS%2C+Mach#H_Mach_Speed @ 2022-08-03T22:17:28Z
	double temp = calculateTemperature(alt);
	double a = calculateSpeedOfSound(temp);

	double mach = (tas * METERS_PER_SECOND_PER_KNOT) / a;

	return mach;
}
