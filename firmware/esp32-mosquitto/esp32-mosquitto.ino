
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// ==========================================
// 1. CONFIGURACION WIFI
// ==========================================

const char* nombreRed = "red-para-esp32";
const char* contrasena = "password123";

// ==========================================
// 2. CONFIGURACION MQTT
// ==========================================

const char* servidorMQTT = "10.51.43.236";
const int puertoMQTT = 1883;

const char* topicoTemperatura =
  "clase/esp32/temperatura";

const char* topicoHumedad =
  "clase/esp32/humedad";

const char* topicoLED1 =
  "clase/esp32/led1";

const char* topicoLED2 =
  "clase/esp32/led2";

WiFiClient clienteWiFi;
PubSubClient clienteMQTT(clienteWiFi);

// ==========================================
// 3. CONFIGURACION DHT11
// ==========================================

#define PIN_DHT 5
#define TIPO_DHT DHT11

DHT_Unified sensorDHT(PIN_DHT, TIPO_DHT);

float temperatura = 0.0;
float humedad = 0.0;

const unsigned long intervaloLectura = 2000;
unsigned long tiempoAnterior = 0;

// ==========================================
// 4. CONFIGURACION LED
// ==========================================

#define PIN_LED1 40
#define PIN_LED2 36

bool estadoLED1 = false;
bool estadoLED2 = false;

// ==========================================
// 5. CONECTAR WIFI
// ==========================================

void conectarWiFi() {

  Serial.println("Conectando a Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, contrasena);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado.");

  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
}

// ==========================================
// 6. CONFIGURAR LED
// ==========================================

void configurarLED() {

  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);

  Serial.println("LED configurados.");
}

// ==========================================
// 7. CONTROLAR LED
// ==========================================

void controlarLED(int numeroLED, bool encender) {

  int pin = (numeroLED == 1) ? PIN_LED1 : PIN_LED2;

  digitalWrite(pin, encender ? HIGH : LOW);

  if (numeroLED == 1) {
    estadoLED1 = encender;
  } else {
    estadoLED2 = encender;
  }

  Serial.print("LED ");
  Serial.print(numeroLED);
  Serial.println(encender ? " ENCENDIDO" : " APAGADO");
}

// ==========================================
// 8. RECIBIR COMANDOS MQTT
// ==========================================

void recibirComandosMQTT(
  char* topico,
  byte* mensaje,
  unsigned int longitud
) {

  String comando = "";

  for (unsigned int i = 0; i < longitud; i++) {
    comando += (char)mensaje[i];
  }

  comando.trim();
  comando.toUpperCase();

  Serial.print("Topico recibido: ");
  Serial.println(topico);

  Serial.print("Comando: ");
  Serial.println(comando);

  if (String(topico) == topicoLED1) {

    if (comando == "ON") {
      controlarLED(1, true);
    }

    if (comando == "OFF") {
      controlarLED(1, false);
    }
  }

  else if (String(topico) == topicoLED2) {

    if (comando == "ON") {
      controlarLED(2, true);
    }

    if (comando == "OFF") {
      controlarLED(2, false);
    }
  }
}

// ==========================================
// 9. CONECTAR MQTT
// ==========================================

void conectarMQTT() {

  while (!clienteMQTT.connected()) {

    Serial.println("Conectando a Mosquitto...");

    String identificador = "ESP32_DHT11_";
    identificador += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (clienteMQTT.connect(identificador.c_str())) {

      Serial.println("MQTT conectado.");

      clienteMQTT.subscribe(topicoLED1);
      clienteMQTT.subscribe(topicoLED2);

      Serial.println("Suscrito a los topicos de control.");

    } else {

      Serial.print("Error MQTT: ");
      Serial.println(clienteMQTT.state());

      delay(3000);
    }
  }
}

// ==========================================
// 10. LEER SENSOR
// ==========================================

bool probarSensor() {

  sensors_event_t eventoTemperatura;
  sensors_event_t eventoHumedad;

  sensorDHT.temperature().getEvent(&eventoTemperatura);
  sensorDHT.humidity().getEvent(&eventoHumedad);

  if (isnan(eventoTemperatura.temperature) ||
      isnan(eventoHumedad.relative_humidity)) {

    Serial.println("Error al leer DHT11.");
    return false;
  }

  temperatura = eventoTemperatura.temperature;
  humedad = eventoHumedad.relative_humidity;

  return true;
}

// ==========================================
// 11. PUBLICAR MEDICIONES
// ==========================================

void publicarMediciones() {

  char valorTemperatura[10];
  char valorHumedad[10];

  dtostrf(temperatura, 1, 2, valorTemperatura);
  dtostrf(humedad, 1, 2, valorHumedad);

  clienteMQTT.publish(
    topicoTemperatura,
    valorTemperatura,
    true
  );

  clienteMQTT.publish(
    topicoHumedad,
    valorHumedad,
    true
  );

  Serial.println("Mediciones publicadas.");
}

// ==========================================
// 12. PROCESAR MEDICIONES
// ==========================================

void procesarMediciones() {

  if (probarSensor()) {

    Serial.println("-------------------------");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    if (clienteMQTT.connected()) {
      publicarMediciones();
    }
  }
}

// ==========================================
// 13. SETUP
// ==========================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("SISTEMA IoT ESP32");

  configurarLED();

  sensorDHT.begin();

  conectarWiFi();

  clienteMQTT.setServer(servidorMQTT, puertoMQTT);
  clienteMQTT.setCallback(recibirComandosMQTT);

  conectarMQTT();
}

// ==========================================
// 14. LOOP
// ==========================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  if (!clienteMQTT.connected()) {
    conectarMQTT();
  }

  clienteMQTT.loop();

  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= intervaloLectura) {

    tiempoAnterior = tiempoActual;

    procesarMediciones();
  }
}
