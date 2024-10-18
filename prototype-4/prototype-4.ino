#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Encoder.h>
#include <FastLED.h>

// LED Strip setup
FASTLED_USING_NAMESPACE
#define DATA_PIN    6
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    30
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
uint8_t gHue = 0;
int ledStripState = 0;

// LCD and RTC setup
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 RTC;

// lock
const int lockPin = 5;

// Temporary variables for setting new time
int targetDay; // = targetDay;
int targetHour; // = targetHour;
int targetMinute; // = targetMinute;
int targetSeconds; // = targetSeconds;

DateTime target;  // Fixed target DateTime

// Status Leds
const int redLED = 7;
const int greenLED = 8;

// Open lid button
const int lidButton = 10; 
int lidState = 0;

// Rotary-Encoder pins
const int clkPin = 2;
const int dtPin = 3;
const int swPin = 4;

// Rotary Encoder using Encoder library
Encoder myEncoder(clkPin, dtPin);

// Rotary Encoder state
long lastPosition = -999;
int buttonState;

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
  //RTC.adjust(DateTime(__DATE__, __TIME__)); // Resets RTC, leave out to maintain chips time

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(lockPin, OUTPUT);
  pinMode(lidButton, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  // Set default target to 00d 00h 00m 00s from now
  DateTime now = RTC.now();
  target = now + TimeSpan(0, 0, 0, 0);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  DateTime now = RTC.now();

  handleEncoder(now);

  if (setMode == 0) {
    displayCurrentTime(now); // Normal countdown mode
    checkCountdown(now);

  } else if (setMode == 1) {
    setTargetTime();  // Set timer mode

  } else if (setMode == 2) {
    confirmLockBox(); // Confirmation mode: Lock box?
  }
  delay(100);
}

void handleEncoder(DateTime now) {
  long newPosition = myEncoder.read();

  if (newPosition != lastPosition) {

    if (newPosition > lastPosition) {

      if (setMode == 1) {
        adjustTargetTime(-1);

      } else if (setMode == 2) {
        lockSelection = 0;  // "Yes"
      }
    } else if (newPosition < lastPosition) {

      if (setMode == 1) {
        adjustTargetTime(1);

      } else if (setMode == 2) {
        lockSelection = 1;  // "No"
      }
    }
    lastPosition = newPosition;
  }


  if (digitalRead(swPin) == LOW) {
    delay(200); // Button debounce, perhaps not nessecary now using library ...............................................................

    if (setMode == 0) { // Enter set time mode
      setMode = 1;
      selectedField = 0;  // Start by adjusting days
    } else if (setMode == 1) {
      selectedField++; // Move to the next field (days -> hours -> minutes -> seconds)
      if (selectedField > 3) {
        setMode = 2;  // Go to lock confirmation screen
      }
    } else if (setMode == 2) { // Lock confirmation

      if (lockSelection == 0) {
        // "Yes" selected, update the target time with new values
        DateTime now = RTC.now();
        target = now + TimeSpan(targetDay, targetHour, targetMinute, targetSeconds);
        setMode = 0; // Return to normal countdown mode
      } else {
        // "No" selected, go back to time setting mode
        setMode = 1;
        selectedField = 0;
      }
    }
  }
}

void adjustTargetTime(int direction) {
  switch (selectedField) {
    case 0: 
      targetDay += direction;
      if (targetDay < 0) targetDay = 0;  // Prevent negative days
      break;
    case 1: 
      targetHour += direction;
      if (targetHour < 0) targetHour = 0;  // Prevent negative hours
      break;
    case 2: 
      targetMinute += direction;
      if (targetMinute < 0) targetMinute = 0;  // Prevent negative minutes
      break;
    case 3: 
      targetSeconds += direction;
      if (targetSeconds < 0) targetSeconds = 0;  // Prevent negative seconds
      break;
  }
  lcd.blink();
}

void setTargetTime() {
  lcd.clear();
  lcd.print("Set Lock:");
  // Display the time values currently being set
  char timeBuffer[16];
  sprintf(timeBuffer, "%02dd %02dh %02dm %02ds", targetDay, targetHour, targetMinute, targetSeconds);
  lcd.setCursor(0, 1);
  lcd.print(timeBuffer);
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
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);
  lcd.blink_off();
  ledStripOff();
  cycleLock();
}

void unlockBox() {
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  lcd.setCursor(0, 1);
  lcd.print("00d 00h 00m 00s"); 
  lcd.blink_off();
  ledStripOn();
  cycleLock();
}

void ledStripOn() {
  Serial.println("LED strip on");
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  EVERY_N_MILLISECONDS(5) { gHue += 3; };
  FastLED.show();
  ledStripState = 1;
}

void ledStripOff() {
  if (ledStripState == 1) {
    fill_solid(leds, NUM_LEDS, CRGB::Black); 
    FastLED.show();
  }
  ledStripState = 0;
}

void cycleLock() {
  digitalWrite(lockPin, HIGH);
  delay(1000);
  digitalWrite(lockPin, LOW);  
}