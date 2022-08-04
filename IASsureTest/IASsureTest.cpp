#include "CppUnitTest.h"

#include "../IASsure/calculations.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IASsureTest
{
	TEST_CLASS(IASsureTest)
	{
	public:
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

			Assert::ExpectException<std::out_of_range>([]() {
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

			Assert::ExpectException<std::out_of_range>([]() {
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

			Assert::ExpectException<std::out_of_range>([]() {
				int alt = 0;
				int tas = -240;
				double ps = IASsure::calculateStaticPressure(alt);
				double temp = IASsure::calculateTemperature(alt);
				double qc = IASsure::calculateDynamicPressure(ps, temp, tas);
				});
		}

		void AssertCAS(int alt, int tas, double expected)
		{
			double cas = IASsure::calculateCAS(alt, tas);
			Assert::AreEqual(expected, cas);
		}

		TEST_METHOD(TestCalculateCAS)
		{
			AssertCAS(-1240, 240, 243.92004106644615);
			AssertCAS(-1240, 420, 426.30259283784403);
			AssertCAS(0, 240, 239.72422759440275);
			AssertCAS(0, 420, 419.51739829020431);
			AssertCAS(10000, 240, 207.11086025038352);
			AssertCAS(10000, 420, 366.10637790948800);
			AssertCAS(38000, 240, 126.95436608760033);
			AssertCAS(38000, 420, 229.79866924051956);

			Assert::ExpectException<std::out_of_range>([]() {
				int alt = 69000;
				int tas = 240;
				double cas = IASsure::calculateCAS(alt, tas);
				});

			Assert::ExpectException<std::out_of_range>([]() {
				int alt = 0;
				int tas = -240;
				double cas = IASsure::calculateCAS(alt, tas);
				});
		}

		void AssertMach(int alt, int tas, double expected)
		{
			double mach = IASsure::calculateMach(alt, tas);
			Assert::AreEqual(expected, mach);
		}

		TEST_METHOD(TestCalculateMach)
		{
			AssertMach(-1240, 240, 0.36089698844032253);
			AssertMach(-1240, 420, 0.63156972977056436);
			AssertMach(0, 240, 0.36243217763739083);
			AssertMach(0, 420, 0.63425631086543399);
			AssertMach(10000, 240, 0.37557352529905308);
			AssertMach(10000, 420, 0.65725366927334283);
			AssertMach(38000, 240, 0.41798116657361573);
			AssertMach(38000, 420, 0.73146704150382746);

			Assert::ExpectException<std::out_of_range>([]() {
				int alt = 69000;
				int tas = 240;
				double mach = IASsure::calculateMach(alt, tas);
				});

			Assert::ExpectException<std::out_of_range>([]() {
				int alt = 0;
				int tas = -240;
				double mach = IASsure::calculateMach(alt, tas);
				});
		}
	};
}
