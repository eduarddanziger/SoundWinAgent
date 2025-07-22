#include "os-dependencies.h"

#include "ApiClient/common/SpdLogger.h"

#include "ApiClient/SodiumCrypt.h"
#include "ServiceObserver.h"
#include "public/CoInitRaiiHelper.h"
#include "public/SoundAgentInterface.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <tchar.h>
#include <vector>

#include <Poco/Util/ServerApplication.h>
#include <Poco/UnicodeConverter.h>
#include <Poco/Util/HelpFormatter.h>
#include <spdlog/spdlog.h>


class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        if (helpRequested_)
        {
            return Application::EXIT_OK;
        }

        try {
            spdlog::info("Starting Sound Agent...");

            const auto coll(SoundAgent::CreateDeviceCollection());

            ServiceObserver serviceObserver(*coll, apiBaseUrl_, universalToken_, codeSpaceName_);
            coll->Subscribe(serviceObserver);

            coll->ResetContent();
            serviceObserver.PostAndPrintCollection();

            waitForTerminationRequest();

            coll->Unsubscribe(serviceObserver);

            spdlog::info("Stopping...");

            return EXIT_OK;
        }
        catch (const Poco::Exception& ex) {
            spdlog::error(ex.displayText());
            return EXIT_SOFTWARE;
        }
    }

    [[nodiscard]] std::string ReadStringConfigProperty(const std::string & propertyName) const
    {
        if (!config().hasProperty(propertyName))
        {
            const auto msg = std::string("FATAL: No \"") + propertyName + "\" property configured.";
            spdlog::error(msg);
            throw std::runtime_error(msg);
        }

        auto returnValue = config().getString(propertyName);
        try
        {
            returnValue = SodiumDecrypt(returnValue, "32-characters-long-secure-key-12");
        }
        catch (const std::exception& ex)  // NOLINT(bugprone-empty-catch)
        {
            spdlog::info("Decryption doesn't work: {}.", ex.what());
        }
        catch (...)
        {
            spdlog::error("Unknown error. Propagating...");
            throw;
        }

        return returnValue;
    }

    static void SetUpLog()
    {
        ed::model::Logger::Inst().SetOutputToConsole(true);
        try
        {
            if (std::filesystem::path logFile;
                ed::utility::AppPath::GetAndValidateLogFileInProgramData(
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

    void initialize(Application& self) override {
        loadConfiguration();
        ServerApplication::initialize(self);

        if (helpRequested_)
        {
            return;
        }

        SetUpLog();

        if (apiBaseUrl_.empty())
        {
            apiBaseUrl_ = ReadStringConfigProperty(API_BASE_URL_PROPERTY_KEY);
        }
       
        apiBaseUrl_ += "/api/AudioDevices";

        universalToken_ = ReadStringConfigProperty(UNIVERSAL_TOKEN_PROPERTY_KEY);

        codeSpaceName_ = ReadStringConfigProperty(CODESPACE_NAME_PROPERTY_KEY);

        setUnixOptions(false);  // Force Windows service behavior
    }

    void defineOptions(Poco::Util::OptionSet& options) override
    {
        ServerApplication::defineOptions(options);

        options.addOption(
            Poco::Util::Option("url", "", "Base Server URL, e.g. http://localhost:5027")
            .required(false)
            .repeatable(false)
            .argument("<url>", true)
            .callback(Poco::Util::OptionCallback<AudioDeviceService>(this, &AudioDeviceService::HandleUrl)));

        options.addOption(
            Poco::Util::Option("help", "h", "Help information")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<AudioDeviceService>(this, &AudioDeviceService::HandleHelp)));

        options.addOption(
            Poco::Util::Option("version", "", "Version information")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<AudioDeviceService>(this, &AudioDeviceService::HandleVersion)));
    }

    void HandleHelp(const std::string& name, const std::string& value)
    {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setHeader("Options:");
        helpFormatter.setUsage("[options]");
        helpFormatter.setFooter("\n");
        helpFormatter.format(std::cout);
        stopOptionsProcessing();
        helpRequested_ = true;
    }

    void HandleVersion(const std::string& name, const std::string& value)
    {
        std::cout << "Version " << PRODUCT_VERSION_ATTRIBUTE << "\n";
        stopOptionsProcessing();
        helpRequested_ = true;
    }

    void HandleUrl(const std::string& name, const std::string& value)
    {
        std::cout << "Got Server URL " << value << "\n";
        apiBaseUrl_ = value;
    }

private:
    std::string apiBaseUrl_;
    std::string universalToken_;
    std::string codeSpaceName_;

    bool helpRequested_ = false;

    static constexpr auto API_BASE_URL_PROPERTY_KEY = "custom.apiBaseUrl";
    static constexpr auto UNIVERSAL_TOKEN_PROPERTY_KEY = "custom.universalToken";
    // ReSharper disable once IdentifierTypo
    // ReSharper disable once StringLiteralTypo
    static constexpr auto CODESPACE_NAME_PROPERTY_KEY = "custom.codespaceName";
};

int _tmain(int argc, _TCHAR * argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

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
