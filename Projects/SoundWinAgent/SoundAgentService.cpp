#include "stdafx.h"

#include <SpdLogger.h>

#include <filesystem>
#include <memory>
#include <tchar.h>

#include <Poco/Util/ServerApplication.h>
#include <Poco/UnicodeConverter.h>
#include <vector>

#include "SodiumCrypt.h"
#include "AudioDeviceApiClient.h"
#include "FormattedOutput.h"
#include "ServiceObserver.h"

#include <public/CoInitRaiiHelper.h>
#include <public/SoundAgentInterface.h>


class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        try {
            const auto msgStart = "Starting Sound Agent...";
            FormattedOutput::LogAndPrint(msgStart);

            const auto coll(SoundAgent::CreateDeviceCollection(L"", true));
            ServiceObserver serviceObserver(*coll, apiBaseUrl_, universalToken_, codespaceName_);
            coll->Subscribe(serviceObserver);

            coll->ResetContent();
            serviceObserver.PostAndPrintCollection();

            waitForTerminationRequest();

            coll->Unsubscribe(serviceObserver);

            const auto msgStop = "Stopping...";
            FormattedOutput::LogAndPrint(msgStop);

            return EXIT_OK;
        }
        catch (const Poco::Exception& ex) {
			SPD_L-> error(ex.displayText());
            return EXIT_SOFTWARE;
        }
    }

    std::wstring ReadWideStringConfigProperty(const std::string & propertyName) const
    {
        if (!config().hasProperty(propertyName))
        {
            const auto msg = std::string("FATAL: No \"") + propertyName + "\" property configured.";
			FormattedOutput::LogAsErrorPrintAndThrow(msg);
        }

        auto narrowVal = config().getString(propertyName);
        try
        {
            narrowVal = SodiumDecrypt(narrowVal, "32-characters-long-secure-key-12");
        }
        catch (const std::exception& ex)
        {
            SPD_L->info("Decryption doesn't work: {}.", ex.what());
        }
        catch (...)
        {
            const auto msg = std::string("Unknown error. Propagating...");
            FormattedOutput::LogAndPrint(msg);
            throw;
        }

		std::wstring returnValue(narrowVal.length(), L' ');
        std::ranges::copy(narrowVal, returnValue.begin());
		return returnValue;
    }

    void initialize(Application& self) override {
        loadConfiguration();
        ServerApplication::initialize(self);

       
		if (argv().size() == 2)
		{
			const auto baseUrlNarrow = argv()[1];
			apiBaseUrl_ = std::wstring(baseUrlNarrow.length(), L' ');
            std::ranges::copy(baseUrlNarrow, apiBaseUrl_.begin());
        }
        else
        {
            apiBaseUrl_ = ReadWideStringConfigProperty(API_BASE_URL_PROPERTY_KEY);
        }

        apiBaseUrl_ += L"/api/AudioDevices";

		universalToken_ = ReadWideStringConfigProperty(UNIVERSAL_TOKEN_PROPERTY_KEY);

		codespaceName_ = ReadWideStringConfigProperty(CODESPACE_NAME_PROPERTY_KEY);

        setUnixOptions(false);  // Force Windows service behavior
    }
private:
	std::wstring apiBaseUrl_;
	std::wstring universalToken_;
    std::wstring codespaceName_;

    // bool isService_ = config().getBool("application.runAsService", false);
    static constexpr auto API_BASE_URL_PROPERTY_KEY = "custom.apiBaseUrl";
    static constexpr auto UNIVERSAL_TOKEN_PROPERTY_KEY = "custom.universalToken";
    static constexpr auto CODESPACE_NAME_PROPERTY_KEY = "custom.codespaceName";
};

int _tmain(int argc, _TCHAR * argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    if (std::filesystem::path logFile;
        ed::utility::AppPath::GetAndValidateLogFileInProgramData(
            logFile, RESOURCE_FILENAME_ATTRIBUTE)
        )
    {
        ed::model::Logger::Inst().SetPathName(logFile);
    }

    ed::CoInitRaiiHelper coInitHelper;

	// Transform Unicode command line arguments to UTF-8
    std::vector<std::string> args;
    std::vector<char*> charPointers;

    for (int i = 0; i < argc; ++i) {
        std::string utf8Arg;
        Poco::UnicodeConverter::toUTF16(argv[i], utf8Arg);
        args.push_back(utf8Arg);
    }

    for (auto& arg : args) {
        charPointers.push_back(arg.data());
    }

    AudioDeviceService app;
    return app.run(static_cast<int>(charPointers.size()), charPointers.data());
}
