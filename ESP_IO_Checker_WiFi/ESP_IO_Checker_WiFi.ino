#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>

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
#define NUM_PINS 13
int pin[NUM_PINS] = { 26, 25, 17, 16, 27, 14, 12, 13, 5, 23, 19, 18, 4 };
int connectionStatus;

#define WIFI_STA_NAME "ESP Wifi IO Checker"
#define WIFI_STA_PASS ""

WiFiServer server(3000);

void setup() {
  Serial.begin(115200);

  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(WiFiName, WiFiPassword);

  Serial.println("");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println("WiFi Initialized");
  server.begin();
  pinMode(LED, OUTPUT);
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(pin[i], INPUT_PULLUP);
  }
  display.begin();
  updateDisplay(0);
  Serial.println("Pin configured");
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("new client");
    while (client.connected()) {
      digitalWrite(LED, HIGH);
      String data = "";
      updateDisplay(1);
      vTaskDelay(400);
      for (int i = 0; i < NUM_PINS; i++) {
        data += String(digitalRead(pin[i]));
        if (i != NUM_PINS - 1) {
          data += ":";
        }
      }
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
    updateDisplay(0);
  }
  delay(10);
}

void updateDisplay(int connectionStatus) {
  display.clearBuffer();
  display.setFlipMode(1);
  display.setFont(u8g2_font_bauhaus2015_tr);
  display.firstPage();
  display.setFontPosTop();
  do {
    display.setCursor(7, 0);
    display.print(F("WiFi IO Monitor"));
    display.drawBitmap(50, 20, 3, 24, wifiIcon);
    if (connectionStatus == 1) {
      display.setCursor(25, 50);
      display.print("Connected");
    } else {
      display.setCursor(12, 50);
      display.print("Not Connected");
    }

  } while (display.nextPage());
}