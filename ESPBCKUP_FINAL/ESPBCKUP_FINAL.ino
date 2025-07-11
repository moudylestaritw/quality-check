#define BLYNK_TEMPLATE_ID "TMPL6FBZJn09A"
#define BLYNK_TEMPLATE_NAME "QualityCheck"
#define BLYNK_AUTH_TOKEN "Mr-UeUQ_cy7j1Ud0PAWP4hT3fKZB2tft"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include "esp_task_wdt.h"

char ssid[] = "DESKTOP";
char pass[] = "809^36dN";

// Pin Konfigurasi
#define HEARTBEAT_PIN 35      // Mendeteksi keberadaan MAIN
#define DEFECT_INPUT_PIN 4    // Input dari ESP32-CAM
#define SERVO_PIN 18          // Servo SG90

Servo SG90;
BlynkTimer timer;

// ✅ Fungsi: Reset watchdog secara rutin
void resetWDT() {
  esp_task_wdt_reset();
}

// ✅ Fungsi: Cek defect dan gerakkan servo
void checkDefectAndMoveServo() {
  bool defectSignal = digitalRead(DEFECT_INPUT_PIN);

  if (defectSignal == HIGH) {
    SG90.write(115);
    Serial.println("Defect detected");
    Blynk.virtualWrite(V3, "⚠️ Damaged");
  } else if (defectSignal == LOW) {
    SG90.write(180);
    Serial.println("Produk OK");
    Blynk.virtualWrite(V3, "✅ Intact");
  }
}

void checkWiFiStatus() {
  Blynk.virtualWrite(V7, WiFi.status() == WL_CONNECTED ? 1 : 0);
  IPAddress ip = WiFi.localIP();
  String ipStr = ip.toString();
  Blynk.virtualWrite(V5, ipStr);
}

void updateESPStatus() {
  Blynk.virtualWrite(V0, 0); // MAIN OFF
  Blynk.virtualWrite(V1, 1); // BACKUP ON
}

void goToSleep() {
  Serial.println("ESP Backup: Masuk deep sleep...");
  delay(500);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)HEARTBEAT_PIN, 0); // Wakeup saat heartbeat LOW
  esp_deep_sleep_start();
}

void startTakeover() {
  Serial.println("ESP Backup: Melakukan takeover...");

  // ✅ Setup WiFi dan Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  SG90.setPeriodHertz(50);
  SG90.attach(SERVO_PIN);
  SG90.write(180);

  // ✅ Timer event
  timer.setInterval(500L, checkDefectAndMoveServo);
  timer.setInterval(3000L, checkWiFiStatus);
  timer.setInterval(7000L, updateESPStatus);
  timer.setInterval(2000L, resetWDT); 

  esp_task_wdt_deinit();  // pastikan watchdog tidak sedang aktif
  const esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 6000,            // timeout 6 detik
    .idle_core_mask = 0,           // JANGAN monitor IDLE0 dan IDLE1
    .trigger_panic = true          // supaya reboot jika macet
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);          // Tambahkan loopTask saat ini

}

void setup() {
  Serial.begin(115200);
  pinMode(HEARTBEAT_PIN, INPUT_PULLDOWN);
  pinMode(DEFECT_INPUT_PIN, INPUT_PULLDOWN);

  esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();

  if (wakeReason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("ESP Backup: Wake dari sleep - Cek heartbeat...");
    delay(300);

    int highCount = 0;
    for (int i = 0; i < 10; i++) {
      if (digitalRead(HEARTBEAT_PIN) == HIGH) highCount++;
      delay(50);
    }

    if (highCount >= 8) {
      Serial.println("ESP Backup: MAIN masih hidup, kembali tidur.");
      goToSleep();
    }

    Serial.println("ESP Backup: MAIN gagal, takeover dimulai.");
    startTakeover();
  } else {
    Serial.println("ESP Backup: Boot pertama, masuk sleep.");
    delay(1000);
    goToSleep();
  }
}

void loop() {
  if (digitalRead(HEARTBEAT_PIN) == HIGH) {
    Serial.println("ESP Backup: MAIN aktif kembali, tidur...");
    goToSleep();
  }

  Blynk.run();
  timer.run();
}
