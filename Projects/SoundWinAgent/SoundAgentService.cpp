#include "os-dependencies.h"

#include "ApiClient/common/SpdLogger.h"

#include "ApiClient/SodiumCrypt.h"
#include "ApiClient/DirectHttpRequestDispatcher.h"
#include "ApiClient/RabbitMqHttpRequestDispatcher.h"
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
#include <Poco/String.h>

#include <spdlog/spdlog.h>


class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        if (onlyConsoleOutputRequested_)
        {
            return Application::EXIT_OK;
        }

        try {
            spdlog::info("Starting Sound Agent...");

            const auto coll(SoundAgent::CreateDeviceCollection());

            std::unique_ptr<HttpRequestDispatcherInterface> requestDispatcherSmartPtr;
            if (Poco::icompare(transportMethod_, API_TRANSPORT_METHOD_VALUE00_NONE) == 0)
            {
                class EmptyDispatcher : public HttpRequestDispatcherInterface
                {
                public:
                    void EnqueueRequest(bool, const std::chrono::system_clock::time_point&,
                                        const std::string&, const std::string&,
                                        const std::unordered_map<std::string, std::string>&, const std::string&
                    ) override
                    {
                        spdlog::info("Request not sent, because the transport method is \"{}\"",
                                     API_TRANSPORT_METHOD_VALUE00_NONE);
                    }
                };
                requestDispatcherSmartPtr.reset(new EmptyDispatcher());
            }
            else if (Poco::icompare(transportMethod_, API_TRANSPORT_METHOD_VALUE01_DIRECT) == 0)
            {
                requestDispatcherSmartPtr.reset(new DirectHttpRequestDispatcher(apiBaseUrl_, universalToken_, codeSpaceName_));
            }
            else if (Poco::icompare(transportMethod_, API_TRANSPORT_METHOD_VALUE02_RABBITMQ) == 0)
            {
                requestDispatcherSmartPtr.reset(new RabbitMqHttpRequestDispatcher());
            }

            ServiceObserver serviceObserver(*coll, *requestDispatcherSmartPtr);
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

    [[nodiscard]] std::string ReadOptionalSimpleConfigProperty(const std::string& propertyName,
                                                               const std::string& defaultValue = "") const
    {
        if (!config().hasProperty(propertyName))
        {
            spdlog::info("Property \"{}\" not found in configuration. Using default value: \"{}\".", propertyName, defaultValue);
            return defaultValue;
        }

        auto returnValue = config().getString(propertyName);
        if (returnValue.empty())
        {
            spdlog::info("Property \"{}\" is configured but empty. Using default value: \"{}\".", propertyName, defaultValue);
            return defaultValue;
        }

        return returnValue;
    }


    [[nodiscard]] std::string ReadMandatoryPossiblyEncryptedConfigProperty(const std::string & propertyName) const
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

    static void FreeLog()
    {
        ed::model::Logger::Inst().Free();
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

    void initialize(Application& self) override
	{
        loadConfiguration();
        ServerApplication::initialize(self);

        if (onlyConsoleOutputRequested_)
        {
            return;
        }

        SetUpLog();

        if (apiBaseUrl_.empty())
        {   // If no URL is provided via command line, read it from the configuration
            apiBaseUrl_ = ReadMandatoryPossiblyEncryptedConfigProperty(API_BASE_URL_PROPERTY_KEY);
        }
        apiBaseUrl_ += "/api/AudioDevices";

        if (transportMethod_.empty())
        {   // If no transport method is provided via command line, read it from the configuration
            spdlog::info("Transport method not provided via command line. Reading from configuration...");
            transportMethod_ = ReadOptionalSimpleConfigProperty(API_TRANSPORT_METHOD_PROPERTY_KEY, API_TRANSPORT_METHOD_VALUE00_NONE);
        }
        if (Poco::icompare(transportMethod_, API_TRANSPORT_METHOD_VALUE00_NONE) != 0
            && Poco::icompare(transportMethod_, API_TRANSPORT_METHOD_VALUE01_DIRECT) != 0
            && Poco::icompare(transportMethod_, API_TRANSPORT_METHOD_VALUE02_RABBITMQ) != 0
        )
        {
            spdlog::info(R"(Invalid transport method "{}". Using default: "{}".)", transportMethod_, API_TRANSPORT_METHOD_VALUE00_NONE);
            transportMethod_ = API_TRANSPORT_METHOD_VALUE00_NONE;
        }
        else
        {
            spdlog::info(R"(Transport method value "{}" validated.)", transportMethod_);
        }

        universalToken_ = ReadMandatoryPossiblyEncryptedConfigProperty(UNIVERSAL_TOKEN_PROPERTY_KEY);

        codeSpaceName_ = ReadMandatoryPossiblyEncryptedConfigProperty(CODESPACE_NAME_PROPERTY_KEY);

        setUnixOptions(false);  // Force Windows service behavior
    }

    void uninitialize() override
	{
        FreeLog();
        ServerApplication::uninitialize();
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
            Poco::Util::Option("transport", "", "Transport method: None, Direct or RabbitMQ")
            .required(false)
            .repeatable(false)
            .argument("<transport>", true)
            .callback(Poco::Util::OptionCallback<AudioDeviceService>(this, &AudioDeviceService::HandleTransport)));

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
        onlyConsoleOutputRequested_ = true;
    }

    void HandleVersion(const std::string& name, const std::string& value)
    {
        std::cout << "Version " << PRODUCT_VERSION_ATTRIBUTE << "\n";
        stopOptionsProcessing();
        onlyConsoleOutputRequested_ = true;
    }

    void HandleUrl(const std::string& name, const std::string& value)
    {
        std::cout << fmt::format(R"(Got Server URL "{}"
)", value);
        apiBaseUrl_ = value;
    }

    void HandleTransport(const std::string& name, const std::string& value)
    {
        std::cout << fmt::format(R"(Got Transport Method "{}"
)", value);
        transportMethod_ = value;
    }

private:
    std::string apiBaseUrl_;
    std::string universalToken_;
    std::string codeSpaceName_;
    std::string transportMethod_;

    bool onlyConsoleOutputRequested_ = false;

    static constexpr auto API_BASE_URL_PROPERTY_KEY = "custom.apiBaseUrl";
    static constexpr auto UNIVERSAL_TOKEN_PROPERTY_KEY = "custom.universalToken";
    // ReSharper disable once IdentifierTypo
    // ReSharper disable once StringLiteralTypo
    static constexpr auto CODESPACE_NAME_PROPERTY_KEY = "custom.codespaceName";
    static constexpr auto API_TRANSPORT_METHOD_PROPERTY_KEY = "custom.transportMethod";
    static constexpr auto API_TRANSPORT_METHOD_VALUE00_NONE = "None";
    static constexpr auto API_TRANSPORT_METHOD_VALUE01_DIRECT = "Direct";
    static constexpr auto API_TRANSPORT_METHOD_VALUE02_RABBITMQ = "RabbitMQ";
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
