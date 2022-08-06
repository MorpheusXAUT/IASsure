#include <CppUnitTest.h>

#include "../IASsure/helpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IASsureTest
{
	TEST_CLASS(Helpers)
	{
	public:
		void AssertRoundToNearestInt(long num, long multiple, long expected)
		{
			long nearest = IASsure::roundToNearest(num, multiple);
			Assert::AreEqual(expected, nearest);
		}

		void AssertRoundToNearestDouble(double num, long multiple, long expected)
		{
			long nearest = IASsure::roundToNearest(num, multiple);
			Assert::AreEqual(expected, nearest);
		}

		TEST_METHOD(TestRoundToNearest)
		{
			AssertRoundToNearestInt(0, 1, 0);
			AssertRoundToNearestInt(1, 1, 1);
			AssertRoundToNearestInt(3, 1, 3);
			AssertRoundToNearestInt(3, 5, 5);
			AssertRoundToNearestInt(3, 10, 0);
			AssertRoundToNearestInt(4, 5, 5);
			AssertRoundToNearestInt(4, 10, 0);
			AssertRoundToNearestInt(5, 5, 5);
			AssertRoundToNearestInt(5, 10, 10);
			AssertRoundToNearestInt(7, 5, 5);
			AssertRoundToNearestInt(7, 10, 10);
			AssertRoundToNearestInt(8, 5, 10);
			AssertRoundToNearestInt(8, 10, 10);

			AssertRoundToNearestDouble(0.0, 1, 0);
			AssertRoundToNearestDouble(0.3, 1, 0);
			AssertRoundToNearestDouble(0.5, 1, 1);
			AssertRoundToNearestDouble(0.9, 1, 1);
			AssertRoundToNearestDouble(1.0, 1, 1);
			AssertRoundToNearestDouble(3.0, 1, 3);
			AssertRoundToNearestDouble(3.3, 1, 3);
			AssertRoundToNearestDouble(3.5, 1, 4);
			AssertRoundToNearestDouble(3.9, 1, 4);
			AssertRoundToNearestDouble(3.0, 5, 5);
			AssertRoundToNearestDouble(3.3, 5, 5);
			AssertRoundToNearestDouble(3.5, 5, 5);
			AssertRoundToNearestDouble(3.9, 5, 5);
			AssertRoundToNearestDouble(3.0, 10, 0);
			AssertRoundToNearestDouble(3.3, 10, 0);
			AssertRoundToNearestDouble(3.5, 10, 0);
			AssertRoundToNearestDouble(3.9, 10, 0);
			AssertRoundToNearestDouble(4.0, 5, 5);
			AssertRoundToNearestDouble(4.3, 5, 5);
			AssertRoundToNearestDouble(4.5, 5, 5);
			AssertRoundToNearestDouble(4.9, 5, 5);
			AssertRoundToNearestDouble(4.0, 10, 0);
			AssertRoundToNearestDouble(4.3, 10, 0);
			AssertRoundToNearestDouble(4.5, 10, 10);
			AssertRoundToNearestDouble(4.9, 10, 10);
			AssertRoundToNearestDouble(5.0, 5, 5);
			AssertRoundToNearestDouble(5.3, 5, 5);
			AssertRoundToNearestDouble(5.5, 5, 5);
			AssertRoundToNearestDouble(5.9, 5, 5);
			AssertRoundToNearestDouble(5.0, 10, 10);
			AssertRoundToNearestDouble(5.3, 10, 10);
			AssertRoundToNearestDouble(5.5, 10, 10);
			AssertRoundToNearestDouble(5.9, 10, 10);
			AssertRoundToNearestDouble(7.0, 5, 5);
			AssertRoundToNearestDouble(7.3, 5, 5);
			AssertRoundToNearestDouble(7.5, 5, 10);
			AssertRoundToNearestDouble(7.9, 5, 10);
			AssertRoundToNearestDouble(7.0, 10, 10);
			AssertRoundToNearestDouble(7.3, 10, 10);
			AssertRoundToNearestDouble(7.5, 10, 10);
			AssertRoundToNearestDouble(7.9, 10, 10);
			AssertRoundToNearestDouble(8.0, 5, 10);
			AssertRoundToNearestDouble(8.3, 5, 10);
			AssertRoundToNearestDouble(8.5, 5, 10);
			AssertRoundToNearestDouble(8.9, 5, 10);
			AssertRoundToNearestDouble(8.0, 10, 10);
			AssertRoundToNearestDouble(8.3, 10, 10);
			AssertRoundToNearestDouble(8.5, 10, 10);
			AssertRoundToNearestDouble(8.9, 10, 10);

			Assert::ExpectException<std::domain_error>([]() {
				long nearest = IASsure::roundToNearest(0l, 0l);
				});
			Assert::ExpectException<std::domain_error>([]() {
				long nearest = IASsure::roundToNearest(0.0, 0l);
				});
		}
	};
}
