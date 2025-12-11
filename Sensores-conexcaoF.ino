#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// --- CONFIGURAÇÃO (ARDUINO A) ---
Adafruit_BME680 bme;
MPU6050 mpu;
Servo servo;

// Flags para saber se o sensor ligou
bool statusBME = false;
bool statusMPU = false;

// Calibração e Pinos
long ax_offset = 0, ay_offset = 0, az_offset = 0;
int pinoServo = 2;            
int potenciometroMotor = A1;  
int potenciometroLeme  = A0;  
const int RPWM = 4;           
const int LPWM = 3; 

// Encoder
const int pinoInterrupt = 3;  
const int pinoAnalogico = A2; // A2
volatile unsigned long ultimoPulso = 0;
volatile unsigned long contadorPulsos = 0;
int girosCompletos = 0;
const int dentesPorVolta = 20;          
const unsigned long debounceUS = 1500;  
float RPM = 0;
unsigned long agoraRPM = 0;

// Tanque
const int pinoTrig = A3; 
const int pinoEcho = A4; 
const int altMax = 16;    
const int distMin = 1;    

// Timer Envio
unsigned long ultimoEnvioSerial = 0;

void contarPulso() {
  unsigned long agora = micros();
  if (agora - ultimoPulso > debounceUS) {
    contadorPulsos++;
    ultimoPulso = agora;
  }
}

int lerNivelTanque() {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH);
  
  if (duracao == 0) return 0; // Sensor não respondeu
  
  int distancia = duracao * 0.034 / 2;
  if (distancia > altMax) distancia = altMax;
  if (distancia < distMin) distancia = distMin;
  
  int nivelLiquido = altMax - distancia;
  return constrain(map(nivelLiquido, 0, (altMax - distMin), 0, 100), 0, 100);
}

void setup() {
  Serial.begin(9600);    // MONITOR SERIAL (USB)
  Serial3.begin(9600);   // ENVIO PARA O ARDUINO B (PINO 14 TX)
  
  Wire.begin(); 
  Serial.println("\n\n==================================");
  Serial.println("   INICIANDO ARDUINO A (SENSORES)  ");
  Serial.println("==================================");

  // --- DIAGNÓSTICO DO BME680 ---
  Serial.print("Testando BME680... ");
  if (!bme.begin()) {
    statusBME = false;
    Serial.println("FALHA! (Verifique fios SDA/SCL)");
  } else {
    statusBME = true;
    Serial.println("OK! (Sensor Conectado)");
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(0, 0);
  }

  // --- DIAGNÓSTICO DO MPU6050 ---
  Serial.print("Testando MPU6050... ");
  mpu.initialize();
  if (!mpu.testConnection()) {
    statusMPU = false;
    Serial.println("FALHA! (Verifique fios SDA/SCL)");
  } else {
    statusMPU = true;
    Serial.println("OK! (Sensor Conectado)");
    Serial.println("   -> Calibrando MPU (Nao mexa)...");
    long soma_ax = 0, soma_ay = 0, soma_az = 0;
    for (int i = 0; i < 100; i++) {
      int16_t ax, ay, az, gx, gy, gz;
      mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      soma_ax += ax; soma_ay += ay; soma_az += az;
      delay(5);
    }
    ax_offset = soma_ax / 100;
    ay_offset = soma_ay / 100;
    az_offset = soma_az / 100;
    Serial.println("   -> Calibracao Concluida.");
  }

  // --- CONFIG Pinos ---
  pinMode(potenciometroMotor, INPUT);
  pinMode(potenciometroLeme, INPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  servo.attach(pinoServo);
  servo.write(90);
  pinMode(pinoInterrupt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinoInterrupt), contarPulso, RISING);
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  Serial.println("==================================");
  Serial.println("SISTEMA PRONTO. ENVIANDO DADOS...");
  Serial.println("==================================\n");
}

void loop() {
  float tempEnvio = 0.0, umidEnvio = 0.0, pressEnvio = 0.0, vibEnvio = 0.0;

  // 1. Leitura MPU
  if (statusMPU) {
    int16_t ax_raw, ay_raw, az_raw, gx, gy, gz;
    mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx, &gy, &gz);
    float ax_g = (ax_raw - ax_offset) / 16384.0;
    float ay_g = (ay_raw - ay_offset) / 16384.0;
    float az_g = (az_raw - az_offset) / 16384.0;
    vibEnvio = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
  }

  // 2. Leitura BME
  if (statusBME) {
    if (bme.performReading()) {
      tempEnvio = bme.temperature;
      umidEnvio = bme.humidity;
      pressEnvio = bme.pressure / 100.0;
    } else {
      Serial.println("Erro na leitura do BME durante o loop!");
    }
  }

  // 3. Motor/Servo
  int valMotor = analogRead(potenciometroMotor);
  analogWrite(RPWM, map(valMotor, 0, 1023, 14, 255));
  analogWrite(LPWM, 0);
  int valLeme = analogRead(potenciometroLeme);
  servo.write(map(valLeme, 0, 1023, 0, 180));
  
  // 4. Encoder
  if (contadorPulsos >= dentesPorVolta) { girosCompletos++; contadorPulsos = 0; }
  if (millis() - agoraRPM >= 1000) {
    agoraRPM = millis();
    RPM = girosCompletos * 60.0; 
    girosCompletos = 0; 
  }

  // 5. ENVIO PARA ARDUINO B (A cada 1s)
  if (millis() - ultimoEnvioSerial >= 2000) {
    ultimoEnvioSerial = millis();
    int nivel = lerNivelTanque();

    // IMPRIME NO COMPUTADOR (DEBUG) PARA VOCE VER SE ESTA ZERO
    Serial.print("SENSOR STATUS -> Temp: "); Serial.print(tempEnvio);
    Serial.print(" | Vib: "); Serial.print(vibEnvio);
    Serial.print(" | Tanque: "); Serial.print(nivel);
    Serial.println("%");

    // ENVIA PARA O OUTRO ARDUINO
    // Formato CSV: Temp,Umid,Press,Vib,RPM,Tanque
    Serial3.print(tempEnvio, 2); Serial3.print(",");
    Serial3.print(umidEnvio, 2); Serial3.print(",");
    Serial3.print(pressEnvio, 2); Serial3.print(",");
    Serial3.print(vibEnvio, 3); Serial3.print(",");
    Serial3.print(RPM, 1); Serial3.print(",");
    Serial3.println(nivel); 
  }
  delay(10);
}