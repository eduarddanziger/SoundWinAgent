#ifndef SOUND_AGENT_API_H
#define SOUND_AGENT_API_H

#include <Windows.h>

#ifdef ED_EXPORTS
#define SAA_EXPORT_IMPORT_DECL __declspec(dllexport)
#else
#define AC_EXPORT_IMPORT_DECL __declspec(dllimport)
#endif

#ifdef __cplusplus
    extern "C" {
#endif

    /**
     * @typedef AcHandle
     * @brief Handle type used to identify a specific audio control session.
     */
    typedef DWORD64 SaaHandle;

    typedef INT32 SaaResult;

    typedef struct {
        CHAR Guid[40];
        CHAR Name[128];
        UINT16 Volume;
    } SaaDescription;

    typedef void(__stdcall* TSaaDiscover)(
        _In_  BOOL  attach
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall AcInitialize(
        _Out_ SaaHandle* handle,
        _In_opt_ TSaaDiscover discoverCallback
    );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall AcGetAttached(
            _In_ SaaHandle handle,
            _Out_  SaaDescription* description
        );

    SAA_EXPORT_IMPORT_DECL
        SaaResult __stdcall AcUnInitialize(
            _In_ SaaHandle handle
        );

#ifdef __cplusplus
}
#endif

#endif // SOUND_AGENT_API_H
