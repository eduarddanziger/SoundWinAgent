#include "stdafx.h"

#include <SpdLogger.h>

#include <filesystem>
#include <iomanip>
#include <memory>
#include <tchar.h>

#include <public/CoInitRaiiHelper.h>
#include <public/SoundAgentInterface.h>
#include <public/DefToString.h>
#include "TimeUtils.h" // Include the header for TimeUtils

namespace
{
    std::wostream& CurrentLocalTimeWithoutDate(std::wostream& os) {
        const std::wstring currentTime = ed::getLocalTimeAsWideString();
        if
        (
            constexpr int beginOfTimeCountingFromTheEnd = 15;
            currentTime.size() >= beginOfTimeCountingFromTheEnd
        )
        {
            os << currentTime.substr(currentTime.size() - beginOfTimeCountingFromTheEnd, 12) << L" ";
        }
        return os;
    }
}


class ServiceObserver final : public SoundDeviceObserverInterface {
public:
    explicit ServiceObserver(SoundDeviceCollectionInterface & collection)
        : collection_(collection)
    {
        if (std::filesystem::path logFile;
            ed::utility::AppPath::GetAndValidateLogFileInProgramData(
                logFile, RESOURCE_FILENAME_ATTRIBUTE)
            )
        {
            ed::model::Logger::Inst().SetPathName(logFile);
        }
    }

    DISALLOW_COPY_MOVE(ServiceObserver);
    ~ServiceObserver() override = default;

public:
    static void PrintDeviceInfo(const SoundDeviceInterface* device, size_t i)
    {
        const auto idString = device->GetPnpId();
        const std::wstring idAsWideString(idString.begin(), idString.end());
        std::wcout << CurrentLocalTimeWithoutDate << L"[" << i << L"]: " << idAsWideString
            << L", \"" << device->GetName()
            << L"\", " << ed::GetFlowAsString(device->GetFlow())
            << L", Volume " << device->GetCurrentRenderVolume()
			<< L" / " << device->GetCurrentCaptureVolume()
            << '\n';
    }

    void PrintCollection() const
    {
        for (size_t i = 0; i < collection_.GetSize(); ++i)
        {
            const std::unique_ptr<SoundDeviceInterface> deviceSmartPtr(collection_.CreateItem(i));
            PrintDeviceInfo(deviceSmartPtr.get(), i);
        }
        std::wcout << '\n' << CurrentLocalTimeWithoutDate << "Press Enter to regenerate device list; To stop, type S or Q and press Enter\n";
    }

    void ResetCollectionContentAndPrintIt() const
    {
        std::wcout << CurrentLocalTimeWithoutDate << L"Regenerating device list.\n";
        collection_.ResetContent();
        PrintCollection();
    }

    void OnCollectionChanged(SoundDeviceEventType event, const std::wstring& devicePnpId) override
    {
        std::wcout << '\n' << CurrentLocalTimeWithoutDate << L"Event caught: " << ed::GetDeviceCollectionEventAsString(event) << L"."
            <<  L" Device PnP id: " << devicePnpId << L'\n';

        PrintCollection();
    }

    void OnTrace(const std::wstring& line) override
    {
        std::string result; result.reserve(line.size());
        std::ranges::for_each(line, [&result](const auto p)
        {
            result += static_cast<char>(p);
        });

        SPD_L->info(result);
    }

    void OnTraceDebug(const std::wstring& line) override
    {
        OnTrace(line);
    }

private:
    SoundDeviceCollectionInterface & collection_;
};

bool StopAndWaitForInput()
{
    for (;;)
    {
        std::wstring line;
        std::getline(std::wcin, line);
        if (line == L"S" || line == L"s" || line == L"Q" || line == L"q")
        {
            return false;
        }
        if (line.empty())
        {
            return true;
        }

        std::wcout << '\n' << CurrentLocalTimeWithoutDate << L"Input " << line << L" not recognized.\n";
    }
}

int _tmain(int argc, _TCHAR * argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    bool bothHeadsetAndMicro = true;
    std::wstring filter;
    if (argc > 1)
    {
        filter = std::wstring(argv[1]);
    }

    if (argc > 2)
    {
        bothHeadsetAndMicro = argv[2][0] != L'0';
    }

    if (argc > 3)
    {
        std::wcout << L"Wrong command line!\nUsage: \"" << argv[0] << "\" <filter substring>\n";
        return -1;
    }

    ed::CoInitRaiiHelper coInitHelper;
    const auto coll(SoundAgent::CreateDeviceCollection(filter, bothHeadsetAndMicro));
    ServiceObserver o(*coll);
    coll->Subscribe(o);

    bool continueLoop = true;

    while (continueLoop)
    {
        o.ResetCollectionContentAndPrintIt();

        continueLoop = StopAndWaitForInput();
    }

    coll->Unsubscribe(o);

    return 0;
}
