#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8);
const byte adresses[][6] = {"00001", "00002"};

#define BUTTON_PIN_0 2
#define BUTTON_PIN_1 3
#define BUTTON_PIN_2 4
#define BUTTON_PIN_3 5
#define BUTTON_PIN_4 9
#define BUTTON_PIN_5 6

int angulos[3];

bool WRITE(){
  radio.stopListening();
  
  bool clicked = false;
  if(!digitalRead(BUTTON_PIN_0) && angulos[0] > 1){
    clicked = true;
    angulos[0] -= 1;
  }
  if(!digitalRead(BUTTON_PIN_1) && angulos[0] < 180){
    clicked = true;
    angulos[0] += 1;
  }
  if(!digitalRead(BUTTON_PIN_2) && angulos[1] > 90){
    clicked = true;
    angulos[1] -= 1;
  }
  if(!digitalRead(BUTTON_PIN_3) && angulos[1] < 180){
    clicked = true;
    angulos[1] += 1;
  }
  if(!digitalRead(BUTTON_PIN_4) && angulos[2] > 1){
    clicked = true;
    angulos[2] -= 1;
  }
  if(!digitalRead(BUTTON_PIN_5) && angulos[2] < 95){
    clicked = true;
    angulos[2] += 1;
  }

  if (clicked) {
  radio.stopListening();
  radio.write(&angulos, sizeof(angulos));
  }

  return clicked;
}

void READ(){
  radio.startListening();

  if (radio.available()) {
    radio.read(&angulos, sizeof(angulos));  // Lê a mensagem recebida
    Serial.println("Ângulo 1: " + (String)angulos[0]);
    Serial.println("Ângulo 2: " + (String)angulos[1]);
    Serial.println("Ângulo 3: " + (String)angulos[2]);
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);

  while(!radio.begin()){
    Serial.println("Conectando ao rádio...");
    delay(1000);
  }
  Serial.println("Conectado!");

  radio.openWritingPipe(adresses[0]); // 00001
  radio.openReadingPipe(1, adresses[1]); // 00002
  radio.setPALevel(RF24_PA_MIN);

  pinMode(BUTTON_PIN_0, INPUT_PULLUP);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  pinMode(BUTTON_PIN_4, INPUT_PULLUP);
  pinMode(BUTTON_PIN_5, INPUT_PULLUP);
}

void loop() {
  WRITE();
  READ();
  delay(5);
}