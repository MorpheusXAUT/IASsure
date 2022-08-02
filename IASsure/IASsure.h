#pragma once

#include <cmath>
#include <iomanip>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>

#include "EuroScope/EuroScopePlugIn.h"

#include "cas.h"
#include "constants.h"
#include "helpers.h"

namespace IASsure {
	class IASsure : public EuroScopePlugIn::CPlugIn {
	public:
		IASsure();
		virtual ~IASsure();

		bool OnCompileCommand(const char* sCommandLine);
		void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
		void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);

	private:
		bool debug;
		int minReportedIAS;
		int maxReportedIAS;
		int intervalReportedIAS;
		std::unordered_map<std::string, int> reportedIAS;
		std::unordered_set<std::string> calculatedIASToggled;

		void RegisterTagItems();

		void SetReportedIAS(const EuroScopePlugIn::CFlightPlan& fp, std::string selected);
		void ClearReportedIAS(const EuroScopePlugIn::CFlightPlan& fp);
		void ToggleCalculatedIAS(const EuroScopePlugIn::CFlightPlan& fp);
		void CalculateIAS(const EuroScopePlugIn::CRadarTarget& rt, char tagItemContent[16], int* tagItemColorCode, COLORREF* tagItemRGB);

		void LoadSettings();
		void SaveSettings();

		void LogMessage(std::string message);
		void LogMessage(std::string message, std::string type);
		void LogDebugMessage(std::string message);
		void LogDebugMessage(std::string message, std::string type);
	};
}