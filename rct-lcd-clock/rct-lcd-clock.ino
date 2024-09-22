#include <Wire.h>                  // Include Wire library (required for I2C devices)
#include <LiquidCrystal_I2C.h>     // Include LiquidCrystal_I2C library 
#include <RTClib.h>                // RTC 
#include <Servo.h>
 
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Configure LiquidCrystal_I2C library with 0x27 address, 16 columns and 2 rows
RTC_DS1307 RTC;                    // Configure RTC
Servo myServo;

  // Countdown setup
  int targetYear = 2024;
  int targetMonth = 9;
  int targetDay = 22;
  int targetHour = 11;
  int targetMinute = 10;
  int targetSeconds = 00;

  int redLED = 3; // Red LED connected to digital pin 3
  int greenLED = 2; // Green LED connected to digital pin 2
 
void setup() {

  // LCD setup
  lcd.init();                        // Initialize I2C LCD module
  lcd.backlight();                   // Turn backlight ON

  // RTC setup
  Serial.begin(9600);
  Wire.begin();
  RTC.begin(); // load the time from computer.
  if (! RTC.isrunning()) {

    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // LED setup
  pinMode(redLED, OUTPUT);   // Set red LED pin as output
  pinMode(greenLED, OUTPUT); // Set green LED pin as output

  myServo.attach(9);



}
 
void loop() {

  // Serial Loop
  DateTime now = RTC.now();
  
  // Create the target DateTime object
  DateTime target(targetYear, targetMonth, targetDay, targetHour, targetMinute, targetSeconds);
  
  // Calculate the time difference (remaining time)
  TimeSpan remaining = target - now;

  // Display current date/time on the first line
  char dateBuffer[16];
  sprintf(dateBuffer, "%02u:%02u:%02u %02u/%02u/%02u", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year() % 100);
  lcd.setCursor(0, 0);
  lcd.print(dateBuffer);

  // Check if the target time has been reached or passed
  if (remaining.totalseconds() > 0) {
    // Display countdown (remaining time) on the second line
    char countDownBuffer[16];
    sprintf(countDownBuffer, "%02ud %02uh %02um %02us", remaining.days(), remaining.hours(), remaining.minutes(), remaining.seconds());
    lcd.setCursor(0, 1);
    lcd.print(countDownBuffer);

    // LED control: Red LED on while counting down
    digitalWrite(redLED, HIGH);  // Turn on the red LED
    digitalWrite(greenLED, LOW); // Turn off the green LED

    myServo.write(90);
    delay(20);

  } else {
    // Target reached, display 00d 00h 00m 00s on the second line
    lcd.setCursor(0, 1);
    lcd.print("00d 00h 00m 00s");

    // LED control: Green LED on when countdown is done
    digitalWrite(redLED, LOW);   // Turn off the red LED
    digitalWrite(greenLED, HIGH); // Turn on the green LED

    myServo.write(0);
    delay(20);
  }

  delay(1000);
}