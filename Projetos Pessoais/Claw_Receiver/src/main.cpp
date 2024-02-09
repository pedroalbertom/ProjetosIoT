#include <Arduino.h>
#include <ESP32Servo.h>
#include <RadioLib.h>

#define PIN_SERVO_1 21
#define PIN_SERVO_2 22
#define PIN_SERVO_3 26

#define PIN_POT_1 13
#define PIN_POT_2 4
#define PIN_POT_3 27

int ANGLE_1;
int ANGLE_2;
int ANGLE_3;

Servo SERVO1;
Servo SERVO2;
Servo SERVO3;

void setup() {
  Serial.begin(115200);
  
  SERVO1.attach(PIN_SERVO_1);
  SERVO2.attach(PIN_SERVO_2);
  SERVO3.attach(PIN_SERVO_3);

  pinMode(PIN_POT_1, INPUT);
  pinMode(PIN_POT_2, INPUT);
  pinMode(PIN_POT_3, INPUT);
}

void loop() {
  ANGLE_1 = map(analogRead(PIN_POT_1), 0, 4095, 0, 180);
  ANGLE_2 = map(analogRead(PIN_POT_2), 0, 4095, 0, 180);
  ANGLE_3 = map(analogRead(PIN_POT_3), 0, 4095, 0, 180);
  Serial.println(ANGLE_1);
  Serial.println(ANGLE_2);
  Serial.println(ANGLE_3);
  SERVO1.write(ANGLE_1);
  SERVO2.write(ANGLE_2);
  SERVO3.write(ANGLE_3);
}
