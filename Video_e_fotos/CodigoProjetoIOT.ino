#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>  //Biblioteca utilizada
#include "DHT.h"
#include <Ultrasonic.h>


#define PORTA 2
#define JANELA 4
#define DHTPIN 22  
#define DHTTYPE DHT11   // DHT 11
#define PIN_LED 21
#define PORTA_TRIGGER   GPIO_NUM_18
#define PORTA_ECHO      GPIO_NUM_19
int ultimo=0;
int verificador=0;
int anguloj=0;
float t=0;
int buz=0;
Servo s;
Servo s1;
const int buzzer = 23;
int pinoChuva_d = 12; // Pino D12 do ESP32 ligado ao D0 do sensor
int pinoChuva_a = 35; // Pino D35 do ESP32 ligado ao A0 do sensor
int val_d = 0;   // Armazena leitura do pino digital
int val_a = 0;   // Armazena leitura do pino analógico
DHT dht(DHTPIN, DHTTYPE);
////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Definicoes para o MQTT */
#define TOPICO_SUBSCRIBE_PORTA         "topico_verifica_porta"
#define TOPICO_SUBSCRIBE_JANELA         "topico_verifica_janela"
#define TOPICO_PUBLISH_TEMPERATURA   "topico_sensor_temperatura_publicado"
#define TOPICO_PUBLISH_ABRIR_PORTA   "TOPICO_PUBLISH_ABRE_PORTA_DA_CASA"
#define TOPICO_PUBLISH_ABRIR_JANELA  "TOPICO_PUBLISH_ABRE_JANELA_DA_CASA"
#define TOPICO_PUBLISH_ACIONA_BUZZER   "TOPICO_PUBLISH_ACIONA_BUZZER"
#define TOPICO_SUBSCRIBE_VERIFICA_BUZZER   "TOPICO_SUBSCRIBE_VERIFICA_BUZZER"
#define TOPICO_SUBSCRIBE_CHUVA "TOPICO_SUBSCRIBE_VERIFICA_CHUVA"
#define TOPICO_PUBLISH_LED         "topico_liga_desliga_led"
#define TOPICO_SUBSCRIBE_ULTRASSONICO "TOPICO_SUBSCRIBE_ULTRASSONICO"
#define ID_MQTT  "dc44fae4-a83f-4303-9e8c-44957228882c"     //id mqtt (para identificação de sessão)
//#define ID_MQTT  "mqttdash-ea4e25ea"
////////////////////////////////////////////////////////////////////////////////////////////////////////
Ultrasonic ultrasonic(PORTA_TRIGGER, PORTA_ECHO);
// declarar variável distancia do tipo inteiro sem sinal
float distancia;

const char* SSID     = "IMPLANTAR_MDS193"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "jan41514"; // Senha da rede WI-FI que deseja se conectar

const char* BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883; // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient


/* Prototypes */
void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);

/*
   Implementações
*/

/* Função: inicializa e conecta-se na rede WI-FI desejada
   Parâmetros: nenhum
   Retorno: nenhum
*/
void initWiFi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}


/* Função: inicializa parâmetros de conexão MQTT(endereço do
           broker, porta e seta função de callback)
   Parâmetros: nenhum
   Retorno: nenhum
*/
void initMQTT(void)
{
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

/* Função: função de callback
           esta função é chamada toda vez que uma informação de
           um dos tópicos subescritos chega)
   Parâmetros: nenhum
   Retorno: nenhum
*/

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;
  /* obtem a string do payload recebido */
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }

  Serial.print("Chegou a seguinte string via MQTT: ");
  Serial.println(msg);

  /* toma ação dependendo da string recebida */
  if (msg.equals("A"))
  {
    if(s.read()!=135){
    for(int i=1;i<136;i++){
    s.write(i);
    delay(10);
    }
    MQTT.publish(TOPICO_SUBSCRIBE_PORTA, "PORTA ABERTA");
    Serial.println("porta aberta mediante comando MQTT");
  }
  }

   if (msg.equals("B"))
  {
    if(s.read()!=0){
    for(int i=134;i>=0;i--){
    s.write(i);
    delay(10);
    }
    MQTT.publish(TOPICO_SUBSCRIBE_PORTA, "PORTA FECHADA");
    Serial.println("Porta fechada mediante comando MQTT");
  }
  }
  if (msg.equals("C"))
  {
    if(s1.read()!=135){
    for(int i=1;i<136;i++){
    s1.write(i);
    delay(10);
    }
    }
    MQTT.publish(TOPICO_SUBSCRIBE_JANELA, "JANELA ABERTA");
    Serial.println("porta aberta mediante comando MQTT");
    if(t<20 || val_a<1200)
    anguloj=1;
  }

   if (msg.equals("D"))
  {

    if(s1.read()!=0){
    for(int i=134;i>=0;i--){
    s1.write(i);
    delay(10);
    }
    }
    anguloj=0;
    MQTT.publish(TOPICO_SUBSCRIBE_JANELA, "JANELA FECHADA");
    Serial.println("Porta fechada mediante comando MQTT");
  }
   if (msg.equals("E")){
    tone(buzzer,2000);
    MQTT.publish(TOPICO_SUBSCRIBE_VERIFICA_BUZZER, "ALARME DISPARADO");
    buz=0;
  }
   if (msg.equals("F")){
    noTone(buzzer);
    MQTT.publish(TOPICO_SUBSCRIBE_VERIFICA_BUZZER, "ALARME DESLIGADO");
    buz=1;
  }
   if (msg.equals("G")){
    digitalWrite(PIN_LED, HIGH);
  }
  if (msg.equals("H")){
    digitalWrite(PIN_LED, LOW);
  }

  if (msg.equals("I")){
    verificador=1;
  }
  if (msg.equals("J")){
    verificador=0;
  }
  }
/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
           em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconnectMQTT(void)
{
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_PUBLISH_LED);
      MQTT.subscribe(TOPICO_PUBLISH_ABRIR_PORTA);
      MQTT.subscribe(TOPICO_PUBLISH_ABRIR_JANELA);
      MQTT.subscribe(TOPICO_PUBLISH_ACIONA_BUZZER);
      MQTT.subscribe(TOPICO_PUBLISH_LED);
      MQTT.subscribe(TOPICO_SUBSCRIBE_ULTRASSONICO);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }  
}

/* Função: verifica o estado das conexões WiFI e ao broker MQTT.
           Em caso de desconexão (qualquer uma das duas), a conexão
           é refeita.
   Parâmetros: nenhum
   Retorno: nenhum
*/
void VerificaConexoesWiFIEMQTT(void)
{
  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

/* Função: reconecta-se ao WiFi
   Parâmetros: nenhum
   Retorno: nenhum
*/
void reconnectWiFi(void)
{
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  s.attach(PORTA);
  s.write(0);
  s1.attach(JANELA);
  s1.write(0);
  Serial.begin(9600); //Enviar e receber dados em 9600 baud
  delay(1000);
  Serial.println("Disciplina IoT: acesso a nuvem via ESP32");
  delay(1000);
  randomSeed(analogRead(0));
  pinMode(buzzer,OUTPUT);
  dht.begin();
  pinMode(pinoChuva_d, INPUT);
  pinMode(pinoChuva_a, INPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  /* Inicializa a conexao wi-fi */
  initWiFi();

  /* Inicializa a conexao ao broker MQTT */
  initMQTT();
}

// the loop function runs over and over again forever
void loop() {
  t = dht.readTemperature();
  // cria string para temperatura
  char temperatura_str[10] = {0};
  sprintf(temperatura_str, "%0.2fc", t);
  MQTT.publish(TOPICO_PUBLISH_TEMPERATURA, temperatura_str);
  if(t<20){
    if(s1.read()!=0){
    if(anguloj!=1){
    MQTT.publish(TOPICO_PUBLISH_ABRIR_JANELA, "D");
    MQTT.publish(TOPICO_PUBLISH_ACIONA_BUZZER, "F");
    delay(500);
    }
    }
    }
  if(t>45){
    MQTT.publish(TOPICO_PUBLISH_ACIONA_BUZZER, "E");
  }
  val_a = analogRead(pinoChuva_a);
  //Serial.println(val_a);
  if(val_a<2500){
    if(s1.read()!=0){
    MQTT.publish(TOPICO_PUBLISH_ABRIR_JANELA, "D");
    }
    MQTT.publish(TOPICO_SUBSCRIBE_CHUVA, "CHOVENDO");
    delay(500);
  }
  else if(val_a>=2500){
   MQTT.publish(TOPICO_SUBSCRIBE_CHUVA, "SEM CHUVA");
  }
    ultrasonic.measure();
    distancia = ultrasonic.get_cm();
    //Serial.println("Leitura do Sensor de Distancia em cm:");
    //Serial.println(distancia);
    if(distancia<5){
      if(verificador==1){
      if(ultimo !=1){
        if(buz==1){
      MQTT.publish(TOPICO_PUBLISH_ACIONA_BUZZER, "E");    
      ultimo = 1;
      delay(200);
        }
      }
      }
    }
    else if(distancia >=5){
      ultimo = 0;
    }
/* garante funcionamento das conexões WiFi e ao broker MQTT */
  VerificaConexoesWiFIEMQTT();
  
  /* keep-alive da comunicação com broker MQTT */
  MQTT.loop();
  delay(100);
}
