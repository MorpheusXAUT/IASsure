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
			AssertStaticPressure(-1240, 105948.44680813159);
			AssertStaticPressure(0, 101325.0);
			AssertStaticPressure(10000, 69682.108717641051);
			AssertStaticPressure(38000, 20646.238661044939);

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
			AssertSpeedOfSound(288.15, 340.66143760058901);
			AssertSpeedOfSound(268.34, 328.74289138954776);
			AssertSpeedOfSound(216.65, 295.38810965761638);
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
			AssertDynamicPressure(-1240, 240, 9999.6037191288324);
			AssertDynamicPressure(-1240, 420, 32720.666620008196);
			AssertDynamicPressure(0, 240, 9647.4278574125346);
			AssertDynamicPressure(0, 420, 31585.850684308385);
			AssertDynamicPressure(10000, 240, 7141.6446983355545);
			AssertDynamicPressure(10000, 420, 23495.651955122459);
			AssertDynamicPressure(38000, 240, 2642.8063762573533);
			AssertDynamicPressure(38000, 420, 8841.6571184346903);

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
			AssertCAS(-1240, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 243.92004106644615);
			AssertCAS(-1240, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 426.30259283784403);
			AssertCAS(0, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 239.72422759440275);
			AssertCAS(0, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 419.51739829020431);
			AssertCAS(10000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 207.11086025038352);
			AssertCAS(10000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 366.10637790948800);
			AssertCAS(38000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 126.95436608760033);
			AssertCAS(38000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 229.79866924051956);

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
			AssertMach(-1240, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.36089698844032253);
			AssertMach(-1240, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.63156972977056436);
			AssertMach(0, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.36243217763739083);
			AssertMach(0, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.63425631086543399);
			AssertMach(10000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.37557352529905308);
			AssertMach(10000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.65725366927334283);
			AssertMach(38000, 0, 240, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.41798116657361573);
			AssertMach(38000, 0, 420, IASsure::WeatherReferenceLevel{ 0, 0, 0 }, 0.73146704150382746);

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
