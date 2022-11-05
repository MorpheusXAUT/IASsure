#include <CppUnitTest.h>

#include <fstream>

#include "../IASsure/weather.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IASsureTest
{
	TEST_CLASS(Weather)
	{
	public:
		TEST_METHOD(TestConstructor)
		{
			std::ifstream ifs = std::ifstream("weather_test.json", std::ios_base::in);
			IASsure::Weather weather = IASsure::Weather(ifs);
			ifs.close();

			Assert::IsTrue(true); // no exception thrown
		}

		TEST_METHOD(TestParse)
		{
			std::ifstream ifs = std::ifstream("weather_test.json", std::ios_base::in);
			IASsure::Weather weather = IASsure::Weather(ifs);
			ifs.close();

			ifs = std::ifstream("weather_test.json", std::ios_base::in);
			weather.parse(ifs);
			ifs.close();

			Assert::IsTrue(true); // no exception thrown
		}

		void AssertFindClosest(const IASsure::Weather& weather, double latitude, double longitude, int altitude, double temperature, double windSpeed, double windDirection)
		{
			IASsure::WeatherReferenceLevel level = weather.findClosest(latitude, longitude, altitude);
			Assert::AreEqual(temperature, level.temperature);
			Assert::AreEqual(windSpeed, level.windSpeed);
			Assert::AreEqual(windDirection, level.windDirection);
		}

		TEST_METHOD(TestFindClosest)
		{
			std::ifstream ifs = std::ifstream("weather_test.json", std::ios_base::in);
			IASsure::Weather weather = IASsure::Weather(ifs);
			ifs.close();

			// point LBL, level 0
			AssertFindClosest(weather, 0, 0, 0, 284.84584490493478, 5.3231006224006521, 6.7634493319426099);
			// point LBL, level 240
			AssertFindClosest(weather, 0, 0, 24000, 240.01082735679188, 59.737288700985573, 211.44368196710610);
			// point LBL, level 240
			AssertFindClosest(weather, 0, 0, 23956, 240.01082735679188, 59.737288700985573, 211.44368196710610);
			// point LBL, level 240
			AssertFindClosest(weather, 0, 0, 23678, 240.01082735679188, 59.737288700985573, 211.44368196710610);
			// point LBL, level 180
			AssertFindClosest(weather, 0, 0, 20000, 250.72392590105898, 21.069514179196641, 206.15627253204514);
			// point LBL, level 390
			AssertFindClosest(weather, 0, 0, 39000, 219.91816840113529, 43.877384316652417, 217.91237049653517);
			// point LBL, level 390
			AssertFindClosest(weather, 0, 0, 42000, 219.91816840113529, 43.877384316652417, 217.91237049653517);
			// point MASUR, level 240
			AssertFindClosest(weather, 48.308947, 15.979947, 24000, 242.17909262367621, 62.880537175094354, 212.64139359442129);
			// point MASUR, level 140
			AssertFindClosest(weather, 48.481497, 16.099261, 12000, 263.11100131229591, 35.759183151158275, 223.92463031561294);
			// point FMD, level 140
			AssertFindClosest(weather, 48.035322, 16.798556, 12000, 263.33092135380110, 32.152820532683080, 222.18511403313391);
		}
	};
}
