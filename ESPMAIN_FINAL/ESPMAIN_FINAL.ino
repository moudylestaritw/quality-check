#define BLYNK_TEMPLATE_ID "TMPL6FBZJn09A"
#define BLYNK_TEMPLATE_NAME "QualityCheck"
#define BLYNK_AUTH_TOKEN "Mr-UeUQ_cy7j1Ud0PAWP4hT3fKZB2tft"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

char ssid[] = "DESKTOP";
char pass[] = "809^36dN";

// ✅ PIN konfigurasi
#define DEFECT_INPUT_PIN 4       // Sinyal dari ESP32-CAM
#define SERVO_PIN 18             // Gunakan pin PWM: 13, 18, 19, 21 dst.
#define HEARTBEAT_PIN 2          // Output heartbeat ke ESP Backup

Servo SG90;
BlynkTimer timer;

void checkWiFiStatus() { // Wifi Status ke Blynk
  Blynk.virtualWrite(V4, WiFi.status() == WL_CONNECTED ? 1 : 0);
  IPAddress ip = WiFi.localIP();
  String ipStr = ip.toString();
  Blynk.virtualWrite(V2, ipStr);
}

void checkDefectAndMoveServo() { // Detect + Gerak Servo
  bool defectSignal = digitalRead(DEFECT_INPUT_PIN);

  if (defectSignal == HIGH) {
    SG90.write(115);
    delay(1000);
    Serial.println("⚠️ Defect detected");
    Blynk.virtualWrite(V3, "⚠️ Damaged");
  } 
  else if (defectSignal == LOW) {
    SG90.write(180);
    Serial.println("✅ Produk OK");
    Blynk.virtualWrite(V3, "✅ Intact");
  }
}

void setup() {
  Serial.begin(115200); // Debug Serial Monitor
  // ✅ Pin setup Defect + Heartbeat
  pinMode(DEFECT_INPUT_PIN, INPUT_PULLDOWN);
  pinMode(HEARTBEAT_PIN, OUTPUT);
  digitalWrite(HEARTBEAT_PIN, HIGH); // Heartbeat aktif untuk Backup

  // ✅ Setup Servo SG90
  SG90.setPeriodHertz(50);  // 50Hz PWM untuk servo
  SG90.attach(SERVO_PIN);
  SG90.write(180);  // Posisi awal

  // ✅ WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V0, 1); // MAIN aktif
  Blynk.virtualWrite(V1, 0); // BACKUP nonaktif

  // ✅ Timer setup
  timer.setInterval(500L, checkDefectAndMoveServo);
  timer.setInterval(3000L, checkWiFiStatus);
}

void loop() {
  Blynk.run();
  timer.run();
}
