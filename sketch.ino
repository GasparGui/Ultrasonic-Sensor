#include <WiFi.h>
#include <PubSubClient.h>

#define LED_PIN 2  // Pino onde o LED (lâmpada) está conectado
#define TRIG_PIN 14
#define ECHO_PIN 27

const char* ssid = "Wokwi-GUEST";         // Nome da sua rede Wi-Fi
const char* password = "";    // Senha do Wi-Fi
const char* mqtt_server = "broker.hivemq.com";  // Endereço do broker MQTT
const char* mosquitto_mqtt_server = "test.mosquitto.org";
const char* eclipse_mqtt_server = "mqtt.eclipseprojects.io";

WiFiClient espClient;
PubSubClient client(espClient);

// Função para reconectar ao MQTT se a conexão cair
void reconnect() {
    while (!client.connected()) {
        Serial.print("Conectando ao MQTT...");
        if (client.connect("ESP32_Client")) {  
            Serial.println("Conectado!");
            client.subscribe("teste/lampada");  // Assina o tópico
            Serial.println("Me inscrevi");
        } else {
            Serial.print("Falha, rc=");
            Serial.print(client.state());
            Serial.println(" Tentando novamente em 5s...");
            delay(5000);
        }
    }
}

// Callback: executado quando uma mensagem MQTT chega
void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Mensagem recebida no tópico: ");
    Serial.println(topic);

    // Converte a mensagem recebida para string
    String msg = "";
    for (int i = 0; i < length; i++) {
        msg += (char)message[i];
    }
    Serial.println("Mensagem: " + msg);
    Serial.println("----------------");

    // Liga ou desliga a lâmpada conforme a mensagem recebida
    if (msg == "ON") {
        digitalWrite(LED_PIN, HIGH);
    } else if (msg == "OFF") {
        digitalWrite(LED_PIN, LOW);
    }

    String mensagem = "Lampada " + msg;
    client.publish("teste/lampada_confirma", mensagem.c_str());
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Conectar ao Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" Conectado!");

    // Configurar o MQTT
    client.setServer(eclipse_mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Envia um pulso de 10us para o TRIG
    digitalWrite(TRIG_PIN, HIGH);  // Envia o pulso de Trigger
    delayMicroseconds(10);  // Espera 10 microsegundos
    digitalWrite(TRIG_PIN, LOW);  // Desliga o TRIG

    // Agora o pino ECHO recebe o sinal refletido
    long duration = pulseIn(ECHO_PIN, HIGH);  // Captura a duração do pulso em microsegundos

    // A distância é calculada pela fórmula: distância = (tempo / 2) * velocidade do som
    // A velocidade do som no ar é de aproximadamente 343 metros por segundo (ou 0.0343 cm por microsegundo)
    long distance = (duration / 2) * 0.0343;  // Converte a duração para distância em cm

    // Exibe a distância medida
    Serial.print("Distância: ");
    Serial.print(distance);
    Serial.println(" cm");

    char distanceStr[10];  // Buffer para armazenar a string
    sprintf(distanceStr, "%ld", distance);  // Converte o valor para string
    client.publish("teste/agua", distanceStr);
    delay(5000);  // Aguarda 1 segundo antes de medir novamente
}
