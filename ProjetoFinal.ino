#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// ========= BME680 / MPU / SERVO =========
Adafruit_BME680 bme;
MPU6050 mpu;
Servo servo;

long ax_offset = 0, ay_offset = 0, az_offset = 0;
int pinoServo = 2;            // Pino Servo
int potenciometroMotor = A1;  // Pino Potenciometro velocidade
int potenciometroLeme  = A0;  // Pino Potenciometro leme
const int RPWM = 4;           // PWM direita (ponte H)
const int LPWM = 3;           // PWM esquerda (ponte H)

bool estadomotor = false;     // Estado do motor (ligado/desligado)
int direcaoMotor = 1;         // 1 = frente, 2 = esquerda, 3 = direita
unsigned long ultimoEnvio = 0;// millis (para prints)
int angulo = 90;              // angulo inicial do leme

// ========= ENCODER (LM393) =========
const int pinoInterrupt = 3;
// D0 do LM393 -> pino 3 (interrupt no Mega)

// *** MUDADO PARA A2 (Solicitado) ***
const int pinoAnalogico = A2; 
// A2 do LM393 -> A2 do Arduino (analógico)

volatile unsigned long ultimoPulso = 0;
volatile unsigned long contadorPulsos = 0; //COnta Pulsos
int girosCompletos = 0; // Conta Giros
const int dentesPorVolta = 20;          // sua roda tem 20 dentes
const unsigned long debounceUS = 1500;  // 1.5 ms de debounce no encoder

float RPM = 0; // Rotações por minuto
unsigned long agoraRPM = 0; //RPM atual

// --- PINOS DO TANQUE ---
const int pinoTrig = A3; 
const int pinoEcho = A4; 

// --- CONFIGURAÇÃO DO TANQUE (cm) ---
const int altMax = 28;    // Altura vazio
const int distMin = 2;    // Altura cheio

// ---------- INTERRUPÇÃO DO ENCODER ----------
void contarPulso() {
  unsigned long agora = micros();
  // Debounce: ignora pulsos muito próximos
  if (agora - ultimoPulso > debounceUS) {
    contadorPulsos++;
    ultimoPulso = agora;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando BME680...");
  Serial.println("Iniciando MPU6050 para detectar vibracao...");

  Wire.begin(); // I2C no Mega: SDA=20, SCL=21
  mpu.initialize();    // inicializa o sensor

  // Tenta iniciar o BME680 no endereco I2C padrao (0x76 ou 0x77)
  if (!bme.begin()) {
    Serial.println("ERRO: BME680 nao encontrado. Verifique as ligacoes e o endereco I2C.");
    while (1);
  }

  // Configuracoes recomendadas pela Adafruit
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(0, 0); // (temperatura em °C, tempo em ms)

  Serial.println("BME680 iniciado com sucesso!");
  
  // Tenta conectar iniciar o MPU
  if (!mpu.testConnection()) {
    Serial.println("ERRO: MPU6050 nao conectado!");
    while (1);
  }
  Serial.println("MPU6050 conectado. Fazendo calibracao rapida...");

  // --- Calibracao simples: tirar a média de N leituras parado ---
  const int N = 200;
  long soma_ax = 0, soma_ay = 0, soma_az = 0;
  for (int i = 0; i < N; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    soma_ax += ax;
    soma_ay += ay;
    soma_az += az;

    delay(10);
  }

  ax_offset = soma_ax / N;
  ay_offset = soma_ay / N;
  az_offset = soma_az / N;

  Serial.println("Calibracao concluida!");
  Serial.print("Offsets -> ax: "); Serial.print(ax_offset);
  Serial.print(" | ay: ");        Serial.print(ay_offset);
  Serial.print(" | az: ");        Serial.println(az_offset);

  Serial.println("Iniciando leitura de vibracao...\n");
  
  // setar os componentes
  pinMode(potenciometroMotor, INPUT);
  pinMode(potenciometroLeme, INPUT);
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);

  // Servo
  servo.attach(pinoServo);
  servo.write(90);
  
  // Encoder
  pinMode(pinoInterrupt, INPUT_PULLUP);
  pinMode(pinoAnalogico, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinoInterrupt), contarPulso, RISING);

  // Tanque
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  Serial.println("Sistema iniciado — encoder (A2) + motor + sensores + tanque (A3/A4).");
}

void loop() {

  // -------- MPU6050: LEITURA BRUTA --------
  int16_t ax_raw, ay_raw, az_raw, gx, gy, gz;
  mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx, &gy, &gz);

  // -------- MOTOR DC: AJUSTE DE VELOCIDADE VIA POTENCIÔMETRO --------
  int potencia = analogRead(potenciometroMotor); // velocidade atual
  int velocidade = map(potencia, 0, 1023, 14, 255);
  analogWrite(RPWM, velocidade); // PWM para o motor (frente)
  analogWrite(LPWM, 0);

  // Estado do motor
  estadomotor = (velocidade > 16);
  
  // -------- SERVO (LEME): AJUSTE DE ÂNGULO --------
  int potenciaL = analogRead(potenciometroLeme); // angulo atual leme
  angulo = map(potenciaL, 0, 1023, 0, 180);
  servo.write(angulo);
  
  // -------- ENCODER: CÁLCULO DE GIROS E RPM --------
  // Verifica se completou uma volta
  if (contadorPulsos >= dentesPorVolta) {
    girosCompletos++;
    contadorPulsos = 0;
  }

  // Leitura analógica do LM393 (debug/ajuste do trimpot)
  int analogValue = analogRead(pinoAnalogico);
  
  // Calcula RPM a cada 1 segundo
  if (millis() - agoraRPM >= 1000) {
    agoraRPM = millis();
    RPM = girosCompletos * 60.0; // giros por segundo * 60 = RPM
    girosCompletos = 0; // zera para próxima janela de 1s
  }

  // -------- PRINT GERAL (MOTOR + DIREÇÃO + ENCODER + TANQUE) --------
  if (millis() - ultimoEnvio >= 1000) {
    ultimoEnvio = millis();
    
    // Leitura do Tanque para o print
    int nivelAtual = lerNivelTanque();

    Serial.print("Motor: ");
    Serial.print(estadomotor ? "LIGADO" : "DESLIGADO");
    Serial.print(" | Velocidade: ");
    Serial.print(velocidade);

    Serial.print(" | Leme: ");
    Serial.print(angulo);

    // Direção
    if (angulo < 80) {
      direcaoMotor = 2;
      Serial.print(" (Esq)");
    } else if (angulo > 100) {
      direcaoMotor = 3;
      Serial.print(" (Dir)");
    } else {
      direcaoMotor = 1;
      Serial.print(" (Frente)");
    }

    // Encoder
    Serial.print(" | RPM: ");
    Serial.print(RPM);
    Serial.print(" | Enc(A2): ");
    Serial.print(analogValue);
    
    // Tanque
    Serial.print(" | Tanque: ");
    Serial.print(nivelAtual);
    Serial.println("%");
  }

  // -------- BME680: LEITURA DE AMBIENTE --------
  if (!bme.performReading()) {
    Serial.println("Falha na leitura do BME680 :(");
  } else {
    float temperatura = bme.temperature;
    float umidade     = bme.humidity;
    float pressao_hPa = bme.pressure / 100.0;

    Serial.println("------ Leitura BME680 ------");
    Serial.print("Temp: ");
    Serial.print(temperatura);
    Serial.print(" C | Umid: ");
    Serial.print(umidade);
    Serial.print(" % | Press: ");
    Serial.print(pressao_hPa);
    Serial.println(" hPa");
  }
  Serial.println("----------------------------\n");

  // -------- MPU6050: CÁLCULO DE INTENSIDADE DE VIBRACAO --------
  long ax_corr = ax_raw - ax_offset;
  long ay_corr = ay_raw - ay_offset;
  long az_corr = az_raw - az_offset;

  float ax_g = ax_corr / 16384.0;
  float ay_g = ay_corr / 16384.0;
  float az_g = az_corr / 16384.0;

  float intensidade = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);

  Serial.print("Vibracao (g): ");
  Serial.println(intensidade, 3);

  delay(1000); 
}

// --- FUNÇÃO DE LEITURA DO TANQUE ---
int lerNivelTanque() {
  // 1. Pulso
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  // 2. Leitura
  long duracao = pulseIn(pinoEcho, HIGH);
  
  // 3. Conversão para cm
  int distancia = duracao * 0.034 / 2;
  
  // 4. Limites de segurança física
  if (distancia > altMax) distancia = altMax;
  if (distancia < distMin) distancia = distMin;

  // 5. Cálculo da porcentagem
  int nivelLiquido = altMax - distancia;
  int porcentagem = map(nivelLiquido, 0, (altMax - distMin), 0, 100);
  
  // Retorna o valor limpo entre 0 e 100
  return constrain(porcentagem, 0, 100);
}