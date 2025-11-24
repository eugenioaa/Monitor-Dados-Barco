#include <TinyGPS++.h>

TinyGPSPlus gps;

bool infoValida = false;
float atualLatitude = 0.0;
float atualLongitude = 0.0;
float atualHDOP = 99.9; 
uint8_t quantSatelites = 0;
uint8_t dia = 0;
uint8_t mes = 0;
uint16_t ano = 0;
uint8_t hora = 0;
uint8_t min = 0;
uint8_t sec = 0;

unsigned long ultimoEnvio = 0;
const unsigned long intervaloEnvio = 15000; 


void setupGPS();
void runGPS();
void populateGPSData();
void displayGPSData();
void enviarLoRa();


void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);

  delay(1000);
  Serial.println("--- Iniciando Sistema GPS + LoRa ---");

  Serial2.println("AT+JOIN");
  delay(2000);
}

void loop()
{
  runGPS();

  if (millis() - ultimoEnvio > intervaloEnvio) {
    
    if (infoValida) {
      enviarLoRa();
      ultimoEnvio = millis();
    } else {
      Serial.println("GPS sem Fix: Aguardando sinal valido para enviar...");
      ultimoEnvio = millis();
    }
  }

  if (Serial2.available()) {
    String resposta = Serial2.readStringUntil('\n');
    if (resposta.length() > 1) {
      Serial.print("[Status LoRa]: "); 
      Serial.println(resposta);
    }
  }

}

void runGPS()
{
  while (Serial1.available() > 0)
  {
    gps.encode(Serial1.read());
  }

  if (gps.location.isUpdated())
  {
    populateGPSData();
    displayGPSData(); 
  }
}


void populateGPSData()
{
  infoValida =  gps.location.isValid();

  //Posição precisa do barco
  atualLatitude = (float)gps.location.lat();
  atualLongitude = (float)gps.location.lng();

  // Data e Hora
  dia = (uint8_t)gps.date.day();
  mes = (uint8_t)gps.date.month();
  ano = (uint16_t)gps.date.year();
  hora = (uint8_t)gps.time.hour();
  min = (uint8_t)gps.time.minute();
  sec = (uint8_t)gps.time.second();
}

void displayGPSData()
{
  Serial.println("--- [GPS] Dados Atuais ---");
  Serial.print("Status: ");
  Serial.println(infoValida ? "VALIDO" : "AGUARDANDO FIX");

  Serial.print("Pos: ");
  Serial.print(atualLatitude, 6);
  Serial.print(", ");
  Serial.println(atualLongitude, 6);

  Serial.print("HDOP/Sats: ");
  Serial.print(atualHDOP, 2);
  Serial.print(" / ");
  Serial.println(quantSatelites);
  
  char dateTimeBuffer[64];
  sprintf(dateTimeBuffer, "Data/Hora (UTC): %02d/%02d/%d %02d:%02d:%02d", 
          dia, mes, ano,
          hora, min, sec);
  Serial.println(dateTimeBuffer);
  
  Serial.println("--------------------------");
}

void enviarLoRa()
{
  String msg = String(atualLatitude, 6) + ";" + String(atualLongitude,6);
  String cmd = "AT+SEND=1:" + msg;
  
  Serial.println("------------------------------------------------");
  Serial.print("Enviando pacote LoRa: ");
  Serial.println(cmd);
  
  Serial2.println(cmd);
}
