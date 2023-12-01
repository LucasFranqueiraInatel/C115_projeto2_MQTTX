#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Substitua com o nome da sua rede Wi-Fi
const char* ssid = "WLL-Inatel";     
// Substitua com a senha da sua rede Wi-Fi
const char* password = "inatelsemfio"; 
// Dados do MQTT
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883; // Porta padrão para MQTT não seguro

WiFiClient espClient;
PubSubClient client(espClient);

const int pinButton = 14; // GPIO14
const int pinBuzzer = 12; // GPIO12
const int pinLM35 = A0;   // Pino analógico para o sensor LM35

void setup() {
  Serial.begin(9600);

  // Configuração do Wi-Fi
  setup_wifi();

  // Configuração do cliente MQTT
  client.setServer(mqtt_server, mqtt_port);

  pinMode(pinButton, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinLM35, INPUT);  // Configurar pino do LM35 como entrada
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temperature = readTemperature();
  if (temperature > 33) {
    char tempMsg[50];
    sprintf(tempMsg, "Temperatura alta: %.2f°C", temperature);
    // Enviar mensagem para o broker MQTT com a temperatura exata
    client.publish("temperatura/alerta", tempMsg);

    // Acionar o buzzer
    tone(pinBuzzer, 1000); // Ligar o buzzer
    delay(200);           
    noTone(pinBuzzer);     // Desligar o buzzer
  }

  if (digitalRead(pinButton) == LOW) {
    tone(pinBuzzer, 1000);
    delay(200);           
    noTone(pinBuzzer);     

    client.publish("botao/teste", "Botão pressionado");
  }
}

float readTemperature() {
  int reading = analogRead(pinLM35);
  float voltage = reading * (3.3 / 1024.0);
  return voltage * 100.0;  // Conversão para Celsius
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}
