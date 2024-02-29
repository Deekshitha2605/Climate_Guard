#include <DHT.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <WiFiClientSecureBearSSL.h>

#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "XXXXXX";     // Enter your WiFi SSID
const char *password = "XXXX";  // Enter Password

const char *connectionString = "HostName=<YourIoTHubName>.azure-devices.net;DeviceId=<YourDeviceId>;SharedAccessKey=<YourSharedAccessKey>";  // Enter the hostname,id and API keys of Azure IoT hub

IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

void setup()
{
    dht.begin();
    Serial.begin(115200);

    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");

    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);

    if (iotHubClientHandle == NULL)
    {
        Serial.println("Failed on IoTHubClient_LL_Create");
        return;
    }

    IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", azure_ca);
}

void loop()
{
    while (IoTHubClient_LL_DoWork(iotHubClientHandle) != IOTHUB_CLIENT_OK)
    {
        delay(100);
    }

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Create a JSON message with temperature and humidity
    String payload = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";

    // Send the message to Azure IoT Hub
    IoTHubMessageHandle messageHandle = IoTHubMessage_CreateFromString(payload.c_str());
    IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, NULL, NULL);

    // Free resources
    IoTHubMessage_Destroy(messageHandle);

    delay(5000); // Send data every 5 seconds (adjust as needed)
}

