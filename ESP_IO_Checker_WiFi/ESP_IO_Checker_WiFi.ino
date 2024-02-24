#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SPI.h>

U8G2_SSD1306_128X64_NONAME_1_SW_I2C display(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);

static unsigned char wifiIcon[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
  0x07, 0xff, 0xf0, 0x0f, 0xff, 0xf8, 0x3f, 0xc3, 0xfc, 0x7f, 0x00, 0x7e,
  0xf8, 0x00, 0x1f, 0xf0, 0x7f, 0x0f, 0xe1, 0xff, 0xc7, 0x03, 0xff, 0xe0,
  0x07, 0xf7, 0xf0, 0x0f, 0x81, 0xf0, 0x0f, 0x00, 0x70, 0x06, 0x00, 0x00,
  0x00, 0x3c, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00,
  0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define LED 2
#define WiFiName "ESP Wifi IO Checker"
#define WiFiPassword "monitor"
#define NUM_PINS 10
int pin[NUM_PINS] = { 26, 25, 17, 16, 27, 14, 12, 13, 23, 4 };
// int pin[NUM_PINS] = { 26, 25, 17, 16, 27, 14, 12, 13, 5, 23, 19, 18, 4 };
int publicConnectionStatus;

#define WIFI_STA_NAME "ESP Wifi IO Checker"
#define WIFI_STA_PASS ""

#define MAX6675_MISO 19
#define MAX6675_SCK 18
#define MAX6675_CS 5

WiFiServer server(3000);

void setup() {
  Serial.begin(115200);

  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(WiFiName, WiFiPassword);

  Serial.println("");
  WiFi.onEvent(WiFiEvent);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
  },
                                       WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println("WiFi Initialized");
  server.begin();

  display.begin();
  Serial.println("Pin configured");

  pinMode(MAX6675_CS, OUTPUT);
  updateDisplay(0,readTemperature());
  SPI.begin(MAX6675_SCK, MAX6675_MISO, -1, MAX6675_CS);
  pinMode(LED, OUTPUT);
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(pin[i], INPUT_PULLUP);
  }
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    publicConnectionStatus = 1;
    while (client.connected() && publicConnectionStatus) {
      digitalWrite(LED, HIGH);
      String data = "";
      updateDisplay(1,readTemperature());
      vTaskDelay(400);
      for (int i = 0; i < NUM_PINS; i++) {
        data += String(digitalRead(pin[i]));
        if (i != NUM_PINS - 1) {
          data += ":";
        }
      }
      data += ",";
      data += String(readTemperature());
      client.println(data);
      // if (client.available()) {
      //   char c = client.read();
      //   Serial.write(c);
      // }
      // delay(1);
    }
    digitalWrite(LED, 0);
    client.stop();
    Serial.println("client disonnected");
  }
  updateDisplay(0,readTemperature());
  vTaskDelay(400);

}

float readTemperature() {
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  digitalWrite(MAX6675_CS, LOW);
  unsigned int returnVal = SPI.transfer(0x00) << 8;
  returnVal |= SPI.transfer(0x00);
  digitalWrite(MAX6675_CS, HIGH);
  SPI.endTransaction();
  int sensorOpen = (returnVal >> 2) & 1;
  if (sensorOpen) {
    Serial.print("Sensor Open");
    return -1.0;
  } else {
    return (returnVal >> 3) / 4.0;
  }
}

void updateDisplay(int connectionStatus,float temp) {
  display.clearBuffer();
  display.setFlipMode(1);
  display.setFont(u8g2_font_bauhaus2015_tr);
  display.firstPage();
  display.setFontPosTop();
  do {
    display.setCursor(7, 0);
    display.print(F("WiFi IO Monitor"));
    int tempLogoStartX = 10;
    int tempLogoStartY = 20;
    display.drawBitmap(tempLogoStartX, tempLogoStartY, 3, 24, wifiIcon);
    if (connectionStatus == 1) {
      display.setCursor(40, 26);
      display.print("Connected");
    } else {
      display.drawLine(tempLogoStartX - 1,tempLogoStartY + 24 + 1 + 1,tempLogoStartX + 24 + 1,tempLogoStartY - 1 + 1);
      display.drawLine(tempLogoStartX - 1,tempLogoStartY + 24 + 1,tempLogoStartX + 24 + 1,tempLogoStartY - 1);
      display.drawLine(tempLogoStartX - 1,tempLogoStartY + 24 + 1 - 1,tempLogoStartX + 24 + 1,tempLogoStartY - 1 - 1);
      display.setCursor(65, 20);
      display.print("Not");
      display.setCursor(40, 33);
      display.print("Connected");
      // display.drawLine(8,18,24+8+2,24+18+2);
      // display.setCursor(12, 50);
      // display.print("Not Connected");
    }
    display.setCursor(10, 50);
    display.print("Temp : "+String(temp)+"°C");
  } while (display.nextPage());
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("WiFi access point started");
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      Serial.println("WiFi access point  stopped");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("Client connected");
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("Client disconnected");
      updateDisplay(0,readTemperature());
      publicConnectionStatus = 0;
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.println("Assigned IP address to client");
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
      Serial.println("Received probe request");
      break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
      Serial.println("AP IPv6 is preferred");
      break;
    default: break;
  }
}
