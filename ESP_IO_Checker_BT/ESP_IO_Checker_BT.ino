#include <BluetoothSerial.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SPI.h>

U8G2_SSD1306_128X64_NONAME_1_SW_I2C display(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);
static unsigned char bluetoothIcon[] = {
  0x00, 0x10, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x3f, 0x00,
  0x00, 0x3b, 0x80, 0x00, 0x39, 0xe0, 0x06, 0x38, 0xe0, 0x07, 0xb9, 0xe0,
  0x03, 0xfb, 0x80, 0x01, 0xff, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x7c, 0x00,
  0x00, 0x3c, 0x00, 0x00, 0xfe, 0x00, 0x01, 0xff, 0x00, 0x01, 0xfb, 0x80,
  0x07, 0xb9, 0xe0, 0x07, 0x38, 0xe0, 0x00, 0x39, 0xe0, 0x00, 0x3b, 0x80,
  0x00, 0x3f, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x10, 0x00
};

BluetoothSerial ESP32BT;
#define LED 2
#define bluetoothName "ESP BT IO Checker"
#define NUM_PINS 10
int pin[NUM_PINS] = { 26, 25, 17, 16, 27, 14, 12, 13, 23, 4 };
int connectionStatus;

#define MAX6675_MISO 19
#define MAX6675_SCK 18
#define MAX6675_CS 5

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Client Connected");
    digitalWrite(LED, HIGH);
    connectionStatus = 1;
  }

  if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("Client disconnected");
    digitalWrite(LED, LOW);
    connectionStatus = 0;
  }
}


void setup() {
  Serial.begin(115200);          // เริ่ม Serial monitor
  ESP32BT.begin(bluetoothName);  // เริ่มการทำงานของบลูทูธ ให้ชื่อ ESP32_MyLED
  display.begin();
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(pin[i], INPUT_PULLUP);
  }
  ESP32BT.register_callback(callback);
  Serial.println("Bluetooth Initialized");

  pinMode(MAX6675_CS, OUTPUT);
  updateDisplay(0, readTemperature());
  SPI.begin(MAX6675_SCK, MAX6675_MISO, -1, MAX6675_CS);
  pinMode(LED, OUTPUT);
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(pin[i], INPUT_PULLUP);
  }
}

void loop() {
  vTaskDelay(400);
  String data = "";
  updateDisplay(connectionStatus, readTemperature());
  vTaskDelay(400);
  for (int i = 0; i < NUM_PINS; i++) {
    data += String(digitalRead(pin[i]));
    if (i != NUM_PINS - 1) {
      data += ":";
    }
  }
  data += ",";
  data += String(readTemperature());
  ESP32BT.println(data);
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

void updateDisplay(int connectionStatus, float temp) {
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
    display.drawBitmap(tempLogoStartX, tempLogoStartY, 3, 24, bluetoothIcon);
    if (connectionStatus == 1) {
      display.setCursor(40, 26);
      display.print("Connected");
    } else {
      display.drawLine(tempLogoStartX - 1, tempLogoStartY + 24 + 1 + 1, tempLogoStartX + 24 + 1, tempLogoStartY - 1 + 1);
      display.drawLine(tempLogoStartX - 1, tempLogoStartY + 24 + 1, tempLogoStartX + 24 + 1, tempLogoStartY - 1);
      display.drawLine(tempLogoStartX - 1, tempLogoStartY + 24 + 1 - 1, tempLogoStartX + 24 + 1, tempLogoStartY - 1 - 1);
      display.setCursor(65, 20);
      display.print("Not");
      display.setCursor(40, 33);
      display.print("Connected");
      // display.drawLine(8,18,24+8+2,24+18+2);
      // display.setCursor(12, 50);
      // display.print("Not Connected");
    }
    display.setCursor(10, 50);
    display.print("Temp : " + String(temp) + "°C");
  } while (display.nextPage());
}