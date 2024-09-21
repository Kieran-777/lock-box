 #include <Wire.h>                  // Include Wire library (required for I2C devices)
#include <LiquidCrystal_I2C.h>     // Include LiquidCrystal_I2C library 
#include <RTClib.h>                // RTC 
 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Configure LiquidCrystal_I2C library with 0x27 address, 16 columns and 2 rows

RTC_DS1307 RTC;                    // Configure RTC
 
void setup() {

  // LCD setup
  lcd.init();                        // Initialize I2C LCD module
  lcd.backlight();                   // Turn backlight ON

  // RTC setup
  Serial.begin(9600);
  Wire.begin();
  RTC.begin(); // load the time from your computer.
  if (! RTC.isrunning()) {

    Serial.println("RTC is NOT running!");// This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}
 
void loop() {

  // Serial Loop
  DateTime now = RTC.now();
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  char dateBuffer[12];

  sprintf(dateBuffer, "%02u-%02u-%02u ", now.day(),now.month(),now.year() % 100);
  lcd.setCursor(0, 0);
  lcd.print(dateBuffer);

  sprintf(dateBuffer, "%02u:%02u:%02u",now.hour(),now.minute(),now.second());
  lcd.setCursor(0, 1);
  lcd.print(dateBuffer);


  delay(1000);
}