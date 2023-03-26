#pragma once

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>

#include "EuroScope/EuroScopePlugIn.h"

#include "calculations.h"
#include "constants.h"
#include "helpers.h"
#include "http.h"
#include "thread.h"
#include "weather.h"

namespace IASsure {
	class IASsure : public EuroScopePlugIn::CPlugIn {
	public:
		IASsure();
		virtual ~IASsure();

		bool OnCompileCommand(const char* sCommandLine);
		void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
		void OnTimer(int Counter);

	private:
		bool debug;
		std::chrono::minutes weatherUpdateInterval;
		std::string weatherUpdateURL;
		bool useReportedGS;
		bool useTrueNorthHeading;
		std::string prefixIAS;
		std::string prefixMach;
		int machDigits;
		int machThresholdFL;

		std::unordered_map<std::string, int> reportedIAS;
		std::unordered_set<std::string> calculatedIASToggled;
		std::unordered_set<std::string> calculatedIASAbbreviatedToggled;
		std::unordered_map<std::string, double> reportedMach;
		std::unordered_set<std::string> calculatedMachToggled;
		std::unordered_set<std::string> calculatedMachAboveThresholdToggled;

		::IASsure::Weather weather;
		::IASsure::thread::PeriodicAction *weatherUpdater;
		int loginState;

		void RegisterTagItems();

		void SetReportedIAS(const EuroScopePlugIn::CFlightPlan& fp, std::string selected);
		void ClearReportedIAS(const EuroScopePlugIn::CFlightPlan& fp);
		void ToggleCalculatedIAS(const EuroScopePlugIn::CFlightPlan& fp, bool abbreviated = false);
		double CalculateIAS(const EuroScopePlugIn::CRadarTarget& rt);
		void ShowCalculatedIAS(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB, bool abbreviated = false, bool onlyToggled = false);

		void SetReportedMach(const EuroScopePlugIn::CFlightPlan& fp, std::string selected);
		void ClearReportedMach(const EuroScopePlugIn::CFlightPlan& fp);
		void ToggleCalculatedMach(const EuroScopePlugIn::CFlightPlan& fp, bool aboveThreshold = false);
		double CalculateMach(const EuroScopePlugIn::CRadarTarget& rt);
		void ShowCalculatedMach(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB, bool aboveThreshold = false, bool onlyToggled = false);

		void UpdateLoginState();
		void CheckLoginState();
		void UpdateWeather();
		void StartWeatherUpdater();
		void StopWeatherUpdater();
		void ResetWeatherUpdater();

		void LoadSettings();
		void SaveSettings();
		void TryLoadConfigFile();

		void LogMessage(std::string message);
		void LogMessage(std::string message, std::string type);
		void LogDebugMessage(std::string message);
		void LogDebugMessage(std::string message, std::string type);
	};
}