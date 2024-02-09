#include <Arduino.h>
#include <ESP32Servo.h>
#include <SPI.h>

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

#define PINO_SERVO_1 21
#define PINO_SERVO_2 22
#define PINO_SERVO_3 26
//#define PINO_SERVO_4 4

#define pot1Pin 13
#define pot2Pin 4
#define pot3Pin 27
int valor1Pot;
int valor2Pot;
int valor3Pot;
int angulo1;
int angulo2;
int angulo3;

void moveServos() {

  // Escreva o código para mover os servo motores aqui
  //if(!digitalRead(BUTTON_SERVO_1)){
  //  servo1.write(servo1.read()+3);
  //  Serial.println(servo1.read());
  //}
  //if(!digitalRead(BUTTON_SERVO_2)){
  //  servo1.write(servo1.read()-3);
  //  Serial.println(servo1.read());
  //}
  //if(!digitalRead(BUTTON_SERVO_3)){
  //  servo3.write(10);
  //}
  //if(!digitalRead(BUTTON_SERVO_4)){
  //  servo4.write(10);
  //}
}

void setup() {
  Serial.begin(115200);
  
  servo1.attach(PINO_SERVO_1);
  servo2.attach(PINO_SERVO_2);
  servo3.attach(PINO_SERVO_3);
  //servo4.attach(PINO_SERVO_4);
  //pinMode(BUTTON_SERVO_1, INPUT_PULLUP);
  //pinMode(BUTTON_SERVO_2, INPUT_PULLUP);
  //pinMode(BUTTON_SERVO_3, INPUT_PULLUP);
  //pinMode(BUTTON_SERVO_4, INPUT_PULLUP);
  pinMode(pot1Pin, INPUT);
  pinMode(pot2Pin, INPUT);
  pinMode(pot3Pin, INPUT);
}

void loop() {
  //moveServos();
  valor1Pot = analogRead(pot1Pin);
  valor2Pot = analogRead(pot2Pin);
  valor3Pot = analogRead(pot3Pin);
  angulo1 = map(valor1Pot, 0, 4095, 0, 180);
  angulo2 = map(valor2Pot, 0, 4095, 0, 180);
  angulo3 = map(valor3Pot, 0, 4095, 0, 180);
  Serial.println(angulo1);
  Serial.println(angulo2);
  Serial.println(angulo3);
  servo1.write(angulo1);
  servo2.write(angulo2);
  servo3.write(angulo3);
}
