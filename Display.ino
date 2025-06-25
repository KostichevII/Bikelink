#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd (0x27, 16, 2);

void setupDisplay(){
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("V0.5!");
  Serial.println("Setup display");
}
void clearDisplay(){
  lcd.clear();
}

void defaultDisplay(double relayPower, double newSpeed, double newDistance){
  lcd.setCursor(0,0);
  lcd.print(String((int)((double)relayPower / maxRelayPower * 100)) + "% ");

  lcd.setCursor(0,1);
  lcd.print("Dist: ");
  lcd.setCursor(6, 1);
  lcd.print(newDistance);
  lcd.setCursor(6, 0);
  lcd.print("Spd:");
  lcd.print(newSpeed);

  Serial.print("speed: ");
  Serial.println(speed);
  Serial.print("dist: ");
  Serial.println(distance);
  Serial.print("relay: ");
  Serial.println(relayPower);
  Serial.println("Default display");
}

void safeDisplay(int counter){
  lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("SAFE MODE");
    lcd.setCursor(0, 1);
    lcd.print(counter);
  Serial.println("Safe display");
}

void dummyDisplay(int counter){
  lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(counter);
  Serial.println("Dummy display");
}

void changeState(){
  lcd.clear();
  Serial.println("Change state!");
}

void lockedDisplay(){
  lcd.setCursor(0,0);
  lcd.print("Locked.");
  Serial.println("Locked display");
}

void displayMessage(String message){
  lcd.setCursor(0,0);
  lcd.print(message);
  Serial.println("Displaying message");
}
//--------------------------------------
void displayShowPassword(String message){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Password:");
  lcd.setCursor(0,1);
  lcd.print(message);
}
//---------------------------------------