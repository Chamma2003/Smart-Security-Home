#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT
#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Fingerprint
SoftwareSerial mySerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Pins
int relayPin = 7;
int pirPin = 4;
int flamePin = 5;
int gasPin = 6;
int buzzer = 8;
int camTrigger = 9;
int ledRelay = 3;

//  Flags
bool gasTriggered = false;
bool pirTriggered = false;
bool flameTriggered = false;
unsigned long ledTimer = 0;
bool ledOn = false;

//  Status
String statusMsg = "System Ready";

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  dht.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(pirPin, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(gasPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(camTrigger, OUTPUT);
  pinMode(ledRelay, OUTPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(buzzer, LOW);
  digitalWrite(camTrigger, LOW);
  digitalWrite(ledRelay, LOW);

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint Ready");
    statusMsg = "Fingerprint OK";
  } else {
    Serial.println("Sensor NOT found");
    while (1);
  }
}

void loop() {

  // READ DHT
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // PIR 
  int pirState = digitalRead(pirPin);

  if (pirState == HIGH && !pirTriggered) {
    Serial.println("🚨 Motion Detected!");
    statusMsg = "Motion Detected";

    //LED ON
    digitalWrite(ledRelay, HIGH);
    ledTimer = millis();
    ledOn = true;

    // Camera trigger
    digitalWrite(camTrigger, HIGH);
    delay(500);
    digitalWrite(camTrigger, LOW);

    alarm();
    pirTriggered = true;
  }

  // RESET PIR FLAG
  if (pirState == LOW) {
    pirTriggered = false;
  }

  // AUTO OFF LED AFTER 6 SEC
  if (ledOn && (millis() - ledTimer >= 6000)) {
    digitalWrite(ledRelay, LOW);
    ledOn = false;
    statusMsg = "System Ready";
  }

  // FLAME
  int flameState = digitalRead(flamePin);

  if (flameState == LOW && !flameTriggered) {
    Serial.println("🔥 Fire Detected!");
    statusMsg = "Fire Detected";
    alarm();
    flameTriggered = true;
  }

  if (flameState == HIGH) flameTriggered = false;

  // GAS
  int gasState = digitalRead(gasPin);

  if (gasState == HIGH && !gasTriggered) {
    Serial.println("💨 Gas Detected!");
    statusMsg = "Gas Detected";
    alarm();
    gasTriggered = true;
  }

  if (gasState == LOW) gasTriggered = false;

  // FINGERPRINT
  int id = getFingerprintID();

  if (id != -1) {
    Serial.println("✅ Access Granted");
    statusMsg = "Access Granted";

    digitalWrite(relayPin, HIGH);
    delay(5000);
    digitalWrite(relayPin, LOW);

    statusMsg = "System Ready";
  }

  // OLED DISPLAY
  display.clearDisplay();

  display.setCursor(0, 0);
  display.print("Tempureture: ");
  display.println(temp);
  

  display.print("Humidity: ");
  display.print(hum);
  display.println("%");

  display.setCursor(20, 20);
  display.println(statusMsg);

  display.display();

  delay(200);
}

// ALARM 
void alarm() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    delay(200);
  }
}

// FINGERPRINT
int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}