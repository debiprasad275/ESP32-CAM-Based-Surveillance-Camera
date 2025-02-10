// Include required libraries
#include <WiFi.h>
#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>
#include "esp_camera.h"

// Replace with your Wi-Fi and Blynk credentials
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_password"
#define BLYNK_AUTH "your_blynk_auth_token"

// PIR Sensor and Buzzer
#define PIR_SENSOR_PIN 13
#define BUZZER_PIN 12

// Global variables
volatile bool motionDetected = false;

// Blynk Notification Users (Parent & Child)
#define PARENT_USER "parent_email@example.com"
#define CHILD_USER "child_email@example.com"

// Function to initialize the camera
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = -1;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
  }
}

// Capture an image and send notification
void captureAndNotify() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.println("Motion detected! Sending notification...");
  Blynk.logEvent("motion_alert", "Motion detected! Check the camera feed.");
  Blynk.email(PARENT_USER, "Security Alert", "Motion detected at home!");
  Blynk.email(CHILD_USER, "Security Alert", "Motion detected at home!");

  esp_camera_fb_return(fb);
}

// PIR Sensor interrupt handler
void IRAM_ATTR detectMotion() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
  
  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH, WIFI_SSID, WIFI_PASS);
  
  // Initialize Camera
  initCamera();
  
  // Setup PIR Sensor and Buzzer
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_PIN), detectMotion, RISING);
}

void loop() {
  Blynk.run();
  
  if (motionDetected) {
    digitalWrite(BUZZER_PIN, HIGH);  // Sound the buzzer
    captureAndNotify();
    delay(5000); // Prevent multiple triggers
    digitalWrite(BUZZER_PIN, LOW);
    motionDetected = false;
  }
}