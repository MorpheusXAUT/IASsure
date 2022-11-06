#include "IASsure.h"

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
	weatherUpdater(nullptr)
{
	std::ostringstream msg;
	msg << "Version " << PLUGIN_VERSION << " loaded.";

	this->LogMessage(msg.str());

	this->RegisterTagItems();

	this->LoadSettings();
}

IASsure::IASsure::~IASsure()
{
	if (this->weatherUpdater != nullptr) {
		this->weatherUpdater->stop();
		delete this->weatherUpdater;
		this->weatherUpdater = nullptr;
	}
}

bool IASsure::IASsure::OnCompileCommand(const char* sCommandLine)
{
	std::vector<std::string> args = ::IASsure::split(sCommandLine);

	if (args[0] == ".ias") {
		if (args.size() == 1) {
			std::ostringstream msg;
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: debug, reset, weather";

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
					msg << "Weather data is automatically updated every " << this->weatherUpdateInterval.count() << " minutes.";
				}
				msg << " Use .ias weather <MIN> to change the update interval (set 0 to disable automatic refreshing)";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			int min;
			try {
				min = std::stoi(args[2]);
			}
			catch (std::exception) {
				this->LogMessage("Invalid automatic weather data update interval", "Config");
				return true;
			}

			this->weatherUpdateInterval = std::chrono::minutes(min);

			if (this->weatherUpdater != nullptr) {
				this->weatherUpdater->stop();
				delete this->weatherUpdater;
				this->weatherUpdater = nullptr;
			}
			if (this->weatherUpdateInterval.count() > 0) {
				this->weatherUpdater = new ::IASsure::thread::PeriodicAction(std::chrono::milliseconds(0), std::chrono::milliseconds(this->weatherUpdateInterval), std::bind(&IASsure::UpdateWeather, this));
			}

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

		int ias = rt.GetPosition().GetReportedGS();
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
	int gs = rt.GetPosition().GetReportedGS(); // ground speed in knots
	int alt = rt.GetPosition().GetPressureAltitude(); // altitude in feet

	WeatherReferenceLevel level = WeatherReferenceLevel();
	if (this->weatherMutex.try_lock_shared()) {
		level = this->weather.findClosest(rt.GetPosition().GetPosition().m_Latitude, rt.GetPosition().GetPosition().m_Longitude, alt);
		this->weatherMutex.unlock_shared();
	}
	else {
		this->LogDebugMessage("Failed to acquire weather data lock, fallback to no winds", rt.GetCallsign());
	}

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
	int gs = rt.GetPosition().GetReportedGS(); // ground speed in knots
	int alt = rt.GetPosition().GetPressureAltitude(); // altitude in feet

	WeatherReferenceLevel level = WeatherReferenceLevel();
	if (this->weatherMutex.try_lock_shared()) {
		level = this->weather.findClosest(rt.GetPosition().GetPosition().m_Latitude, rt.GetPosition().GetPosition().m_Longitude, alt);
		this->weatherMutex.unlock_shared();
	}
	else {
		this->LogDebugMessage("Failed to acquire weather data lock, fallback to no winds", rt.GetCallsign());
	}

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
		if (this->weatherUpdater != nullptr) {
			this->weatherUpdater->stop();
			delete this->weatherUpdater;
			this->weatherUpdater = nullptr;
		}
		break;
	default:
		// user just connected, start weather update if it's not running yet
		if (this->weatherUpdater == nullptr && this->weatherUpdateInterval.count() > 0) {
			this->weatherUpdater = new ::IASsure::thread::PeriodicAction(std::chrono::milliseconds(0), std::chrono::milliseconds(this->weatherUpdateInterval), std::bind(&IASsure::UpdateWeather, this));
		}
	}
}

void IASsure::IASsure::UpdateWeather()
{
	this->LogDebugMessage("Retrieving weather data");

	std::string weatherJSON;
	try {
		weatherJSON = ::IASsure::HTTP::get(WEATHER_UPDATE_URL);
	}
	catch (std::exception) {
		this->LogMessage("Failed to load weather data");
		return;
	}

	this->LogDebugMessage("Locking weather data");
	std::unique_lock<std::shared_mutex> lock(this->weatherMutex);

	this->LogDebugMessage("Parsing weather data");
	try {
		this->weather.parse(weatherJSON);
	}
	catch (std::exception) {
		this->LogMessage("Failed to parse weather data");
		return;
	}

	this->LogDebugMessage("Successfully updated weather data");
}

void IASsure::IASsure::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = ::IASsure::split(settings, SETTINGS_DELIMITER);

		if (splitSettings.size() < 2) {
			this->LogMessage("Invalid saved settings found, reverting to default.");

			this->SaveSettings();
			return;
		}

		std::istringstream(splitSettings[0]) >> this->debug;
		int weatherUpdateMin;
		std::istringstream(splitSettings[1]) >> weatherUpdateMin;
		this->weatherUpdateInterval = std::chrono::minutes(weatherUpdateMin);

		this->LogDebugMessage("Successfully loaded settings.");
	}
	else {
		this->LogMessage("No saved settings found, using defaults.");
	}
}

void IASsure::IASsure::SaveSettings()
{
	std::ostringstream ss;
	ss << this->debug << SETTINGS_DELIMITER
		<< this->weatherUpdateInterval.count();

	this->SaveDataToSettings(PLUGIN_NAME, "Settings", ss.str().c_str());
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