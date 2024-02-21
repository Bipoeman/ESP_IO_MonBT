#include <BluetoothSerial.h>
#include <Wire.h>
#include <U8g2lib.h>

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
#define bluetoothName "ESP32 Led With Display"
#define NUM_PINS 13
int pin[NUM_PINS] = { 26, 25, 17, 16, 27, 14, 12, 13, 5, 23, 19, 18, 4 };
int connectionStatus;

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
  pinMode(2, OUTPUT);
  ESP32BT.register_callback(callback);
  Serial.println("Bluetooth Initialized");
}

void loop() {
  vTaskDelay(400);
  String data = "";
  for (int i = 0; i < NUM_PINS; i++) {
    data += String(digitalRead(pin[i]));
    if (i != NUM_PINS - 1) {
      data += ":";
    }
  }
  // Serial.println(data);
  ESP32BT.println(data);
  updateDisplay(connectionStatus);

}



void updateDisplay(int connectionStatus) {
  display.clearBuffer();
  display.setFlipMode(1);
  display.setFont(u8g2_font_bauhaus2015_tr);
  display.firstPage();
  display.setFontPosTop();
  do {
    display.setCursor(15, 0);
    display.print(F("BT IO Monitor"));
    display.drawBitmap(50, 20, 3, 24, bluetoothIcon);
    if (connectionStatus == 1) {
      display.setCursor(25, 50);
      display.print("Connected");
    }
    else{
      display.setCursor(12, 50);
      display.print("Not Connected");
    }

  } while (display.nextPage());

}