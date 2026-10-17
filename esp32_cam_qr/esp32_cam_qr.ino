#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <ESP32QRCodeReader.h>

// ===============================
// ESP32-CAM QR + ESP-NOW
// ===============================

// Motor ESP32 MAC address (chnages for each esp32)
uint8_t motorESP32MAC[] = {
  0x88, 0x57, 0x21, 0xB7, 0x7F, 0xB0
};

// QR reader
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

struct QRCodeData qrCodeData;

// Command cooldown
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_COOLDOWN = 2000;

// ===============================
// SEND COMMAND
// ===============================

void sendCommand(String command)
{
  command.trim();
  command.toUpperCase();

  // Valid commands only
  if (
    command != "GO" &&
    command != "FORWARD" &&
    command != "LEFT" &&
    command != "RIGHT" &&
    command != "BACK" &&
    command != "BACKWARD" &&
    command != "STOP"
  )
  {
    Serial.print("Invalid command: ");
    Serial.println(command);
    return;
  }

  // Cooldown
  if (millis() - lastCommandTime < COMMAND_COOLDOWN)
  {
    return;
  }

  esp_err_t result = esp_now_send(
    motorESP32MAC,
    (uint8_t *)command.c_str(),
    command.length() + 1
  );

  if (result == ESP_OK)
  {
    Serial.print("ESP-NOW SENT: ");
    Serial.println(command);

    lastCommandTime = millis();
  }
  else
  {
    Serial.print("ESP-NOW send failed: ");
    Serial.println(result);
  }
}

// ===============================
// QR TASK
// ===============================

void qrCodeTask(void *pvParameters)
{
  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("QR detected");

      if (qrCodeData.valid)
      {
        String payload =
          String((const char *)qrCodeData.payload);

        payload.trim();
        payload.toUpperCase();

        Serial.print("QR DATA: ");
        Serial.println(payload);

        sendCommand(payload);
      }
      else
      {
        Serial.println("Invalid QR");
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ===============================
// SETUP
// ===============================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32-CAM QR + ESP-NOW");
  Serial.println("==============================");

  // -------------------------------
  // WiFi
  // -------------------------------

  WiFi.mode(WIFI_STA);

  // Force channel 1
  esp_wifi_set_channel(
    1,
    WIFI_SECOND_CHAN_NONE
  );

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  // -------------------------------
  // ESP-NOW
  // -------------------------------

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW INIT FAILED");
    return;
  }

  Serial.println("ESP-NOW initialized");

  // -------------------------------
  // Add motor ESP32 as peer
  // -------------------------------

  esp_now_peer_info_t peerInfo = {};

  memcpy(
    peerInfo.peer_addr,
    motorESP32MAC,
    6
  );

  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) == ESP_OK)
  {
    Serial.println("Motor ESP32 peer added");
  }
  else
  {
    Serial.println("Failed to add peer");
  }

  // -------------------------------
  // QR reader
  // -------------------------------

  reader.setup();

  Serial.println("QR reader setup complete");

  reader.beginOnCore(1);

  xTaskCreate(
    qrCodeTask,
    "QRCodeTask",
    4 * 1024,
    NULL,
    1,
    NULL
  );

  Serial.println();
  Serial.println("==============================");
  Serial.println("READY");
  Serial.println("==============================");
}

// ===============================
// LOOP
// ===============================

void loop()
{
  delay(100);
}
