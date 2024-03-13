#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8);
const byte adresses[][6] = {"00001", "00002"};
String username = "";
String dataInput;
char dataToSend[200];
char dataReceived[200];

void setup() {
  Serial.begin(115200);

  while(!radio.begin()){
    Serial.println("Conectando ao rádio...");
    delay(1000);
  }
  Serial.println("Conectado!");

  Serial.println("Enter username...");

  radio.openWritingPipe(adresses[1]); // 00002
  radio.openReadingPipe(1, adresses[0]); // 00001
  radio.setPALevel(RF24_PA_MIN);
}

void loop() {
  while(username == ""){
    if(Serial.available()){
      username = Serial.readStringUntil('\n');
      Serial.print("Welcome ");
      Serial.println(username);
    }
  }

  radio.startListening();

  if(radio.available()){
    radio.read(&dataReceived, sizeof(dataReceived));
    Serial.println(dataReceived);
  }

  if(Serial.available()){
    radio.stopListening();

    dataInput = "[" + username + "]" + Serial.readStringUntil('\n');
    Serial.println(dataInput);
    dataInput.toCharArray(dataToSend, 32);

    radio.write(&dataToSend, sizeof(dataToSend));
  }
}