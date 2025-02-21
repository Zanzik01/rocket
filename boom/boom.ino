#include <SoftwareSerial.h>
#include <VB_BMP280.h>
#include "MPU6050.h"

const byte PIN_TX_RADIO = 7;
const byte PIN_RX_RADIO = 8;

const byte PIN_LED = 2;

const byte START_ALT_BAROMETER = 0;     

SoftwareSerial HC12(PIN_TX_RADIO, PIN_RX_RADIO);

VB_BMP280 barometer;
MPU6050 accelerometer;

int16_t ax, ay, az;
int16_t gx, gy, gz;

unsigned long currentTime;
float currentAlt;

void setup() {
  Serial.begin(9600);
  HC12.begin(9600);

  pinMode(PIN_LED, OUTPUT);

  delay(1000);

  setupBarometer();
  setupAccelerometer();
   
  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  accelerometer.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  barometer.read();

  currentAlt = barometer.alti;
  currentTime = millis();

  sendTelemetry();
}

void sendTelemetry() {
  String dataString = 
    String(currentTime) + ";"
  + String(currentAlt) + ";"
  + String(ax) + ";"
  + String(ay) + ";"
  + String(az) + ";"
  + "\n";

  char dataArr[80];
  dataString.toCharArray(dataArr, 80);

  Serial.println(dataString);
  HC12.write(dataArr);
  delay(80); 
}

void setupAccelerometer() {
  accelerometer.initialize();

  if (accelerometer.testConnection()) {
    return;
  } 
  else {
    reportError();
  }
}

void setupBarometer() {
  barometer.start_altitude = START_ALT_BAROMETER;

  if (barometer.begin()){
    return;
  }
  else {
    reportError();
  }
}

void reportError() {
    delay(200);
    digitalWrite(PIN_LED, !(digitalRead(PIN_LED)));
}
