﻿#include "IASsure.h"

IASsure::IASsure::IASsure() :
	EuroScopePlugIn::CPlugIn(
		EuroScopePlugIn::COMPATIBILITY_CODE,
		PLUGIN_NAME,
		PLUGIN_VERSION,
		PLUGIN_AUTHOR,
		PLUGIN_LICENSE
	),
	debug(false),
	weatherUpdateInterval(5),
	loginState(0),
	weatherUpdater(nullptr),
	useReportedGS(true)
{
	std::ostringstream msg;
	msg << "Version " << PLUGIN_VERSION << " loaded.";

	this->LogMessage(msg.str());

	this->RegisterTagItems();

	this->TryLoadConfigFile();
	this->LoadSettings();
}

IASsure::IASsure::~IASsure()
{
	this->StopWeatherUpdater();
}

bool IASsure::IASsure::OnCompileCommand(const char* sCommandLine)
{
	std::vector<std::string> args = ::IASsure::split(sCommandLine);

	if (args[0] == ".ias") {
		if (args.size() == 1) {
			std::ostringstream msg;
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: debug, reset, weather, gs";

			this->LogMessage(msg.str());
			return true;
		}

		if (args[1] == "debug") {
			if (this->debug) {
				this->LogMessage("Disabling debug mode", "Config");
			}
			else {
				this->LogMessage("Enabling debug mode", "Config");
			}

			this->debug = !this->debug;

			this->SaveSettings();
			return true;
		}
		else if (args[1] == "reset") {
			this->LogMessage("Resetting plugin state", "Config");

			this->reportedIAS.clear();
			this->reportedMach.clear();
			this->calculatedIASToggled.clear();
			this->calculatedIASAbbreviatedToggled.clear();
			this->calculatedMachToggled.clear();
			return true;
		}
		else if (args[1] == "weather") {
			if (args.size() == 2) {
				std::ostringstream msg;
				if (this->weatherUpdateInterval.count() == 0) {
					msg << "Automatic weather data update disabled.";
				}
				else {
					msg << "Weather data is automatically updated every " << this->weatherUpdateInterval.count() << (this->weatherUpdateInterval.count() > 1 ? " minutes." : " minute.");
				}
				msg << " Use .ias weather update <MIN> to change the update interval (set 0 to disable automatic refreshing).";
				msg << " Use .ias weather url <URL> to set the URL to retrieve weather data from.";
				msg << " Use .ias weather clear to clear all currently stored weather data, falling back to windless speed calculations.";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			if (args[2] == "update") {
				if (args.size() == 3) {
					this->LogMessage("Automatic weather data update interval is missing. Usage: .ias weather update <MIN>");
					return true;
				}
				int min;
				try {
					min = std::stoi(args[3]);
				}
				catch (std::exception) {
					this->LogMessage("Invalid automatic weather data update interval. Usage: .ias weather update <MIN>", "Config");
					return true;
				}

				this->weatherUpdateInterval = std::chrono::minutes(min);
				this->ResetWeatherUpdater();

				std::ostringstream msg;
				if (this->weatherUpdateInterval.count() > 0) {
					msg << "Automatic weather data update interval set to " << this->weatherUpdateInterval.count() << (this->weatherUpdateInterval.count() > 1 ? " minutes" : " minute");
				}
				else {
					msg << "Automatic weather data update disabled";
				}

				this->LogMessage(msg.str(), "Config");

				this->SaveSettings();
				return true;
			}
			else if (args[2] == "url") {
				if (args.size() == 3) {
					this->LogMessage("Weather update URL is missing. Usage: .ias weather url <URL>");
					return true;
				}

				this->weatherUpdateURL = args[3];
				this->ResetWeatherUpdater();

				std::ostringstream msg;
				msg << "Weather update URL set to " << this->weatherUpdateURL;

				this->LogMessage(msg.str(), "Config");

				this->SaveSettings();
				return true;
			}
			else if (args[2] == "clear") {
				this->weather.clear();
				this->LogMessage("Cleared weather data", "Config");
				return true;
			}
		}
		else if (args[1] == "gs") {
			if (this->useReportedGS) {
				this->LogMessage("Switched to using ground speed estimated by EuroScope for CAS/Mach calculations", "Config");
			}
			else {
				this->LogMessage("Switched to using ground speed reported by pilot client for CAS/Mach calculations", "Config");
			}

			this->useReportedGS = !this->useReportedGS;

			this->SaveSettings();
			return true;
		}
	}

	return false;
}

void IASsure::IASsure::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid()) {
		return;
	}

	switch (ItemCode) {
	case TAG_ITEM_CALCULATED_IAS:
		this->SetCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB);
		break;
	case TAG_ITEM_CALCULATED_IAS_ABBREVIATED:
		this->SetCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB, true);
		break;
	case TAG_ITEM_CALCULATED_IAS_TOGGLABLE:
		if (this->calculatedIASToggled.contains(FlightPlan.GetCallsign())) {
			this->SetCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB);
		}
		break;
	case TAG_ITEM_CALCULATED_IAS_ABBREVIATED_TOGGLABLE:
		if (this->calculatedIASAbbreviatedToggled.contains(FlightPlan.GetCallsign())) {
			this->SetCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB, true);
		}
		break;
	case TAG_ITEM_CALCULATED_MACH:
		this->SetCalculatedMach(RadarTarget, sItemString, pColorCode, pRGB);
		break;
	case TAG_ITEM_CALCULATED_MACH_TOGGLABLE:
		if (this->calculatedMachToggled.contains(FlightPlan.GetCallsign())) {
			this->SetCalculatedMach(RadarTarget, sItemString, pColorCode, pRGB);
		}
		break;
	}
}

void IASsure::IASsure::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	EuroScopePlugIn::CFlightPlan fp = this->FlightPlanSelectASEL();
	if (!fp.IsValid()) {
		return;
	}

	switch (FunctionId) {
	case TAG_FUNC_OPEN_REPORTED_IAS_MENU: {
		EuroScopePlugIn::CRadarTarget rt = fp.GetCorrelatedRadarTarget();
		if (!rt.IsValid()) {
			return;
		}

		int ias = this->useReportedGS ? rt.GetPosition().GetReportedGS() : rt.GetGS();
		double calculatedIAS = this->CalculateIAS(rt);
		if (calculatedIAS >= 0) {
			ias = ::IASsure::roundToNearest(calculatedIAS, INTERVAL_REPORTED_IAS);
		}

		this->OpenPopupList(Area, "Speed", 1);
		for (int i = MAX_REPORTED_IAS; i >= MIN_REPORTED_IAS; i -= INTERVAL_REPORTED_IAS) {
			this->AddPopupListElement(std::to_string(i).c_str(), NULL, TAG_FUNC_SET_REPORTED_IAS, i >= ias && ias >= i - INTERVAL_REPORTED_IAS, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		this->AddPopupListElement("---", NULL, 0, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true, true);
		this->AddPopupListElement("Clear", NULL, TAG_FUNC_CLEAR_REPORTED_IAS, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break;
	}
	case TAG_FUNC_CLEAR_REPORTED_IAS:
		this->ClearReportedIAS(fp);
		break;
	case TAG_FUNC_TOGGLE_CALCULATED_IAS:
		this->ToggleCalculatedIAS(fp);
		break;
	case TAG_FUNC_TOGGLE_CALCULATED_IAS_ABBREVIATED:
		this->ToggleCalculatedIAS(fp, true);
		break;
	case TAG_FUNC_SET_REPORTED_IAS:
		this->SetReportedIAS(fp, sItemString);
		break;
	case TAG_FUNC_OPEN_REPORTED_MACH_MENU: {
		EuroScopePlugIn::CRadarTarget rt = fp.GetCorrelatedRadarTarget();
		if (!rt.IsValid()) {
			return;
		}

		int mach = 0;
		double calculatedMach = this->CalculateMach(rt);
		if (calculatedMach >= 0) {
			mach = ::IASsure::roundToNearest(calculatedMach * 100, INTERVAL_REPORTED_MACH);
		}

		this->OpenPopupList(Area, "Mach", 1);
		for (int i = MAX_REPORTED_MACH; i >= MIN_REPORTED_MACH; i -= INTERVAL_REPORTED_MACH) {
			this->AddPopupListElement(std::to_string(i).c_str(), NULL, TAG_FUNC_SET_REPORTED_MACH, i >= mach && mach >= i - INTERVAL_REPORTED_MACH, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		this->AddPopupListElement("---", NULL, 0, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true, true);
		this->AddPopupListElement("Clear", NULL, TAG_FUNC_CLEAR_REPORTED_MACH, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break;
	}
	case TAG_FUNC_CLEAR_REPORTED_MACH:
		this->ClearReportedMach(fp);
		break;
	case TAG_FUNC_TOGGLE_CALCULATED_MACH:
		this->ToggleCalculatedMach(fp);
		break;
	case TAG_FUNC_SET_REPORTED_MACH:
		this->SetReportedMach(fp, sItemString);
		break;
	}
}

void IASsure::IASsure::OnTimer(int Counter)
{
	if (Counter % 2) {
		this->CheckLoginState();
	}
}

void IASsure::IASsure::RegisterTagItems()
{
	this->RegisterTagItemType("Calculated IAS", TAG_ITEM_CALCULATED_IAS);
	this->RegisterTagItemType("Calculated IAS (togglable)", TAG_ITEM_CALCULATED_IAS_TOGGLABLE);
	this->RegisterTagItemType("Calculated IAS (abbreviated)", TAG_ITEM_CALCULATED_IAS_ABBREVIATED);
	this->RegisterTagItemType("Calculated IAS (abbreviated, togglable)", TAG_ITEM_CALCULATED_IAS_ABBREVIATED_TOGGLABLE);
	this->RegisterTagItemType("Calculated Mach", TAG_ITEM_CALCULATED_MACH);
	this->RegisterTagItemType("Calculated Mach (togglable)", TAG_ITEM_CALCULATED_MACH_TOGGLABLE);

	this->RegisterTagItemFunction("Open reported IAS menu", TAG_FUNC_OPEN_REPORTED_IAS_MENU);
	this->RegisterTagItemFunction("Clear reported IAS", TAG_FUNC_CLEAR_REPORTED_IAS);
	this->RegisterTagItemFunction("Toggle calculated IAS", TAG_FUNC_TOGGLE_CALCULATED_IAS);
	this->RegisterTagItemFunction("Toggle calculated IAS (abbreviated)", TAG_FUNC_TOGGLE_CALCULATED_IAS_ABBREVIATED);
	this->RegisterTagItemFunction("Open reported Mach menu", TAG_FUNC_OPEN_REPORTED_MACH_MENU);
	this->RegisterTagItemFunction("Clear reported Mach", TAG_FUNC_CLEAR_REPORTED_MACH);
	this->RegisterTagItemFunction("Toggle calculated Mach", TAG_FUNC_TOGGLE_CALCULATED_MACH);
}

void IASsure::IASsure::SetReportedIAS(const EuroScopePlugIn::CFlightPlan& fp, std::string selected)
{
	int ias;
	try {
		ias = std::stoi(selected);
	}
	catch (std::exception const& ex) {
		std::ostringstream msg;
		msg << "Failed to parse reported IAS: " << ex.what();

		this->LogMessage(msg.str());
		return;
	}

	this->reportedIAS.insert_or_assign(fp.GetCallsign(), ias);
}

void IASsure::IASsure::ClearReportedIAS(const EuroScopePlugIn::CFlightPlan& fp)
{
	this->reportedIAS.erase(fp.GetCallsign());
}

void IASsure::IASsure::ToggleCalculatedIAS(const EuroScopePlugIn::CFlightPlan& fp, bool abbreviated)
{
	std::string cs = fp.GetCallsign();
	if (abbreviated) {
		if (this->calculatedIASAbbreviatedToggled.contains(cs)) {
			this->calculatedIASAbbreviatedToggled.erase(cs);
		}
		else {
			this->calculatedIASAbbreviatedToggled.insert(cs);
		}
	}
	else {
		if (this->calculatedIASToggled.contains(cs)) {
			this->calculatedIASToggled.erase(cs);
		}
		else {
			this->calculatedIASToggled.insert(cs);
		}
	}
}

double IASsure::IASsure::CalculateIAS(const EuroScopePlugIn::CRadarTarget& rt)
{
	if (!rt.IsValid()) {
		return -1;
	}

	int hdg = rt.GetPosition().GetReportedHeading(); // heading in degrees
	int gs = this->useReportedGS ? rt.GetPosition().GetReportedGS() : rt.GetGS(); // ground speed in knots
	int alt = rt.GetPosition().GetPressureAltitude(); // altitude in feet

	WeatherReferenceLevel level = this->weather.findClosest(rt.GetPosition().GetPosition().m_Latitude, rt.GetPosition().GetPosition().m_Longitude, alt);

	try {
		return ::IASsure::calculateCAS(alt, hdg, gs, level);
	}
	catch (std::exception const&) {
		// gs or alt outside of supported ranges. no value to display in tag
		return -1;
	}
}

void IASsure::IASsure::SetCalculatedIAS(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB, bool abbreviated)
{
	if (!rt.IsValid()) {
		return;
	}

	double cas = this->CalculateIAS(rt);
	if (cas < 0) {
		// gs or alt outside of supported ranges. no value to display in tag
		return;
	}

	std::ostringstream tag;
	if (!abbreviated) {
		tag << "I";
	}
	tag << std::setfill('0');

	auto it = this->reportedIAS.find(rt.GetCallsign());
	if (it == this->reportedIAS.end()) {
		if (abbreviated) {
			tag << std::setw(2) << std::round(cas / 10.0);
		}
		else {
			tag << std::setw(3) << std::round(cas);
		}
	}
	else {
		double diff = it->second - cas;
		if (diff > 0) {
			tag << "+";
		}
		else if (diff < 0) {
			tag << "-";
		}

		if (abbreviated) {
			tag << std::setw(2) << std::round(std::abs(diff / 10.0));
		}
		else {
			tag << std::setw(3) << std::round(std::abs(diff));
		}
	}

	strcpy_s(tagItemContent, 16, tag.str().c_str());
}

void IASsure::IASsure::SetReportedMach(const EuroScopePlugIn::CFlightPlan& fp, std::string selected)
{
	int mach;
	try {
		mach = std::stoi(selected);
	}
	catch (std::exception const& ex) {
		std::ostringstream msg;
		msg << "Failed to parse reported Mach: " << ex.what();

		this->LogMessage(msg.str());
		return;
	}

	this->reportedMach.insert_or_assign(fp.GetCallsign(), (double)mach / 100.0);
}

void IASsure::IASsure::ClearReportedMach(const EuroScopePlugIn::CFlightPlan& fp)
{
	this->reportedMach.erase(fp.GetCallsign());
}

void IASsure::IASsure::ToggleCalculatedMach(const EuroScopePlugIn::CFlightPlan& fp)
{
	std::string cs = fp.GetCallsign();
	if (this->calculatedMachToggled.contains(cs)) {
		this->calculatedMachToggled.erase(cs);
	}
	else {
		this->calculatedMachToggled.insert(cs);
	}
}

double IASsure::IASsure::CalculateMach(const EuroScopePlugIn::CRadarTarget& rt)
{
	if (!rt.IsValid()) {
		return -1;
	}

	int hdg = rt.GetPosition().GetReportedHeading(); // heading in degrees
	int gs = this->useReportedGS ? rt.GetPosition().GetReportedGS() : rt.GetGS(); // ground speed in knots
	int alt = rt.GetPosition().GetPressureAltitude(); // altitude in feet

	WeatherReferenceLevel level = this->weather.findClosest(rt.GetPosition().GetPosition().m_Latitude, rt.GetPosition().GetPosition().m_Longitude, alt);

	try {
		return ::IASsure::calculateMach(alt, hdg, gs, level);
	}
	catch (std::exception const&) {
		// gs or alt outside of supported ranges. no value to display in tag
		return -1;
	}
}

void IASsure::IASsure::SetCalculatedMach(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB)
{
	if (!rt.IsValid()) {
		return;
	}

	double mach = this->CalculateMach(rt);
	if (mach < 0) {
		// gs or alt outside of supported ranges. no value to display in tag
		return;
	}

	std::ostringstream tag;
	tag << "M";

	auto it = this->reportedMach.find(rt.GetCallsign());
	if (it == this->reportedMach.end()) {
		tag << std::setfill('0') << std::setw(2) << std::round(mach * 100);
	}
	else {
		double diff = it->second - mach;
		if (diff > 0) {
			tag << "+";
		}
		else if (diff < 0) {
			tag << "-";
		}

		tag << std::setfill('0') << std::setw(2) << std::round(std::abs(diff * 100));
	}

	strcpy_s(tagItemContent, 16, tag.str().c_str());
}

void IASsure::IASsure::CheckLoginState()
{
	// login state has not changed, nothing to do
	if (this->loginState == this->GetConnectionType()) {
		return;
	}

	this->loginState = this->GetConnectionType();

	switch (this->loginState) {
	case EuroScopePlugIn::CONNECTION_TYPE_NO:
	case EuroScopePlugIn::CONNECTION_TYPE_PLAYBACK:
		// user just disconnected, stop weather updater if it's running
		this->StopWeatherUpdater();
		break;
	default:
		// user just connected, start weather update if it's not running yet
		this->StartWeatherUpdater();
	}
}

void IASsure::IASsure::UpdateWeather()
{
	this->LogDebugMessage("Retrieving weather data", "Weather");

	std::string weatherJSON;
	try {
		weatherJSON = ::IASsure::HTTP::get(this->weatherUpdateURL);
	}
	catch (std::exception) {
		this->LogMessage("Failed to load weather data", "Weather");
		return;
	}

	this->LogDebugMessage("Parsing weather data", "Weather");
	try {
		this->weather.parse(weatherJSON);
	}
	catch (std::exception) {
		this->LogMessage("Failed to parse weather data", "Weather");
		return;
	}

	this->LogDebugMessage("Successfully updated weather data", "Weather");
}

void IASsure::IASsure::StartWeatherUpdater()
{
	if (this->weatherUpdateURL.empty() && this->weatherUpdateInterval.count() > 0) {
		this->LogMessage("Weather update URL is empty, cannot fetch weather data for calculations. Configure via config file (config.json in same directory as IASsure.dll) or .ias weather url <URL>.", "Config");
		return;
	}

	if (this->weatherUpdater == nullptr && this->weatherUpdateInterval.count() > 0) {
		this->weatherUpdater = new ::IASsure::thread::PeriodicAction(std::chrono::milliseconds(0), std::chrono::milliseconds(this->weatherUpdateInterval), std::bind(&IASsure::UpdateWeather, this));
	}
}

void IASsure::IASsure::StopWeatherUpdater()
{
	if (this->weatherUpdater != nullptr) {
		this->weatherUpdater->stop();
		delete this->weatherUpdater;
		this->weatherUpdater = nullptr;
	}
}

void IASsure::IASsure::ResetWeatherUpdater()
{
	this->StopWeatherUpdater();
	this->StartWeatherUpdater();
}

void IASsure::IASsure::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = ::IASsure::split(settings, SETTINGS_DELIMITER);

		if (splitSettings.size() < 4) {
			this->LogMessage("Invalid saved settings found, reverting to default.");

			this->SaveSettings();
			return;
		}

		std::istringstream(splitSettings[0]) >> this->debug;
		int weatherUpdateMin;
		std::istringstream(splitSettings[1]) >> weatherUpdateMin;
		this->weatherUpdateInterval = std::chrono::minutes(weatherUpdateMin);
		if (!splitSettings[2].empty()) {
			this->weatherUpdateURL = splitSettings[2];
		}
		std::istringstream(splitSettings[3]) >> this->useReportedGS;

		this->LogDebugMessage("Successfully loaded settings.", "Config");
	}
	else {
		this->LogMessage("No saved settings found, using defaults.");
	}
}

void IASsure::IASsure::SaveSettings()
{
	std::ostringstream ss;
	ss << this->debug << SETTINGS_DELIMITER
		<< this->weatherUpdateInterval.count() << SETTINGS_DELIMITER
		<< this->weatherUpdateURL << SETTINGS_DELIMITER
		<< this->useReportedGS;

	this->SaveDataToSettings(PLUGIN_NAME, "Settings", ss.str().c_str());
}

void IASsure::IASsure::TryLoadConfigFile()
{
	this->LogDebugMessage("Attempting to load config file", "Config");

	nlohmann::json cfg;
	try {
		std::filesystem::path base(::IASsure::getPluginDirectory());
		base.append(CONFIG_FILE_NAME);

		std::ifstream ifs(base);
		if (!ifs.good()) {
			this->LogDebugMessage("Failed to read config file, might not exist. Ignoring", "Config");
			return;
		}

		cfg = nlohmann::json::parse(ifs);
	}
	catch (std::exception) {
		this->LogMessage("Failed to read config file", "Config");
	}

	try {
		auto& weatherCfg = cfg.at("weather");
		this->weatherUpdateURL = weatherCfg.value<std::string>("url", this->weatherUpdateURL);
		this->weatherUpdateInterval = std::chrono::minutes(weatherCfg.value<int>("update", this->weatherUpdateInterval.count()));

		this->ResetWeatherUpdater();
	}
	catch (std::exception) {
		this->LogDebugMessage("Failed to parse weather section of config file, might not exist. Ignoring", "Config");
	}

	this->LogDebugMessage("Successfully loaded config file", "Config");
}

void IASsure::IASsure::LogMessage(std::string message)
{
	this->DisplayUserMessage("Message", PLUGIN_NAME, message.c_str(), true, true, true, false, false);
}

void IASsure::IASsure::LogMessage(std::string message, std::string type)
{
	this->DisplayUserMessage(PLUGIN_NAME, type.c_str(), message.c_str(), true, true, true, false, false);
}

void IASsure::IASsure::LogDebugMessage(std::string message)
{
	if (this->debug) {
		this->LogMessage(message);
	}
}

void IASsure::IASsure::LogDebugMessage(std::string message, std::string type)
{
	if (this->debug) {
		this->LogMessage(message, type);
	}
}

IASsure::IASsure* pPlugin;

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pPlugin = new IASsure::IASsure();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete pPlugin;
}