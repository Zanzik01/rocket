#include <SoftwareSerial.h>

const byte PIN_SET_RADIO = 12;
const byte PIN_TX_RADIO = 3;
const byte PIN_RX_RADIO = 2;
  
SoftwareSerial hc12(PIN_TX_RADIO, PIN_RX_RADIO);

void setup() {
  Serial.begin(9600);
  hc12.begin(9600);

  pinMode(PIN_SET_RADIO, HIGH);
}

void loop() {
  digitalWrite(PIN_SET_RADIO, LOW);
  delay(100);

  Serial.println("Получение конфигурации модуля HC-12");
  hc12.print("AT+RX");
  delay(200);

  while(hc12.available()){
    Serial.write(hc12.read());
  }

  Serial.println("Изменение конфигурации:");

  // Возвращение к заводским настройкам
  hc12.print("AT+DEFAULT");
  delay(200);

  while(hc12.available()){
    Serial.write(hc12.read());
  }
 
  // Смена канала с 1 до 100
 hc12.print("AT+C024");
 delay(200);

 while(hc12.available()){
   Serial.write(hc12.read());
 }

  // Установка скорости передачи данных, по умолчанию 9600
 hc12.print("AT+B9600");
 delay(200);

 while(hc12.available()){
   Serial.write(hc12.read());
 }

 //Установка режима передачи данных 1-4 (выбирай 3 и все будет ок)
 hc12.print("AT+FU3");
 delay(200);

 while(hc12.available()){
   Serial.write(hc12.read());
 }

  Serial.println("Channel successfully changed");
  digitalWrite(PIN_SET_RADIO, HIGH);
}
