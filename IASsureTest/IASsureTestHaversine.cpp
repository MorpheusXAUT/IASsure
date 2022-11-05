#include <CppUnitTest.h>

#include "../IASsure/haversine.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IASsureTest
{
	TEST_CLASS(Haversine)
	{
	public:
		void AssertDegToRad(double degrees, double expected)
		{
			double rad = IASsure::degToRad(degrees);
			Assert::AreEqual(expected, rad);
		}

		TEST_METHOD(TestDegToRad)
		{
			AssertDegToRad(0, 0);
			AssertDegToRad(42, 0.73303828583761843);
			AssertDegToRad(69, 1.2042771838760873);
			AssertDegToRad(180, 3.1415926535897931);
			AssertDegToRad(360, 6.2831853071795862);
			AssertDegToRad(420, 7.3303828583761845);
			AssertDegToRad(720, 12.566370614359172);
		}

		void AssertHaversineDistance(double lat1, double long1, double lat2, double long2, double expected)
		{
			double distance = IASsure::haversine(lat1, long1, lat2, long2);
			Assert::AreEqual(expected, distance);
		}

		TEST_METHOD(TestHaversineDistance)
		{
			AssertHaversineDistance(0, 0, 0, 0, 0);
			AssertHaversineDistance(0, 0, 1, 1, 157249.59776813339);
			AssertHaversineDistance(42, 42, 69, 69, 3387050.1327445381);
			AssertHaversineDistance(-90, -180, 90, 180, 20015114.352186374);
			AssertHaversineDistance(-180, -180, 180, 180, 2.2068054519693118e-09);
		}
	};
}
