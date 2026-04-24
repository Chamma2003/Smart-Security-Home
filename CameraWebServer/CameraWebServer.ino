#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h> 

// WIFI
const char* ssid = "Sena";
const char* password = "23456789";

// TELEGRAM
String BOTtoken = "8719766757:AAFbmF5eEQsluE50bxEKjVmTwslLhiW66fo";  
String CHAT_ID = "8222943690";

// SERVER
String serverURL = "http://10.217.169.205:3000/update"; 

// TRIGGER
#define TRIGGER_PIN 13

// CAMERA MODEL
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER_PIN, INPUT);

  // CAMERA CONFIG
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 25;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QQVGA);

  // WIFI
  WiFi.begin(ssid, password);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
}


//SEND DATA TO SERVER

void sendToServer() {

  HTTPClient http;

  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  // YOU CAN UPDATE THESE VALUES LATER
  String json = "{";
  json += "\"temp\":30,";
  json += "\"hum\":70,";
  json += "\"motion\":\"DETECTED\",";
  json += "\"gas\":\"SAFE\",";
  json += "\"fire\":\"SAFE\"";
  json += "}";

  int response = http.POST(json);

  Serial.print("Server response: ");
  Serial.println(response);

  http.end();
}


// SEND PHOTO 

void sendPhotoTelegram() {

  WiFiClientSecure client;
  client.setInsecure();

  Serial.println("Connecting to Telegram...");

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Telegram failed");
    return;
  }

  delay(1500);

  camera_fb_t * fb = NULL;

  for (int i = 0; i < 3; i++) {
    fb = esp_camera_fb_get();
    if (fb) {
      Serial.println("Capture success!");
      break;
    }
    Serial.println("Retry capture...");
    delay(500);
  }

  if (!fb) {
    Serial.println("Capture failed after retries");
    return;
  }

  String boundary = "----ESP32";

  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  head += CHAT_ID + "\r\n";

  head += "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"photo\"; filename=\"pic.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";

  uint32_t totalLen = head.length() + fb->len + tail.length();

  client.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Content-Length: " + String(totalLen));
  client.println();

  client.print(head);
  client.write(fb->buf, fb->len);
  client.print(tail);

  esp_camera_fb_return(fb);

  Serial.println("✅ Photo sent!");
}


// LOOP

void loop() {

  if (digitalRead(TRIGGER_PIN) == HIGH) {
    Serial.println("📷 Motion detected");

    sendPhotoTelegram();   
    sendToServer();       

    delay(15000);
  }
}