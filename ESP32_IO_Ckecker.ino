#include <BluetoothSerial.h>

BluetoothSerial ESP32BT;
#define LED 2
#define bluetoothName "ESP32_MyLED"
#define NUM_PINS 13
int pin[NUM_PINS] = {26,25,17,16,27,14,12,13,5,23,19,18,4};

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    digitalWrite(LED,HIGH);
  }
 
  if(event == ESP_SPP_CLOSE_EVT ){
    Serial.println("Client disconnected");
    digitalWrite(LED,LOW);
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
  String data = "";
  for (int i = 0;i < NUM_PINS;i++){
    data += String(digitalRead(pin[i]));
    if (i != NUM_PINS - 1){
      data += ":";
    }
  }
  Serial.println(data);
  ESP32BT.println(data);
}