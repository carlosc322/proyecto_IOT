#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Configuración WiFi
const char* ssid = "vivo V";         
const char* password = "carlos12345"; 

// Configuración Firebase https://appmovilb50-14c3d-default-rtdb.firebaseio.com
const char* firebase_host = "appmovilb50-14c3d-default-rtdb.firebaseio.com";
const char* firebase_auth = ""; // Vacío para reglas públicas, o tu auth key

// URLs completas para Firebase REST API
String sensor_url = "https://" + String(firebase_host) + "/SensorData.json";
String actuator_url = "https://" + String(firebase_host) + "/ActuatorControl.json";

// Pines 
const int ledPin = D4;     // LED integrado (GPIO2)
const int ledPWMPin = D5;  // LED externo con PWM (GPIO5)

//sensor---
const float ADC_VREF = 3.3; //Voltaje máximo que A0 puede medi
const int   ADC_MAX  = 1023; //Resolución del ADC del ESP8266
const int sensorPin = A0;
const char* DEVICE_ID = "ESSP8266"; //<- Es el nombre del hardware que envía los datos.
const float TEMP_THRESHOLD = 30.0;//editable

// Variables
unsigned long lastSensorSend = 0;
unsigned long lastLedRead = 0;
const unsigned long sensorInterval = 3000;  // Enviar sensor cada 8 segundos
const unsigned long ledInterval = 5000;     // Leer LED cada 5 segundos

// Cliente HTTPS
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // Configurar pines
  pinMode(ledPin, OUTPUT);
  pinMode(ledPWMPin, OUTPUT);
  
  // Test inicial LEDs
  digitalWrite(ledPin, LOW);   
  analogWrite(ledPWMPin, 256); 
  delay(1000);
  digitalWrite(ledPin, HIGH);  
  analogWrite(ledPWMPin, 0);   
  
  // Conectar WiFi
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("✓ WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Configurar cliente HTTPS
  client.setInsecure(); // Para desarrollo - ignora certificados SSL
  // Para producción usar: client.setFingerprint() o client.setCACert()
  
  Serial.println("✓ Cliente HTTPS configurado");
  
  // Configurar estructura inicial en Firebase
  setupFirebaseStructure();
  
  Serial.println();
  Serial.println("=== SISTEMA FIREBASE HTTPS ===");
  Serial.println("✓ Sensor: A0 → /sensor_data");
  Serial.println("✓ Actuador: /actuator_control → D1");
  Serial.println("✓ Protocolo: HTTPS (SSL)");
  Serial.println("==============================");
  Serial.println();
}


  //BLOQUE LOOP() -> CONTROL DE TIEMPO-----------------------------------------------------------------------
void loop() {
  unsigned long currentTime = millis();
  
  // Verificar conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {//Si no está conectado, entra al if.
    Serial.println("WiFi desconectado, reconectando...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {//Este while se queda en un bucle revisando cada segundo, Sale del bucle solo cuando logra conectarse.
      delay(1000);
      Serial.print(".");
    }
    Serial.println("WiFi reconectado");
  }
  // Enviar datos del sensor cada 8 segundos
  if (currentTime - lastSensorSend >= sensorInterval) { 
    //currentTime → es el tiempo actual (generalmente millis()).
    //lastSensorSend → guarda la última vez que se envió el sensor.
    //sensorInterval → el tiempo configurado (por ejemplo, 8000 ms = 8 segundos).
    //Si ya pasaron 8 segundos, entonces:
    sendSensorDataToFirebase();
    lastSensorSend = currentTime;//Se envían los datos a Firebase, Se actualiza el tiempo.
  }
  
  // Leer control del actuador cada 5 segundos
  if (currentTime - lastLedRead >= ledInterval) {
    //ledInterval normalmente vale 5000 ms = 5 segundos.
    //Cada 5 segundos:
    readActuatorControlFromFirebase();
    lastLedRead = currentTime;//Esto lee el estado del ventilador o LED desde Firebase.
  }
  
  delay(500);//Esto solo: pausa el programa 0,5 segundos
}

void setupFirebaseStructure() { //<-Crear datos iniciales en Firebase cuando el Wemos prende, Crea una estructura JSON:
  Serial.println("Configurando estructura inicial en Firebase...");
  
  // Estructura inicial para el actuador
  StaticJsonDocument<200> actuatorConfig;//Crea un contenedor para guardar datos en formato JSON.
  //Después llena ese JSON con valores por defecto:
  actuatorConfig["intensity"] = 100;        
  actuatorConfig["enabled"] = true;         
  actuatorConfig["mode"] = "manual";        
  actuatorConfig["last_update"] = millis();
  
  String jsonString;
  serializeJson(actuatorConfig, jsonString);//Pasa el JSON a un String, porque Firebase necesita texto plano para recibir datos.
  
  if (sendHttpsRequest("PUT", actuator_url, jsonString)) {
    //Usa el método PUT (sobre-escribe datos).
    //Envía el JSON a la URL de Firebase.
    Serial.println("✓ Estructura actuador creada");//Si funciona:
  } else {
    Serial.println("✗ Error creando estructura");//Si falla:
  }
  
  delay(2000);//Solo espera 2 segundos antes de continuar.
}

//SENSOR ---------------------------------------------------------------------------------------------------------------

void sendSensorDataToFirebase() {
  // Leer sensor
  int rawValue = analogRead(sensorPin); //Paso 1: Leer el sensor, Devuelve un número entre 0(frio) y 1023(caliente).
  float voltage = rawValue * (ADC_VREF / ADC_MAX);//Paso 2: Convertirlo a voltaje
  float temp = (voltage - 0.5)*100.0;
  
  //Mostrar datos por Serial, Esto es solo para que tú lo veas en la computadora
  Serial.println("=== ENVIANDO DATOS SENSOR ==="); 
  Serial.print("Valor: ");
  Serial.print(rawValue);
  Serial.print(" (");
  Serial.print(voltage, 2);
  Serial.println("V)");

  bool supera = (temp >= TEMP_THRESHOLD);
  // Crear JSON
  StaticJsonDocument<300> sensorJson;//Paso 3: Crear un paquete JSON, Esto es lo que realmente se enviará a Firebase.
  sensorJson["raw_value"] = rawValue;
  sensorJson["voltage"] = round(voltage * 100) / 100.0;
  sensorJson["timestamp"] = millis();
  sensorJson["device"] = DEVICE_ID;
  sensorJson["wifi_rssi"] = WiFi.RSSI();
  sensorJson["temp"] = round(temp * 10)/10.0 ;
  sensorJson["supera_max"]= supera;
  String jsonString;
  serializeJson(sensorJson, jsonString);//Convierte el JSON a texto plano para poder enviarlo.
  
  Serial.print("JSON: ");
  Serial.println(jsonString);
  
  // Enviar vía HTTPS
  if (sendHttpsRequest("PUT", sensor_url, jsonString)) {//Paso 4: Enviarlo a Firebase
    //Usa HTTPS
    //Hace un PUT
    //Envía los datos a Firebase
    Serial.println("✓ Datos enviados via HTTPS");
  } else {
    Serial.println("✗ Error enviando datos");
  }
  Serial.println();
}

//ACTUADOR -----------------------------------------------------------------------------------------------
void readActuatorControlFromFirebase() {
  Serial.println("=== LEYENDO CONTROL ACTUADOR ===");

  //Hacer una petición HTTPS a Firebase
  String response = "";
  if (getHttpsRequest(actuator_url, response)) {
    //actuator_url → la ruta en Firebase donde está la configuración del ventilador.
    //response → aquí se guarda el JSON que Firebase responde.
    //Esto te muestra el JSON crudo para poder revisarlo:
    Serial.print("Respuesta: ");
    Serial.println(response);
    
    // Parsear JSON response, Convertir el JSON recibido en variables
    StaticJsonDocument<300> doc;
    DeserializationError error = deserializeJson(doc, response);

    //Extraer los valores del JSON
    if (!error) {
      int intensity = doc["intensity"] | 100;//nivel de potencia del ventilador
      bool enabled = doc["enabled"] | true;
      String mode = doc["mode"] | "manual";
      
      Serial.print("Intensidad: ");
      Serial.println(intensity);
      Serial.print("Habilitado: ");
      Serial.println(enabled ? "SÍ" : "NO");
      Serial.print("Modo: ");
      Serial.println(mode);
      
      // Aplicar control, Aplicar el control al ventilador
      applyActuatorControl(intensity, enabled, mode);//Esta es la línea que controla físicamente el actuador.
      //Aquí es donde el Wemos:
      //enciende / apaga el ventilador
      //Cambia la velocidad
      //Aplica el modo de funcionamiento
    } else {
      Serial.print("✗ Error JSON: ");
      Serial.println(error.c_str());
    }
    
  } else {
    Serial.println("✗ Error leyendo de Firebase");
  }
  
  Serial.println();
}

//APLIACION AL ACTUADOR
void applyActuatorControl(int intensity, bool enabled, String mode) {//<- funcion: Traducir esos valores en una señal PWM real
  if (!enabled) {//Si enabled es false, apaga todo inmediatamente y sale de la función.
    digitalWrite(ledPin, HIGH);    // OFF, apaga el LED integrado(en ESP8266 el LED suele ser active LOW, por eso HIGH = OFF).
    analogWrite(ledPWMPin, 0);     // OFF, Apaga el ventilador o LED PWM
    Serial.println("→ Actuador DESHABILITADO");
    return;
  }
  
  int pwmValue = 0;//Inicializa la variable que contendrá la potencia que se aplicará al actuador (0 = apagado, 255 = máximo en este código).

  //MODO MANUAL
  if (mode == "manual") {
    pwmValue = constrain(intensity, 0, 255);//<- ES COMO UNA VALIDACION _ Lo guarda en pwmValue para usarlo en analogWrite()
    //Usa directamente el valor intensity que venga desde Firebase.
    //constrain(...,0,255) asegura que pwmValue quede entre 0 y 255 (evita valores fuera de rango).

    //MODO AUTO
    //Nota práctica: analogRead en ESP8266 da 0–1023; map aquí transforma esa escala a 0–255.
  } else if (mode == "auto") {
    int sensorValue = analogRead(sensorPin);//Lee  el sensor ahora mismo (analogRead devuelve ~0–1023).
    pwmValue = map(sensorValue, 0, 1024, 0, 255);//map(sensorValue, 0, 1024, 0, 255) convierte la lectura del sensor a la escala 0–255 para PWMEj.: sensor=512 → pwm ≈ 127..
    //map(valor, minimoEntrada, maximoEntrada, minimoSalida, maximoSalida)<- Convertir un valor del sensor que está entre 0 y 1024 a un valor de PWM entre 0 y 255.
    Serial.print("→ AUTO - Sensor: ");
    Serial.print(sensorValue);
    Serial.print(" → PWM: ");
    Serial.println(pwmValue);
  } else if (mode == "off") {
    pwmValue = 0;//Fuerza apagado total (pwm = 0).
  }
  
  // Aplicar PWM
  analogWrite(ledPWMPin, pwmValue);//Envía el valor PWM al pin que controla la potencia del actuador (LED externo o ventilador vía driver).
  
  // LED indicador
  if (pwmValue > 10) {
    digitalWrite(ledPin, LOW);   // ON <- Si la potencia es pequeña (>10) enciende el LED indicador (LOW = ON en la placa).
  } else {
    digitalWrite(ledPin, HIGH);  // OFF <- Si es muy bajo o 0, apaga el indicador.
  }

  //Mostrar porcentaje
  //Calcula el % relativo a 255 y lo imprime con 1 decimal.
  //Ej.: pwmValue = 128 → ~50.2%.
  float porcentaje = (pwmValue / 255.0) * 100.0;
  Serial.print("→ LED PWM: ");
  Serial.print(pwmValue);
  Serial.print("/255 (");
  Serial.print(porcentaje, 1);
  Serial.println("%)");
  //Se usa para depuración (para que TÚ lo veas en el Serial Monitor).
}

bool sendHttpsRequest(String method, String url, String payload) {//una función genérica para enviar peticiones HTTPS al servidor de Firebase.
  HTTPClient https;
  
  // Agregar auth si existe
  if (strlen(firebase_auth) > 0) {
    url += "?auth=" + String(firebase_auth);
  }
  
  Serial.print("→ ");
  Serial.print(method);
  Serial.print(" ");
  Serial.println(url);
  
  if (https.begin(client, url)) {
    https.addHeader("Content-Type", "application/json");
    https.setTimeout(10000); // 10 segundos timeout
    
    int httpCode;
    if (method == "PUT") {
      httpCode = https.PUT(payload);
    } else if (method == "POST") {
      httpCode = https.POST(payload);
    } else {
      httpCode = https.GET();
    }
    
    Serial.print("→ HTTP Code: ");
    Serial.println(httpCode);
    
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String response = https.getString();
        Serial.print("→ Response: ");
        Serial.println(response.substring(0, 100)); // Primeros 100 chars
        https.end();
        return true;
      }
    } else {
      Serial.print("→ Error: ");
      Serial.println(https.errorToString(httpCode));
    }
    
    https.end();
  } else {
    Serial.println("→ Error: No se pudo conectar HTTPS");
  }
  
  return false;
}

bool getHttpsRequest(String url, String &response) {
  HTTPClient https;
  
  // Agregar auth si existe
  if (strlen(firebase_auth) > 0) {
    url += "?auth=" + String(firebase_auth);
  }
  
  Serial.print("→ GET ");
  Serial.println(url);
  
  if (https.begin(client, url)) {
    https.setTimeout(10000);
    
    int httpCode = https.GET();
    Serial.print("→ HTTP Code: ");
    Serial.println(httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
      response = https.getString();
      https.end();
      return true;
    } else {
      Serial.print("→ Error: ");
      Serial.println(https.errorToString(httpCode));
    }
    
    https.end();
  }
  
  return false;
}

// Función para mostrar memoria disponible
void printMemoryInfo() {
  Serial.println("=== MEMORIA ===");
  Serial.print("Heap libre: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Fragmentación: ");
  Serial.println(ESP.getHeapFragmentation());
  Serial.println("===============");
}
