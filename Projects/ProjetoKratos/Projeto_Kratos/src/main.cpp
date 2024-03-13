#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <ESP32Servo.h>

// Lista de tópicos MQTT para inscrição
const char *topicos[] = {"casa/temperatura", "casa/umidade", "casa/sensoresLeds", 
                        "casa/sensorPorta1", "casa/sensorPorta2", "casa/tranca1",
                        "casa/tranca2", "casa/estadoPorta1", "casa/estadoPorta2",
                        "casa/estadoTranca1", "casa/estadoTranca2"};

// Defina as informações da sua rede Wi-Fi
const char *ssid = "VORTEX";
const char *password = "pprKN@Fv";

// Configurações do broker MQTT
const char *mqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;

const int watchdogTimeout = 5;  // Tempo limite do watchdog em segundos

// Inicialização sensor de Temperatura e Umidade
#define DHT_SENSOR_PIN  13
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

Preferences preferences;

// Objeto para manipulação do cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char *, byte *, unsigned int);
void reconnect();
void updateSensores(String message);
void controlaLEDs();
void controlaTrancas(char *, String);
void readDHTAndPublishData(PubSubClient &client);
void portas();

// Inicialização dos pinos e declaração de variáveis
const int numLEDs = 4;
const int numPortas = 2;
const int numServos = 2;
const int ledPins[numLEDs] = {5, 18, 19, 21};
const int sensorPins[numLEDs] = {27, 26, 25, 33};
const int interruptoresPins[numLEDs] = {32, 15, 4, 14};
const int servoPins[numServos] = {23, 22};
const int sensorPortaPins[numPortas] = {16, 17};
unsigned long tempoLigado = preferences.getInt("tempoLigado", 10000); // Tempo em milissegundos
unsigned long lastMotionTime[numLEDs] = {0}; // Armazena o tempo que o sensor detectou movimento pela última vez
unsigned long intervaloDHT = 10250;
unsigned long lastTimeDHT = 0;
unsigned long intervaloPorta = 226;
unsigned long lastTimePorta = 0;
bool estadoBotao = preferences.getBool("estadoBotao", false); // botão que sinaliza se os sensores de presença estão ligados
bool buttonPressed = false; // Flag para indicar se o botão foi pressionado

//TRUE == FECHADO E FALSE == ABERTO
bool estadoTrancas[] = {preferences.getBool("estadoTranca1", false), preferences.getBool("estadoTranca2", false)};


// Vetor de objetos Servo
Servo servos[numServos];

void setup() {
  Serial.begin(115200);

  preferences.begin("casa", false);  // Inicia o namespace "casa"

  dht_sensor.begin();

  // Inicializa os servo motores e define os pinos
  for (int i = 0; i < numServos; i++) {
    servos[i].attach(servoPins[i]);
  }

  // Configurar o watchdog timer com o tempo limite especificado
  esp_task_wdt_init(watchdogTimeout, true); // This function configures and initializes the TWDT. This function will subscribe the idle tasks if configured to do so.
  esp_task_wdt_add(NULL); // Input NULL to subscribe the current running task to the TWDT

  // Conecte-se à rede Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");

  // Configurar o cliente MQTT
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);

  // Inicialize os pins
  for (int i = 0; i < numLEDs; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(sensorPins[i], INPUT_PULLUP);
    pinMode(interruptoresPins[i], INPUT_PULLUP);

    // Recupere o último estado salvo e defina nos LEDs
    char key[20];
    sprintf(key, "estadoLED%d", i);
    digitalWrite(ledPins[i], preferences.getBool(key, false));
  }
  for(int i = 0; i < numPortas; i++){
    pinMode(sensorPortaPins[i], INPUT_PULLUP);
  }
}

void loop() {
  // Alimentar o watchdog timer
  esp_task_wdt_reset();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  portas();
  controlaLEDs();
  readDHTAndPublishData(client);
}

void callback(char *topic, byte *payload, unsigned int length) {
  String topicStr = String(topic);
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Mensagem recebida no tópico: " + String(topic));
  Serial.println(message);

  if (topicStr.equals("casa/sensoresLeds")) {
    if(message.endsWith("dos")){
      updateSensores(message);
    } else {
      preferences.putInt("tempoLigado", message.toInt());
      tempoLigado = message.toInt();
    }
  }
  if (strncmp(topic, "casa/tranca", 11) == 0) {
    controlaTrancas(topic, message);
  }
}

void subscribeToMQTTTopics() {
  for (int i = 0; i < sizeof(topicos) / sizeof(topicos[0]); i++) {
    client.subscribe(topicos[i]);
    Serial.println("Inscrito no tópico: " + String(topicos[i]));
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("esp32Kratos")) {
      Serial.println("Conectado");
      subscribeToMQTTTopics();
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void portas(){
  unsigned long currentTimePorta = millis();
  if(currentTimePorta - lastTimePorta >= intervaloPorta){
    for(int i = 0; i < numPortas; i++){
      char topicTranca[20];
      sprintf(topicTranca, "casa/estadoTranca%d", i+1);
      char topicPorta[20];
      sprintf(topicPorta, "casa/sensorPorta%d", i+1);
      
      if(estadoTrancas[i] == true){
        client.publish(topicTranca, "Trancada");
      } else {
        client.publish(topicTranca, "Destrancada");
      }
      if((digitalRead(sensorPortaPins[i]) == LOW)){
        client.publish(topicPorta, "Fechada");
      } else {
        client.publish(topicPorta, "Aberta");
      }
    }
    lastTimePorta = currentTimePorta;
  }
}

void controlaTrancas(char *topic, String message){
  int trancaIndex = (topic[11] - '0') - 1;

  char keyTranca[20];
    sprintf(keyTranca, "estadoTranca%d", trancaIndex + 1);
    char keyPorta[20];
    sprintf(keyPorta, "estadoPorta%d", trancaIndex + 1);
    char topicEstadoPorta[20];
    sprintf(topicEstadoPorta, "casa/estadoPorta%d", trancaIndex + 1);

  if(message == "trancar"){
    if(digitalRead(sensorPortaPins[trancaIndex]) == LOW){
      servos[trancaIndex].write(95);
      estadoTrancas[trancaIndex] = true;
      preferences.putBool(keyTranca, true);
      client.publish(topicEstadoPorta, "A porta foi trancada.");
    } else {
      client.publish(topicEstadoPorta, "A porta está aberta.");
    }
  } else {
    if(digitalRead(sensorPortaPins[trancaIndex]) == LOW){
      servos[trancaIndex].write(5);
      estadoTrancas[trancaIndex] = false;
      preferences.putBool(keyTranca, false);
      client.publish(topicEstadoPorta, "A porta foi destrancada.");
    } else {
      client.publish(topicEstadoPorta, "A porta está aberta.");
    }
  }
}

void controlaLEDs() {
  for (int i = 0; i < numLEDs; i++) {
    char key[20];
    sprintf(key, "estadoLED%d", i);
    if (estadoBotao) {
      if (digitalRead(sensorPins[i]) == LOW) {
        preferences.putInt(key, LOW);
        digitalWrite(ledPins[i], LOW);
        lastMotionTime[i] = millis();
      }
      if (millis() - lastMotionTime[i] > (tempoLigado + 100)) {
        preferences.putInt(key, HIGH);
        digitalWrite(ledPins[i], HIGH);
      }
    } else {
      if (digitalRead(interruptoresPins[i]) == HIGH && !buttonPressed) {  // Verifica se o botão está pressionado e a flag é false
        buttonPressed = true;  // Define a flag como true para evitar múltiplas ações enquanto o botão está pressionado
        digitalWrite(ledPins[i], !digitalRead(ledPins[i]));  // Inverte o estado do LED
      }

      if (digitalRead(interruptoresPins[i]) == LOW && buttonPressed) {  // Verifica se o botão foi solto
        buttonPressed = false;  // Reseta a flag quando o botão é solto
        delay(250);  // Adiciona um pequeno atraso para evitar a leitura rápida do botão
      }     
    }
  }
}

void updateSensores(String message) {
  if (message == "ligados") {
    preferences.putBool("estadoBotao", true);
    estadoBotao = true;
  } else if (message == "desligados") {
    preferences.putBool("estadoBotao", false);
    estadoBotao = false;
  }
}

void readDHTAndPublishData(PubSubClient &client) {
  float temperatura = dht_sensor.readTemperature();
  float umidade = dht_sensor.readHumidity();

  String tempTopic = "casa/temperatura";
  String humidityTopic = "casa/umidade";

  unsigned long currentTimeDHT = millis();
  if(currentTimeDHT - lastTimeDHT >= intervaloDHT){
    client.publish(tempTopic.c_str(), String(temperatura).c_str());
    client.publish(humidityTopic.c_str(), String(umidade).c_str());
    lastTimeDHT = currentTimeDHT;    
  }
}
