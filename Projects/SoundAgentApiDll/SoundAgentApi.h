#ifndef SOUND_AGENT_API_H
#define SOUND_AGENT_API_H

#include <Windows.h>

#ifdef ED_EXPORTS
#define SAA_EXPORT_IMPORT_DECL __declspec(dllexport)
#else
#define SAA_EXPORT_IMPORT_DECL __declspec(dllimport)
#endif

#ifdef __cplusplus
    extern "C" {
#endif

    
    typedef DWORD64 SaaHandle;

    typedef INT32 SaaResult;

    typedef struct {
        CHAR PnpId[80];
        CHAR Name[128];
        BOOL IsRender;
        BOOL IsCapture;
        UINT16 RenderVolume;
        UINT16 CaptureVolume;
    } SaaDescription;

    typedef void(__stdcall* TSaaDefaultChangedCallback)(
        _In_ BOOL presentOrAbsent
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaInitialize(
            _Out_ SaaHandle* handle
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaRegisterCallbacks(
            _In_ SaaHandle handle,
            _In_opt_ TSaaDefaultChangedCallback defaultRenderChangedCallback,
            _In_opt_ TSaaDefaultChangedCallback defaultCaptureChangedCallback
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaGetDefaultRender(
            _In_ SaaHandle handle,
            _Out_ SaaDescription* description
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaGetDefaultCapture(
            _In_ SaaHandle handle,
            _Out_ SaaDescription* description
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaUnInitialize(
            _In_ SaaHandle handle
        );

#ifdef __cplusplus
}
#endif

#endif // SOUND_AGENT_API_H
