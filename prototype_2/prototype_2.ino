#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Servo.h>

// LCD and RTC setup
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 RTC;
Servo myServo;

// Countdown setup (Default target time: 00d 00h 01m 00s)
int targetYear = 2024;
int targetMonth = 9;
int targetDay = 0;
int targetHour = 0;
int targetMinute = 0;
int targetSeconds = 0;

// Temporary variables for setting new time
int tempTargetDay = targetDay;
int tempTargetHour = targetHour;
int tempTargetMinute = targetMinute;
int tempTargetSeconds = targetSeconds;

DateTime target;  // Fixed target DateTime

const int redLED = 7;
const int greenLED = 8;

// Rotary-Encoder pins
const int clkPin = 2;
const int dtPin = 3;
const int swPin = 4;

// Rotary Encoder state
int counter = 0;
int currentStateClk;
int lastStateClk;
int buttonState;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 5;

// Set mode variables
int setMode = 0;  // 0: Normal countdown mode, 1: Setting time mode, 2: Lock confirmation mode
int selectedField = 0;  // Tracks which part of the time (days, hours, minutes, seconds) is being adjusted
int lockSelection = 0;  // 0 for "Yes", 1 for "No"

void setup() {
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  myServo.attach(9);

  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  lastStateClk = digitalRead(clkPin);

  // Set default target to 00d 00h 00m 00s from now
  DateTime now = RTC.now();
  target = now + TimeSpan(0, 0, 0, 0);  // Default 0 minute from now
}

void loop() {
  DateTime now = RTC.now();

  handleEncoder(now);

  if (setMode == 0) {
    // Normal countdown operation
    displayCurrentTime(now);
    checkCountdown(now);
  } else if (setMode == 1) {
    // Setting time mode
    setTargetTime();  // This is called every time the user is adjusting the time
  } else if (setMode == 2) {
    // Confirmation mode: Lock box?
    confirmLockBox();
  }

  delay(100);
}

void handleEncoder(DateTime now) {
  currentStateClk = digitalRead(clkPin);

  if (currentStateClk != lastStateClk) {
    if (digitalRead(dtPin) != currentStateClk) {
      counter++;
    } else {
      counter++; // Previously counter--; but encoder noise kept decrementing values, maybe fix, might be nicer...
    }

    if (setMode == 1) {
      // Adjust the selected field (days, hours, minutes, seconds) based on encoder rotation
      switch (selectedField) {
        case 0: 
          tempTargetDay += (counter > 0) ? 1 : -1;
          if (tempTargetDay < 0) tempTargetDay = 0;  // Prevent negative days
          break;
        case 1: 
          tempTargetHour += (counter > 0) ? 1 : -1;
          if (tempTargetHour < 0) tempTargetHour = 0;  // Prevent negative hours
          break;
        case 2: 
          tempTargetMinute += (counter > 0) ? 1 : -1;
          if (tempTargetMinute < 0) tempTargetMinute = 0;  // Prevent negative minutes
          break;
        case 3: 
          tempTargetSeconds += (counter > 0) ? 1 : -1;
          if (tempTargetSeconds < 0) tempTargetSeconds = 0;  // Prevent negative seconds
          break;
      }
      lcd.blink();
    }

    if (setMode == 2) {
      // Toggle between "Yes" and "No" for lock confirmation
      if (counter > 0) {
        lockSelection = 0;  // "Yes"
      } else if (counter < 0) {
        lockSelection = 1;  // "No"
      }
    }

    counter = 0;
  }

  lastStateClk = currentStateClk;

  if (digitalRead(swPin) == LOW) {
    delay(200);  // Button debounce

    if (setMode == 0) {
      // Enter set time mode
      setMode = 1;
      selectedField = 0;  // Start by adjusting days
    } else if (setMode == 1) {
      // Move to the next field (days -> hours -> minutes -> seconds)
      selectedField++;
      if (selectedField > 3) {
        setMode = 2;  // Go to lock confirmation screen
      }
    } else if (setMode == 2) {
      // Handle lock confirmation
      if (lockSelection == 0) {
        // "Yes" selected, update the target time with new values
        DateTime now = RTC.now();
        target = now + TimeSpan(tempTargetDay, tempTargetHour, tempTargetMinute, tempTargetSeconds);

        // Return to normal countdown mode
        setMode = 0;

        // Print updated target time for debug purposes
        Serial.print("Updated target time: ");
        Serial.print(target.day());
        Serial.print("d ");
        Serial.print(target.hour());
        Serial.print("h ");
        Serial.print(target.minute());
        Serial.print("m ");
        Serial.print(target.second());
        Serial.println("s");

      } else {
        // "No" selected, go back to time setting mode
        setMode = 1;
        selectedField = 0;
      }
    }
  }
}

void setTargetTime() {
  lcd.clear();
  lcd.print("Set Lock:");

  // Display the time values currently being set
  char timeBuffer[16];
  sprintf(timeBuffer, "%02dd %02dh %02dm %02ds", tempTargetDay, tempTargetHour, tempTargetMinute, tempTargetSeconds);
  lcd.setCursor(0, 1);
  lcd.print(timeBuffer);

  // // Indicate which field is being adjusted 
  lcd.setCursor(selectedField * 4, 1);  // Position cursor under the current field
  lcd.blink();
}

void confirmLockBox() {
  lcd.clear();
  lcd.print("Lock Box?");
  lcd.setCursor(0, 1);
  lcd.blink_off();

  if (lockSelection == 0) {
    lcd.print(" >Yes   No");
  } else {
    lcd.print("  Yes  >No");
  }
}

void displayCurrentTime(DateTime now) {
  char dateBuffer[16];
  sprintf(dateBuffer, "%02u:%02u:%02u %02u/%02u/%02u", now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year() % 100);
  lcd.setCursor(0, 0);
  lcd.print(dateBuffer);
}

void checkCountdown(DateTime now) {
  TimeSpan remaining = target - now;

  if (remaining.totalseconds() > 0) {
    // Display the countdown on the second line
    char countDownBuffer[16];
    sprintf(countDownBuffer, "%02ud %02uh %02um %02us", remaining.days(), remaining.hours(), remaining.minutes(), remaining.seconds());
    lcd.setCursor(0, 1);
    lcd.print(countDownBuffer);

    lockBox();

  } else {

    unlockBox();

  }
}

void lockBox() {
    // Turn on red LED
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    // Move servo to lock position
    myServo.write(90);
    delay(20);
    // Turn off blinking LCD cursor   
    lcd.blink_off();
}

void unlockBox() {
    // Turn on green LED
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    // Display 00
    lcd.setCursor(0, 1);
    lcd.print("00d 00h 00m 00s");
    // Move servo to unlock position
    myServo.write(0);
    delay(20); 
    // Turn off blinking LCD cursor   
    lcd.blink_off();
}