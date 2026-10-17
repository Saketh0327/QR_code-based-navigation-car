#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// ============================================================
// L298N MOTOR PINS
// ============================================================

// LEFT MOTOR
const int ENA = 25;
const int IN1 = 26;
const int IN2 = 27;

// RIGHT MOTOR
const int ENB = 13;
const int IN3 = 14;
const int IN4 = 12;

// ============================================================
// SINGLE IR SENSOR
// ============================================================

const int IR_SENSOR = 34;

// Most IR modules:
// LOW = black detected
const int BLACK_STATE = LOW;

// ============================================================
// MOTOR SPEEDS
// ============================================================

// Separate speeds for each wheel
const int LEFT_FORWARD_SPEED  = 150;
const int RIGHT_FORWARD_SPEED = 180;

const int LEFT_TURN_SPEED  = 165;
const int RIGHT_TURN_SPEED = 180;

// ============================================================
// 90 DEGREE TURN TIMES
// ============================================================

const unsigned long TURN_RIGHT_90_MS = 500;
const unsigned long TURN_LEFT_90_MS  = 500;

// ============================================================
// IR DEBOUNCE
// ============================================================

const unsigned long LINE_DEBOUNCE_MS = 40;

// ============================================================
// ROBOT STATES
// ============================================================

enum RobotState
{
  WAITING_FOR_COMMAND,
  MOVING_TO_LINE,
  TURNING_LEFT,
  TURNING_RIGHT,
  HALTED
};

RobotState state = WAITING_FOR_COMMAND;

// ============================================================
// STATE VARIABLES
// ============================================================

unsigned long turnStartTime = 0;

bool lineDetectionArmed = false;

bool blackStarted = false;
unsigned long blackStartTime = 0;

// ============================================================
// MOTOR FUNCTIONS
// ============================================================

void stopMotors()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void moveForward()
{
  // LEFT MOTOR
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // RIGHT MOTOR
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(
    ENA,
    LEFT_FORWARD_SPEED
  );

  analogWrite(
    ENB,
    RIGHT_FORWARD_SPEED
  );
}

void moveBackward()
{
  // LEFT MOTOR
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // RIGHT MOTOR
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(
    ENA,
    LEFT_FORWARD_SPEED
  );

  analogWrite(
    ENB,
    RIGHT_FORWARD_SPEED
  );
}

void turnRight()
{
  // LEFT MOTOR -> FORWARD
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // RIGHT MOTOR -> BACKWARD
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(
    ENA,
    LEFT_TURN_SPEED
  );

  analogWrite(
    ENB,
    RIGHT_TURN_SPEED
  );
}

void turnLeft()
{
  // LEFT MOTOR -> BACKWARD
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  // RIGHT MOTOR -> FORWARD
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(
    ENA,
    LEFT_TURN_SPEED
  );

  analogWrite(
    ENB,
    RIGHT_TURN_SPEED
  );
}

// ============================================================
// IR
// ============================================================

bool isBlack()
{
  return digitalRead(IR_SENSOR) == BLACK_STATE;
}

bool confirmedBlack()
{
  bool blackNow = isBlack();

  if (!blackNow)
  {
    blackStarted = false;
    return false;
  }

  if (!blackStarted)
  {
    blackStarted = true;
    blackStartTime = millis();
  }

  if (
    millis() - blackStartTime
    >= LINE_DEBOUNCE_MS
  )
  {
    return true;
  }

  return false;
}

// ============================================================
// PROCESS QR COMMAND
// ============================================================

void processCommand(String command)
{
  command.trim();
  command.toUpperCase();

  Serial.print("QR COMMAND: ");
  Serial.println(command);

  // ==========================================================
  // STOP
  // ==========================================================

  if (command == "STOP")
  {
    stopMotors();

    state = HALTED;

    Serial.println(
      "STOP -> Motors stopped"
    );

    return;
  }

  // ==========================================================
  // IGNORE NEW MOVEMENT COMMANDS WHILE MOVING
  // ==========================================================

  if (
    state != WAITING_FOR_COMMAND &&
    state != HALTED
  )
  {
    Serial.println(
      "Ignoring command - car is already executing."
    );

    return;
  }

  // ==========================================================
  // GO / FORWARD
  // ==========================================================

  if (
    command == "GO" ||
    command == "FORWARD"
  )
  {
    Serial.println();
    Serial.println(
      "GO -> Moving until BLACK TAPE"
    );

    moveForward();

    state = MOVING_TO_LINE;

    lineDetectionArmed = false;
    blackStarted = false;

    return;
  }

  // ==========================================================
  // BACK
  // ==========================================================

  if (
    command == "BACK" ||
    command == "BACKWARD"
  )
  {
    Serial.println();
    Serial.println(
      "BACK -> Moving backward"
    );

    moveBackward();

    state = MOVING_TO_LINE;

    lineDetectionArmed = false;
    blackStarted = false;

    return;
  }

  // ==========================================================
  // LEFT
  // ==========================================================

  if (command == "LEFT")
  {
    Serial.println();
    Serial.println(
      "LEFT -> Turning 90 degrees"
    );

    turnLeft();

    state = TURNING_LEFT;

    turnStartTime = millis();

    lineDetectionArmed = false;
    blackStarted = false;

    return;
  }

  // ==========================================================
  // RIGHT
  // ==========================================================

  if (command == "RIGHT")
  {
    Serial.println();
    Serial.println(
      "RIGHT -> Turning 90 degrees"
    );

    turnRight();

    state = TURNING_RIGHT;

    turnStartTime = millis();

    lineDetectionArmed = false;
    blackStarted = false;

    return;
  }

  Serial.println(
    "Unknown QR command"
  );
}

// ============================================================
// ESP-NOW RECEIVE CALLBACK
// ============================================================

void onDataRecv(
  const esp_now_recv_info_t *info,
  const uint8_t *data,
  int len
)
{
  if (len <= 0)
    return;

  char received[32];

  int copyLength =
    min(
      len,
      (int)sizeof(received) - 1
    );

  memcpy(
    received,
    data,
    copyLength
  );

  received[copyLength] = '\0';

  processCommand(
    String(received)
  );
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // ==========================================================
  // MOTOR PINS
  // ==========================================================

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // ==========================================================
  // IR SENSOR
  // ==========================================================

  pinMode(
    IR_SENSOR,
    INPUT
  );

  // SAFETY
  stopMotors();

  // ==========================================================
  // WIFI / ESP-NOW
  // ==========================================================

  WiFi.mode(WIFI_STA);

  // Same channel as ESP32-CAM
  esp_wifi_set_channel(
    1,
    WIFI_SECOND_CHAN_NONE
  );

  Serial.println();
  Serial.println(
    "================================"
  );
  Serial.println(
    "QR CAR MOTOR ESP32"
  );
  Serial.println(
    "================================"
  );

  Serial.print(
    "Motor ESP32 MAC: "
  );

  Serial.println(
    WiFi.macAddress()
  );

  if (
    esp_now_init() != ESP_OK
  )
  {
    Serial.println(
      "ESP-NOW initialization FAILED!"
    );

    while (true)
    {
      stopMotors();
      delay(1000);
    }
  }

  esp_now_register_recv_cb(
    onDataRecv
  );

  Serial.println(
    "ESP-NOW ready."
  );

  Serial.println(
    "IR sensor ready."
  );

  Serial.println(
    "Motors stopped."
  );

  Serial.println(
    "Waiting for QR command..."
  );
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  // ==========================================================
  // HALTED
  // ==========================================================

  if (state == HALTED)
  {
    stopMotors();
    delay(10);
    return;
  }

  // ==========================================================
  // WAITING
  // ==========================================================

  if (state == WAITING_FOR_COMMAND)
  {
    stopMotors();
    delay(10);
    return;
  }

  // ==========================================================
  // LEFT TURN
  // ==========================================================

  if (state == TURNING_LEFT)
  {
    if (
      millis() - turnStartTime
      >= TURN_LEFT_90_MS
    )
    {
      Serial.println(
        "LEFT 90-degree turn complete."
      );

      // Immediately move forward
      Serial.println(
        "Moving forward until BLACK TAPE."
      );

      moveForward();

      state = MOVING_TO_LINE;

      lineDetectionArmed = false;
      blackStarted = false;
    }

    return;
  }

  // ==========================================================
  // RIGHT TURN
  // ==========================================================

  if (state == TURNING_RIGHT)
  {
    if (
      millis() - turnStartTime
      >= TURN_RIGHT_90_MS
    )
    {
      Serial.println(
        "RIGHT 90-degree turn complete."
      );

      // Immediately move forward
      Serial.println(
        "Moving forward until BLACK TAPE."
      );

      moveForward();

      state = MOVING_TO_LINE;

      lineDetectionArmed = false;
      blackStarted = false;
    }

    return;
  }

  // ==========================================================
  // MOVING UNTIL BLACK TAPE
  // ==========================================================

  if (state == MOVING_TO_LINE)
  {
    // Keep moving.
    moveForward();

    // --------------------------------------------------------
    // First leave the previous black tape.
    // --------------------------------------------------------

    if (!lineDetectionArmed)
    {
      if (!isBlack())
      {
        lineDetectionArmed = true;
        blackStarted = false;

        Serial.println(
          "IR ARMED - looking for next black tape."
        );
      }

      return;
    }

    // --------------------------------------------------------
    // Detect next black tape
    // --------------------------------------------------------

    if (confirmedBlack())
    {
      stopMotors();

      state = WAITING_FOR_COMMAND;

      lineDetectionArmed = false;
      blackStarted = false;

      Serial.println();
      Serial.println(
        "================================"
      );
      Serial.println(
        "BLACK TAPE DETECTED"
      );
      Serial.println(
        "CAR STOPPED"
      );
      Serial.println(
        "WAITING FOR NEXT QR COMMAND"
      );
      Serial.println(
        "================================"
      );
    }

    return;
  }
}