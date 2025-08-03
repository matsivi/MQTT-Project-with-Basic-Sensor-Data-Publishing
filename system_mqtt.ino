#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>

// Wi-Fi Credentials
const char* ssid = "";
const char* password = "";

// Local MQTT Broker (Mosquitto) for Bridge to Bridge
// If you use Mosquitto, make sure you have installed and configured the Mosquitto broker on your computer with the correct settings.
const char* mqtt_server = "";
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_Client";
const char* publish_topic = "channels/id/publish";


// DHT Sensor
#define DHTPIN 33
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MQ-135 Sensor
#define MQ135_PIN 35

WiFiClient espClient;
PubSubClient client(espClient);

// Sequence number
static int sequence_number = 1;

// Counters for tracking reconnections
int wifi_reconnect_count = 0;
int mqtt_reconnect_count = 0;
int total_messages = 0;
int failed_messages = 0;

// NTP Server details
const char* ntp_server = "pool.ntp.org";
const long gmt_offset_sec = 2 * 3600; 
const int daylight_offset_sec = 3600;

// Timing variables
const unsigned long MAX_RUNTIME = 10 * 60 * 1000; // 10 minutes in milliseconds
unsigned long start_time;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    wifi_reconnect_count++;
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
}

void reconnectMQTT() {
  client.setKeepAlive(60); 
  while (!client.connected()) {
    Serial.println("Attempting to connect to local MQTT broker...");
    if (client.connect(mqtt_client_id)) { 
      Serial.println("Successfully connected to local MQTT broker!");
    } else {
      mqtt_reconnect_count++;
      Serial.print("Connection failed. Error code: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

String getTimestamp() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

  return String(buffer);
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);
  Serial.println("Process starting...");

  // Connect to Wi-Fi
  connectToWiFi();

  // Configure MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Initialize DHT Sensor
  dht.begin();
  Serial.println("DHT22 activated.");

  // Initialize NTP
  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
  Serial.println("NTP Server configured.");

  // Record the start time
  start_time = millis();
}

void loop() {
  // Check if the runtime has exceeded the maximum allowed time
  unsigned long current_time = millis();
  if (current_time - start_time > MAX_RUNTIME) {
    Serial.println("Time limit exceeded. Stopping data transmission.");
    while (true) {
      delay(1000);
    }
  }

  // Verify Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost...");
    connectToWiFi();
  }

  // Verify MQTT connection
  if (!client.connected()) {
    Serial.println("MQTT connection lost...");
    mqtt_reconnect_count++;
    reconnectMQTT();
  }
  client.loop();

  // Read sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int airQuality = analogRead(MQ135_PIN);

  if (!isnan(humidity) && !isnan(temperature)) {
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Air Quality: ");
    Serial.println(airQuality);

    // Get the current timestamp
    String timestamp = getTimestamp();

    // Create payload with timestamp
    String payload = "field1=" + String(temperature) +
                     "&field2=" + String(humidity) +
                     "&field3=" + String(airQuality) +
                     "&field4=" + String(sequence_number++) +
                     "&field5=" + String(total_messages - failed_messages) +
                     "&field6=" + String(failed_messages);

    // Publish to Local MQTT Broker
    total_messages++;
    if (client.publish(publish_topic, payload.c_str())) {
      Serial.println("Data sent successfully to local broker!");
      Serial.print("Timestamp: ");
      Serial.println(timestamp);
    } else {
      failed_messages++;
      Serial.println("Failed to send data to local broker.");
    }
  } else {
    Serial.println("Sensor read error.");
  }

  // Calculate success/failure rates
  float success_rate = (total_messages > 0) ? 
                       (100.0 * (total_messages - failed_messages) / total_messages) : 0.0;
  float failure_rate = 100.0 - success_rate;

  // Show statistics report
  Serial.println("\n--- Statistics Report ---");
  Serial.printf("WiFi Reconnections: %d\n", wifi_reconnect_count);
  Serial.printf("MQTT Reconnections: %d\n", mqtt_reconnect_count);
  Serial.printf("Total messages: %d\n", total_messages);
  Serial.printf("Failed messages: %d (%.2f%%)\n", failed_messages, failure_rate);
  Serial.printf("Success Rate: %.2f%%\n", success_rate);
  Serial.println("---------------------------");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Wait before next measurement
  Serial.println("Waiting for next measurement...");
  delay(20000);  // Wait for 20 seconds
}
