#include "stdafx.h"

#include <CppUnitTest.h>

#include <fmt/chrono.h>

#include "public/generate-uuid.h"

#include "ApiClient/common/TimeUtil.h"

using namespace std::literals;
using namespace std::chrono;
using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed::audio
{
    TEST_CLASS(TimeTests)
    {
        TEST_METHOD(NowTimeTest)
        {
            const auto nowTime = floor<microseconds>(system_clock::now());

            // as system time
            auto fromTimeUtil = TimePointToStringAsUtc(nowTime, true, true);
            auto fromFmtDirectly = fmt::format("{:%FT%TZ}", nowTime);
            Logger::WriteMessage(fromFmtDirectly.c_str());
            Assert::AreEqual(fromFmtDirectly, fromTimeUtil);

            // as local time
            fromTimeUtil = TimePointToStringAsLocal(nowTime, true, true);
            const auto zonedTime = zoned_time{ std::chrono::current_zone(), nowTime };
            auto zonedTimeAsSysTime = zonedTime.get_local_time();

            fromFmtDirectly = fmt::format("{:%FT%T%z}", zonedTimeAsSysTime);
            Assert::AreEqual(fromFmtDirectly, fromTimeUtil);
        }

        TEST_METHOD(LocalTimeTest)
        {
            // time as we see it in on the normal clock
            constexpr auto yearMonthDay = 2025y / 5 / 29;
            constexpr auto periodInMicroseconds = 12h + 34min + 56s + 223709us;

            constexpr auto localTimePoint = local_days{ yearMonthDay } + periodInMicroseconds;
            const zoned_time zt{ current_zone(), localTimePoint };
            const auto timePoint = zt.get_sys_time();

            // with T as delimiter
            auto fromTimeUtil = TimePointToStringAsLocal(timePoint, true, false);
            Assert::AreEqual("2025-05-29T12:34:56.223709"s, fromTimeUtil);

            // with space as delimiter
            fromTimeUtil = TimePointToStringAsLocal(timePoint, false, false);
            Assert::AreEqual("2025-05-29 12:34:56.223709"s, fromTimeUtil);

            // with space as delimiter and a time zone
            fromTimeUtil = TimePointToStringAsLocal(timePoint, false, true);
            const auto fromFmtDirectly = fmt::format("{:%F %T%z}", localTimePoint);

            const auto zoneAsString = fmt::format("{:%z}", localTimePoint);
            const auto expectedWithTimeZone = "2025-05-29 12:34:56.223709"s + zoneAsString;
            Assert::AreEqual(expectedWithTimeZone, fromTimeUtil);
            Assert::AreEqual(expectedWithTimeZone, fromFmtDirectly);
        }

        TEST_METHOD(SystemTimeTest)
        {
            // time as we see it in on the atomic clock
            constexpr auto yearMonthDay = 2025y / 5 / 29;
            constexpr auto periodInMicroseconds = 10h + 34min + 56s + 223709us;

            constexpr auto utcTimePoint = sys_days{ yearMonthDay } + periodInMicroseconds;

            // with T as delimiter
            auto fromTimeUtil = TimePointToStringAsUtc(utcTimePoint, true, false);
            Assert::AreEqual("2025-05-29T10:34:56.223709"s, fromTimeUtil);

            // with space as delimiter
            fromTimeUtil = TimePointToStringAsUtc(utcTimePoint, false, false);
            Assert::AreEqual("2025-05-29 10:34:56.223709"s, fromTimeUtil);

            // with space as delimiter and a time zone
            fromTimeUtil = TimePointToStringAsUtc(utcTimePoint, false, true);
            const auto fromFmtDirectly = fmt::format("{:%F %TZ}", utcTimePoint);

            const auto expectedWithTimeZone = "2025-05-29 10:34:56.223709Z"s;
            Assert::AreEqual(expectedWithTimeZone, fromTimeUtil);
            Assert::AreEqual(expectedWithTimeZone, fromFmtDirectly);
        }

        TEST_METHOD(SystemTimeTestWithMilliseconds)
        {
            // time as we see it in on the atomic clock
            constexpr auto yearMonthDay = 2025y / 5 / 29;
            constexpr auto periodInSeconds = 10h + 34min + 56s + 500ms;

            constexpr auto utcTimePoint = floor<microseconds>(sys_days{ yearMonthDay } + periodInSeconds);
            // Note: The microseconds tail is zero in this case

            // with T as delimiter
            auto fromTimeUtil = TimePointToStringAsUtc(utcTimePoint, true, false);
            Assert::AreEqual("2025-05-29T10:34:56.500000"s, fromTimeUtil);

            // with space as delimiter
            fromTimeUtil = TimePointToStringAsUtc(utcTimePoint, false, false);
            Assert::AreEqual("2025-05-29 10:34:56.500000"s, fromTimeUtil);

            // with space as delimiter and a time zone
            fromTimeUtil = TimePointToStringAsUtc(utcTimePoint, false, true);
            const auto fromFmtDirectly = fmt::format("{:%F %TZ}", utcTimePoint);

            const auto expectedWithTimeZone = "2025-05-29 10:34:56.500000Z"s;
            Assert::AreEqual(expectedWithTimeZone, fromTimeUtil);
            Assert::AreEqual(expectedWithTimeZone, fromFmtDirectly);
        }
    };

    TEST_CLASS(TimeOldTests)
    {
        constexpr static size_t TIME_STR_LENGTH =
            4 + 1 + 2 + 1 + 2
            + 1
            + 2 + 1 + 2 + 1 + 2
            + 1
            + 6;
        constexpr static size_t OFFSET_TO_MICROSECONDS_FRACTION = TIME_STR_LENGTH - (1 + 6);
        constexpr static size_t OFFSET_TO_SECONDS_FRACTION = OFFSET_TO_MICROSECONDS_FRACTION - (1 + 2);
        constexpr static size_t OFFSET_TO_MINUTES_FRACTION = OFFSET_TO_SECONDS_FRACTION - (1 + 2);
        constexpr static size_t OFFSET_TO_HOURS_FRACTION = OFFSET_TO_MINUTES_FRACTION - (1 + 2);

        TEST_METHOD(CheckTimeUtcCreatedFromSeconds)
        {
            constexpr system_clock::time_point s54{ 54s };
            const auto strS54 = TimePointToStringAsUtc(s54, false, true);
            Assert::AreEqual(strS54.substr(OFFSET_TO_SECONDS_FRACTION, 3), std::string(":54"));
        }

        TEST_METHOD(CheckNowTimeLocalMicrosecondsFraction)
        {
            const auto timeNow = system_clock::now();
            const auto str = TimePointToStringAsLocal(timeNow, false, false);
            Assert::AreEqual(str.length(), TIME_STR_LENGTH);

            const auto timeNowRoundedToSeconds = time_point_cast<seconds>(timeNow);
            const auto strRoundedToSeconds = TimePointToStringAsLocal(timeNowRoundedToSeconds, false, false);
            Assert::AreEqual(str.substr(0, OFFSET_TO_MICROSECONDS_FRACTION), str.substr(0, OFFSET_TO_MICROSECONDS_FRACTION));
            Assert::AreEqual(std::string(".000000"),strRoundedToSeconds.substr(OFFSET_TO_MICROSECONDS_FRACTION, 7));

            const auto timeNowWith11Microseconds = timeNowRoundedToSeconds + microseconds(11);
            const auto strWith11Microseconds = TimePointToStringAsLocal(timeNowWith11Microseconds, false, false);
            Assert::AreEqual(str.substr(0, OFFSET_TO_MICROSECONDS_FRACTION), strWith11Microseconds.substr(0, OFFSET_TO_MICROSECONDS_FRACTION));
            Assert::AreEqual(std::string(".000011"), strWith11Microseconds.substr(OFFSET_TO_MICROSECONDS_FRACTION, 7));
        }

    };
}
