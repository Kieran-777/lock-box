// Pin definitions
const int clkPin = 2;
const int dtPin = 3;
const int swPin = 4;

int counter = 0; // To store the encoder position
int currentStateClk;
int lastStateClk;
int buttonState;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 5; // 5 milliseconds debounce time

void setup() {
  // Set pins as input
  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP); // Button with pull-up
  
  // Initialize serial communication
  Serial.begin(9600);

  // Read the initial state of the CLK
  lastStateClk = digitalRead(clkPin);
}

void loop() {
  // Read the current state of the CLK
  currentStateClk = digitalRead(clkPin);

  // Only act if the state has changed (rotation detected)
  if (currentStateClk != lastStateClk) {
    // Debouncing - add a small delay to avoid noise
    if (millis() - lastDebounceTime > debounceDelay) {
      // Check the DT pin to determine direction
      if (digitalRead(dtPin) != currentStateClk) {
        counter++;
      } else {
        counter--;
      }

      // Print the current counter value
      Serial.print("Counter: ");
      Serial.println(counter);

      // Update debounce time
      lastDebounceTime = millis();
    }
  }

  // Update lastStateClk
  lastStateClk = currentStateClk;

  // Read the state of the button (SW pin)
  buttonState = digitalRead(swPin);
  
  // If button is pressed (LOW state), print a message
  if (buttonState == LOW) {
    Serial.println("Button Pressed!");
    delay(200); // Debounce delay for button
  }
}
