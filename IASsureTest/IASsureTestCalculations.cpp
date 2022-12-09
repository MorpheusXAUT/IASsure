#include <CppUnitTest.h>

#include "../IASsure/calculations.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IASsureTest
{
	TEST_CLASS(Calculations)
	{
	public:
		void AssertTAS(double hdg, double gs, IASsure::WeatherReferenceLevel lvl, double expected)
		{
			double tas = IASsure::calculateTAS(hdg, gs, lvl);
			Assert::AreEqual(expected, tas);
		}

		TEST_METHOD(TestCalculateTAS)
		{
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 0 }, 340.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 60 }, 320.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 90 }, 300.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 120 }, 280.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 150 }, 265.35898384862247);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 180 }, 260.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 210 }, 265.35898384862247);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 240 }, 280.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 270 }, 300.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 300 }, 320.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 330 }, 334.64101615137753);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 40, 360 }, 340.00000000000000);
			AssertTAS(0, 300, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 300.00000000000000);

			Assert::ExpectException<std::domain_error>([]() {
				double tas = IASsure::calculateTAS(0, -240, IASsure::WeatherReferenceLevel{ 0, 40, 0 });
				});
		}

		void AssertTemperature(int alt, double expected)
		{
			double temp = IASsure::calculateTemperature(alt);
			Assert::AreEqual(expected, temp);
		}

		TEST_METHOD(TestCalculateTemperature)
		{
			AssertTemperature(-1240, 290.60668792138597);
			AssertTemperature(0, 288.14999999999998);
			AssertTemperature(10000, 268.33800063398394);
			AssertTemperature(38000, 216.65000000000001);

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 69000;
				double temp = IASsure::calculateTemperature(alt);
				});
		}

		void AssertStaticPressure(int alt, double expected)
		{
			double ps = IASsure::calculateStaticPressure(alt);
			Assert::AreEqual(expected, ps);
		}

		TEST_METHOD(TestCalculateStaticPressure)
		{
			AssertStaticPressure(-1240, 105948.53122625740);
			AssertStaticPressure(0, 101325.0);
			AssertStaticPressure(10000, 69681.642852424309);
			AssertStaticPressure(38000, 20646.204802311604);

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 69000;
				double ps = IASsure::calculateTemperature(alt);
				});
		}

		void AssertSpeedOfSound(double temp, double expected)
		{
			double a = IASsure::calculateSpeedOfSound(temp);
			Assert::AreEqual(expected, a);
		}

		TEST_METHOD(TestCalculateSpeedOfSound)
		{
			AssertSpeedOfSound(288.15, 340.65839598822350);
			AssertSpeedOfSound(268.34, 328.73995619250707);
			AssertSpeedOfSound(216.65, 295.38547227038146);
		}

		void AssertDynamicPressure(int alt, int tas, double expected)
		{
			double ps = IASsure::calculateStaticPressure(alt);
			double temp = IASsure::calculateTemperature(alt);
			double qc = IASsure::calculateDynamicPressure(ps, temp, tas);
			Assert::AreEqual(expected, qc);
		}

		TEST_METHOD(TestCalculateDynamicPressure)
		{
			AssertDynamicPressure(-1240, 240, 9999.7960285418503);
			AssertDynamicPressure(-1240, 420, 32721.334039000241);
			AssertDynamicPressure(0, 240, 9647.6057539946978);
			AssertDynamicPressure(0, 420, 31586.470247316036);
			AssertDynamicPressure(10000, 240, 7141.7289467243008);
			AssertDynamicPressure(10000, 420, 23495.958718488586);
			AssertDynamicPressure(38000, 240, 2642.8512782540383);
			AssertDynamicPressure(38000, 420, 8841.8210256694692);

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 0;
				int tas = -240;
				double ps = IASsure::calculateStaticPressure(alt);
				double temp = IASsure::calculateTemperature(alt);
				double qc = IASsure::calculateDynamicPressure(ps, temp, tas);
				});
		}

		void AssertCAS(int alt, double hdg, double gs, IASsure::WeatherReferenceLevel lvl, double expected)
		{
			double cas = IASsure::calculateCAS(alt, hdg, gs, lvl);
			Assert::AreEqual(expected, cas);
		}

		TEST_METHOD(TestCalculateCAS)
		{
			AssertCAS(-1240, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 243.92230995503945);
			AssertCAS(-1240, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 426.30653966366435);
			AssertCAS(0, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 239.72636800304551);
			AssertCAS(0, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 419.52114400532901);
			AssertCAS(10000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 207.11205278318369);
			AssertCAS(10000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 366.10860038082717);
			AssertCAS(38000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 126.95543475346444);
			AssertCAS(38000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 229.80073722586474);

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 69000;
				int tas = 240;
				double cas = IASsure::calculateCAS(alt, 0, tas, IASsure::WeatherReferenceLevel{ 0, 0, 0 });
				});

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 0;
				int tas = -240;
				double cas = IASsure::calculateCAS(alt, 0, tas, IASsure::WeatherReferenceLevel{ 0, 0, 0 });
				});
		}

		void AssertMach(int alt, double hdg, double gs, IASsure::WeatherReferenceLevel lvl, double expected)
		{
			double mach = IASsure::calculateMach(alt, hdg, gs, lvl);
			Assert::AreEqual(expected, mach);
		}

		TEST_METHOD(TestCalculateMach)
		{
			AssertMach(-1240, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.36090021075556744);
			AssertMach(-1240, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.63157536882224297);
			AssertMach(0, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.36243541365977028);
			AssertMach(0, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.63426197390459793);
			AssertMach(10000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.37557687865564771);
			AssertMach(10000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.65725953764738343);
			AssertMach(38000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.41798489857229759);
			AssertMach(38000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.73147357250152079);

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 69000;
				int tas = 240;
				double mach = IASsure::calculateMach(alt, 0, tas, IASsure::WeatherReferenceLevel{ 0, 0, 0 });
				});

			Assert::ExpectException<std::domain_error>([]() {
				int alt = 0;
				int tas = -240;
				double mach = IASsure::calculateMach(alt, 0, tas, IASsure::WeatherReferenceLevel{ 0, 0, 0 });
				});
		}
	};
}
