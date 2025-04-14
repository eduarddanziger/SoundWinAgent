#pragma once

#include <public/SoundAgentInterface.h>


class FormattedOutput final
{
public:
    ~FormattedOutput() = delete;
protected:
	DISALLOW_IMPLICIT_CONSTRUCTORS(FormattedOutput);
public:
    static void LogAndPrint(const std::wstring& mess);
    static void LogAndPrint(const std::string& mess);
    static void LogAsErrorPrintAndThrow(const std::string& mess);

    static void PrintEvent(SoundDeviceEventType event, const std::wstring& devicePnpId);

    static void PrintDeviceInfo(const SoundDeviceInterface* device);

    static std::string WString2StringTruncate(const std::wstring& str);
    static std::wostream & CurrentLocalTimeWithoutDate(std::wostream & os);

    static std::ostream & CurrentLocalTimeWithoutDate(std::ostream & os);
};
