#pragma once

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
        CHAR PnPId[90];
        CHAR Name[128];
        UINT16 RenderVolume;
    } SaaDescription;

    typedef void(__stdcall* TSaaDiscover)(
        _In_  BOOL  attach
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaInitialize(
        _Out_ SaaHandle* handle,
        _In_opt_ TSaaDiscover discoverCallback
    );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaGetAttached(
            _In_ SaaHandle handle,
            _Out_  SaaDescription* description
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall SaaUnInitialize(
            _In_ SaaHandle handle
        );

#ifdef __cplusplus
}
#endif
