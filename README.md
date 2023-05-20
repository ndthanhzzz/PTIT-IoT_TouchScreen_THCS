# ESP32 - Dwin

## Lưu ý trong báo cáo

- Cần hàn ở mặt sau màn hình để bật chế độ TTL
    ![image](https://github.com/ndthanhzzz/IoT_TouchScreen_THCS/assets/101911807/834761ff-9e62-4dca-ac4a-2ac294f0e03a)

    Hàn 2 điểm để bật giao thức TTL - mặc định màn chạy với giao thức RS232

## Lưu ý khi nối dây:

- Nối chung đất 2 x ESP32 với màn để các thiết bị được đồng bộ khi dùng giao thức UART
- **ESP32 - Xử lí sensors**
    
    ```c
    // Libraries
    #include <HTTPClient.h>
    #include <DHT.h>
    #include <string.h>
    
    // Define UART pin(s)
    #define RX_PIN 16
    #define TX_PIN 17
    
    // Sensor(s) pin(s)
    #define DHT_PIN  18
    #define DHT_TYPE DHT11
    #define LDR_PIN 19    // Replace with other pin
    
    // Sensor(s) address
    #define TEMP_ADD 0x61
    #define HUMI_ADD 0x62
    #define LIGHT_ADD 0x63
    
    // Timer
    unsigned long lastTime = 0;
    unsigned long timerDelay = 2000;
    
    // ThinkSpeak
    String endpoint_api = "https://api.thingspeak.com/update?api_key=YOUR_API_KEY"; // YOUR_API_KEY = ThinkSpeak Write API
    
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
            Serial.print("Pushed data to ThinkSpeak!");
          }
          else {
              Serial.print("Failed to push data to ThinkSpeak!");
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
    ```
    
    Sơ đồ chân
    
    | ESP32 | Màn hình/cảm biến | Ghi chú |
    | --- | --- | --- |
    | 17 (TX2) | RX2 - Màn hình | Truyền dữ liệu thu được từ cảm biến lên màn hình |
    | 18 | Data (DHT11) | Nối với chân data của cảm biến DHT11 |
    | 19 | Quang trở | Nối với quang trở như demo cũ |
    
- **ESP32 - Xử lí thiết bị**
    
    ```c
    #define RX_PIN 16
    #define TX_PIN 17
    
    // Device(s) pin(s)
    #define LIGHT 19
    #define FAN 21
    #define PUMP 22
    #define AC 23
    
    HardwareSerial dwin(1);
    
    unsigned char Buffer[9];
     
    void setup()
    {
      Serial.begin(115200);
      // Begin serial communication DWIN
      dwin.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    
      // Setup devices 
      pinMode(LIGHT, OUTPUT);
      pinMode(FAN, OUTPUT);
      pinMode(PUMP, OUTPUT);
      pinMode(AC, OUTPUT);
    
      // Reset state 
      digitalWrite(LIGHT, LOW);
      digitalWrite(FAN, LOW);
      digitalWrite(PUMP, LOW);
      digitalWrite(AC, LOW);
    }
     
    void loop()
    {
      devices_control();
    }
    
    void devices_control () {
        if (dwin.available()) {
            for (int i = 0; i<= 8; i++) {
                Buffer[i] = dwin.read();
              }
            if (Buffer[0] == 0x5A) {
                switch (Buffer[4]) {
                    case 0x65: // light
                      if (Buffer[8] == 1) {
                          digitalWrite(LIGHT, HIGH);
                          Serial.println("Light status: ON");
                        }
                        else {
                            digitalWrite(LIGHT, LOW);
                            Serial.println("Light status: OFF");
                          }
                          break;
    	                case 0x66: // Fan
                        if (Buffer[8] == 1) {
                            digitalWrite(FAN, HIGH);
                            Serial.println("Fan status: ON");
    
                          }
                          else {
                              digitalWrite(FAN, LOW);
                              Serial.println("Fan status: OFF");
                            }
                            break;
    	                case 0x67: // Pump
                        if (Buffer[8] == 1) {
                            digitalWrite(PUMP, HIGH);
                            Serial.println("Pump status: ON");
                        }
                          else {
                              digitalWrite(PUMP, LOW);
                              Serial.println("Pump status: OFF");
                          }
                          break;
    	                case 0x68: // Air Conditional
                        if (Buffer[8] == 1) {
                            digitalWrite(AC, HIGH);
                            Serial.println("AC status: ON");
                          }
                          else {
                              digitalWrite(AC, LOW);
                              Serial.println("AC status: OFF");
                          }
                          break;
                    default:
                        Serial.println("No input data...");
                  }
              }  
          }
      }
    ```
    
    Sơ đồ chân
    
    | ESP32 | Màn hình/relay | Ghi chú |
    | --- | --- | --- |
    | 16 (RX2) | Nối với chân TX2 trên màn hình | Nhận dữ liệu từ các công tắc trên màn hình |
    | 19 | Nối với relay | Điều khiển relay |
    | 21 | Nối với relay | Điều khiển relay |
    | 22 | Nối với relay | Điều khiển relay |
    | 23 | Nối với relay | Điều khiển relay |
- **Cấu hình ThinkSpeak**
    1. Đăng kí tài khoản [ThinkSpeak](https://thingspeak.com/)
    2. Tạo channel
 
        Nhấn `New Channel` để tạo channel mới
        
    3. Thiết lập Channel với 3 field: `field1`, `field2`, `field3` lần lượt ứng với nhiệt độ, độ ẩm, ánh sáng, rồi nhấn `Save Channel`       
    4. Tại tab `API Keys` copy **`Write API Key`** thu được vào code ESP32 - Xử lí cảm biến (thay thế `YOUR_API_KEY` bằng API key vừa thu được
    5. Có thể chỉnh sửa trục x, y của chart ThinkSpeak, thay thế bằng đơn vị phù hợp (không cần thiết với ánh sáng)
        
        

## Một vài điểm cần cải tiến

- Hiện tại project cần sử dụng 2 mạch ESP32 để có thể xử lí 2 công việc: đọc/ghi dữ liệu thu được từ các cảm biến lên màn hình + đẩy dữ liệu lên ThinkSpeak và nhận dữ liệu các công tắc trên màn hình. ⇒ Cần một giải pháp tối ưu hơn trong tương lai.
- API của ThinkSpeak hạn chế số lần gửi data ⇒ Có thể sử dụng giao thức MQTT cũng như Websocket để viết trang web hiển thị thông số cảm biến và điều khiển các thiết bị.
