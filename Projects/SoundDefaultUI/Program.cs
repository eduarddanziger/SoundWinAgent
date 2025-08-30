using NLog;
using NLog.Extensions.Logging;
using SoundDefaultUI;
using System.Runtime.Versioning;


[assembly: SupportedOSPlatform("Windows7.0")]

IConfiguration config = new ConfigurationBuilder()
    .AddEnvironmentVariables()
    .AddJsonFile("appsettings.json", false, true)
    .AddCommandLine(args)
    .Build();

LogManager.Setup()
    .SetupExtensions(ext => ext.RegisterLayoutRenderer<SoundDefaultUI.NativeThreadIdLayoutRenderer>("native-thread-id"));

LogManager.Configuration = new NLogLoggingConfiguration(config.GetSection("NLog"));


// Create a builder by specifying the application and main window.
var builder = WpfApplication<App, MainWindow>.CreateBuilder(args);
builder.Services.AddSingleton<SoundDeviceService>();
builder.Services.AddTransient<MainViewModel>();
//builder.Services.AddSingleton(config);

// Build and run the application.
var app = builder.Build();

await app.RunAsync();
