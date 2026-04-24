#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// -------- Pins --------
int relayPin = 7;
int pirPin = 4;
int flamePin = 5;
int gasPin = 6;
int buzzer = 8;
int camTrigger = 9;

// -------- Flags --------
bool gasTriggered = false;
bool pirTriggered = false;
bool flameTriggered = false;

void setup() {
  Serial.begin(9600);
  finger.begin(57600);

  pinMode(relayPin, OUTPUT);
  pinMode(pirPin, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(gasPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(camTrigger, OUTPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(buzzer, LOW);
  digitalWrite(camTrigger, LOW);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint Ready");
  } else {
    Serial.println("Sensor NOT found");
    while (1);
  }
}

void loop() {

  // -------- PIR --------
  int pirState = digitalRead(pirPin);

  if (pirState == HIGH && pirTriggered == false) {
    Serial.println("🚨 Motion Detected!");

    digitalWrite(camTrigger, HIGH); // trigger camera
    delay(1000);
    digitalWrite(camTrigger, LOW);

    alarm();
    pirTriggered = true;
  }

  if (pirState == LOW) {
    pirTriggered = false;
  }

  // -------- FLAME --------
  int flameState = digitalRead(flamePin);

  if (flameState == LOW && flameTriggered == false) {
    Serial.println("🔥 Fire Detected!");
    alarm();
    flameTriggered = true;
  }

  if (flameState == HIGH) {
    flameTriggered = false;
  }

  // -------- GAS --------
  int gasState = digitalRead(gasPin);

  if (gasState == HIGH && gasTriggered == false) {
    Serial.println("💨 Gas Detected!");
    alarm();
    gasTriggered = true;
  }

  if (gasState == LOW) {
    gasTriggered = false;
  }

  // -------- FINGERPRINT --------
  int id = getFingerprintID();

  if (id != -1) {
    Serial.println("✅ Access Granted");

    digitalWrite(relayPin, HIGH);
    delay(5000);
    digitalWrite(relayPin, LOW);
  }

  delay(200);
}

// -------- ALARM --------
void alarm() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(100);
  }
}

// -------- FINGERPRINT --------
int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}