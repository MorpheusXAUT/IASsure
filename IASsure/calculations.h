#pragma once

#include <cmath>
#include <stdexcept>

namespace IASsure {
	double calculateTemperature(double alt);
	double calculateStaticPressure(double altM);
	double calculateDynamicPressure(double ps, double temp, double tas);
	double calculateSpeedOfSound(double temp);
	double calculateCAS(double alt, double tas);
	double calculateMach(double alt, double tas);
}

constexpr double HEAT_CAPACITY_RATIO_AIR = 1.403; // https://en.wikipedia.org/wiki/Heat_capacity_ratio
constexpr double ATMOSPHERIC_PRESSURE_SEA_LEVEL = 101325; // in Pa https://en.wikipedia.org/wiki/Atmospheric_pressure#Altitude_variation
constexpr double FEET_PER_METER = 3.28084;
constexpr double METER_PER_FEET = 1.0 / FEET_PER_METER;
constexpr double ALTITUDE_LOW_UPPER_LIMIT = 11000; // in m
constexpr double ALTITUDE_LOW_UPPER_LIMIT_FEET = ALTITUDE_LOW_UPPER_LIMIT * FEET_PER_METER; // in ft
constexpr double ALTITUDE_HIGH_UPPER_LIMIT = 20000; // in m
constexpr double ALTITUDE_HIGH_UPPER_LIMIT_FEET = ALTITUDE_HIGH_UPPER_LIMIT * FEET_PER_METER; // in ft
constexpr double SPEED_OF_SOUND_LOW = 340.27; // in m/s https://en.wikipedia.org/wiki/Speed_of_sound#Tables
constexpr double SPEED_OF_SOUND_HIGH = 295.0; // in m/s https://en.wikipedia.org/wiki/Speed_of_sound#Tables
constexpr double STATIC_PRESSURE_LOW = 101325.0; // in Pa https://en.wikipedia.org/wiki/Barometric_formula#Pressure_equations
constexpr double STATIC_PRESSURE_HIGH = 22632.1; // in Pa https://en.wikipedia.org/wiki/Barometric_formula#Pressure_equations
constexpr double STANDARD_TEMPERATURE_LOW = 288.15; // in K https://en.wikipedia.org/wiki/Barometric_formula#Pressure_equations
constexpr double STANDARD_TEMPERATURE_HIGH = 216.65; // in K https://en.wikipedia.org/wiki/Barometric_formula#Pressure_equations
constexpr double TEMPERATURE_LAPSE_RATE_LOW = -0.0065; // in K/m https://en.wikipedia.org/wiki/Barometric_formula#Pressure_equations
constexpr double TEMPERATURE_LAPSE_RATE_HIGH = 0.0; // in K/m https://en.wikipedia.org/wiki/Barometric_formula#Pressure_equations
constexpr double GRAVITATIONAL_ACCELERATION_SEA_LEVEL = 9.80665; // in m/s^2 https://en.wikipedia.org/wiki/Gravitational_acceleration
constexpr double SPECIFIC_GAS_CONSTANT_DRY_AIR = 287.058; // in J/kg/K https://en.wikipedia.org/wiki/Gas_constant#Specific_gas_constant
constexpr double KELVIN_CELSIUS_OFFSET = 273.15;
constexpr double TEMPERATURE_ISOTHERM = -55.0;
constexpr double METERS_PER_SECOND_PER_KNOT = 0.51444444444444;
constexpr double KNOTS_PER_METER_PER_SECOND = 1.0 / METERS_PER_SECOND_PER_KNOT;