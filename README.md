# ESP32 MQTT Sensor System

This project is an ESP32-based system that reads environmental data from sensors and publishes it to an MQTT broker. It is designed for use with a local Mosquitto MQTT broker and supports Wi-Fi connectivity, NTP time synchronization, and periodic reporting.

## Features
- Connects to Wi-Fi using provided credentials
- Reads temperature and humidity from a DHT22 sensor
- Reads air quality from an MQ-135 sensor
- Publishes sensor data to a local MQTT broker at regular intervals
- Tracks and reports connection statistics and message delivery success rates
- Uses NTP to synchronize time for timestamping data

## Hardware Requirements
- ESP32 microcontroller
- DHT22 temperature and humidity sensor (connected to GPIO 33)
- MQ-135 air quality sensor (connected to GPIO 35)

## Software Requirements
- Arduino IDE or PlatformIO
- Required libraries:
  - WiFi.h
  - PubSubClient.h
  - DHT.h
  - time.h
- Mosquitto MQTT broker running locally (default IP: 192.168.1.88, port: 1883)

## Configuration
Edit the following variables in `system_mqtt.ino`:
- `ssid` and `password`: Your Wi-Fi credentials
- `mqtt_server`: IP address of your MQTT broker
- `mqtt_port`: Port of your MQTT broker (default: 1883)

## Usage
1. Connect the sensors to the ESP32 as described above.
2. Flash the code to your ESP32.
3. Ensure your MQTT broker is running and accessible.
4. Open the Serial Monitor to view connection status, sensor readings, and statistics.

## Data Format
Published data is sent to the topic `channels/id/publish` in the following format:
```
field1=<temperature>&field2=<humidity>&field3=<airQuality>&field4=<sequence_number>&field5=<successful_messages>&field6=<failed_messages>
```

## Statistics
The system prints statistics to the Serial Monitor after each data transmission, including:
- Wi-Fi reconnections
- MQTT reconnections
- Total messages sent
- Failed messages
- Success rate
- ESP32 IP address

## License
This project is provided as-is for educational and prototyping purposes.
