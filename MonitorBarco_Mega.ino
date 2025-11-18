#include <TinyGPS++.h>

TinyGPSPlus gps;

void setup()
{
  // Inicia o Serial do Computador (Debug)
  Serial.begin(9600);
  Serial.println("Iniciando GPS do Barco...");
  Serial.println("-----------------------------------");

  // Inicializa o Módulo GPS
  setupGPS();
}

void loop()
{
  runGPS();
}


void setupGPS()
{
  // Configuração específica do GPS
  Serial1.begin(9600);
  Serial.println("[GPS] Módulo Iniciado na Serial1");
}

void runGPS()
{
  // 1. Alimentar a biblioteca com dados brutos
  while (Serial1.available() > 0)
  {
    gps.encode(Serial1.read());
  }

  // 2. Verificar se a localização foi atualizada
  if (gps.location.isUpdated())
  {
    displayGPSData();
  }
}

// Função auxiliar apenas para imprimir os dados (Deixa o código mais limpo)
void displayGPSData()
{
  Serial.println("--- [GPS] Nova Leitura ---");

  // 1. Latitude e Longitude
  if (gps.location.isValid())
  {
    Serial.print("Pos: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
  }
  else
  {
    Serial.println("Pos: Aguardando Satélites...");
  }

  // 2. Velocidade (SOG)
  if (gps.speed.isValid())
  {
    Serial.print("SOG (Nós): ");
    Serial.println(gps.speed.knots());
  }

  // 3. Curso (COG)
  if (gps.course.isValid())
  {
    Serial.print("COG (Graus): ");
    Serial.println(gps.course.deg());
  }

  // 4. Diagnóstico
  if (gps.hdop.isValid())
  {
    Serial.print("HDOP: ");
    Serial.println(gps.hdop.value());
  }
  
  if (gps.satellites.isValid())
  {
    Serial.print("Sats: ");
    Serial.println(gps.satellites.value());
  }

  // 5. Data e Hora
  if (gps.date.isValid())
  {
    char dateBuffer[32];
    sprintf(dateBuffer, "Data: %02d/%02d/%d", gps.date.day(), gps.date.month(), gps.date.year());
    Serial.println(dateBuffer);
  }
  
  if (gps.time.isValid())
  {
    char timeBuffer[32];
    sprintf(timeBuffer, "Hora (UTC): %02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.println(timeBuffer);
  }
  
  Serial.println("--------------------------");
}
