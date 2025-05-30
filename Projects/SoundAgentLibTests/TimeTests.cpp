#include "stdafx.h"

#include <CppUnitTest.h>

#include <fmt/chrono.h>

#include <TimeUtils.h>

#include "public/generate-uuid.h"
#include "public/TimeUtil.h"

using namespace std::literals;
using namespace std::chrono;
using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed
{

    template<typename Char_, typename Clock_, class Duration_ = typename Clock_::duration>
    static std::basic_string<Char_> systemTimeToString(const std::chrono::time_point<Clock_, Duration_>& time, const std::basic_string<Char_>& betweenDateAndTime)
    {
        const time_t timeT = to_time_t(time);

        tm utcTimeT{};
        if (gmtime_s(&utcTimeT, &timeT) != 0)
        {
            return std::basic_string<Char_>();
        }

        const auto microsecondsFraction = chr::duration_cast<chr::microseconds>(
            time.time_since_epoch()
        ).count() % 1000000;

        std::basic_ostringstream<Char_> oss; oss
            << std::setbase(10)
            << std::setfill(any_string_array<Char_>("0").data()[0])
            << std::setw(4) << utcTimeT.tm_year + 1900
            << any_string_array<Char_>("-").data()
            << std::setw(2) << utcTimeT.tm_mon + 1
            << any_string_array<Char_>("-").data()
            << std::setw(2) << utcTimeT.tm_mday
            << betweenDateAndTime
            << std::setw(2) << utcTimeT.tm_hour
            << any_string_array<Char_>(":").data()
            << std::setw(2) << utcTimeT.tm_min
            << any_string_array<Char_>(":").data()
            << std::setw(2) << utcTimeT.tm_sec
            << any_string_array<Char_>(".").data()
            << std::setw(6)
            << microsecondsFraction;

        return oss.str();
    }

    template<typename Clock_, class Duration_ = typename Clock_::duration>
    [[nodiscard]] std::string systemTimeAsStringWithSystemTime(const std::chrono::time_point<Clock_, Duration_>& time, const std::string& betweenDateAndTime = " ")
    {
        return systemTimeToString(time, betweenDateAndTime);
    }

    [[nodiscard]] inline std::string getSystemTimeAsString(const std::string& betweenDateAndTime = " ")
    {
        return systemTimeAsStringWithSystemTime(chr::system_clock::now(), betweenDateAndTime);
    }

}

namespace ed::audio {
TEST_CLASS(TimeTests) {
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
		auto fromTimeUtil = ed::TimePointToStringAsLocal(timePoint, true, false);
        Assert::AreEqual("2025-05-29T12:34:56.223709"s, fromTimeUtil);

        // with space as delimiter
        fromTimeUtil = ed::TimePointToStringAsLocal(timePoint, false, false);
        Assert::AreEqual("2025-05-29 12:34:56.223709"s, fromTimeUtil);

        // with space as delimiter and a time zone
        fromTimeUtil = ed::TimePointToStringAsLocal(timePoint, false, true);
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
        auto fromTimeUtil = ed::TimePointToStringAsUtc(utcTimePoint, true, false);
        Assert::AreEqual("2025-05-29T10:34:56.223709"s, fromTimeUtil);

        // with space as delimiter
        fromTimeUtil = ed::TimePointToStringAsUtc(utcTimePoint, false, false);
        Assert::AreEqual("2025-05-29 10:34:56.223709"s, fromTimeUtil);

    	// with space as delimiter and a time zone
        fromTimeUtil = ed::TimePointToStringAsUtc(utcTimePoint, false, true);
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
        auto fromTimeUtil = ed::TimePointToStringAsUtc(utcTimePoint, true, false);
        Assert::AreEqual("2025-05-29T10:34:56.500000"s, fromTimeUtil);

        // with space as delimiter
        fromTimeUtil = ed::TimePointToStringAsUtc(utcTimePoint, false, false);
        Assert::AreEqual("2025-05-29 10:34:56.500000"s, fromTimeUtil);

        // with space as delimiter and a time zone
        fromTimeUtil = ed::TimePointToStringAsUtc(utcTimePoint, false, true);
        const auto fromFmtDirectly = fmt::format("{:%F %TZ}", utcTimePoint);

        const auto expectedWithTimeZone = "2025-05-29 10:34:56.500000Z"s;
        Assert::AreEqual(expectedWithTimeZone, fromTimeUtil);
        Assert::AreEqual(expectedWithTimeZone, fromFmtDirectly);
    }
};
}
