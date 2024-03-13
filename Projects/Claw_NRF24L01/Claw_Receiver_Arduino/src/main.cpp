#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(7, 8);
const byte adresses[][6] = {"00001", "00002"};

#define PIN_SERVO_0 A0
#define PIN_SERVO_1 A1
#define PIN_SERVO_2 A2

Servo SERVO0;
Servo SERVO1;
Servo SERVO2;

#define TEMPO_PARA_ESCREVER 200
unsigned long ultimaLeitura = 0;

bool angle_changed = false;

int angulos[3] = {90, 90, 90};

bool READ(){
  radio.startListening();
  int novos_angulos[3];
  if (radio.available()) {
    ultimaLeitura = millis();
    radio.read(&novos_angulos, sizeof(angulos));
    if (novos_angulos[0] != angulos[0] || novos_angulos[1] != angulos[1] || novos_angulos[2] != angulos[2])
      angle_changed = true;

    angulos[0] = novos_angulos[0];
    angulos[1] = novos_angulos[1];
    angulos[2] = novos_angulos[2];

    Serial.println("Angulo 1 recebido:" + (String)novos_angulos[0]);
    Serial.println("Angulo 2 recebido:" + (String)novos_angulos[1]);
    Serial.println("Angulo 3 recebido:" + (String)novos_angulos[2]);
    Serial.println();

    SERVO0.write(novos_angulos[0]);
    SERVO1.write(novos_angulos[1]);
    SERVO2.write(novos_angulos[2]);

    return true;
  }
  return false;
}

void WRITE(){
  if (angle_changed && !radio.available() && millis() - ultimaLeitura > TEMPO_PARA_ESCREVER) {
    radio.stopListening();

    angulos[0] = SERVO0.read();
    angulos[1] = SERVO1.read();
    angulos[2] = SERVO2.read();

    radio.write(&angulos, sizeof(angulos));
    angle_changed = false;
  }else if (angle_changed && radio.available()){
    ultimaLeitura = millis();
  }
}

void setup() {
  Serial.begin(115200);

  while(!radio.begin()){
    Serial.println("Conectando ao rádio...");
    delay(1000);
  }
  Serial.println("Conectado!");

  radio.openWritingPipe(adresses[1]); // 00002
  radio.openReadingPipe(1, adresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);

  SERVO0.attach(PIN_SERVO_0);
  SERVO1.attach(PIN_SERVO_1);
  SERVO2.attach(PIN_SERVO_2);
}

void loop() {
  READ();
  WRITE();
  delay(5);
}