#include "stdafx.h"

#include <queue>

#include <CppUnitTest.h>

#include <public/SoundAgentInterface.h>
#include "SoundDevice.h"
#include <public/generate-uuid.h>
#include "CaseInsensitiveSubstr.h"

using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed::audio {
TEST_CLASS(SoundAgentLibTests) {
    // TEST_METHOD(CollectionEmpty)
    // {
    //     SoundAgent ac;
    //     const std::unique_ptr<SoundDeviceCollectionInterface> coll(ac.CreateDeviceCollection(L"Nothing At All"));
    //     Assert::IsTrue(coll->GetSize() == 0);
    // }

    TEST_METHOD(DeviceCtorTest)
    {
        const auto nameExpected = L"name01"s;
        const auto pnpIdExpected = generate_w_uuid();

        const SoundDevice dv(pnpIdExpected, nameExpected, SoundDeviceFlowType::Capture, 0, 200);

        Assert::AreEqual(nameExpected, dv.GetName());
        Assert::AreEqual(pnpIdExpected, dv.GetPnpId());
    }

    TEST_METHOD(FindSubstrCaseInsensitiveTest)
    {
        const auto substr01 = L"name01"s;
        const auto substr02 = L"nAmE01"s;
        const auto string01 = L"uu name01mm"s;

        Assert::IsTrue(FindSubstrCaseInsensitive(string01, substr01));
        Assert::IsTrue(FindSubstrCaseInsensitive(string01, substr02));
        Assert::IsTrue(FindSubstrCaseInsensitive(string01, L""s));

        Assert::IsFalse(FindSubstrCaseInsensitive(string01, substr02 + L"2"));
    }
};
}
