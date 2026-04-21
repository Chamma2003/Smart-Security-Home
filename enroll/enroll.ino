#include <DHT.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

// ---------------- DHT ----------------
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------------- PINS ----------------
int pirPin = 2;
int flamePin = 3;
int gasPin = 4;
int relayPin = 7;
int buzzer = 8;
int camTrigger = 9;

// ---------------- FINGERPRINT ----------------
SoftwareSerial mySerial(10, 11); // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);

  pinMode(pirPin, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(gasPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(camTrigger, OUTPUT);

  digitalWrite(relayPin, HIGH); // LOCKED
  digitalWrite(buzzer, LOW);
  digitalWrite(camTrigger, LOW);

  dht.begin();

  // Fingerprint init
  mySerial.begin(57600);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint Ready");
  } else {
    Serial.println("Fingerprint NOT found");
  }
}

// ---------------- FINGERPRINT FUNCTION ----------------
int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

// ---------------- MAIN LOOP ----------------
void loop() {

  int motion = digitalRead(pirPin);
  int fire = digitalRead(flamePin);
  int gas = digitalRead(gasPin);

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.println("-----------");
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("C  Hum: ");
  Serial.println(hum);

  // 🔥 FIRE
  if (fire == LOW) {
    Serial.println("🔥 FIRE ALERT!");
    digitalWrite(buzzer, HIGH);
    digitalWrite(relayPin, LOW);
  }

  // 💨 GAS
  else if (gas == LOW) {
    Serial.println("💨 GAS LEAK!");
    digitalWrite(buzzer, HIGH);
    digitalWrite(relayPin, LOW);
  }

  // 👀 MOTION → CAMERA
  else if (motion == HIGH) {
    Serial.println("🚨 INTRUDER!");

    digitalWrite(buzzer, HIGH);
    digitalWrite(relayPin, LOW);

    digitalWrite(camTrigger, HIGH);  // trigger ESP32
    delay(500);
    digitalWrite(camTrigger, LOW);
  }

  // 🔐 FINGERPRINT
  else {
    digitalWrite(buzzer, LOW);

    Serial.println("Place Finger...");
    int id = getFingerprintID();

    if (id > 0) {
      Serial.println("Access Granted");

      digitalWrite(relayPin, LOW);   // unlock
      delay(10000);                  // keep unlocked
      digitalWrite(relayPin, HIGH);  // lock again
    } 
    else {
      Serial.println("Access Denied");
      digitalWrite(relayPin, HIGH);
    }
  }

  delay(500);
}