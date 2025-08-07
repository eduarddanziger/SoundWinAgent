using System.Runtime.InteropServices;

namespace SoundDefaultUI;


[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct SaaDescription
{
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
    public string PnpId;

    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
    public string Name;

    [MarshalAs(UnmanagedType.Bool)]
    public bool IsRender;
    [MarshalAs(UnmanagedType.Bool)]
    public bool IsCapture;

    public ushort RenderVolume;
    public ushort CaptureVolume;
}

[UnmanagedFunctionPointer(CallingConvention.StdCall)]
public delegate void TSaaDefaultRenderChangedDelegate(
    [MarshalAs(UnmanagedType.Bool)] bool presentOrAbsent
);

public static class SoundAgentApi
{
    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    public static extern int SaaInitialize(
        out ulong handle,
        TSaaDefaultRenderChangedDelegate? defaultRenderChangedCallback
    );

    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    public static extern int SaaGetDefaultRender(
        ulong handle,
        out SaaDescription description
    );

    [DllImport("SoundAgentApiDll.dll", CallingConvention = CallingConvention.StdCall)]
    public static extern int SaaUnInitialize(
        ulong handle
    );
}


