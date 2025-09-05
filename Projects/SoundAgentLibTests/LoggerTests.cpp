#include "stdafx.h"

#include "ApiClient/common/SpdLogger.h"

#include <CppUnitTest.h>

#include <format>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std::literals::string_literals;


namespace ed::tests
{
    TEST_CLASS(LoggerTests)
    {
        TEST_METHOD(AppVersionTest)
        {
            const auto spLogBuffer = std::make_shared<model::LogBuffer>();
            static constexpr auto APP_NAME = "TestApp";
            static constexpr auto APP_VERSION = "0.1";
            model::Logger::Inst().ConfigureAppNameAndVersion(APP_NAME, APP_VERSION).SetLogBuffer(spLogBuffer);
            model::Logger::Inst().Free();

            const auto logEntries = spLogBuffer->GetAndClearNextQueueChunk();
            Assert::IsTrue(!logEntries.empty(), L"LogBuffer must contain at least 1 element");

            const auto& entry = logEntries[0];

            const auto errMessage = std::format("LogBuffer (size {}) first record \"{}\" does not contain both {} and {}.", logEntries.size(), entry, APP_NAME, APP_VERSION);
            std::wstring widenedErrMessage(errMessage.length(), L' ');
            std::ranges::copy(errMessage, widenedErrMessage.begin());

            Assert::IsTrue
            (
    entry.find(APP_NAME) != std::wstring::npos && entry.find(APP_VERSION) != std::wstring::npos
            , widenedErrMessage.c_str()
            );
        }
    };
}
