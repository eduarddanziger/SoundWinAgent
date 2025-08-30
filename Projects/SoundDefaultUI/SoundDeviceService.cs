using NLog;
using System.Reflection;
using System.Text;
using System.Windows.Threading;
using System.Xml.Linq;
using static SoundDefaultUI.SoundAgentApi;

namespace SoundDefaultUI;

public sealed class SoundDeviceService : IDisposable
{
    private static Logger _logger = LogManager.GetCurrentClassLogger();
    private ulong _serviceHandle;
    private bool _disposed;
    private readonly object _disposeLock = new();

    

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
        var messageText = Encoding.UTF8.GetString(logMessage.Content).TrimEnd('\0');
        _logger.Info(messageText);
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

    private static SoundDeviceInfo SaaDescription2SoundDeviceInfo(in SaaDescription device) =>
        new()
        {
            PnpId = device.PnpId,
            DeviceName = Encoding.UTF8.GetString(device.Name).TrimEnd('\0'),
            IsRenderingAvailable = device.IsRender,
            IsCapturingAvailable = device.IsCapture,
            RenderVolumeLevel = device.RenderVolume,
            CaptureVolumeLevel = device.CaptureVolume
        };

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