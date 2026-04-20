#include <Servo.h>

// --- Pins ---
const int entryBtn = 2;
const int exitBtn = 3;
const int ldrPin = A0;
const int rainPin = A1;
const int ledPin = 10;
const int servoPin = 9;

// --- OPTIMUM VALUES (Based on your data) ---
const int darkThreshold = 500;  // Room is ~170, so 500 is a safe "Dark" trigger
const int rainThreshold = 500;  // Dry is 1019, so below 500 is definitely "Wet"
const int hourlyRate = 20;
const int secondsPerHour = 10;

// --- Multi-Car Storage ---
const int MAX_SLOTS = 5;
int carIDs[MAX_SLOTS] = {0};
unsigned long entryTimes[MAX_SLOTS] = {0};

Servo gate;
unsigned long lastDisplayTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(10000);

  pinMode(entryBtn, INPUT_PULLUP);
  pinMode(exitBtn, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  gate.attach(servoPin);
  gate.write(0);

  randomSeed(analogRead(A5));

  Serial.println("=====================================");
  Serial.println("SECURE PARKING SYSTEM: pes1ug24am921");
  Serial.println("SYSTEM STATUS: ONLINE & CALIBRATED");
  Serial.println("=====================================");
}

void loop() {
  int ldrVal = analogRead(ldrPin);
  int rainVal = analogRead(rainPin);

  // --- 1. Weather Update (Every 5 Seconds) ---
  if (millis() - lastDisplayTime > 5000) {
    Serial.println("-------------------------");
    Serial.print("ID: pes1ug24am921 | ");
    if (rainVal < rainThreshold) Serial.println("Weather: Rain! Drive Safe");
    else Serial.println("Weather: Clear");
    lastDisplayTime = millis();
  }

  // --- 2. Streetlight Control ---
  if (ldrVal > darkThreshold) digitalWrite(ledPin, HIGH);
  else digitalWrite(ledPin, LOW);

  // --- 3. ENTRY LOGIC ---
  if (digitalRead(entryBtn) == LOW) {
    delay(200); // Debounce
    int slot = -1;
    for (int i = 0; i < MAX_SLOTS; i++) {
      if (carIDs[i] == 0) { slot = i; break; }
    }

    if (slot != -1) {
      carIDs[slot] = random(10, 99);
      entryTimes[slot] = millis();

      Serial.println("\n[NEW ENTRY]");
      Serial.print("TICKET ID: "); Serial.println(carIDs[slot]);
      Serial.println("ACTION: Gate Opening...");
      gate.write(90); delay(2000); gate.write(0);
    } else {
      Serial.println("\n![ALERT] PARKING FULL");
    }
    while (digitalRead(entryBtn) == LOW);
  }

  // --- 4. EXIT LOGIC ---
  if (digitalRead(exitBtn) == LOW) {
    delay(200);
    while (Serial.available() > 0) Serial.read(); // Clear buffer

    Serial.println("\n[EXIT REQUESTED]");
    Serial.println("ACTION: Type your Ticket ID and press ENTER:");

    while (Serial.available() == 0) { delay(10); } // Wait for typing

    int inputID = Serial.parseInt();
    int carIndex = -1;

    for (int i = 0; i < MAX_SLOTS; i++) {
      if (carIDs[i] == inputID && inputID != 0) { carIndex = i; break; }
    }

    if (carIndex != -1) {
      unsigned long duration = (millis() - entryTimes[carIndex]) / 1000;
      int hours = (duration / secondsPerHour);
      if (hours < 1) hours = 1;

      Serial.println("\n--- RECEIPT ---");
      Serial.print("STAY TIME: "); Serial.print(duration); Serial.println("s");
      Serial.print("FEE: Rs "); Serial.println(hours * hourlyRate);

      gate.write(90); delay(3000); gate.write(0);
      carIDs[carIndex] = 0;
      Serial.println("STATUS: SLOT CLEARED\n");
    } else {
      Serial.println("![ERROR] INVALID ID");
    }
    while (digitalRead(exitBtn) == LOW);
  }
}
