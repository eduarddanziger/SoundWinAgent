using System.Runtime.InteropServices;

namespace SoundDefaultUI;


[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct SaaDescription
{
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
    public string PnpId;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
    public byte[] Name; // UTF-8 encoded string

    [MarshalAs(UnmanagedType.Bool)]
    public bool IsRender;

    [MarshalAs(UnmanagedType.Bool)]
    public bool IsCapture;

    public ushort RenderVolume;
    public ushort CaptureVolume;
}

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct SaaLogMessage
{
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 12)]
    public string Level;

    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 256)]
    public byte[] Content; // UTF-8 encoded string
}

[UnmanagedFunctionPointer(CallingConvention.StdCall)]
public delegate void SaaDefaultChangedDelegate(
    [MarshalAs(UnmanagedType.Bool)] bool presentOrAbsent
);

[UnmanagedFunctionPointer(CallingConvention.StdCall)]
public delegate void SaaGotLogMessageDelegate(SaaLogMessage message);


internal static class SoundAgentApi
{
#pragma warning disable SYSLIB1054 // Warning for DllImport -> LibraryImport
#pragma warning disable CA2101 // Specify marshaling for P/Invoke string arguments
    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    internal static extern int SaaInitialize(
        out ulong handle,
        SaaGotLogMessageDelegate? gotLogMessageCallback,
        string? appName,
        string? appVersion
#pragma warning restore CA2101
    );

    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    internal static extern int SaaRegisterCallbacks(
        ulong handle,
        SaaDefaultChangedDelegate? defaultRenderChangedCallback,
        SaaDefaultChangedDelegate? defaultCaptureChangedCallback
    );

    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    internal static extern int SaaGetDefaultRender(
        ulong handle,
        out SaaDescription description
    );

    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    internal static extern int SaaGetDefaultCapture(
        ulong handle,
        out SaaDescription description
    );

    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall)]
    internal static extern int SaaUnInitialize(
        ulong handle
    );
}


