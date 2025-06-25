#include <Preferences.h>
#include <TickTwo.h>
//---------------------
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
//----------------------
#define LIGHT_SENSOR 34
#define HOLLA_SENSOR 35
#define RELAY 32

#define KEY 33
#define S2 25
#define S1 26

//-------------------------------------------
#define BUFFER_SIZE 100
#define Zoomer 12
#define VibrationSensor 13

MPU6050 mpu;
//LiquidCrystal_I2C lcd(0x27, 16,2);
int xyz[]={0,0,0};
unsigned short int password [] = {5,1,3,7};
unsigned short int passwordInputed [] = {0,0,0,0};
unsigned short int pointer=0;
int startPosition[]={0,0,0};
int prevS1;
int curS1, curS2;
bool flag=false;
bool zoomerSound=false;
unsigned long whenKeyPress = 0;

unsigned short int moutionCounter=0;
unsigned short int moutionCheck=0; 
unsigned short int CheckStage=0;
void ShowPassword();
void ClearPassword();
void MoutionChecking();
void checkVibr();
void MPUsetup();
void BeepStop();

TickTwo CheckVibration(checkVibr, 1000);
TickTwo CheckMoution(MoutionChecking, 1000);
TickTwo DisplayTicker(ShowPassword,1000);
TickTwo ClearTicker(ClearPassword, 30000);
TickTwo ZoomerStop(BeepStop, 10000);
bool inputed=false;
//--------------------------------------

void handleSpeed();
void handleLight();
void handleInput();
void debounceSpeedComplete();
void debounceInputComplete();
void performSaveDistanceTask();
void displayTask();

double distance = 0;
double speed = 0;
bool onSecurity = false;
bool forcedLight = false;
int safeCounter = 40;

int current = 0;

Preferences preferences;

int turnOversCounter = 0;
bool lastHollaValue = true;
unsigned long lastHollaTime = 0; 
int relayPower = 0;
int maxRelayPower = 135; //Changes from time to time (WTFF)
volatile bool speedDebouncing = false;
volatile bool inputDebouncing = false;

bool voltageConfigMode = false;
bool pickMaxVoltage = false;

TickTwo saveDistanceTicker(performSaveDistanceTask, 1000*60*15, 0);
TickTwo speedTicker(handleSpeed, 100, 0, MICROS_MICROS);
TickTwo inputTicker(handleInput, 200, 0, MICROS_MICROS);
TickTwo inputDebouncerTicker(debounceInputComplete, 200, 1);
TickTwo speedDebouncerTicker(debounceSpeedComplete, 50, 1);
TickTwo lightSensorTicker(handleLight, 2000);
TickTwo displayTicker(displayTask, 500);

void debounceSpeedComplete(){
  speedDebouncing = false;
}

void debounceInputComplete(){
  inputDebouncing = false;
}

void handleInput(){
  if(!digitalRead(KEY)){
    forcedLight = !forcedLight;
    inputDebouncing = true;
    inputDebouncerTicker.start();
  }

  if(!forcedLight)
    return;

  if(!inputDebouncing){
    if(!digitalRead(S1)){
      if(!digitalRead(S2)){
        relayPower++;
      }
      else
        relayPower--;
      
      inputDebouncing = true;
      inputDebouncerTicker.start();
    }
  }
/*
  if(!digitalRead(KEY)){
    if(!voltageConfigMode && !pickMaxVoltage){
      pickMaxVoltage = true;
      voltageConfigMode = true;
      lcd.setCursor(4, 0);
      lcd.print("DEBUG MAX");
    }
    else if(voltageConfigMode && pickMaxVoltage){
      pickMaxVoltage = false;
      lcd.setCursor(4, 0);
      lcd.print("DEBUG CUR");
    }
    else if(voltageConfigMode && !pickMaxVoltage){
      voltageConfigMode = false;
      lcd.setCursor(4, 0);
      lcd.print("NORMAL     ");
    }

    inputDebouncing = true;
    inputDebouncerTicker.start();
  }

  if(!voltageConfigMode)
    return;

  int value;
  if(pickMaxVoltage)
    value = maxRelayPower;
  else
    value = relayPower;

  if(!inputDebouncing){
    if(!digitalRead(S1)){
      if(!digitalRead(S2)){
        value++;
      }
      else
        value--;
      
      inputDebouncing = true;
      inputDebouncerTicker.start();
    }
  }

  if(pickMaxVoltage)
    maxRelayPower = value;
  else
    relayPower = value;

  lcd.setCursor(3, 1);
  lcd.print("MX:" + String(maxRelayPower) + " " + "CR:" + String(relayPower) + " ");
  */
}

void handleLight(){
  if(forcedLight)
    return;

  int lightValue = analogRead(LIGHT_SENSOR);

  if(!voltageConfigMode){
    if(lightValue > 700){
      relayPower = 0;
    }
    else{
      relayPower = map(1023 - lightValue, 0, 900, 0, maxRelayPower);
      relayPower = constrain(relayPower, 0, maxRelayPower);
    } 
  }
}

void handleSpeed(){
  bool currentStatus = digitalRead(HOLLA_SENSOR);

  if(!speedDebouncing && lastHollaValue == HIGH && currentStatus == LOW){
    turnOversCounter++;
    unsigned int deltaTime = lastHollaTime - millis();
    lastHollaTime += deltaTime;

    speed = (2.3140971 / deltaTime) * 3600; // kilometers per second

    distance += 2.3140971;

    Serial.println("T:" + String(turnOversCounter));

    speedDebouncing = true;
    
    speedDebouncerTicker.start();
  }

  lastHollaValue = currentStatus;
}

void performSaveDistanceTask(){
  preferences.putDouble("distance", distance);
  Serial.print("Saving distance... ");
  Serial.println(distance);
}

void setup() {
  analogReadResolution(10);
  analogWriteResolution(RELAY, 8);
  analogWriteFrequency(RELAY, 100);

  preferences.begin("storage", false);
  setupDisplay();
  setupBLE();
  pinMode(LIGHT_SENSOR, INPUT);
  pinMode(HOLLA_SENSOR, INPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(KEY, INPUT);
  pinMode(S2, INPUT);
  pinMode(S1, INPUT);
  //----------------------
  pinMode(VibrationSensor, INPUT);
  pinMode(Zoomer, OUTPUT);
  mpu.initialize();
  //----------------------
  Serial.begin(9600);
  Serial.println("Startup successful!");
  delay(2000);

  clearDisplay();
  speedTicker.start();
  lightSensorTicker.start();
  inputTicker.start();
  saveDistanceTicker.start();
  displayTicker.start();

  Serial.println("Server is working fine!");

  distance = preferences.getDouble("distance", 0.0);
  Serial.print("Loading previous distance...");
  Serial.println(distance);
}

void displayTask(){
  if(!onSecurity){
    defaultDisplay(relayPower, speed, distance);
  }
  else{
    lockedDisplay();
  }
}

void loop() {
  if(!onSecurity){
    lightSensorTicker.update();
    speedTicker.update();
    inputTicker.update();
    speedDebouncerTicker.update();
    inputDebouncerTicker.update();
    saveDistanceTicker.update();
    displayTicker.update();

    String command = getCommand();
    if(command != ""){
      if(command == "reset"){
          distance = 0;
          performSaveDistanceTask();
      }
      else if(command == "setSecurity"){
          onSecurity = "true";
          setFlags(onSecurity);

          //--------------------
          SecuritySetUp();
          //--------------------

          changeState();
      }
      else{ 
          displayMessage("No command!");
          delay(1000);
      }
    }
    updateSpeedChar(speed);
    updateDistanceChar(distance);

    //LIGHT PWM
    analogWrite(RELAY, relayPower);
  }
  else {
    Serial.println("The device is locked!");
    if(checkPassword()){
      onSecurity = false;
      setFlags(onSecurity);

      //-------------------
      SecurityStop();
      //-------------------

      changeState();
    }
    //---------------------
    DisplayTicker.update();
    CheckVibration.update();
    ClearTicker.update();
    CheckMoution.update();
    ZoomerStop.update();
    SecureFunc();
    //--------------------
    analogWrite(RELAY, 0);
  }
}

//--------------------------------------
//данная функция для включения защиты
void SecuritySetUp(){
  passwordInputed[0]=passwordInputed[1]=passwordInputed[2]=passwordInputed[3]=0;
  //passwordInputed = {0,0,0,0};
  pointer=0;
  startPosition[0]=startPosition[1]=startPosition[2]=0;
  //startPosition={0,0,0};
  flag=false;
  whenKeyPress = 0;
  moutionCounter=0;
  moutionCheck=0; 
  CheckStage=0;
  inputed=false;
  zoomerSound=false;
  xyz[0]=xyz[1]=xyz[2]=0;
  //xyz={0,0,0};
  TickersSet();
  MPUsetup();
}
//данная функция для отключения защиты
void SecurityStop(){
  DisplayTicker.stop();
  ClearTicker.stop();
  switch(CheckStage){
    case 0:
    {
      CheckVibration.stop();
      break;
    }
    case 1:
    {
      CheckMoution.stop();
      break;
    }
    case 2:
    {
      CheckVibration.stop();
      break;
    }
    case 3:
    {
      if(zoomerSound){
        BeepStop();
      }
      CheckVibration.stop();
      break;
    }
    default:{
      break;
    }
  }
}
// функции для ввода пароля
void TickersSet(){
  DisplayTicker.start();
  CheckVibration.start();
}
void SecureFunc(){
  ReadButton();
}
void ShowPassword(){
  String stringPassword="";
  int size=sizeof(passwordInputed)/sizeof(passwordInputed[0]);
  for(int i=0; i< size; i++){
    stringPassword+=String(passwordInputed[i]);
  }
  displayShowPassword(stringPassword);
}
void ClearPassword(){
  passwordInputed[0] = passwordInputed[1]=passwordInputed[2]=passwordInputed[3]=0;
  pointer=0;
  ClearTicker.stop();
  inputed=false;
}
void CheckPassword(){
  bool correct=true;
  int size=sizeof(passwordInputed)/sizeof(passwordInputed[0]);
  for(int i=0; i<size; i++){
    if (passwordInputed[i]!=password[i]){
       ClearPassword();
       displayShowPassword("Incorrect");
       int timerdelay=millis();
       while(millis()-timerdelay<1000){}
       ClearPassword();
       return;
    }
  }

  displayShowPassword("Correct");
  int timerdelay=millis();
  while(millis()-timerdelay<1000){}
  ClearPassword();
  SecurityStop();
  return;    
}
void UpdateClear(){
  ClearTicker.stop();
  ClearTicker.start();
}
void ReadButton(){

  // запуск таймера
 if (!inputed && passwordInputed[0]!=0 && pointer==0){
  inputed=true;
  ClearTicker.start();
 }

  curS1 = digitalRead(S1);
  //обработка поворота
  if (curS1!=prevS1){
    curS2 = digitalRead(S2);
    if (flag){
        if (curS2==curS1){
            passwordInputed[pointer]++;
            if(passwordInputed[pointer]==10){
              passwordInputed[pointer]=0;
            }
         }
         else {
            passwordInputed[pointer]--;
            if(passwordInputed[pointer]==-1){
              passwordInputed[pointer]=9;
            }
         }

         flag=false;
    }
    else {
      flag=true;
    }

    if(inputed){
      UpdateClear();
    }
  }
  prevS1=curS1;
  //обработка нажатия
  int curKey = digitalRead(KEY);
  if (curKey == LOW) {
    if (millis() - whenKeyPress > 50) {
      pointer++;
      if(pointer>=4){
        CheckPassword();
      }
    }
    whenKeyPress = millis();

    if(inputed){
      UpdateClear();
    }
  }
}

//Экспериментально 
//приведение защиты в начальное положение:
void ClearSecurity(){
  CheckStage=0;
  moutionCounter=0;
  moutionCheck=0;
  CheckVibration.start();
}
//калибровка аксселерометра. Запускается каждый раз при установке
//режима защиты
void calibration() {
  long offsets[6];
  long offsetsOld[6];
  int16_t mpuGet[6];

  // используем стандартную точность
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

  // обнуляем оффсеты
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  delay(10);
  Serial.println("Calibration start. It will take about 5 seconds");
  for (byte n = 0; n < 10; n++) {     // 10 итераций калибровки
    for (byte j = 0; j < 6; j++) {    // обнуляем калибровочный массив
      offsets[j] = 0;
    }
    for (byte i = 0; i < 100 + BUFFER_SIZE; i++) { // делаем BUFFER_SIZE измерений для усреднения
      mpu.getMotion6(&mpuGet[0], &mpuGet[1], &mpuGet[2], &mpuGet[3], &mpuGet[4], &mpuGet[5]);
      if (i >= 99) {                         // пропускаем первые 99 измерений
        for (byte j = 0; j < 6; j++) {
          offsets[j] += (long)mpuGet[j];   // записываем в калибровочный массив
        }
      }
    }
    for (byte i = 0; i < 6; i++) {
      offsets[i] = offsetsOld[i] - ((long)offsets[i] / BUFFER_SIZE); // учитываем предыдущую калибровку
      if (i == 2) offsets[i] += 16384;                               // если ось Z, калибруем в 16384
      offsetsOld[i] = offsets[i];
    }
    // ставим новые оффсеты
    mpu.setXAccelOffset(offsets[0] / 8);
    mpu.setYAccelOffset(offsets[1] / 8);
    mpu.setZAccelOffset(offsets[2] / 8);
    mpu.setXGyroOffset(offsets[3] / 4);
    mpu.setYGyroOffset(offsets[4] / 4);
    mpu.setZGyroOffset(offsets[5] / 4);
    delay(2);
 }
}
//запуск калибровки и сохранение начальный значений
void MPUsetup() {
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  calibration();

  xyz[0]=mpu.getAccelerationX();
  xyz[1]=mpu.getAccelerationY();
  xyz[2]=mpu.getAccelerationZ();
  
  Serial.println("xyz:");
  Serial.println(xyz[0]);
  Serial.println(xyz[1]);
  Serial.println(xyz[2]);
  Serial.println("-----");
}
//проверка движения аксселерометра (часть MoutionChecking)
bool checkMotion(){
  int16_t x, y, z;
  int accuracy=2048;
  mpu.getAcceleration(&x, &y, &z);

  //Serial.print(x); Serial.print(" ; ");
  //Serial.print(y); Serial.print(" ; ");
  //Serial.println(z); 

  Serial.println("замер:");
  Serial.println(x);
  Serial.println(y);
  Serial.println(z);
  Serial.println("-----");
  if((x<xyz[0]-accuracy || x>xyz[0]+accuracy) || 
     (y<xyz[1]-accuracy || y>xyz[1]+accuracy) ||
     (z<xyz[2]-accuracy || z>xyz[2]+accuracy))
    {
    Serial.println("moution!");
    return true;
  }
  return false;
}
//функция проверки движения
void MoutionChecking(){
  if(checkMotion()){
    moutionCounter++;
  }
  moutionCheck++;
  if(moutionCheck==5){
    CheckMoution.stop();
    if(moutionCounter>=4){
      CheckStage=3;
      callUser();
    }
    else{
      CheckStage=2;
      CheckVibration.start();
    }
    moutionCounter=0;
    moutionCheck=0;
  }
}
//вызов пользователя
void callUser(){
  Serial.println("Тревога!");
  //Сделать отправку сообщения пользователю и включение сигнализации
  analogWrite(Zoomer, 256);
  zoomerSound=true;
  ZoomerStop.start();
  ClearSecurity();
}
//проверка вибрации
void checkVibr(){
  bool vibr=digitalRead(VibrationSensor);
  Serial.print(CheckStage);
  Serial.print(" vibr:");
  Serial.println(vibr);
  if(CheckStage==0){
    if(!vibr){
      CheckStage=1;
      CheckVibration.stop();
      CheckMoution.start();
      //Serial.println("Проверка движения");
    }
  }
  else{
    //проверка 10-вибраций
    moutionCheck++;
    if(!vibr){
      moutionCounter++;
    }
    if(moutionCheck==10){
      CheckVibration.stop();
      if(moutionCounter>=8){
        CheckStage=3;
        callUser();
      }
      else{
        ClearSecurity();
      }
    }
  }
  
}
//остановка пищалки
void BeepStop(){
  analogWrite(Zoomer,0);
  ZoomerStop.stop();
  zoomerSound=false;
}
//---------------------------------------