#include "IASsure.h"

IASsure::IASsure() : EuroScopePlugIn::CPlugIn(
	EuroScopePlugIn::COMPATIBILITY_CODE,
	PLUGIN_NAME,
	PLUGIN_VERSION,
	PLUGIN_AUTHOR,
	PLUGIN_LICENSE
)
{
	std::ostringstream msg;
	msg << "Version " << PLUGIN_VERSION << " loaded.";

	this->LogMessage(msg.str());

	this->RegisterTagItems();

	this->debug = false;
	this->minReportedIAS = 150;
	this->maxReportedIAS = 400;
	this->intervalReportedIAS = 5;

	this->LoadSettings();
}

IASsure::~IASsure()
{
}

bool IASsure::OnCompileCommand(const char* sCommandLine)
{
	std::vector<std::string> args = split(sCommandLine);

	if (args[0] == ".ias") {
		if (args.size() == 1) {
			std::ostringstream msg;
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: min, max, interval, debug, reset";

			this->LogMessage(msg.str());
			return true;
		}

		if (args[1] == "min") {
			if (args.size() < 2) {
				this->LogMessage("Provide value to set minimum speed of reported IAS popup", "Config");
				return true;
			}

			int min;
			try {
				min = std::stoi(args[2]);
			}
			catch (std::invalid_argument const& ex) {
				this->LogMessage("Invalid value for minimum reported IAS speed, ensure you enter a valid number", "Config");
				return true;
			}
			catch (std::out_of_range const& ex) {
				std::ostringstream msg;
				msg << "Value for minimum reported IAS speed is outside of valid range (" << MIN_MIN_REPORTED_IAS << " - " << MAX_MIN_REPORTED_IAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			if (min < MIN_MIN_REPORTED_IAS || min > MAX_MIN_REPORTED_IAS) {
				std::ostringstream msg;
				msg << "Value for minimum reported IAS speed is outside of valid range (" << MIN_MIN_REPORTED_IAS << " - " << MAX_MIN_REPORTED_IAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}
			else if (min > this->maxReportedIAS) {
				std::ostringstream msg;
				msg << "Value for minimum reported IAS speed cannot be greater than configured maximum reported IAS (" << this->maxReportedIAS<< ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			this->minReportedIAS = min;
			
			this->SaveSettings();
			return true;
		}
		else if (args[1] == "max") {
			if (args.size() < 2) {
				this->LogMessage("Provide value to set maximum speed of reported IAS popup", "Config");
				return true;
			}

			int max;
			try {
				max = std::stoi(args[2]);
			}
			catch (std::invalid_argument const& ex) {
				this->LogMessage("Invalid value for maximum reported IAS speed, ensure you enter a valid number", "Config");
				return true;
			}
			catch (std::out_of_range const& ex) {
				std::ostringstream msg;
				msg << "Value for maximum reported IAS speed is outside of valid range (" << MIN_MAX_REPORTED_IAS << " - " << MAX_MAX_REPORTED_IAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			if (max < MIN_MAX_REPORTED_IAS || max > MAX_MAX_REPORTED_IAS) {
				std::ostringstream msg;
				msg << "Value for maximum reported IAS speed is outside of valid range (" << MIN_MAX_REPORTED_IAS << " - " << MAX_MAX_REPORTED_IAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}
			else if (max < this->minReportedIAS) {
				std::ostringstream msg;
				msg << "Value for maximum reported IAS speed cannot be less than configured minimum reported IAS (" << this->minReportedIAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			this->maxReportedIAS = max;

			this->SaveSettings();
			return true;
		}
		else if (args[1] == "interval") {
			if (args.size() < 2) {
				this->LogMessage("Provide value to set interval for speeds of reported IAS popup", "Config");
				return true;
			}

			int interval;
			try {
				interval = std::stoi(args[2]);
			}
			catch (std::invalid_argument const& ex) {
				this->LogMessage("Invalid value for interval of reported IAS speed, ensure you enter a valid number", "Config");
				return true;
			}
			catch (std::out_of_range const& ex) {
				std::ostringstream msg;
				msg << "Value for interval of reported IAS speed is outside of valid range (" << MIN_INTERVAL_REPORTED_IAS << " - " << MAX_INTERVAL_REPORTED_IAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			if (interval < MIN_INTERVAL_REPORTED_IAS || interval > MAX_INTERVAL_REPORTED_IAS) {
				std::ostringstream msg;
				msg << "Value for interval of reported IAS speed is outside of valid range (" << MIN_INTERVAL_REPORTED_IAS << " - " << MAX_INTERVAL_REPORTED_IAS << ")";

				this->LogMessage(msg.str(), "Config");
				return true;
			}

			this->intervalReportedIAS = interval;

			this->SaveSettings();
			return true;
		}
		else if (args[1] == "debug") {
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
			return true;
		}
	}

	return false;
}

void IASsure::OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid()) {
		return;
	}

	switch (ItemCode) {
	case TAG_ITEM_CORRECTED_IAS:
		this->CalculateCorrectedAirspeed(RadarTarget, sItemString, pColorCode, pRGB);
		break;
	}
}

void IASsure::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area)
{
	EuroScopePlugIn::CFlightPlan fp = this->FlightPlanSelectASEL();
	if (!fp.IsValid()) {
		return;
	}

	switch (FunctionId) {
	case TAG_FUNC_OPEN_REPORTED_IAS_MENU:
		this->OpenPopupList(Area, "Speed", 1);
		for (int i = this->maxReportedIAS; i >= this->minReportedIAS; i -= this->intervalReportedIAS) {
			this->AddPopupListElement(std::to_string(i).c_str(), NULL, TAG_FUNC_SET_REPORTED_IAS, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		break;
	case TAG_FUNC_SET_REPORTED_IAS:
		this->SetReportedIAS(fp, sItemString);
		break;
	case TAG_FUNC_CLEAR_REPORTED_IAS:
		this->ClearReportedIAS(fp);
		break;
	}
}

void IASsure::RegisterTagItems()
{
	this->RegisterTagItemType("Corrected IAS", TAG_ITEM_CORRECTED_IAS);

	this->RegisterTagItemFunction("Open reported IAS menu", TAG_FUNC_OPEN_REPORTED_IAS_MENU);
	this->RegisterTagItemFunction("Set reported IAS", TAG_FUNC_SET_REPORTED_IAS);
	this->RegisterTagItemFunction("Clear reported IAS", TAG_FUNC_CLEAR_REPORTED_IAS);
}

void IASsure::SetReportedIAS(const EuroScopePlugIn::CFlightPlan& fp, std::string selected)
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

void IASsure::ClearReportedIAS(const EuroScopePlugIn::CFlightPlan& fp)
{
	this->reportedIAS.erase(fp.GetCallsign());
}

void IASsure::CalculateCorrectedAirspeed(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB)
{
	if (!rt.IsValid()) {
		return;
	}

	// adapted from https://www.omnicalculator.com/physics/true-airspeed @ 2022-07-31T15:00:33Z
	int gs = rt.GetPosition().GetReportedGS(); // ground speed in knots
	double oat = 0.02; // Outside Air Temperature correction term
	int alt = rt.GetPosition().GetPressureAltitude(); // altitude in feet

	double w = 0; // wind speed in knots
	double theta = 0; // wind correction angle

	double tas = gs - w * cos(theta); // true air speed in knots

	int altCorr = (alt * oat + 1000);
	if (altCorr == 0) {
		// TODO handle
		return;
	}

	int calcIAS = std::round((1000 * tas) / (alt * oat + 1000));

	std::ostringstream tag;
	tag << "I";

	auto it = this->reportedIAS.find(rt.GetCallsign());
	if (it == this->reportedIAS.end()) {
		tag << std::setw(3) << std::setfill('0') << calcIAS;
	}
	else {
		int diff = it->second - calcIAS;
		if (diff > 0) {
			tag << "+";
			tag << std::setw(3);
		}
		else if (diff < 0) {
			tag << std::setw(4); // one extra char due to leading "-"
		}
		else {
			tag << std::setw(3);
		}

		tag << std::setfill('0') << diff;
	}

	strcpy_s(tagItemContent, 16, tag.str().c_str());
}

void IASsure::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = split(settings, SETTINGS_DELIMITER);

		if (splitSettings.size() < 4) {
			this->LogMessage("Invalid saved settings found, reverting to default.");

			this->SaveSettings();
			return;
		}

		std::istringstream(splitSettings[0]) >> this->debug;
		std::istringstream(splitSettings[1]) >> this->minReportedIAS;
		std::istringstream(splitSettings[2]) >> this->maxReportedIAS;
		std::istringstream(splitSettings[3]) >> this->intervalReportedIAS;

		this->LogDebugMessage("Successfully loaded settings.");
	}
	else {
		this->LogMessage("No saved settings found, using defaults.");
	}
}

void IASsure::SaveSettings()
{
	std::ostringstream ss;
	ss << this->debug << SETTINGS_DELIMITER 
		<< this->minReportedIAS << SETTINGS_DELIMITER
		<< this->maxReportedIAS << SETTINGS_DELIMITER
		<< this->intervalReportedIAS;

	this->SaveDataToSettings(PLUGIN_NAME, "Settings", ss.str().c_str());
}

void IASsure::LogMessage(std::string message)
{
	this->DisplayUserMessage("Message", PLUGIN_NAME, message.c_str(), true, true, true, false, false);
}

void IASsure::LogMessage(std::string message, std::string type)
{
	this->DisplayUserMessage(PLUGIN_NAME, type.c_str(), message.c_str(), true, true, true, false, false);
}

void IASsure::LogDebugMessage(std::string message)
{
	if (this->debug) {
		this->LogMessage(message);
	}
}

void IASsure::LogDebugMessage(std::string message, std::string type)
{
	if (this->debug) {
		this->LogMessage(message, type);
	}
}

IASsure* pPlugin;

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
	*ppPlugInInstance = pPlugin = new IASsure();
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete pPlugin;
}