// Libraries
#include <HTTPClient.h>
#include <DHT.h>
#include <string.h>

// Define UART pins
#define RX_PIN 16
#define TX_PIN 17

// Sensor(s) pin(s)
#define DHT_PIN  18
#define DHT_TYPE DHT11
#define LDR_PIN 39

// Sensor(s) address
#define TEMP_ADD 0x61
#define HUMI_ADD 0x62
#define LIGHT_ADD 0x63

// Timer
unsigned long lastTime = 0;
unsigned long timerDelay = 2000;

// ThinkSpeak
String endpoint_api = "https://api.thingspeak.com/update?api_key=U7E583HZWDU0CZYR"; // YOUR_API_KEY = ThinkSpeak API

// Wifi
const char* wifi_ssid = "phone"; // Wifi ssid
const char* wifi_pass = "123456788"; // Wifi pass


// Init lib(s)

DHT dht(DHT_PIN, DHT_TYPE);
HardwareSerial dwin(1);

unsigned char Buffer[9];
unsigned char Temperature[8] = {0x5a, 0xa5, 0x05, 0x82, TEMP_ADD , 0x00, 0x00, 0x00};
unsigned char Humidity[8] = {0x5a, 0xa5, 0x05, 0x82, HUMI_ADD, 0x00, 0x00, 0x00};
unsigned char Light[8] = {0x5a, 0xa5, 0x05, 0x82, LIGHT_ADD , 0x00, 0x00, 0x00};

void setup()
{
  Serial.begin(115200);
  // Begin serial communication DWIN
  dwin.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  wifi_setup();
  dht.begin();
}


void loop()
{
  handle_sensors_data();
}

void wifi_setup() {
    WiFi.begin(wifi_ssid, wifi_pass);
    Serial.print("Connecting to ");
    Serial.print(wifi_ssid);
    while(WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
    }
  Serial.print("Connected to Wifi with IP address: ");
  Serial.println(WiFi.localIP());
  }

void handle_sensors_data() {
        int t = dht.readTemperature();
        int h = dht.readHumidity();
        int l = map(analogRead(LDR_PIN), 400, 4200, 0, 100);

        if (isnan(t) || isnan(h) || isnan(l)) {
            Serial.println("Failed to read sensor(s) data");
        }
        else
        {
            // Print data to serial monitor
            Serial.println();
            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.println("oC");
            Serial.print("Humidity: ");
            Serial.print(h);
            Serial.println("%");
            Serial.print("Light: ");
            Serial.print(l);
            Serial.println("lux");

            // Push data to monitor
            push_monitor(t,h,l);

            // Push data to ThinkSpeak
            push_thinkspeak(t, h, l);
    }
}

void push_thinkspeak (int t, int h, int l) {
  delay(2000);
  char params[100];
  sprintf(params, "&field1=%d&field2=%d&field3=%d", t,h,l);
  String update_url = endpoint_api + String(params);
  send_request(update_url.c_str());

}

void send_request(const char* url) {
    HTTPClient http;
    http.begin(url);
    int response_code = http.GET();
    String response_body = "{}";
    if (response_code == 200) {
        Serial.print("Pushed data to ThinkSpeak: ");
      }
      else {
          Serial.print("Failed to push data to ThinkSpeak: ");
    }
    http.end();
}

void push_monitor (int t, int h, int l) {
    Temperature[6] = highByte(t);
    Temperature[7] = lowByte(t);
    dwin.write(Temperature, 8);

    Humidity[6] = highByte(h);
    Humidity[7] = lowByte(h);
    dwin.write(Humidity, 8);

    Light[6] = highByte(l);
    Light[7] = lowByte(l);
    dwin.write(Light, 8);
}
