#include "stdafx.h"

#include <CppUnitTest.h>

#include <fmt/chrono.h>
#include <TimeUtils.h>

#include "public/generate-uuid.h"
#include "public/TimeUtil.h"

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


using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ed::audio {
TEST_CLASS(TimeTests) {
    TEST_METHOD(LocalTimeTest)
    {
		const auto nowTime = std::chrono::system_clock::now();

		// with T as delimiter
    	auto fromNuget = systemTimeAsStringWithLocalTime(nowTime, "T");
		auto fromTimeUtil = ed::TimePointToStringAsLocal(nowTime, true, false);
        Assert::AreEqual(fromTimeUtil, fromNuget);

        // with space as delimiter
        fromNuget = systemTimeAsStringWithLocalTime(nowTime, " ");
        fromTimeUtil = ed::TimePointToStringAsLocal(nowTime, false, false);
        Assert::AreEqual(fromTimeUtil, fromNuget);

        // with space as delimiter and a time zone
        fromTimeUtil = ed::TimePointToStringAsLocal(nowTime, false, true);
        const auto lengthWithTimeZone = fromTimeUtil.size();
        const auto lengthWithoutTimeZone = fromNuget.size();
        Assert::AreEqual(5ull, lengthWithTimeZone - lengthWithoutTimeZone );
    }
    TEST_METHOD(SystemTimeTest)
    {
        const auto nowTime = std::chrono::system_clock::now();

        // with T as delimiter
        auto fromNuget = systemTimeAsStringWithSystemTime(nowTime, "T");
        auto fromTimeUtil = ed::TimePointToStringAsUtc(nowTime, true, false);
        Assert::AreEqual(fromNuget, fromTimeUtil);

        // with space as delimiter
        fromNuget = systemTimeAsStringWithSystemTime(nowTime, " ");
        fromTimeUtil = ed::TimePointToStringAsUtc(nowTime, false, false);
        Assert::AreEqual(fromNuget, fromTimeUtil);

    	// with space as delimiter and a time zone
        fromTimeUtil = ed::TimePointToStringAsUtc(nowTime, false, true);
        const auto lengthWithTimeZone = fromTimeUtil.size();
        const auto lengthWithoutTimeZone = fromNuget.size();
        Assert::AreEqual("Z"s, fromTimeUtil.substr(lengthWithoutTimeZone, lengthWithTimeZone - lengthWithoutTimeZone));
    }

};
}
