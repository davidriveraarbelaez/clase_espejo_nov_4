
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// ==========================================
// CONFIGURACIÓN WIFI
// ==========================================

const char* nombreRed = "ClaseEspejo";
const char* contrasena = "12345678";

// ==========================================
// CONFIGURACIÓN MQTT
// ==========================================

const char* servidorMQTT = "10.51.43.236";
const int puertoMQTT = 1883;

const char* topicoTemperatura =
  "clase/esp32/temperatura";

const char* topicoHumedad =
  "clase/esp32/humedad";

// Objetos de conexión
WiFiClient clienteWiFi;
PubSubClient clienteMQTT(clienteWiFi);

// ==========================================
// CONFIGURACIÓN SENSOR DHT11
// ==========================================

#define PIN_DHT 5
#define TIPO_DHT DHT11

DHT_Unified sensorDHT(PIN_DHT, TIPO_DHT);

// Variables de medición
float temperatura = 0.0;
float humedad = 0.0;

const unsigned long intervaloLectura = 2000;

unsigned long tiempoAnterior = 0;

// ==========================================
// FUNCIÓN: CONECTAR WIFI
// ==========================================

void conectarWiFi() {

  Serial.println();
  Serial.println("Conectando a Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, contrasena);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado.");

  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
}



// ==========================================
// FUNCIÓN: CONECTAR MQTT
// ==========================================

void conectarMQTT() {

  while (!clienteMQTT.connected()) {

    Serial.println("Conectando al broker MQTT...");

    // Identificador del dispositivo
    String identificador = "ESP32_DHT11_";
    identificador += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (clienteMQTT.connect(identificador.c_str())) {

      Serial.println("Conexion MQTT establecida.");

    } else {

      Serial.print("Error MQTT. Estado: ");
      Serial.println(clienteMQTT.state());

      Serial.println("Nuevo intento en 3 segundos.");

      delay(3000);
    }
  }
}

// ==========================================
// FUNCIÓN: PROBAR Y LEER SENSOR
// ==========================================

bool probarSensor() {

  sensors_event_t eventoTemperatura;
  sensors_event_t eventoHumedad;

  sensorDHT.temperature().getEvent(&eventoTemperatura);
  sensorDHT.humidity().getEvent(&eventoHumedad);

  if (isnan(eventoTemperatura.temperature) ||
      isnan(eventoHumedad.relative_humidity)) {

    Serial.println("Error al leer el DHT11.");

    return false;
  }

  temperatura = eventoTemperatura.temperature;
  humedad = eventoHumedad.relative_humidity;

  return true;
}

// ==========================================
// FUNCIÓN: MOSTRAR MEDICIONES
// ==========================================

void mostrarMediciones() {

  Serial.println("-----------------------------");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.println("-----------------------------");
}

// ==========================================
// FUNCIÓN: PUBLICAR MEDICIONES MQTT
// ==========================================

void publicarMediciones() {

  if (!clienteMQTT.connected()) {
    conectarMQTT();
  }

  char valorTemperatura[10];
  char valorHumedad[10];

  dtostrf(temperatura, 1, 2, valorTemperatura);
  dtostrf(humedad, 1, 2, valorHumedad);

  bool envioTemperatura = clienteMQTT.publish(
    topicoTemperatura,
    valorTemperatura,
    true
  );

  bool envioHumedad = clienteMQTT.publish(
    topicoHumedad,
    valorHumedad,
    true
  );

  if (envioTemperatura && envioHumedad) {
    Serial.println("Mediciones publicadas por MQTT.");
  } else {
    Serial.println("Error al publicar las mediciones.");
  }
}

// ==========================================
// FUNCIÓN: LEER Y PUBLICAR
// ==========================================

void procesarMediciones() {

  if (probarSensor()) {

    mostrarMediciones();

    if (WiFi.status() == WL_CONNECTED) {
      publicarMediciones();
    } else {
      Serial.println("Wi-Fi desconectado.");
    }

  } else {

    Serial.println("No se publicaron datos.");

  }
}

// ==========================================
// CONFIGURACIÓN INICIAL
// ==========================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" SISTEMA IoT - ESP32 + MQTT");
  Serial.println("==============================");

  // Inicializar sensor
  sensorDHT.begin();

  // Conectar Wi-Fi
  conectarWiFi();
  probarConexionBroker();
  // Configurar servidor MQTT
  clienteMQTT.setServer(servidorMQTT, puertoMQTT);

  // Conectar al broker
  conectarMQTT();
}

// ==========================================
// BUCLE PRINCIPAL
// ==========================================

void loop() {

  // Mantener activa la conexión MQTT
  clienteMQTT.loop();

  // Controlar intervalo de lectura
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= intervaloLectura) {

    tiempoAnterior = tiempoActual;

    procesarMediciones();
  }
}

void probarConexionBroker() {

  WiFiClient pruebaTCP;

  Serial.println();
  Serial.println("Probando conexion TCP con Mosquitto...");

  Serial.print("Broker: ");
  Serial.println(servidorMQTT);

  if (pruebaTCP.connect(servidorMQTT, puertoMQTT, 5000)) {

    Serial.println("EXITO: ESP32 alcanza el broker MQTT.");

    pruebaTCP.stop();

  } else {

    Serial.println("ERROR: ESP32 no alcanza el broker.");
  }
}