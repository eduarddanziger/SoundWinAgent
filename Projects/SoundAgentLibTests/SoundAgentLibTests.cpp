#include "stdafx.h"

#include <queue>

#include <CppUnitTest.h>

#include "public/SoundAgentInterface.h"
#include "SoundDevice.h"
#include "public/generate-uuid.h"

using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed::audio
{
    TEST_CLASS(SoundAgentLibTests)
    {
        // TEST_METHOD(CollectionEmpty)
        // {
        //     SoundAgent ac;
        //     const std::unique_ptr<SoundDeviceCollectionInterface> coll(ac.CreateDeviceCollection(L"Nothing At All"));
        //     Assert::IsTrue(coll->GetSize() == 0);
        // }

        TEST_METHOD(DeviceCtorTest)
        {
            const auto nameExpected = "name01"s;
            const auto pnpIdExpected = generate_uuid();

            const SoundDevice dv(pnpIdExpected, nameExpected, SoundDeviceFlowType::Capture, 0, 200,false,false);

            Assert::AreEqual(nameExpected, dv.GetName());
            Assert::AreEqual(pnpIdExpected, dv.GetPnpId());
        }
    };
}
