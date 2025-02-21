#include <VB_BMP180.h>
#include <MPU6050.h>
#include <I2Cdev.h>
#include <SPI.h>
#include <SD.h>

MPU6050 accelerometer;
VB_BMP180 barometer;
File dataFile;

const byte PIN_PH_RES = A0;                  // пин фоторезистора
const byte PIN_SD = 10;                      // пин SD-картридера
const byte PIN_TONE = 9;                     // пин пьезоизлучателя

const byte START_ALT  = 0;                   // высота старта
const byte VAL_PH_RES = 20;                  // граничное значение фоторезистора

const byte ALT_DISC_SATELLITE = 120;         // высота вывода ПН - включение пьезопищалки
const byte DIFF_ALT_MAX_CURRENT = 10;        // высота для проверки апогея

int16_t ax, ay, az;                          // ускорение по осям XYZ

String dataString;                           // записываемые данные
unsigned long currentTime;                   // время от начала работы программы
float currentAcc, currentAlt;                // модуль вектора ускорения, высота
int currentPhRes;                            // значения фоторезистора

float maxAlt(0);                             // апогей


void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_TONE, OUTPUT);
  pinMode(PIN_PH_RES, INPUT);

  setupSD();
  setupAccelerometer();
  setupBarometer();
}

void setupSD(){
  if (!SD.begin(PIN_SD)){
    reportError();
  }

  if (SD.exists("C305_RD2.csv"))
    SD.remove("C305_RD2.csv");
  
  dataFile = SD.open("C305_RD2.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.println("Time,Altitude,Acceleration,Satellite");
    dataFile.close();
  }
  else{
    reportError();
  }
}

void setupAccelerometer() {
  accelerometer.initialize();
  accelerometer.setXAccelOffset(12);
  accelerometer.setYAccelOffset(20);
  accelerometer.setZAccelOffset(16380);
  accelerometer.setXGyroOffset(26);
  accelerometer.setYGyroOffset(-3);
  accelerometer.setZGyroOffset(-6);
  
  if (accelerometer.testConnection())
    return;
  else {
//    Serial.println("MPU6050 соединение НЕ установлено"); 
    reportError();
  }
}

void setupBarometer(){
  // Высота стартовой точки, если не задать, то 0
  barometer.start_altitude = START_ALT;

  if (barometer.begin())
    return;
  else{
//    Serial.println("BMP180 соединение НЕ установлено");
    reportError();
  }
}

void loop() {
  accelerometer.getAcceleration(&ax, &ay, &az);
  barometer.read();

  currentAcc = sqrt(
    pow(transtaleSI(ax), 2) +
    pow(transtaleSI(ay), 2) +
    pow(transtaleSI(az), 2));

  currentAlt = barometer.alti;
  currentTime = millis();
  currentPhRes = analogRead(PIN_PH_RES);

  if (maxAlt < currentAlt) {
    maxAlt = currentAlt;
  }
  else if ((maxAlt - currentAlt) >= DIFF_ALT_MAX_CURRENT &&
      currentAlt <= ALT_DISC_SATELLITE) {
//      Serial.println("Выход спутника");
      turnSignalAltitude();
  }
  
  writeDataSD();
}

float transtaleSI(int16_t value){
  return float(value) / 32768 * 2;
}

void writeDataSD(){
    
  dataString = 
  String(currentTime) + "," 
  + String(currentAlt) + ","
  + String(currentAcc);
  
  if (currentPhRes >= VAL_PH_RES) {
    dataString += ",1";
  }

  dataFile = SD.open("C305_RD2.csv", FILE_WRITE);

  if (dataFile) {
    
    dataFile.println(dataString);
    dataFile.close();
  }
  else{
//    Serial.println("Проблема с записью");
    reportError();
  }   
}

void turnSignalAltitude() {
  tone(PIN_TONE, 300, 100);
  tone(PIN_TONE, 700, 100);
  tone(PIN_TONE, 300, 100);
}

void reportError() {
  while (1) {
    delay(200);
    tone(PIN_TONE, 400);
  }
}
