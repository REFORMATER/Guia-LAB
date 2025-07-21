#include <Wire.h>
#include <RTClib.h>

// Definición de pines
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
const int BUZZER_PIN = 10;
const int SENSOR_IR = 16;
const int UMBRAL_CM = 100;
const int LED_1 = 8;
const int LED_2 = 9;

RTC_DS3231 rtc;  // Objeto para controlar el módulo RTC

bool personaDetectada = false;  // Estado actual del sensor IR
bool objetoPresente = false;    // Estado del sensor ultrasónico
unsigned long tiempoUltimaSalida = 0;  // Tiempo de la última detección

void setup() {
  Serial.begin(9600);
  Wire.begin();  // Inicializa la comunicación I2C

  // Configuración de pines
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SENSOR_IR, INPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  // Verifica si el módulo RTC está conectado correctamente
  if (!rtc.begin()) {
    Serial.println("No se detectó el módulo RTC");
    while (1);  // Detiene el programa si falla el RTC
  }

  // Ajusta manualmente la hora del RTC (año, mes, día, hora, minuto, segundo)
  rtc.adjust(DateTime(2025, 7, 20, 18, 30, 0));
}

void loop() {
  long duracion, distancia;
  unsigned long ahoraMillis = millis();

  // Enviar pulso ultrasónico
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Calcular duración y convertir a distancia en cm
  duracion = pulseIn(ECHO_PIN, HIGH);
  distancia = duracion * 0.034 / 2;

  // Si la distancia es menor o igual al umbral, hay un objeto cerca
  if (distancia > 0 && distancia <= UMBRAL_CM) {
    if (!objetoPresente) {
      // Imprimir la hora actual al detectar el objeto
      DateTime ahora = rtc.now();
      Serial.print("Objeto detectado a las ");
      if (ahora.hour() < 10) Serial.print("0");
      Serial.print(ahora.hour());
      Serial.print(":");
      if (ahora.minute() < 10) Serial.print("0");
      Serial.print(ahora.minute());
      Serial.print(":");
      if (ahora.second() < 10) Serial.print("0");
      Serial.println(ahora.second());
    }

    objetoPresente = true;
    digitalWrite(BUZZER_PIN, HIGH);  // Encender buzzer
    digitalWrite(LED_1, HIGH);       // Encender LEDs
    digitalWrite(LED_2, HIGH);
    tiempoUltimaSalida = ahoraMillis;  // Guardar tiempo actual
  }

  // Si ya no hay objeto y han pasado 5 segundos, apagar todo
  else if (objetoPresente && (ahoraMillis - tiempoUltimaSalida >= 5000)) {
    objetoPresente = false;
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("Zona despejada");
  }

  // Leer sensor infrarrojo
  int lecturaIR = digitalRead(SENSOR_IR);

  // Si se detecta una persona (lectura baja en el FC-51)
  if (lecturaIR == LOW && !personaDetectada) {
    personaDetectada = true;
    Serial.println("Persona detectada");
  }

  // Si ya no hay persona (lectura alta)
  if (lecturaIR == HIGH && personaDetectada) {
    personaDetectada = false;
    Serial.println("Zona libre");
  }

  delay(100);  // Pequeño retardo para evitar lecturas constantes
}