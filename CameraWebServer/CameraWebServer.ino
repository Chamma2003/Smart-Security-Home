#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// -------- CAMERA --------
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// -------- WIFI --------
const char* ssid = "Redmi 9T";
const char* password = "12345678";

// -------- TELEGRAM --------
String BOTtoken = "8719766757:AAGLo9OqcuptkfLNQuBE4xa2Hd4PCiPP_KM";
String CHAT_ID = "8222943690";

// -------- TRIGGER --------
#define TRIGGER_PIN 13

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);

  pinMode(TRIGGER_PIN, INPUT);

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

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  WiFi.begin(ssid, password);
  client.setInsecure();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
}

bool sendPhotoTelegram() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) return false;

  Serial.println("Sending photo to Telegram...");

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Connection failed");
    return false;
  }

  String head = "--123\r\nContent-Disposition: form-data; name=\"chat_id\";\r\n\r\n" + CHAT_ID + "\r\n--123\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"image.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--123--\r\n";

  uint32_t imageLen = fb->len;
  uint32_t totalLen = imageLen + head.length() + tail.length();

  client.println("POST /bot" + BOTtoken + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Length: " + String(totalLen));
  client.println("Content-Type: multipart/form-data; boundary=123");
  client.println();

  client.print(head);

  client.write(fb->buf, fb->len);

  client.print(tail);

  esp_camera_fb_return(fb);

  Serial.println("Photo sent!");
  return true;
}

bool captured = false;

void loop() {
  if (digitalRead(TRIGGER_PIN) == HIGH && !captured) {
    sendPhotoTelegram();
    captured = true;
  }

  if (digitalRead(TRIGGER_PIN) == LOW) {
    captured = false;
  }
}