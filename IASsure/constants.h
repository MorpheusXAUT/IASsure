#pragma once

constexpr auto PLUGIN_NAME = "IASsure";
constexpr auto PLUGIN_VERSION = "1.0.0";
constexpr auto PLUGIN_AUTHOR = "Nick Mueller";
constexpr auto PLUGIN_LICENSE = "(c) 2022, MIT License";

constexpr auto SETTINGS_DELIMITER = '|';

const int TAG_ITEM_CORRECTED_IAS = 1;

const int TAG_FUNC_OPEN_REPORTED_IAS_MENU = 100;
const int TAG_FUNC_SET_REPORTED_IAS = 101;
const int TAG_FUNC_CLEAR_REPORTED_IAS = 102;

const int MIN_MIN_REPORTED_IAS = 0;
const int MAX_MIN_REPORTED_IAS = 500;
const int MIN_MAX_REPORTED_IAS = 0;
const int MAX_MAX_REPORTED_IAS = 500;
const int MIN_INTERVAL_REPORTED_IAS = 1;
const int MAX_INTERVAL_REPORTED_IAS = 500;

const COLORREF TAG_COLOR_GREEN = RGB(0, 200, 0);