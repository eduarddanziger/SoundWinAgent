using NLog;
using System.Reflection;
using System.Text;
using System.Windows.Threading;
using System.Xml.Linq;
using static SoundDefaultUI.SoundAgentApi;
using System.Collections.Generic;
using LogLevel = NLog.LogLevel;

namespace SoundDefaultUI;

public sealed class SoundDeviceService : IDisposable
{
    private static Logger Logger { get; } = LogManager.GetCurrentClassLogger();
    private ulong _serviceHandle;
    private bool _disposed;
    private readonly object _disposeLock = new();

    private static readonly Dictionary<string, LogLevel> SpdLogToNlog =
        new(StringComparer.OrdinalIgnoreCase)
        {
            ["trace"] = LogLevel.Trace,
            ["debug"] = LogLevel.Debug,
            ["info"] = LogLevel.Info,
            ["warn"] = LogLevel.Warn,
            ["warning"] = LogLevel.Warn,
            ["error"] = LogLevel.Error,
            ["critical"] = LogLevel.Fatal,
            ["off"] = LogLevel.Off
        };
    public SoundDeviceService()
    {
        var assembly = typeof(SoundDeviceService).Assembly;
        var assemblyName = assembly.GetName();
        var versionAttribute = assembly.GetCustomAttribute<AssemblyInformationalVersionAttribute>();
#pragma warning disable CA1806
        SaaInitialize(out _serviceHandle, OnLogMessage, assemblyName.Name, versionAttribute?.InformationalVersion);
#pragma warning restore CA1806
    }

    private static void OnLogMessage(SaaLogMessage logMessage)
    {
        var nulIndex = Array.IndexOf(logMessage.Content, (byte)0);
        var length = (nulIndex >= 0) ? nulIndex : logMessage.Content.Length;
        var messageText = Encoding.UTF8.GetString(logMessage.Content, 0, length);

        if (SpdLogToNlog.TryGetValue(logMessage.Level, out var nlogLevel) && nlogLevel != LogLevel.Off)
        {
            Logger.Log(nlogLevel, messageText);
        }
        else
        {
            Logger.Info(messageText);
        }
    }


    public void InitializeAndBind(SaaDefaultChangedDelegate renderNotification, SaaDefaultChangedDelegate captureNotification)
    {
#pragma warning disable CA1806
        SaaRegisterCallbacks(_serviceHandle, renderNotification, captureNotification);
#pragma warning restore CA1806
    }

    private static SoundDeviceInfo EmptyDeviceInfo() =>
        new()
        {
            PnpId = "",
            DeviceName = "",
            IsRenderingAvailable = false,
            IsCapturingAvailable = false,
            RenderVolumeLevel = 0,
            CaptureVolumeLevel = 0
        };

    private static SoundDeviceInfo SaaDescription2SoundDeviceInfo(in SaaDescription device)
    {
        var nameNulIndex = Array.IndexOf(device.Name, (byte)0);
        var nameLength = (nameNulIndex >= 0) ? nameNulIndex : device.Name.Length;

        return new SoundDeviceInfo
        {
            PnpId = device.PnpId,
            DeviceName = Encoding.UTF8.GetString(device.Name, 0, nameLength),
            IsRenderingAvailable = device.IsRender,
            IsCapturingAvailable = device.IsCapture,
            RenderVolumeLevel = device.RenderVolume,
            CaptureVolumeLevel = device.CaptureVolume
        };
    }

    private SoundDeviceInfo GetDevice(Func<ulong, SaaDescription> fetch)
    {
        if (_serviceHandle == 0)
        {
            return EmptyDeviceInfo();
        }
        var dev = fetch(_serviceHandle);
        return SaaDescription2SoundDeviceInfo(dev);
    }

    public SoundDeviceInfo GetRenderDevice()
    {
        return GetDevice(handle =>
        {
#pragma warning disable CA1806
            SaaGetDefaultRender(handle, out var device);
#pragma warning restore CA1806
            return device;
        });
    }

    public SoundDeviceInfo GetCaptureDevice()
    {
        return GetDevice(handle =>
        {
#pragma warning disable CA1806
            SaaGetDefaultCapture(handle, out var device);
#pragma warning restore CA1806
            return device;
        });
    }

    ~SoundDeviceService()
    {
        Dispose(false);
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    private void Dispose(bool disposing)
    {
        ulong handleToRelease = 0;
        lock (_disposeLock)
        {
            if (_disposed) return;
            _disposed = true;
            handleToRelease = _serviceHandle;
            _serviceHandle = 0;
        }

        if (handleToRelease != 0)
        {
#pragma warning disable CA1806
            SaaUnInitialize(handleToRelease);
#pragma warning restore CA1806
        }
    }
}