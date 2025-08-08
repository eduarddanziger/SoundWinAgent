namespace HttpRequestProcessor;

using Microsoft.Extensions.Options;
using RabbitMQ.Client;
using RabbitMQ.Client.Events;
using System.Text;
using System.Text.Json;

public class RabbitMqConsumerService(
    IConnectionFactory connectionFactory,
    IOptions<RabbitMqSettings> rabbitSettings,
    IOptions<ApiBaseUrlSettings> apiSettings,
    ILogger<RabbitMqConsumerService> logger)
    : BackgroundService
{
    private readonly string _queueName = rabbitSettings.Value.QueueName;
    private readonly string _apiEndpoint = apiSettings.Value.Azure; //! Azure, not Codespace

    private IConnection? _connection;
    private IChannel? _channel;

    protected override async Task ExecuteAsync(CancellationToken cancellationToken)
    {
        _connection = await connectionFactory.CreateConnectionAsync(cancellationToken);
        _channel = await _connection.CreateChannelAsync(null, cancellationToken);

        await _channel.QueueDeclareAsync(
            queue: _queueName,
            durable: true,
            exclusive: false,
            autoDelete: false,
            cancellationToken: cancellationToken);

        var consumer = new AsyncEventingBasicConsumer(_channel);
        consumer.ReceivedAsync += async (model, ea) =>
        {
            try
            {
                var body = ea.Body.ToArray();
                var message = JsonDocument.Parse(body);
                var httpRequest = message.RootElement.GetProperty("httpRequest").GetString();
                var urlSuffix = message.RootElement.GetProperty("urlSuffix").GetString();

                using var httpClient = new HttpClient();
                var response = httpRequest?.ToUpper() == "PUT"
                    ? await httpClient.PutAsync(_apiEndpoint + urlSuffix, new StringContent(JsonSerializer.Serialize(message)),
                        cancellationToken)
                    : await httpClient.PostAsync(_apiEndpoint + urlSuffix, new StringContent(JsonSerializer.Serialize(message)),
                        cancellationToken);

                if (response.IsSuccessStatusCode)
                {
                    await _channel.BasicAckAsync(ea.DeliveryTag, false, cancellationToken);
                    logger.LogInformation("Processed message with {Method}", httpRequest);
                }
                else
                {
                    await _channel.BasicNackAsync(ea.DeliveryTag, false, true, cancellationToken);
                    logger.LogWarning("API rejected message. Status: {Status}", response.StatusCode);
                }
            }
            catch (Exception ex)
            {   
                logger.LogError(ex, "Message processing failed");
                if (_channel != null)
                    await _channel.BasicNackAsync(ea.DeliveryTag, false, false, cancellationToken);
            }
        };

        await _channel.BasicConsumeAsync(
            queue: _queueName,
            autoAck: false,
            consumer: consumer,
            cancellationToken: cancellationToken);

        await Task.Delay(Timeout.Infinite, cancellationToken);
    }

    public override async Task StopAsync(CancellationToken cancellationToken)
    {
        _channel?.Dispose();
        _connection?.Dispose();
        await base.StopAsync(cancellationToken);
    }
}

