# 🤖 QR-Based Automated Guided Vehicle (AGV)

A low-cost AGV that reads QR commands using an **AI-Thinker ESP32-CAM** and sends them to a motor-control **ESP32** through ESP-NOW. An **L298N** drives the motors, while an **IR sensor** detects black tape to stop at marked points.

## ✨ Features
- 📷 QR-code detection with ESP32-CAM
- 📡 Wireless commands using ESP-NOW
- 🚗 Motor control using ESP32 DevKit and L298N
- 🛑 IR-based black-line stopping
- 🎯 Commands: `GO`, `LEFT`, `RIGHT`, `BACK`, `STOP`

## 🧩 System Architecture

```text
QR Marker
   ↓
ESP32-CAM (QR detection)
   ↓
Command validation
   ↓ ESP-NOW
ESP32 DevKit ← IR Sensor (black-line detection)
   ↓
L298N Motor Driver
   ↓
DC Motors → AGV movement
```

## 📁 Project Folder Structure

```text
QR-Based-AGV/
|
├── esp32_cam_qr/
│     └── esp32_cam_qr.ino
|
├── esp32_motor_controller/
│     └── esp32_motor_controller.ino
|
├── demo/
|     └── demovideo.mp4
|     
└── README.md              
```

## 🔧 Hardware & Software
- **Hardware:** ESP32-CAM, ESP32 DevKit V1, L298N, DC motors, chassis, IR sensor, battery.
- **Software:** Arduino IDE, ESP32 board package, `ESP32QRCodeReader` library.

## ⚙️ How It Works
1. ESP32-CAM scans and decodes a QR marker.
2. The command is sent to the motor ESP32 via ESP-NOW.
3. The motor ESP32 drives the motors through the L298N.
4. The IR sensor detects the black line and triggers stopping, based on the motor firmware.

## 📡 Communication
Motor ESP32 MAC used in the current camera-code version: `88:57:21:B7:7F:B0`.

Both boards must use compatible ESP-NOW channel settings. Update the MAC address if your motor board differs.

## 🧪 Setup
1. Install Arduino IDE, ESP32 board support, and `ESP32QRCodeReader`.
2. Upload the QR/ESP-NOW firmware to the ESP32-CAM.
3. Upload the motor-control/IR firmware to the ESP32 DevKit.
4. Check the MAC address and ESP-NOW channel on both boards.
5. Test with the wheels lifted before running the AGV.

## 🛠️ Troubleshooting
- **QR not detected:** Check camera connection, lighting, and QR size.
- **ESP-NOW fails:** Check MAC address, peer setup, and Wi-Fi channel.
- **Motors do not move:** Check L298N wiring, motor supply, enable pins, and ground.
- **IR does not stop the car:** Check sensor output pin and active logic level.

## 🚀 Future Improvements
- Add wheel encoders for accurate movement.
- Add obstacle detection and avoidance.
- Add command acknowledgements and status display.



