#include <iarduino_VCC.h>
#include <VB_BMP180.h>
#include <Arduino.h>
#include <MPU6050.h>
#include <I2Cdev.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

const byte PIN_SET_RADIO = 12;

const byte PIN_BTN_SATELLITE = 10;                                // пин кнопки тестового вывода ПН
const byte PIN_BTN_ROCKET = 8;                                    // пин кнопки тестовой активации СС
const byte PIN_LED = A1;                                          // пин светодиода
const byte PIN_SERVO_SATELLITE = 7;                               // пин сервопривода ПН
const byte PIN_SERVO_ROCKET = 9;                                  // пин сервопривода СС

const byte teamID = 20;                                           // код команды
const byte START_ALT = 0;                                         // высота старта
const byte FINISH_ALT = 1;                                        // высота приземления
const byte START_ACC = 5;                                         // стартовое ускорение
const byte ALT_DISC_SATELLITE = 120;                              // высота вывода ПН
const byte DIFF_ALT_APOGEE = 10;                                  // разница между высотой в апогее и текущей для СС

Servo servoSatellite;                                             // сервопривод Спутника
Servo servoRocket;                                                // сервопривод Системы Спасения

VB_BMP180 barometer;
MPU6050 accelerometer;

int16_t ax, ay, az;                                               // ускорение по осям XYZ

byte checkSattelite(0), checkRocket(0);                           // состояния системы вывода ПН, СС; 0 - до срабатывания; 1 - активация; 2 - отработала;

unsigned long currentTime;                                        // время от начала работы программы
float currentVCC, currentAlt, currentAcc;                         // текущие напряжение, высота, модуль вектора ускорения

byte pointStart(0), pointApogee(0);                               // старт РН, точка апогея
byte pointActivate(0), pointSatellite(0), pointLanding(0);        // активация СС, выход спутника, приземление РН

float maxAlt(0);                                                  // апогей


void setup() {
  pinMode(PIN_SET_RADIO, OUTPUT);
  digitalWrite(PIN_SET_RADIO, HIGH);
  
  Serial.begin(9600);

  servoSatellite.attach(PIN_SERVO_SATELLITE);
  servoRocket.attach(PIN_SERVO_ROCKET);

  pinMode(PIN_BTN_SATELLITE, INPUT);
  pinMode(PIN_BTN_ROCKET, INPUT);
  
  pinMode(PIN_LED, OUTPUT);


  setupAccelerometer();
  setupBarometer();

  digitalWrite(PIN_LED, HIGH);                                    // инициализация прошла успешно
}

void setupAccelerometer() {
  accelerometer.initialize();
  accelerometer.setXAccelOffset(0);
  accelerometer.setYAccelOffset(0);
  accelerometer.setZAccelOffset(0);
  accelerometer.setXGyroOffset(0);
  accelerometer.setYGyroOffset(0);
  accelerometer.setZGyroOffset(0);

  if (accelerometer.testConnection())
    return;
  else {
    reportError();
  }
}

void setupBarometer() {
  barometer.start_altitude = START_ALT;

  if (barometer.begin())
    return;
  else {
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
  currentVCC = analogRead_VCC();
  currentTime = millis();
  
  if (currentAcc >= START_ACC)
    pointStart = 1;                                               // старт ракеты

  if (maxAlt < currentAlt)                                        // отслеживание максимальной высоты
    maxAlt = currentAlt;
  else if ((maxAlt - currentAlt >= DIFF_ALT_APOGEE)){
    pointApogee = 1;                                              // точка апогея была зафиксирована
    pointActivate = 1;                                            // система спасения была активирована
    
    if (checkRocket != 2){
      checkRocket = 1;
    }
  }

  if (currentAlt <= ALT_DISC_SATELLITE && checkRocket == 2){
    pointSatellite = 1;                                           // выведена ПН
    
    if (checkSattelite != 2){
      checkSattelite == 1;
    }
  }

  if (digitalRead(PIN_BTN_ROCKET) == HIGH){
    servoRocket.write(0);
    delay(1000);
    servoRocket.write(90);
  }

  if (digitalRead(PIN_BTN_SATELLITE) == HIGH){
    servoSatellite.write(0);
    delay(1000);
    servoSatellite.write(90);
  }
  
  if (checkRocket == 1){ 
    checkRocket = 2;

    servoRocket.write(0);
    delay(1000);
    servoRocket.write(90);
  }
  
  if (checkSattelite == 1){
    checkSattelite = 2;
    
    servoSatellite.write(0);
    delay(1000);
    servoSatellite.write(90);
  }

  if (currentAlt <= FINISH_ALT && checkRocket == 2)
    pointLanding = 1;                                             // РН на земле
  
  sendTelemetry();
}

float transtaleSI(int16_t value){
  return float(value) / 32768 * 2;
}

void sendTelemetry() {

  String dataString = 
    String(teamID) + ";" 
  + String(currentTime) + ";"
  + String(currentAlt) + ";"
  + String(currentVCC) + ";"
  + String(currentAcc) + ";"
  + String(pointStart) + ";"
  + String(pointApogee) + ";"
  + String(pointActivate) + ";"
  + String(pointSatellite) + ";"
  + String(pointLanding) + ";"
  + "\n";

  char dataArr[80];
  dataString.toCharArray(dataArr, 80);
  
  Serial.write(dataArr);
  delay(80);
}

void reportError() {
  while (1) {
    delay(200);
    digitalWrite(PIN_LED, !(digitalRead(PIN_LED)));
  }
}
