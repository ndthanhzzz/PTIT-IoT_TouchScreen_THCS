// Define pins
#define RX_PIN 16
#define TX_PIN 17

#define LIGHT 19
#define FAN 21
#define PUMP 22
#define AC 23

// Init lib(s)

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

