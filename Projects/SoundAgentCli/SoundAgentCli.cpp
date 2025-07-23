#include "stdafx.h"

#include "ApiClient/common/TimeUtil.h"

#include "public/CoInitRaiiHelper.h"
#include "public/SoundAgentInterface.h"

#include <filesystem>
#include <memory>
#include <tchar.h>
#include <magic_enum/magic_enum_iostream.hpp>

#include "ApiClient/common/SpdLogger.h"


namespace
{
    std::ostream& CurrentLocalTimeAsStringShort(std::ostream& os) {
        const std::string currentTime = ed::TimePointToStringAsLocal(
            std::chrono::system_clock::now(),
            false, // insertTBetweenDateAndTime
            false // addTimeZone
        );
        if
        (
            constexpr int beginOfTimeCountingFromTheEnd = 15;
            currentTime.size() >= beginOfTimeCountingFromTheEnd
        )
        {
            os << currentTime.substr(currentTime.size() - beginOfTimeCountingFromTheEnd, 12) << " ";
        }
        return os;
    }
}


class ServiceObserver final : public SoundDeviceObserverInterface {
public:
    explicit ServiceObserver(SoundDeviceCollectionInterface & collection)
        : collection_(collection)
    {
        SetUpLog();
    }

    static void SetUpLog()
    {
        ed::model::Logger::Inst().SetOutputToConsole(true);
        try
        {
            if (std::filesystem::path logFile;
                ed::utility::AppPath::GetAndValidateLogFilePathName(
                    logFile, RESOURCE_FILENAME_ATTRIBUTE)
                )
            {
                ed::model::Logger::Inst().SetPathName(logFile);
            }
            else
            {
                spdlog::warn("Log file can not be written.");
            }
        }
        catch (const std::exception& ex)
        {
            spdlog::warn("Logging set-up partially done; Log file can not be used: {}.", ex.what());
        }
    }


    DISALLOW_COPY_MOVE(ServiceObserver);
    ~ServiceObserver() override = default;

public:
    static void PrintDeviceInfo(const SoundDeviceInterface* device, size_t i)
    {
        using magic_enum::iostream_operators::operator<<;

        const auto idString = device->GetPnpId();
        std::cout << CurrentLocalTimeAsStringShort << "[" << i << "]: " << idString
            << ", \"" << device->GetName()
            << "\", " << device->GetFlow() // magic to string
            << ", Volume " << device->GetCurrentRenderVolume()
            << " / " << device->GetCurrentCaptureVolume()
            << '\n';
    }

    void PrintCollection() const
    {
        for (size_t i = 0; i < collection_.GetSize(); ++i)
        {
            const std::unique_ptr<SoundDeviceInterface> deviceSmartPtr(collection_.CreateItem(i));
            PrintDeviceInfo(deviceSmartPtr.get(), i);
        }
        std::cout << CurrentLocalTimeAsStringShort << "...Collection print finished.";
    }

    void ResetCollectionContentAndPrintIt() const
    {
        std::cout << CurrentLocalTimeAsStringShort << "Regenerating device list.\n";
        collection_.ResetContent();
        PrintCollection();
        std::cout << '\n' << CurrentLocalTimeAsStringShort << "Press Enter to regenerate device list; To stop, type S or Q and press Enter\n";
    }

    void OnCollectionChanged(SoundDeviceEventType event, const std::string& devicePnpId) override
    {
        using magic_enum::iostream_operators::operator<<; // out-of-the-box stream operators for enums

        std::cout << '\n' << CurrentLocalTimeAsStringShort << "Event caught: " << event << "."
            <<  " Device PnP id: " << devicePnpId << '\n';

        std::cout << '\n' << CurrentLocalTimeAsStringShort << "Print collection...\n";
        PrintCollection();
        std::cout << '\n' << CurrentLocalTimeAsStringShort << "Press Enter to regenerate device list; To stop, type S or Q and press Enter\n";
    }

private:
    SoundDeviceCollectionInterface & collection_;
};

namespace
{
    bool StopAndWaitForInput()
    {
        for (;;)
        {
            std::string line;
            std::getline(std::cin, line);
            if (line == "S" || line == "s" || line == "Q" || line == "q")
            {
                return false;
            }
            if (line.empty())
            {
                return true;
            }

            std::cout << '\n' << CurrentLocalTimeAsStringShort << "Input " << line << " not recognized.\n";
        }
    }
}

int _tmain(int argc, _TCHAR * argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    ed::CoInitRaiiHelper coInitHelper;

    const auto coll(SoundAgent::CreateDeviceCollection());

    ServiceObserver o(*coll);
    coll->Subscribe(o);

    bool continueLoop = true;

    while (continueLoop)
    {
        o.ResetCollectionContentAndPrintIt();

        continueLoop = StopAndWaitForInput();
    }

    std::cout << '\n' << CurrentLocalTimeAsStringShort << "Print collection final state...\n";
    o.PrintCollection();

    coll->Unsubscribe(o);

    return 0;
}
