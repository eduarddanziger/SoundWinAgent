#include "stdafx.h"

#include <cstdlib>

#include <queue>

#include <CppUnitTest.h>

#include "ApiClient/common/SpdLogger.h"

#include "SoundDeviceCollection.h"


using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed::audio {
    TEST_CLASS(SoundDeviceCollectionTests) {
#if 0
//#ifdef _DEBUG
private:
    _CrtMemState sOld_ = {};
    _CrtMemState sNew_ = {};
    _CrtMemState sDiff_ = {};
public:

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    static int MyReportHook(int reportType, wchar_t* message, int* returnValue)
        {
            if (message) {
                Logger::WriteMessage(message);
            }
            return 0;
        
        }

        TEST_METHOD_INITIALIZE(MyInit)
        {
            model::Logger::Inst().SetOutputToConsole(true);

            _CrtMemCheckpoint(&sOld_); //take a snapshot
        }

        TEST_METHOD_CLEANUP(CleanUp)
        {
            _CrtMemCheckpoint(&sNew_);
            _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, MyReportHook);

            model::Logger::Inst().Free();

            if (_CrtMemDifference(&sDiff_, &sOld_, &sNew_)) // if there is a difference
            {
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n----------- _CrtMemDumpStatistics ---------\n");
                _CrtMemDumpStatistics(&sDiff_);
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n----------- _CrtMemDumpAllObjectsSince ---------\n");
                _CrtMemDumpAllObjectsSince(&sOld_);
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n----------- _CrtDumpMemoryLeaks ---------\n");
                _CrtDumpMemoryLeaks();
                Assert::IsTrue(false, L"Memory leak detected");
            }
            else
            {
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n-----------No memory leaks found ---------\n");
            }
        }
            

        TEST_METHOD(CtorMemoryLeakTest)
        {
            SoundDeviceCollection devColl();

            // char* s = new char[17];
            // strcpy_s(s, 17, "allocate_no_free");
            // char* ss = new char[14];
            // strcpy_s(ss, 14, "stackoverflow");
            // delete[] ss;
        }
        TEST_METHOD(ResetContentMemoryLeakTest)
        {
            SoundDeviceCollection devColl();
            devColl.ResetContent();
        }
#endif
    };
}
