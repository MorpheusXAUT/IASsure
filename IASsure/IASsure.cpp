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
	weatherUpdater(nullptr),
	useReportedGS(true),
	useTrueNorthHeading(true),
	prefixIAS("I"),
	prefixMach("M"),
	machDigits(2),
	machThresholdFL(24500),
	unreliableIASIndicator("DIAS"),
	unreliableIASColor(nullptr),
	unreliableMachIndicator("DMACH"),
	unreliableMachColor(nullptr),
	broadcastUnreliableSpeed(true)
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
			msg << "Version " << PLUGIN_VERSION << " loaded. Available commands: debug, reset, reload, weather, gs, hdg, prefix, mach";

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
			this->calculatedMachAboveThresholdToggled.clear();
			this->unreliableSpeedToggled.clear();

			this->weather.clear();
			this->ResetWeatherUpdater();

			return true;
		}
		else if (args[1] == "reload") {
			this->LogMessage("Reloading plugin config", "Config");

			this->TryLoadConfigFile();

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
		else if (args[1] == "hdg") {
			if (this->useTrueNorthHeading) {
				this->LogMessage("Switched to using magnetic heading for CAS/Mach calculations", "Config");
			}
			else {
				this->LogMessage("Switched to using true north heading for CAS/Mach calculations", "Config");
			}

			this->useTrueNorthHeading = !this->useTrueNorthHeading;

			this->SaveSettings();
			return true;
		}
		else if (args[1] == "prefix") {
			if (args.size() == 2) {
				this->LogMessage("Use .ias prefix ias <PREFIX> to set the indicated air speed prefix. Use .ias prefix mach <PREFIX> to set the mach number prefix.", "Config");
				return true;
			}

			if (args[2] == "ias") {
				if (args.size() == 3) {
					this->LogMessage("Disabling indicated air speed prefix", "Config");
					this->prefixIAS = "";
				}
				else {
					if (args[3].size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
						std::ostringstream msg;
						msg << "Indicated air speed prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less";
						this->LogMessage(msg.str(), "Config");
						return true;
					}

					this->LogMessage("Configured indicated air speed prefix", "Config");
					this->prefixIAS = args[3];
				}

				this->SaveSettings();
				return true;
			}
			else if (args[2] == "mach") {
				if (args.size() == 3) {
					this->LogMessage("Disabling mach number prefix", "Config");
					this->prefixMach = "";
				}
				else {
					if (args[3].size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
						std::ostringstream msg;
						msg << "Mach number prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less";
						this->LogMessage(msg.str(), "Config");
						return true;
					}

					this->LogMessage("Configured mach number prefix", "Config");
					this->prefixMach = args[3];
				}

				this->SaveSettings();
				return true;
			}
		}
		else if (args[1] == "mach") {
			if (args.size() == 2) {
				this->LogMessage("Use .ias mach digits <DIGITS> to set the desired digits displayed for mach numbers (range 1-13). Use .ias mach threshold <FLIGHTLEVEL> to set a flight level threshold below which no mach number will be calculated.", "Config");
				return true;
			}

			if (args[2] == "digits") {
				if (args.size() == 3) {
					this->LogMessage("Digit count for mach numbers is missing. Usage: .ias mach digits <DIGITS>");
					return true;
				}

				int digits;
				try {
					digits = std::stoi(args[3]);
				}
				catch (std::exception) {
					this->LogMessage("Invalid digit count for mach numbers. Usage: .ias mach digits <DIGITS>", "Config");
					return true;
				}

				if (digits < MIN_MACH_DIGITS || digits > MAX_MACH_DIGITS) {
					std::ostringstream msg;
					msg << "Invalid digit count for mach numbers. Must be between " << MIN_MACH_DIGITS << " and " << MAX_MACH_DIGITS;
					this->LogMessage(msg.str(), "Config");
					return true;
				}

				this->machDigits = digits;

				std::ostringstream msg;
				msg << "Displaying mach numbers with " << this->machDigits << " digits precision";
				this->LogMessage(msg.str(), "Config");

				this->SaveSettings();
				return true;
			}
			else if (args[2] == "threshold") {
				if (args.size() == 3) {
					this->LogMessage("Flight level threshold for mach calculations is missing. Usage: .ias mach threshold <FLIGHTLEVEL>");
					return true;
				}

				int threshold;
				try {
					threshold = std::stoi(args[3]);
				}
				catch (std::exception) {
					this->LogMessage("Invalid flight level threshold for mach calculations. Usage: .ias mach threshold <FLIGHTLEVEL>", "Config");
					return true;
				}

				if (threshold < 0) {
					this->LogMessage("Invalid flight level threshold for mach calculations. Must be greater than 0", "Config");
					return true;
				}

				this->machThresholdFL = threshold * 100;

				std::ostringstream msg;
				msg << "Displaying mach numbers for aircraft flying higher than FL" << this->machThresholdFL << " in threshold tag item";
				this->LogMessage(msg.str(), "Config");

				this->SaveSettings();
				return true;
			}
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
		this->ShowCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB);
		break;
	case TAG_ITEM_CALCULATED_IAS_ABBREVIATED:
		this->ShowCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB, true);
		break;
	case TAG_ITEM_CALCULATED_IAS_TOGGLABLE:
		this->ShowCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB, false, true);
		break;
	case TAG_ITEM_CALCULATED_IAS_ABBREVIATED_TOGGLABLE:
		this->ShowCalculatedIAS(RadarTarget, sItemString, pColorCode, pRGB, true, true);
		break;
	case TAG_ITEM_CALCULATED_MACH:
		this->ShowCalculatedMach(RadarTarget, sItemString, pColorCode, pRGB);
		break;
	case TAG_ITEM_CALCULATED_MACH_ABOVE_THRESHOLD:
		this->ShowCalculatedMach(RadarTarget, sItemString, pColorCode, pRGB, true);
		break;
	case TAG_ITEM_CALCULATED_MACH_TOGGLABLE:
		this->ShowCalculatedMach(RadarTarget, sItemString, pColorCode, pRGB, false, true);
		break;
	case TAG_ITEM_CALCULATED_MACH_ABOVE_THRESHOLD_TOGGLABLE:
		this->ShowCalculatedMach(RadarTarget, sItemString, pColorCode, pRGB, true, true);
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
			this->AddPopupListElement(std::to_string(i).c_str(), nullptr, TAG_FUNC_SET_REPORTED_IAS, i >= ias && ias >= i - INTERVAL_REPORTED_IAS, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		this->AddPopupListElement("---", nullptr, 0, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true, true);
		this->AddPopupListElement("Clear", nullptr, TAG_FUNC_CLEAR_REPORTED_IAS, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, true);

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
			this->AddPopupListElement(std::to_string(i).c_str(), nullptr, TAG_FUNC_SET_REPORTED_MACH, i >= mach && mach >= i - INTERVAL_REPORTED_MACH, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, false);
		}
		this->AddPopupListElement("---", nullptr, 0, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, true, true);
		this->AddPopupListElement("Clear", nullptr, TAG_FUNC_CLEAR_REPORTED_MACH, false, EuroScopePlugIn::POPUP_ELEMENT_NO_CHECKBOX, false, true);

		break;
	}
	case TAG_FUNC_CLEAR_REPORTED_MACH:
		this->ClearReportedMach(fp);
		break;
	case TAG_FUNC_TOGGLE_CALCULATED_MACH:
		this->ToggleCalculatedMach(fp);
		break;
	case TAG_FUNC_TOGGLE_CALCULATED_MACH_ABOVE_THRESHOLD:
		this->ToggleCalculatedMach(fp, true);
		break;
	case TAG_FUNC_SET_REPORTED_MACH:
		this->SetReportedMach(fp, sItemString);
		break;
	case TAG_FUNC_TOGGLE_UNRELIABLE_SPEED:
		this->ToggleUnreliableSpeed(fp);
		break;
	}
}

void IASsure::IASsure::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan, int DataType)
{
	if (!FlightPlan.IsValid()) {
		return;
	}

	switch (DataType) {
	case EuroScopePlugIn::CTR_DATA_TYPE_SCRATCH_PAD_STRING:
		this->CheckScratchPadBroadcast(FlightPlan);
		break;
	}
}

void IASsure::IASsure::OnFlightPlanFlightStripPushed(EuroScopePlugIn::CFlightPlan FlightPlan, const char* sSenderController, const char* sTargetController)
{
	if (!FlightPlan.IsValid()) {
		return;
	}

	auto cad = FlightPlan.GetControllerAssignedData();

	if (strcmp(sTargetController, this->ControllerMyself().GetCallsign()) == 0) {
		// tag is being pushed to us, check if we need to set unreliable speed indication from flightstrip
		this->CheckFlightStripAnnotations(FlightPlan);
	}
}

void IASsure::IASsure::OnTimer(int Counter)
{
	if (Counter % 2) {
		this->UpdateLoginState();
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
	this->RegisterTagItemType("Calculated Mach (above threshold)", TAG_ITEM_CALCULATED_MACH_ABOVE_THRESHOLD);
	this->RegisterTagItemType("Calculated Mach (above threshold, togglable)", TAG_ITEM_CALCULATED_MACH_ABOVE_THRESHOLD_TOGGLABLE);

	this->RegisterTagItemFunction("Open reported IAS menu", TAG_FUNC_OPEN_REPORTED_IAS_MENU);
	this->RegisterTagItemFunction("Clear reported IAS", TAG_FUNC_CLEAR_REPORTED_IAS);
	this->RegisterTagItemFunction("Toggle calculated IAS", TAG_FUNC_TOGGLE_CALCULATED_IAS);
	this->RegisterTagItemFunction("Toggle calculated IAS (abbreviated)", TAG_FUNC_TOGGLE_CALCULATED_IAS_ABBREVIATED);
	this->RegisterTagItemFunction("Open reported Mach menu", TAG_FUNC_OPEN_REPORTED_MACH_MENU);
	this->RegisterTagItemFunction("Clear reported Mach", TAG_FUNC_CLEAR_REPORTED_MACH);
	this->RegisterTagItemFunction("Toggle calculated Mach", TAG_FUNC_TOGGLE_CALCULATED_MACH);
	this->RegisterTagItemFunction("Toggle calculated Mach (above threshold)", TAG_FUNC_TOGGLE_CALCULATED_MACH_ABOVE_THRESHOLD);
	this->RegisterTagItemFunction("Toggle unreliable speed", TAG_FUNC_TOGGLE_UNRELIABLE_SPEED);
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

	int hdg = this->useTrueNorthHeading ? rt.GetPosition().GetReportedHeadingTrueNorth() : rt.GetPosition().GetReportedHeading(); // heading in degrees
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

void IASsure::IASsure::ShowCalculatedIAS(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB, bool abbreviated, bool onlyToggled)
{
	if (!rt.IsValid()) {
		return;
	}

	if (onlyToggled && ((abbreviated && !this->calculatedIASAbbreviatedToggled.contains(rt.GetCallsign())) ||
		(!abbreviated && !this->calculatedIASToggled.contains(rt.GetCallsign())))) {
		return;
	}

	if (this->unreliableSpeedToggled.contains(rt.GetCallsign()) && this->unreliableIASIndicator.size() > 0) {
		// aircraft has been flagged as having unreliable speed, indicator for unreliable IAS was configured, set and skip calculations
		strcpy_s(tagItemContent, 16, this->unreliableIASIndicator.c_str());
		if (this->unreliableIASColor != nullptr) {
			*tagItemColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
			*tagItemRGB = *this->unreliableIASColor;
		}
		return;
	}

	double cas = this->CalculateIAS(rt);
	if (cas < 0) {
		// gs or alt outside of supported ranges. no value to display in tag
		return;
	}

	std::ostringstream tag;
	if (!this->prefixIAS.empty() && !abbreviated) {
		tag << this->prefixIAS;
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
	if (this->unreliableSpeedToggled.contains(rt.GetCallsign()) && this->unreliableIASColor != nullptr) {
		// aircraft has been flagged as having unreliable speed, but no unreliable IAS indicator was configured, but a color was set
		*tagItemColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
		*tagItemRGB = *this->unreliableIASColor;
	}
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

void IASsure::IASsure::ToggleCalculatedMach(const EuroScopePlugIn::CFlightPlan& fp, bool aboveThreshold)
{
	std::string cs = fp.GetCallsign();
	if (aboveThreshold) {
		if (this->calculatedMachAboveThresholdToggled.contains(cs)) {
			this->calculatedMachAboveThresholdToggled.erase(cs);
		}
		else {
			this->calculatedMachAboveThresholdToggled.insert(cs);
		}
	}
	else {
		if (this->calculatedMachToggled.contains(cs)) {
			this->calculatedMachToggled.erase(cs);
		}
		else {
			this->calculatedMachToggled.insert(cs);
		}
	}
}

double IASsure::IASsure::CalculateMach(const EuroScopePlugIn::CRadarTarget& rt)
{
	if (!rt.IsValid()) {
		return -1;
	}

	int hdg = this->useTrueNorthHeading ? rt.GetPosition().GetReportedHeadingTrueNorth() : rt.GetPosition().GetReportedHeading(); // heading in degrees
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

void IASsure::IASsure::ShowCalculatedMach(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB, bool aboveThreshold, bool onlyToggled)
{
	if (!rt.IsValid()) {
		return;
	}

	if (onlyToggled && ((aboveThreshold && !this->calculatedMachAboveThresholdToggled.contains(rt.GetCallsign())) || 
		(!aboveThreshold && !this->calculatedMachToggled.contains(rt.GetCallsign())))) {
		return;
	}

	if (aboveThreshold && rt.GetPosition().GetFlightLevel() < this->machThresholdFL) {
		return;
	}

	if (this->unreliableSpeedToggled.contains(rt.GetCallsign()) && this->unreliableMachIndicator.size() > 0) {
		// aircraft has been flagged as having unreliable speed, indicator for unreliable Mach number was configured, set and skip calculations
		strcpy_s(tagItemContent, 16, this->unreliableMachIndicator.c_str());
		if (this->unreliableMachColor != nullptr) {
			*tagItemColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
			*tagItemRGB = *this->unreliableMachColor;
		}
		return;
	}

	double mach = this->CalculateMach(rt);
	if (mach < 0) {
		// gs or alt outside of supported ranges. no value to display in tag
		return;
	}

	std::ostringstream tag;
	if (!this->prefixMach.empty()) {
		tag << this->prefixMach;
	}

	auto it = this->reportedMach.find(rt.GetCallsign());
	if (it == this->reportedMach.end()) {
		tag << std::setfill('0') << std::setw(this->machDigits) << std::round(mach * std::pow(10, this->machDigits));
	}
	else {
		double diff = it->second - mach;
		if (diff > 0) {
			tag << "+";
		}
		else if (diff < 0) {
			tag << "-";
		}

		tag << std::setfill('0') << std::setw(this->machDigits) << std::round(std::abs(diff * std::pow(10, this->machDigits)));
	}

	strcpy_s(tagItemContent, 16, tag.str().c_str());
	if (this->unreliableSpeedToggled.contains(rt.GetCallsign()) && this->unreliableMachColor != nullptr) {
		// aircraft has been flagged as having unreliable speed, but no unreliable Mach number indicator was configured, but a color was set
		*tagItemColorCode = EuroScopePlugIn::TAG_COLOR_RGB_DEFINED;
		*tagItemRGB = *this->unreliableMachColor;
	}
}

void IASsure::IASsure::ToggleUnreliableSpeed(const EuroScopePlugIn::CFlightPlan& fp)
{
	std::string cs = fp.GetCallsign();
	bool enabled = false;
	if (this->unreliableSpeedToggled.contains(cs)) {
		this->unreliableSpeedToggled.erase(cs);
	}
	else {
		this->unreliableSpeedToggled.insert(cs);
		enabled = true;
	}

	if (this->broadcastUnreliableSpeed) {
		std::ostringstream msg;
		msg << BROADCAST_PREFIX << BROADCAST_DELIMITER
			<< BROADCAST_UNRELIABLE_SPEED;

		this->SetFlightStripAnnotation(fp, enabled ? msg.str() : "");

		msg << BROADCAST_DELIMITER << enabled;
		this->BroadcastScratchPad(fp, msg.str());
	}
}

void IASsure::IASsure::BroadcastScratchPad(const EuroScopePlugIn::CFlightPlan& fp, std::string msg)
{
	if (!fp.IsValid()) {
		return;
	}

	if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerId(), "") != 0) {
		return;
	}

	auto cad = fp.GetControllerAssignedData();
	std::string scratch = cad.GetScratchPadString();

	if (!cad.SetScratchPadString(msg.c_str())) {
		this->LogMessage("Failed to set broadcast message in scratch pad", fp.GetCallsign());
	}

	if (!cad.SetScratchPadString(scratch.c_str())) {
		this->LogMessage("Failed to reset scratch pad after setting broadcast message", fp.GetCallsign());
	}
}

void IASsure::IASsure::CheckScratchPadBroadcast(const EuroScopePlugIn::CFlightPlan& fp)
{
	std::vector<std::string> scratch = split(fp.GetControllerAssignedData().GetScratchPadString(), BROADCAST_DELIMITER);

	if (scratch.size() < 3 || scratch[0] != BROADCAST_PREFIX) {
		return;
	}

	if (this->broadcastUnreliableSpeed && scratch[1] == BROADCAST_UNRELIABLE_SPEED) {
		if (scratch[2] == "1") {
			this->LogDebugMessage("Enabling unreliable speed indication for aircraft after broadcast", fp.GetCallsign());
			this->unreliableSpeedToggled.insert(fp.GetCallsign());
		}
		else {
			this->LogDebugMessage("Disabling unreliable speed indication for aircraft after broadcast", fp.GetCallsign());
			this->unreliableSpeedToggled.erase(fp.GetCallsign());
		}
	}
}

void IASsure::IASsure::SetFlightStripAnnotation(const EuroScopePlugIn::CFlightPlan& fp, std::string msg, int index)
{
	if (!fp.IsValid()) {
		return;
	}

	if (!fp.GetTrackingControllerIsMe() && strcmp(fp.GetTrackingControllerId(), "") != 0) {
		return;
	}

	auto cad = fp.GetControllerAssignedData();

	if (!cad.SetFlightStripAnnotation(index, msg.c_str())) {
		this->LogMessage("Failed to set message in flight strip annotations", fp.GetCallsign());
	}
}

void IASsure::IASsure::CheckFlightStripAnnotations(const EuroScopePlugIn::CFlightPlan& fp)
{
	if (!fp.IsValid()) {
		return;
	}

	std::string annotation = fp.GetControllerAssignedData().GetFlightStripAnnotation(BROADCAST_FLIGHT_STRIP_INDEX);
	std::vector<std::string> msg = split(annotation, BROADCAST_DELIMITER);

	if (msg.size() < 2 || msg[0] != BROADCAST_PREFIX) {
		return;
	}

	if (this->broadcastUnreliableSpeed) {
		if (msg[1] == BROADCAST_UNRELIABLE_SPEED) {
			this->LogDebugMessage("Enabling unreliable speed indication for aircraft due to flight strip annotation", fp.GetCallsign());
			this->unreliableSpeedToggled.insert(fp.GetCallsign());
		}
		else {
			this->LogDebugMessage("Disabling unreliable speed indication for aircraft due to empty flight strip annotation", fp.GetCallsign());
			this->unreliableSpeedToggled.erase(fp.GetCallsign());
		}
	}
}

void IASsure::IASsure::CheckFlightStripAnnotationsForAllAircraft()
{
	if (this->broadcastUnreliableSpeed) {
		this->unreliableSpeedToggled.clear();

		for (EuroScopePlugIn::CFlightPlan fp = this->FlightPlanSelectFirst(); fp.IsValid(); fp = this->FlightPlanSelectNext(fp)) {
			this->CheckFlightStripAnnotations(fp);
		}
	}
}

void IASsure::IASsure::UpdateLoginState()
{
	// login state has not changed, nothing to do
	if (this->loginState == this->GetConnectionType()) {
		return;
	}

	this->loginState = this->GetConnectionType();

	this->CheckLoginState();
}

void IASsure::IASsure::CheckLoginState()
{
	switch (this->loginState) {
	case EuroScopePlugIn::CONNECTION_TYPE_DIRECT:
	case EuroScopePlugIn::CONNECTION_TYPE_VIA_PROXY:
		// user is connected, start weather update if it's not running yet
		this->StartWeatherUpdater();
		this->CheckFlightStripAnnotationsForAllAircraft();
		break;
	default:
		// user is disconnected or is using incompatible connection (e.g. sweatbox), stop weather updater if it's running
		this->StopWeatherUpdater();
	}
}

void IASsure::IASsure::UpdateWeather()
{
	this->LogDebugMessage("Retrieving weather data", "Weather");

	std::string weatherJSON;
	try {
		weatherJSON = ::IASsure::HTTP::get(this->weatherUpdateURL);
	}
	catch (std::exception ex) {
		this->LogMessage("Failed to load weather data", "Weather");
		this->LogDebugMessage(ex.what(), "Weather");
		return;
	}

	this->LogDebugMessage("Parsing weather data", "Weather");
	try {
		this->weather.parse(weatherJSON);
	}
	catch (std::exception ex) {
		this->LogMessage("Failed to parse weather data", "Weather");
		this->LogDebugMessage(ex.what(), "Weather");
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
	this->CheckLoginState();
}

void IASsure::IASsure::LoadSettings()
{
	const char* settings = this->GetDataFromSettings(PLUGIN_NAME);
	if (settings) {
		std::vector<std::string> splitSettings = ::IASsure::split(settings, SETTINGS_DELIMITER);

		size_t settingCount = splitSettings.size();
		if (settingCount < 8) {
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
		int machDigits;
		std::istringstream(splitSettings[4]) >> machDigits;
		if (machDigits < MIN_MACH_DIGITS || machDigits > MAX_MACH_DIGITS) {
			std::ostringstream msg;
			msg << "Invalid digit count for mach numbers. Must be between " << MIN_MACH_DIGITS << " and " << MAX_MACH_DIGITS << ", falling back to default (2)";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->machDigits = machDigits;
		}
		if (splitSettings[5].size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
			std::ostringstream msg;
			msg << "Indicated air speed prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less. Falling back to default (I)";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->prefixIAS = splitSettings[5];
		}
		if (splitSettings[6].size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
			std::ostringstream msg;
			msg << "Mach number prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less. Falling back to default (I)";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->prefixIAS = splitSettings[6];
		}
		int machThresholdFL;
		std::istringstream(splitSettings[7]) >> machThresholdFL;
		if (machThresholdFL < 0) {
			this->LogMessage("Invalid mach threshold flight level. Must be greater than 0, falling back to default (245)", "Config");
		}
		else {
			this->machThresholdFL = machThresholdFL;
		}

		if (settingCount >= 9) {
			std::istringstream(splitSettings[8]) >> this->useTrueNorthHeading;
		}

		if (settingCount >= 10) {
			if (splitSettings[9].size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH)) {
				std::ostringstream msg;
				msg << "Unreliable IAS indicator is too long, must be " << TAG_ITEM_MAX_CONTENT_LENGTH << " characters or less. Falling back to default (DIAS)";
				this->LogMessage(msg.str(), "Config");
			}
			else {
				std::istringstream(splitSettings[9]) >> this->unreliableIASIndicator;
			}
		}

		if (settingCount >= 11) {
			if (splitSettings[10].size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH)) {
				std::ostringstream msg;
				msg << "Unreliable Mach number indicator is too long, must be " << TAG_ITEM_MAX_CONTENT_LENGTH << " characters or less. Falling back to default (DMACH)";
				this->LogMessage(msg.str(), "Config");
			}
			else {
				std::istringstream(splitSettings[10]) >> this->unreliableIASIndicator;
			}
		}

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
		<< this->useReportedGS << SETTINGS_DELIMITER
		<< this->machDigits << SETTINGS_DELIMITER
		<< this->prefixIAS << SETTINGS_DELIMITER
		<< this->prefixMach << SETTINGS_DELIMITER
		<< this->machThresholdFL << SETTINGS_DELIMITER
		<< this->useTrueNorthHeading << SETTINGS_DELIMITER
		<< this->unreliableIASIndicator << SETTINGS_DELIMITER
		<< this->unreliableMachIndicator;

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
		return;
	}

	try {
		auto& machCfg = cfg.at("mach");

		int machDigits = machCfg.value<int>("digits", this->machDigits);
		if (machDigits < MIN_MACH_DIGITS || machDigits > MAX_MACH_DIGITS) {
			this->LogMessage("Invalid digit count for mach numbers. Must be between 1 and 13, falling back to default (2)", "Config");
		}
		else {
			this->machDigits = machDigits;
		}
		int machThresholdFL = machCfg.value<int>("thresholdFL", this->machThresholdFL);
		if (machThresholdFL < 0) {
			this->LogMessage("Invalid mach threshold flight level. Must be greater than 0, falling back to default (245)", "Config");
		}
		else {
			this->machThresholdFL = machThresholdFL * 100;
		}

		std::string prefixMach = machCfg.value<std::string>("prefix", this->prefixMach);
		if (prefixMach.size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
			std::ostringstream msg;
			msg << "Mach number prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less. Falling back to default (" << this->prefixMach << ")";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->prefixMach = prefixMach;
		}

		std::string unreliableMachIndicator = machCfg.value<std::string>("unreliableIndicator", this->unreliableMachIndicator);
		if (unreliableMachIndicator.size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH)) {
			std::ostringstream msg;
			msg << "Unreliable Mach number indicator is too long, must be " << TAG_ITEM_MAX_CONTENT_LENGTH << " characters or less. Falling back to default (" << this->unreliableMachIndicator << ")";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->unreliableMachIndicator = unreliableMachIndicator;
		}

		std::string unreliableMachColor = machCfg.value<std::string>("unreliableColor", "");
		if (unreliableMachColor.size() > 0) {
			this->unreliableMachColor = parseRGBString(unreliableMachColor);
			if (this->unreliableMachColor == nullptr) {
				this->LogMessage("Unreliable Mach number color is invalid, must be in comma-separated integer RGB format (e.g. \"123,123,123\"). Falling back to no color", "Config");
			}
		}
	}
	catch (std::exception) {
		this->LogDebugMessage("Failed to parse mach section of config file, might not exist. Ignoring", "Config");
	}

	try {
		auto& iasCfg = cfg.at("ias");

		std::string prefixIAS = iasCfg.value<std::string>("prefix", this->prefixIAS);
		if (prefixIAS.size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
			std::ostringstream msg;
			msg << "Indicated air speed prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less. Falling back to default (" << this->prefixIAS << ")";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->prefixIAS = prefixIAS;
		}

		std::string unreliableIASIndicator = iasCfg.value<std::string>("unreliableIndicator", this->unreliableIASIndicator);
		if (unreliableIASIndicator.size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH)) {
			std::ostringstream msg;
			msg << "Unreliable IAS indicator is too long, must be " << TAG_ITEM_MAX_CONTENT_LENGTH << " characters or less. Falling back to default (" << this->unreliableIASIndicator << ")";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->unreliableIASIndicator = unreliableIASIndicator;
		}

		std::string unreliableIASColor = iasCfg.value<std::string>("unreliableColor", "");
		if (unreliableIASColor.size() > 0) {
			this->unreliableIASColor = parseRGBString(unreliableIASColor);
			if (this->unreliableIASColor == nullptr) {
				this->LogMessage("Unreliable IAS color is invalid, must be in comma-separated (integer) RGB format (e.g. \"123,123,123\"). Falling back to no color", "Config");
			}
		}
	}
	catch (std::exception) {
		this->LogDebugMessage("Failed to parse ias section of config file, might not exist. Ignoring", "Config");
	}

	try {
		// deprecated prefix configuration in separate section
		// TODO remove in next major release
		auto& prefixCfg = cfg.at("prefix");
		this->LogMessage("prefix config section is deprecated and will be removed in a future update. Use mach.prefix and ias.prefix to specify respective values.", "Config");

		std::string prefixIAS = prefixCfg.value<std::string>("ias", this->prefixIAS);
		if (prefixIAS.size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
			std::ostringstream msg;
			msg << "Indicated air speed prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less. Falling back to default (" << this->prefixIAS << ")";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->prefixIAS = prefixIAS;
		}
		std::string prefixMach = prefixCfg.value<std::string>("mach", this->prefixMach);
		if (prefixMach.size() > (size_t)(TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits)) {
			std::ostringstream msg;
			msg << "Mach number prefix is too long, must be " << (TAG_ITEM_MAX_CONTENT_LENGTH - this->machDigits) << " characters or less. Falling back to default (" << this->prefixMach << ")";
			this->LogMessage(msg.str(), "Config");
		}
		else {
			this->prefixMach = prefixMach;
		}
	}
	catch (std::exception) {
		this->LogDebugMessage("Failed to parse prefix section of config file, might not exist. Ignoring", "Config");
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

	try {
		auto& broadcastCfg = cfg.at("broadcast");

		this->broadcastUnreliableSpeed = broadcastCfg.value<bool>("unreliableSpeed", this->broadcastUnreliableSpeed);
	}
	catch (std::exception) {
		this->LogDebugMessage("Failed to parse broadcast section of config file, might not exist. Ignoring", "Config");
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