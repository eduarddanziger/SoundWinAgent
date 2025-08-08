using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using System.Text;
using System.Text.Json;
using HttpRequestProcessor;

var builder = Host.CreateDefaultBuilder(args)
    // .UseWindowsService(options =>
    // {
    //     options.ServiceName = "RabbitMQ Message Processor";
    // })
    .ConfigureServices((context, services) =>
    {
        var config = context.Configuration;
        services.Configure<RabbitMqSettings>(config.GetSection("RabbitMQ"));
        services.Configure<ApiBaseUrlSettings>(config.GetSection("ApiBaseUrl"));

        services.AddSingleton<IConnectionFactory>(sp =>
            new ConnectionFactory()
            {
                HostName = config["RabbitMQ:HostName"],
                UserName = config["RabbitMQ:UserName"],
                Password = config["RabbitMQ:Password"]
            });

        // Add hosted service
        services.AddHostedService<RabbitMqConsumerService>();
    });

await builder.RunConsoleAsync();



// Configuration classes
#pragma warning disable CA1050
public record RabbitMqSettings
#pragma warning restore CA1050
{
    public string HostName { get; init; } = string.Empty;
    public string QueueName { get; init; } = string.Empty;
}
#pragma warning disable CA1050
public record ApiBaseUrlSettings
#pragma warning restore CA1050
{
    public string Codespace { get; init; } = string.Empty;
    public string Azure { get; init; } = string.Empty;

}
