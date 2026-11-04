# Sistema IoT: ESP32 + DHT11 + MQTT + Node-RED

Plataforma de monitoreo ambiental y control remoto de actuadores
mediante Internet de las Cosas (IoT), utilizando un ESP32, sensor DHT11,
protocolo MQTT, broker Eclipse Mosquitto y Node-RED.

El proyecto implementa una arquitectura IoT que permite adquirir
variables ambientales, transmitirlas mediante mensajería MQTT,
visualizarlas en un dashboard web y controlar remotamente dos LED.

## 1. Descripción del proyecto

El sistema integra los siguientes componentes:

-   **ESP32:** dispositivo de adquisición y procesamiento local.
-   **DHT11:** sensor de temperatura y humedad relativa.
-   **Wi-Fi:** conectividad inalámbrica del dispositivo.
-   **MQTT:** protocolo de mensajería basado en el modelo
    *Publish/Subscribe*.
-   **Eclipse Mosquitto:** broker MQTT encargado de recibir y distribuir
    los mensajes.
-   **Docker Compose:** herramienta para desplegar los servicios de
    infraestructura.
-   **Node-RED:** entorno de programación visual para integrar los
    componentes.
-   **Node-RED Dashboard 2.0:** interfaz web para visualizar mediciones
    y controlar los actuadores.

### Funcionalidades

-   Adquisición periódica de temperatura y humedad.
-   Conexión inalámbrica del ESP32 mediante Wi-Fi.
-   Publicación de mediciones utilizando MQTT.
-   Visualización de variables ambientales en tiempo real.
-   Representación gráfica del comportamiento de las mediciones.
-   Control remoto de dos LED desde una interfaz web.
-   Comunicación bidireccional entre Node-RED y el ESP32.

## 2. Arquitectura del sistema

La solución implementa dos flujos principales de comunicación.

### 2.1 Telemetría ambiental

El ESP32 adquiere los datos del sensor DHT11 y los publica en el broker
MQTT. Node-RED se suscribe a los tópicos correspondientes y procesa los
mensajes para su visualización.

``` text
DHT11
  |
  v
ESP32
  |
  | Wi-Fi
  v
Mosquitto MQTT
  |
  v
Node-RED
  |
  v
Dashboard
```

### 2.2 Control remoto de actuadores

El dashboard permite enviar comandos MQTT al ESP32 para encender y
apagar dos LED conectados a sus GPIO.

``` text
Dashboard
   |
   v
Node-RED
   |
   v
Mosquitto MQTT
   |
   | Wi-Fi
   v
ESP32
   |
   +---- GPIO LED 1
   |
   +---- GPIO LED 2
```

## 3. Estructura del repositorio

La estructura principal del repositorio es:

``` text
.
├── dashboard/
│   └── Flujos y configuración del dashboard
│
├── firmware/
│   └── esp32-mosquitto/
│       └── Código fuente del ESP32
│
├── mosquitto/
│   ├── config/
│   │   └── mosquitto.conf
│   ├── data/
│   └── log/
│
├── nodered/
│   └── data/
│
├── .gitignore
├── README.md
└── docker-compose.yml
```

> **Nota:** ajusta el nombre de la carpeta interna de `firmware/` y los
> nombres de los archivos de `dashboard/` para que coincidan exactamente
> con los que están versionados en el repositorio.

## 4. Tecnologías utilizadas

  Tecnología          Función
  ------------------- ---------------------------------------
  ESP32               Dispositivo IoT
  Arduino IDE         Desarrollo del firmware
  DHT11               Sensado ambiental
  Wi-Fi               Comunicación inalámbrica
  MQTT                Protocolo de mensajería
  Eclipse Mosquitto   Broker MQTT
  Docker              Contenedorización
  Docker Compose      Orquestación de servicios
  Node-RED            Integración y procesamiento de flujos
  Dashboard 2.0       Visualización y control

## 5. Requisitos previos

### Hardware

-   Placa ESP32 compatible con Arduino IDE.
-   Sensor DHT11 (módulo o sensor desnudo).
-   Dos LED.
-   Dos resistencias limitadoras (por ejemplo, 220 Ω).
-   Protoboard y cables Dupont.
-   Cable USB de datos.
-   Computador con Docker Desktop y Arduino IDE.

### Software

-   Docker Desktop con Docker Compose.
-   Arduino IDE.
-   Paquete de placas ESP32 instalado en Arduino IDE.
-   Librería `PubSubClient` (Nick O'Leary).
-   Librería `DHT sensor library` de Adafruit.
-   Librería `Adafruit Unified Sensor`.
-   Node-RED.
-   Paquete `@flowfuse/node-red-dashboard`.

### Red

El ESP32 debe poder comunicarse con el computador donde se ejecuta
Docker, a través del puerto TCP `1883`. Una red institucional puede
aplicar aislamiento Wi-Fi, VLAN, reglas de firewall o restricciones de
enrutamiento que impidan la comunicación entre dispositivos.

## 6. Despliegue de infraestructura

### 6.1 Clonar el repositorio

``` bash
git clone <URL_DEL_REPOSITORIO>
cd <NOMBRE_DEL_REPOSITORIO>
```

### 6.2 Verificar la configuración de Mosquitto

El archivo de configuración se encuentra en:

``` text
mosquitto/config/mosquitto.conf
```

Configuración utilizada para la demostración:

``` ini
listener 1883 0.0.0.0
allow_anonymous true

persistence true
persistence_location /mosquitto/data/

log_dest stdout
log_type all
```

> **Seguridad:** `allow_anonymous true` se utiliza para simplificar una
> demostración en una red de laboratorio controlada. No expongas el
> puerto `1883` a Internet ni utilices esta configuración en producción.
> Para despliegues reales, configura autenticación, ACL, TLS y reglas de
> firewall.

### 6.3 Verificar Docker Compose

El archivo `docker-compose.yml` debe definir los servicios Mosquitto y
Node-RED. Una configuración de referencia es:

``` yaml
services:
  mosquitto:
    image: eclipse-mosquitto:2
    container_name: mosquitto
    restart: unless-stopped
    ports:
      - "1883:1883"
    volumes:
      - ./mosquitto/config:/mosquitto/config
      - ./mosquitto/data:/mosquitto/data
      - ./mosquitto/log:/mosquitto/log

  nodered:
    image: nodered/node-red:latest
    container_name: nodered
    restart: unless-stopped
    ports:
      - "1880:1880"
    volumes:
      - ./nodered/data:/data
    depends_on:
      - mosquitto
```

Si el archivo ya existe en el repositorio, verifica que los nombres de
carpetas y volúmenes coincidan con la estructura real del proyecto.

### 6.4 Iniciar los servicios

Desde la raíz del repositorio:

``` bash
docker compose up -d
```

Verificar los contenedores:

``` bash
docker ps
```

Deben aparecer los servicios `mosquitto` y `nodered` en estado `Up`.

Consultar registros:

``` bash
docker logs mosquitto
docker logs -f nodered
```

### 6.5 Acceder a Node-RED

-   **Editor:** <http://localhost:1880>
-   **Dashboard:** <http://localhost:1880/dashboard>

## 7. Configuración MQTT

### 7.1 Tópicos utilizados

  ---------------------------------------------------------------------------------
  Tópico                      Dirección         Payload esperado  Descripción
  --------------------------- ----------------- ----------------- -----------------
  `clase/esp32/temperatura`   ESP32 → Node-RED  Número, por       Temperatura en °C
                                                ejemplo `25.50`   

  `clase/esp32/humedad`       ESP32 → Node-RED  Número, por       Humedad relativa
                                                ejemplo `61.00`   en %

  `clase/esp32/led1`          Node-RED → ESP32  `ON` / `OFF`      Control del LED 1

  `clase/esp32/led2`          Node-RED → ESP32  `ON` / `OFF`      Control del LED 2
  ---------------------------------------------------------------------------------

Los tópicos MQTT distinguen mayúsculas y minúsculas. Mantén exactamente
los mismos nombres en el firmware y en Node-RED.

### 7.2 Conexión desde Node-RED

Como Mosquitto y Node-RED se ejecutan en la misma red de Docker Compose,
la conexión MQTT desde Node-RED debe utilizar:

  -----------------------------------------------------------------------
  Parámetro                           Valor
  ----------------------------------- -----------------------------------
  Broker / Server                     `mosquitto`

  Puerto                              `1883`

  Protocolo                           MQTT 3.1.1

  Usuario / contraseña                Vacíos para la configuración
                                      anónima de laboratorio

  TLS                                 Desactivado en esta práctica
  -----------------------------------------------------------------------

**No utilices `localhost` como broker desde el contenedor Node-RED.**
Dentro del contenedor, `localhost` hace referencia al propio contenedor
Node-RED. El nombre de servicio `mosquitto` permite acceder al broker
por la red interna de Docker Compose.

### 7.3 Conexión desde el ESP32

El ESP32 debe utilizar la dirección IP LAN del computador donde se
ejecuta Docker:

``` cpp
const char* servidorMQTT = "IP_LAN_DEL_COMPUTADOR";
const int puertoMQTT = 1883;
```

No utilizar `localhost` ni la IP del propio ESP32 como dirección del
broker.

Para consultar la IP Wi-Fi habitual del Mac:

``` bash
ipconfig getifaddr en0
```

Para comprobar si el puerto está disponible desde el Mac:

``` bash
nc -vz IP_LAN_DEL_COMPUTADOR 1883
```

La IP `10.51.43.236` utilizada durante las pruebas corresponde al Mac en
aquella sesión. Debe verificarse nuevamente en cada entorno.

## 8. Configuración del firmware

El código fuente del ESP32 se encuentra en:

``` text
firmware/esp32-mosquitto/
```

Abre el sketch correspondiente en Arduino IDE.

### 8.1 Librerías

Instalar desde el administrador de bibliotecas de Arduino IDE:

-   `PubSubClient`.
-   `DHT sensor library` de Adafruit.
-   `Adafruit Unified Sensor`.

### 8.2 Credenciales Wi-Fi y broker

Antes de cargar el firmware, configurar:

``` cpp
const char* nombreRed = "NOMBRE_DE_TU_WIFI";
const char* contrasena = "CONTRASENA_WIFI";

const char* servidorMQTT = "IP_LAN_DEL_COMPUTADOR";
const int puertoMQTT = 1883;
```

No publicar credenciales reales en el repositorio.

### 8.3 Asignación de pines de referencia

  Componente     GPIO propuesto
  ------------ ----------------
  DHT11                       5
  LED 1                      18
  LED 2                      19

Los GPIO deben verificarse según el modelo específico de ESP32. En
particular, si se utiliza una ESP32-C5 Mini u otra variante, consultar
su pinout y confirmar que los pines estén expuestos y disponibles.

### 8.4 Cargar y verificar

1.  Seleccionar la placa ESP32 correcta.
2.  Seleccionar el puerto correspondiente.
3.  Compilar y cargar el firmware.
4.  Abrir el monitor serial a `115200` baudios.
5.  Verificar la conexión Wi-Fi.
6.  Verificar la conexión MQTT y la suscripción a los tópicos de los
    LED.
7.  Confirmar que se publican las mediciones.

## 9. Configuración de Node-RED

### 9.1 Instalar Dashboard 2.0

En Node-RED:

1.  Abrir el menú superior derecho.
2.  Seleccionar **Manage Palette**.
3.  Abrir la pestaña **Install**.
4.  Buscar `@flowfuse/node-red-dashboard`.
5.  Instalar el paquete.

### 9.2 Flujo de telemetría

Crear dos nodos `mqtt in`:

-   `clase/esp32/temperatura`
-   `clase/esp32/humedad`

Conectar cada entrada a un nodo `debug` para verificar los mensajes.

Después, añadir componentes Dashboard 2.0:

-   Indicador de temperatura (`ui-gauge`).
-   Indicador de humedad (`ui-gauge`).
-   Gráfica de temperatura (`ui-chart`).
-   Gráfica de humedad (`ui-chart`).

Si el payload llega como texto, convertirlo a número antes de alimentar
los indicadores o gráficas:

``` javascript
msg.payload = Number(msg.payload);
return msg;
```

### 9.3 Flujo de control de actuadores

Crear cuatro botones `ui-button` con los siguientes valores:

  Botón            Payload   Topic
  ---------------- --------- --------------------
  Encender LED 1   `ON`      `clase/esp32/led1`
  Apagar LED 1     `OFF`     `clase/esp32/led1`
  Encender LED 2   `ON`      `clase/esp32/led2`
  Apagar LED 2     `OFF`     `clase/esp32/led2`

Conectar las salidas de los botones a un nodo `mqtt out` configurado con
el broker `mosquitto:1883`.

En el nodo `mqtt out`, dejar vacío el tópico fijo para que utilice
`msg.topic` enviado por cada botón. Mantener desactivada la opción
`retain` para los comandos.

### 9.4 Importar flujos JSON

Si el repositorio contiene flujos exportados:

1.  Abrir Node-RED.
2.  Seleccionar **Menu → Import**.
3.  Pegar o seleccionar el JSON.
4.  Importar el flujo.
5.  Revisar la configuración del broker y los nodos Dashboard.
6.  Pulsar **Deploy**.

La estructura interna de los nodos Dashboard 2.0 puede variar según la
versión instalada. Si un nodo aparece como desconocido, comprobar que el
paquete `@flowfuse/node-red-dashboard` esté instalado.

## 10. Pruebas de funcionamiento

### 10.1 Suscribirse a los mensajes MQTT

Ejecutar en una terminal:

``` bash
docker exec -it mosquitto mosquitto_sub \
  -h localhost \
  -t "clase/esp32/#" \
  -v
```

Deben aparecer los mensajes de temperatura y humedad publicados por el
ESP32.

### 10.2 Enviar comandos manuales

Encender LED 1:

``` bash
docker exec mosquitto mosquitto_pub \
  -h localhost \
  -t "clase/esp32/led1" \
  -m "ON"
```

Apagar LED 1:

``` bash
docker exec mosquitto mosquitto_pub \
  -h localhost \
  -t "clase/esp32/led1" \
  -m "OFF"
```

Para controlar el LED 2, utilizar el tópico `clase/esp32/led2`.

### 10.3 Validación desde el dashboard

1.  Abrir <http://localhost:1880/dashboard>.
2.  Confirmar que los indicadores y gráficas reciben valores.
3.  Pulsar **Encender LED 1** y verificar el LED físico y el monitor
    serial.
4.  Pulsar **Apagar LED 1** y verificar el apagado.
5.  Repetir las pruebas con el LED 2.
6.  Confirmar que la telemetría continúa durante el control remoto.

## 11. Solución de problemas

  -----------------------------------------------------------------------------------------------
  Síntoma                            Posible causa               Acción correctiva
  ---------------------------------- --------------------------- --------------------------------
  `undefined reference to setup()` o El sketch compilado no      Verificar que `setup()` y
  `loop()`                           contiene las funciones      `loop()` estén en el archivo
                                     Arduino requeridas o está   `.ino` principal y fuera de
                                     mal organizado.             comentarios. Probar con un
                                                                 sketch mínimo.

  Monitor serial vacío o ilegible    Baud rate, puerto o cable   Usar `115200` con el firmware de
                                     incorrecto.                 esta práctica; seleccionar el
                                                                 puerto correcto y un cable USB
                                                                 de datos.

  DHT11 devuelve `NaN`               Cableado, GPIO,             Verificar VCC, DATA y GND;
                                     alimentación o sensor       revisar `PIN_DHT` y la
                                     defectuoso.                 resistencia pull-up si es un
                                                                 sensor desnudo.

  ESP32 no conecta a Wi-Fi           Credenciales incorrectas,   Verificar SSID y contraseña;
                                     señal débil o red           comprobar compatibilidad de la
                                     incompatible/restringida.   placa y de la red.

  PubSubClient devuelve `-2`         No se establece conexión    Verificar IP LAN del computador,
                                     TCP con el broker.          puerto 1883, Docker, firewall y
                                                                 conectividad entre redes.

  El firmware apunta a la IP del     Se configuró la IP del      Utilizar la IP LAN del
  ESP32                              dispositivo como broker.    computador donde corre
                                                                 Mosquitto.

  `Connection refused` al probar el  No hay listener o el puerto Revisar `docker ps`,
  puerto                             no está publicado.          `mosquitto.conf`, puertos de
                                                                 Compose y logs.

  `Operation timed out`              Firewall, VLAN, aislamiento Probar una red que permita
                                     Wi-Fi o ruta bloqueada.     comunicación entre dispositivos
                                                                 y revisar las reglas de red.

  Node-RED no conecta con Mosquitto  Se usó `localhost` o la IP  Usar `mosquitto:1883` desde
                                     del host en la conexión     Node-RED.
                                     interna de Docker.          

  Nodos Dashboard desconocidos       Dashboard 2.0 no está       Instalar
                                     instalado.                  `@flowfuse/node-red-dashboard` y
                                                                 reiniciar Node-RED.

  Dashboard vacío                    No llegan mensajes, tópico  Revisar nodos Debug, tópicos,
                                     equivocado o payload no     grupos, página y conversión
                                     numérico.                   numérica.

  Los botones no controlan los LED   Tópico o payload            Revisar `ON`/`OFF`, tópicos
                                     incorrecto, ESP32 no        exactos, monitor serial y
                                     suscrito o GPIO equivocado. pinout.

  LED no responde o lógica invertida Polaridad incorrecta o      Revisar ánodo/cátodo y adaptar
                                     módulo activo en LOW.       la lógica de salida al circuito.
  -----------------------------------------------------------------------------------------------

### 11.1 Estados de conexión de PubSubClient

    Código Interpretación habitual
  -------- ----------------------------------------
      `-4` Tiempo de espera de conexión agotado.
      `-3` Conexión perdida.
      `-2` No se pudo establecer conexión TCP.
      `-1` Cliente desconectado.
       `1` Versión de protocolo MQTT no aceptada.
       `2` Identificador de cliente rechazado.
       `3` Servidor MQTT no disponible.
       `4` Usuario o contraseña incorrectos.
       `5` No autorizado.

Para el estado `-2`, comenzar por revisar IP, puerto, firewall, Docker y
conectividad de red. No modificar credenciales como primer intento.

## 12. Seguridad y buenas prácticas

-   No exponer el puerto `1883` a Internet.
-   No mantener `allow_anonymous true` fuera de un laboratorio
    controlado.
-   En producción, configurar autenticación, ACL por tópico, TLS y
    firewall.
-   Utilizar identificadores MQTT únicos por dispositivo.
-   Validar los payloads recibidos antes de activar salidas físicas.
-   No publicar credenciales Wi-Fi ni secretos en el repositorio.
-   Para instalaciones permanentes, reservar una IP DHCP para el
    computador o utilizar un nombre DNS estable.
-   Verificar los GPIO antes de conectar los actuadores.

## 13. Aplicación académica

Este proyecto puede utilizarse como práctica de integración de sistemas
IoT para estudiar:

-   Arquitecturas IoT.
-   Sistemas embebidos.
-   Sensores y actuadores.
-   Comunicación inalámbrica.
-   Protocolos MQTT.
-   Modelo Publish/Subscribe.
-   Contenedorización con Docker.
-   Programación visual con Node-RED.
-   Visualización y monitoreo de variables.
-   Control remoto de dispositivos.

### Secuencia sugerida para una clase de 60 minutos

  -----------------------------------------------------------------------
  Tiempo                  Actividad               Evidencia de
                                                  aprendizaje
  ----------------------- ----------------------- -----------------------
  0--5 min                Presentación del caso y Identifica sensor,
                          arquitectura.           nodo, broker y
                                                  dashboard.

  5--15 min               DHT11 y lectura ESP32.  Observa mediciones en
                                                  Serial.

  15--25 min              Wi-Fi y publicación     Reconoce tópicos y
                          MQTT.                   payloads.

  25--35 min              Node-RED y dashboard.   Visualiza indicadores y
                                                  gráficas.

  35--48 min              Control bidireccional   Envía comandos ON/OFF
                          de LED.                 desde el dashboard.

  48--55 min              Desconexión y           Interpreta un fallo y
                          diagnóstico.            plantea una solución.

  55--60 min              Cierre y socialización. Explica el recorrido
                                                  del dato y del comando.
  -----------------------------------------------------------------------

## 14. Referencias técnicas

-   [Espressif
    Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/)
-   [Eclipse Mosquitto](https://mosquitto.org/documentation/)
-   [Docker Compose](https://docs.docker.com/compose/)
-   [Node-RED](https://nodered.org/docs/)
-   [FlowFuse Dashboard 2.0](https://dashboard.flowfuse.com/)
-   [MQTT 3.1.1
    Specification](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/)
-   [PubSubClient](https://github.com/knolleary/pubsubclient)
-   [Adafruit DHT Sensor
    Library](https://github.com/adafruit/DHT-sensor-library)
-   [Adafruit Unified
    Sensor](https://github.com/adafruit/Adafruit_Sensor)

------------------------------------------------------------------------

**Proyecto académico de Internet de las Cosas (IoT)**

ESP32 · DHT11 · MQTT · Mosquitto · Docker · Node-RED
