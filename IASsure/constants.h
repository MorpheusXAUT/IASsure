#pragma once

#include <Windows.h>

constexpr auto PLUGIN_NAME = "IASsure";
constexpr auto PLUGIN_VERSION = "1.3.0-rc.4";
constexpr auto PLUGIN_AUTHOR = "Nick Mueller";
constexpr auto PLUGIN_LICENSE = "(c) 2022, MIT License";

constexpr auto SETTINGS_DELIMITER = '|';

constexpr auto DEFAULT_WEATHER_UPDATE_URL = "https://weather.morpheusxaut.net/weather.json";

const int TAG_ITEM_CALCULATED_IAS = 1;
const int TAG_ITEM_CALCULATED_IAS_TOGGLABLE = 2;
const int TAG_ITEM_CALCULATED_IAS_ABBREVIATED = 3;
const int TAG_ITEM_CALCULATED_IAS_ABBREVIATED_TOGGLABLE = 4;
const int TAG_ITEM_CALCULATED_MACH = 5;
const int TAG_ITEM_CALCULATED_MACH_TOGGLABLE = 6;

const int TAG_FUNC_OPEN_REPORTED_IAS_MENU = 100;
const int TAG_FUNC_CLEAR_REPORTED_IAS = 101;
const int TAG_FUNC_TOGGLE_CALCULATED_IAS = 102;
const int TAG_FUNC_TOGGLE_CALCULATED_IAS_ABBREVIATED = 103;
const int TAG_FUNC_OPEN_REPORTED_MACH_MENU = 104;
const int TAG_FUNC_CLEAR_REPORTED_MACH = 105;
const int TAG_FUNC_TOGGLE_CALCULATED_MACH = 106;
const int TAG_FUNC_SET_REPORTED_IAS = 200;
const int TAG_FUNC_SET_REPORTED_MACH = 201;

const int MIN_REPORTED_IAS = 100;
const int MAX_REPORTED_IAS = 600;
const int INTERVAL_REPORTED_IAS = 5;
const int MIN_REPORTED_MACH = 01;
const int MAX_REPORTED_MACH = 99;
const int INTERVAL_REPORTED_MACH = 1;

constexpr auto CONFIG_FILE_NAME = "config.json";