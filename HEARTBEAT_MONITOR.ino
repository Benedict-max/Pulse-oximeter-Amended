#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Initialize the LCD with the I2C address 0x27 and dimensions (16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// SoftwareSerial for GSM module (pins 3 and 4)
SoftwareSerial gsmSerial(2, 3);

// Define pins for the buzzer and heartbeat sensors
const int alarmPin = 8; // Buzzer
const int heartRatePin = A0; // Heartbeat sensor 1
const int spO2Pin = A1; // Heartbeat sensor 2 (simulated SpO2)

// Timing variables
unsigned long lastUpdate = 0;
unsigned long lastAlertTime = 0;

// Alert thresholds and debounce time
const int heartRateThreshold = 100;
const unsigned long alertInterval = 60000; // 60 seconds between alerts

// Variables to store sensor readings
int heartRateValue = 0;
int spO2Value = 0;

void setup() {
  Serial.begin(9600);
  gsmSerial.begin(9600);
  
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight

  lcd.setCursor(0, 0);
  lcd.print("SpO2 & HR Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000); // Display the initializing message for 2 seconds

  lcd.clear(); // Clear the LCD screen

  pinMode(alarmPin, OUTPUT); // Set the alarm pin as output
  
  // Simulated setup for heartbeat sensors
  pinMode(heartRatePin, INPUT);
  pinMode(spO2Pin, INPUT);
}

void loop() {
  // Read analog values from the sensors
  heartRateValue = analogRead(heartRatePin);
  spO2Value = analogRead(spO2Pin);

  // Convert sensor values to usable data
  int heartRate = map(heartRateValue, 0, 1023, 60, 120); // Map to a typical heart rate range
  int spO2 = map(spO2Value, 0, 1023, 90, 100); // Map to a typical SpO2 range

  if (millis() - lastUpdate > 500) { // Update every 500 ms
    lastUpdate = millis();

    lcd.setCursor(0, 0);
    lcd.print("SpO2: ");
    lcd.print(spO2);
    lcd.print("%   "); // Add spaces to ensure the display is cleared properly

    lcd.setCursor(0, 1);
    lcd.print("HR: ");
    lcd.print(heartRate);
    lcd.print(" bpm "); // Add spaces to ensure the display is cleared properly

    if (heartRate > heartRateThreshold) {
      lcd.print("Alert!");
      soundAlarm();

      // Send an SMS alert only if enough time has passed since the last alert
      if (millis() - lastAlertTime > alertInterval) {
        sendSMSAlert();
        lastAlertTime = millis();
      }
    } else {
      noTone(alarmPin); // Stop the alarm sound
    }
  }
}

void soundAlarm() {
  tone(alarmPin, 1000); // Sound the alarm (buzzer) at 1000 Hz
  delay(200); // Shorter alarm duration to reduce delay impact
  noTone(alarmPin); // Stop the alarm sound
}

void sendSMSAlert() {
  gsmSerial.println("AT+CMGF=1"); // Set SMS mode to text
  delay(100);
  gsmSerial.println("AT+CMGS=\"+1234567890\""); // Replace with your recipient's phone number
  delay(100);
  gsmSerial.print("Heart rate alert! HR is above 100 bpm."); // SMS message content
  delay(100);
  gsmSerial.println((char)26); // End SMS with Ctrl+Z
  delay(1000); // SMS sending time
}
