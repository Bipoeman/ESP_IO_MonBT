#include <BluetoothSerial.h>

BluetoothSerial ESP32BT;
#define LED 2
#define bluetoothName "ESP32_MyLED"
#define NUM_PINS 13
bool isconnected = false;
bool hasconnected = false;
int pin[NUM_PINS] = {26,25,17,16,27,14,12,13,5,23,19,18,4};

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    digitalWrite(LED,HIGH);
    isconnected = true;
    hasconnected = true;
  }
 
  if(event == ESP_SPP_CLOSE_EVT ){
    Serial.println("Client disconnected");
    digitalWrite(LED,LOW);
    isconnected = false;
    // lastchecked = millis();
  }
}


void setup() {
  Serial.begin(115200);           // เริ่ม Serial monitor
  ESP32BT.begin(bluetoothName);   // เริ่มการทำงานของบลูทูธ ให้ชื่อ ESP32_MyLED
  for (int i = 0; i< NUM_PINS;i++){
    pinMode(pin[i],INPUT_PULLUP);
  }
  pinMode(2,OUTPUT);
  ESP32BT.register_callback(callback);
  Serial.println("Bluetooth Initialized");
}

void loop() {
  vTaskDelay(400);
  // delay(100);
  String data = "";
  for (int i = 0;i < NUM_PINS;i++){
    data += String(digitalRead(pin[i]));
    if (i != NUM_PINS - 1){
      data += ":";
    }
  }
  Serial.println(data);
  ESP32BT.println(data);
  // ถ้าได้รับข้อมูลอะไรก็ตามทางบลูทูธ
  // if(ESP32BT.available()){
  //   incoming = ESP32BT.read();    // อ่านค่าว่าค่าที่เข้ามาคืออะไร
  //   // ถ้าข้อมูลที่เข้ามาคือ ASCII รหัส 49 (เลข 1) ให้กลับสถานะ LED 
  //   // และแสดงทาง Serial monitor, Bluetooth
  //   if(incoming == 49){
  //     digitalWrite(LED, !(digitalRead(LED)));
  //     Serial.println("LED Toggled");
  //     ESP32BT.println("LED Toggled");
  //   }
  // }
}