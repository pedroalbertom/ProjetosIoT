#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>
#include <Preferences.h>

Preferences preferences;

// Defina as informações da sua rede Wi-Fi
const char *ssid = "VORTEX";
const char *password = "pprKN@Fv";

// Configurações do broker MQTT
const char *mqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;

const int watchdogTimeout = 5;  // Tempo limite do watchdog em segundos

// Objeto para manipulação do cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Inicialização dos pinos e declaração de variáveis
int numFitas = 3;
int coresRGB1[] = {preferences.getInt("Fita1R", 255), preferences.getInt("Fita1G", 255), preferences.getInt("Fita1B", 255)};
int coresRGB2[] = {preferences.getInt("Fita2R", 255), preferences.getInt("Fita2G", 255), preferences.getInt("Fita2B", 255)};
int coresRGB3[] = {preferences.getInt("Fita3R", 255), preferences.getInt("Fita3G", 255), preferences.getInt("Fita3B", 255)};
const int numReles = 3;
const int relePins[] = {13, 12, 14};
const int pinAr = 32;
int temperaturaAr = preferences.getInt("temperaturaAr", 0);
bool arLigado = preferences.getBool("estadoAr", false);  // Inicialmente, o ar condicionado está desligado

// Estrutura para representar cada fita LED RGB
struct FitaLED {
  int pinRed;
  int pinGreen;
  int pinBlue;
  int redValue;
  int greenValue;
  int blueValue;
  bool ligada;
};

// Defina as configurações das fitas RGB
FitaLED fitas[] = {
  {15, 16, 17, coresRGB1[0], coresRGB1[1], coresRGB1[2], preferences.getBool("estadoFita1", false)},  // Fita 1
  {18, 19, 21, coresRGB2[0], coresRGB2[1], coresRGB2[2], preferences.getBool("estadoFita2", false)},  // Fita 2
  {22, 23, 25, coresRGB3[0], coresRGB3[1], coresRGB3[2], preferences.getBool("estadoFita3", false)}   // Fita 3
};

// Lista de tópicos MQTT para inscrição
const char *topicos[] = {"casa/fita1-control", "casa/fita2-control", "casa/fita3-control",
                        "casa/rele1-control", "casa/rele2-control", "casa/rele3-control",
                        "casa/arcondicionado"};

void callback(char *, byte *, unsigned int);
void reconnect();
void controlaRGB(char *topic, String message);
void controlaRELEs(char *topic, String message);
void controlaAr(String message);
void ligarAr();
void desligarAr();
void ajustarTemperatura();

void setup() {
  Serial.begin(115200);

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

  // Configuração dos pinos para as fitas LED RGB
  for (auto &fita : fitas) {
    pinMode(fita.pinRed, OUTPUT);
    pinMode(fita.pinGreen, OUTPUT);
    pinMode(fita.pinBlue, OUTPUT);
  }
  for (int i = 0; i < numReles; i++) {
    pinMode(relePins[i], OUTPUT);
    char key[20];
    sprintf(key, "estadoRele%d", i + 1);
    digitalWrite(relePins[i], preferences.getBool(key, true));
  }
  pinMode(pinAr, OUTPUT);
}

void loop() {
  // Alimentar o watchdog timer
  esp_task_wdt_reset();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char *topic, byte *payload, unsigned int length) {

  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  String topicStr = String(topic);

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.println(message);

  if (strncmp(topic, "casa/fita", 9) == 0) {
    controlaRGB(topic, message);
  } else if (strncmp(topic, "casa/rele", 9) == 0) {
    controlaRELEs(topic, message);
  } else if (strcmp(topic, "casa/arcondicionado") == 0) {
    controlaAr(message);
  }
}

void ligarAr() {
  arLigado = true;
  preferences.putBool("estadoAr", true);
  ajustarTemperatura();
}

void desligarAr() {
  arLigado = false;
  preferences.putBool("estadoAr", false);
  analogWrite(pinAr, 0);
}

void ajustarTemperatura() {
  if (arLigado) {
    analogWrite(pinAr, (255 - (8 * temperaturaAr)));
  }
}

void controlaAr(String message) {
  if (message == "ligar") {
    ligarAr();
  } else if (message == "desligar") {
    desligarAr();
  } else {
    temperaturaAr = message.toInt() - 17;
    preferences.putInt("temperaturaAr", temperaturaAr);
    ajustarTemperatura();
  }
}

void controlaRELEs(char *topic, String message) {
  int releIndex = (topic[9] - '0') - 1;
  if (message == "ligar") {
    digitalWrite(relePins[releIndex], LOW);
    char key[20];
    sprintf(key, "estadoRele%d", releIndex + 1);
    preferences.putBool(key, false);
  } else if (message == "desligar") {
    digitalWrite(relePins[releIndex], HIGH);
    char key[20];
    sprintf(key, "estadoRele%d", releIndex + 1);
    preferences.putBool(key, true);
  }
}

void extrairValoresRGB(String mensagem, int valoresRGB[3]) {
  // Extrair os valores usando sscanf
  sscanf(mensagem.c_str(), "rgb(%d, %d, %d)", &valoresRGB[0], &valoresRGB[1], &valoresRGB[2]);
}

void updateLED(FitaLED &fita) {
  analogWrite(fita.pinRed, fita.redValue);
  analogWrite(fita.pinGreen, fita.greenValue);
  analogWrite(fita.pinBlue, fita.blueValue);
}

void updateColors(int *valores, int &red, int &green, int &blue) {
  // Inverter os valores para atender à lógica das fitas LEDs
  red = 255 - valores[0];
  green = 255 - valores[1];
  blue = 255 - valores[2];
}

void ExtractAndUpdate(int index, String message){
  if(index == 0){
      extrairValoresRGB(message, coresRGB1);
      updateColors(coresRGB1, fitas[index].redValue, fitas[index].greenValue, fitas[index].blueValue);
      preferences.putInt("FITA1R", fitas[index].redValue);
      preferences.putInt("FITA1G", fitas[index].greenValue);
      preferences.putInt("FITA1B", fitas[index].blueValue);
    } else if(index == 1){
      extrairValoresRGB(message, coresRGB2);
      updateColors(coresRGB2, fitas[index].redValue, fitas[index].greenValue, fitas[index].blueValue);
      preferences.putInt("FITA2R", fitas[index].redValue);
      preferences.putInt("FITA2G", fitas[index].greenValue);
      preferences.putInt("FITA2B", fitas[index].blueValue);
    } else if(index == 2){
      extrairValoresRGB(message, coresRGB3);
      updateColors(coresRGB3, fitas[index].redValue, fitas[index].greenValue, fitas[index].blueValue);
      preferences.putInt("FITA3R", fitas[index].redValue);
      preferences.putInt("FITA3G", fitas[index].greenValue);
      preferences.putInt("FITA3B", fitas[index].blueValue);
    }
}

void controlaRGB(char *topic, String message) {
  int fitaIndex = (topic[9] - '0') - 1;

  if (message == "ligar") {
    fitas[fitaIndex].ligada = true;
    updateLED(fitas[fitaIndex]);
    char key[20];
    sprintf(key, "estadoFita%d", fitaIndex + 1);
    preferences.putBool(key, true);
  }
  if (message == "desligar") {
    analogWrite(fitas[fitaIndex].pinRed, 255);
    analogWrite(fitas[fitaIndex].pinGreen, 255);
    analogWrite(fitas[fitaIndex].pinBlue, 255);
    fitas[fitaIndex].ligada = false;
    char key[20];
    sprintf(key, "estadoFita%d", fitaIndex + 1);
    preferences.putBool(key, false);
  } 
  if(message.startsWith("rgb") && fitas[fitaIndex].ligada){
    ExtractAndUpdate(fitaIndex, message);
    updateLED(fitas[fitaIndex]);
  }
  if(message.startsWith("rgb") && !fitas[fitaIndex].ligada){
    ExtractAndUpdate(fitaIndex, message);
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
    if (client.connect("esp32Kratos5V")) {
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
