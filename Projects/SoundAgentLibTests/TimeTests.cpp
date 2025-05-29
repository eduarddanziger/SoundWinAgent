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
    TEST_METHOD(LocalTimeTest)
    {
	    constexpr auto localTimePoint = local_days{ 2025y / 5 / 29 } + 12h + 34min + 56s + 223709us;
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
		const auto zoneAsString = fmt::format("{:%z}", localTimePoint);
        const auto fromFmt = fmt::format("{:%F %T%z}", localTimePoint);
        Assert::AreEqual("2025-05-29 12:34:56.223709"s + zoneAsString, fromTimeUtil);
        Assert::AreEqual("2025-05-29 12:34:56.223709"s + zoneAsString, fromFmt);
    }
    TEST_METHOD(SystemTimeTest)
    {
        constexpr auto timePoint = sys_days{ 2025y / 5 / 29 } + 10h + 34min + 56s + 223709us;

        // with T as delimiter
        auto fromTimeUtil = ed::TimePointToStringAsUtc(timePoint, true, false);
        Assert::AreEqual("2025-05-29T10:34:56.223709"s, fromTimeUtil);

        // with space as delimiter
        fromTimeUtil = ed::TimePointToStringAsUtc(timePoint, false, false);
        Assert::AreEqual("2025-05-29 10:34:56.223709"s, fromTimeUtil);

    	// with space as delimiter and a time zone
        fromTimeUtil = ed::TimePointToStringAsUtc(timePoint, false, true);
        Assert::AreEqual("2025-05-29 10:34:56.223709Z"s, fromTimeUtil);
    }

};
}
