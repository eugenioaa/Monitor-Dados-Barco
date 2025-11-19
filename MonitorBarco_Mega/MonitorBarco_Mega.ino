#include <TinyGPS++.h>

TinyGPSPlus gps;

unsigned long tempo = 0;
unsigned long auxiliar = 0; 

bool isDataValid = false;
float currentLatitude = 0.0;
float currentLongitude = 0.0;
float currentSpeedKnots = 0.0;
float currentCourseDeg = 0.0;
float currentHDOP = 99.9; 
uint8_t currentSatellites = 0;
uint8_t currentDay = 0;
uint8_t currentMonth = 0;
uint16_t currentYear = 0;
uint8_t currentHour = 0;
uint8_t currentMinute = 0;
uint8_t currentSecond = 0;#include <TinyGPS++.h>

TinyGPSPlus gps;

//unsigned long tempo = millis();
//unsigned long auxiliar = 0; 
static const uint32_t GPSBaud = 9600; 

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

void setupGPS();
void runGPS();
void populateGPSData();
void displayGPSData();
void testaHora();

void setup()
{
  Serial.begin(9600);
  Serial1.begin(GPSBaud);
  Serial2.begin()
  Serial.println("Sistema Pronto.\n");
}

void loop()
{
  runGPS();

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

  // HDOP - qualidade do sinal, quantidade de Satelites, essecial para saber a precissao do sinal
  atualHDOP = gps.hdop.value();
  quantSatelites = (uint8_t)gps.satellites.value();
  
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


void setupGPS();
void runGPS();
void populateGPSData();
void displayGPSData();
void testaHora();

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("Sistema Pronto.\n");
}

void loop()
{
  runGPS();

  unsigned long tempo = millis();
  testaHora(tempo);
}

void runGPS()
{
  while (Serial1.available() > 0)
  {
    //char incomingChar = Serial1.read();
    //Serial.print(incomingChar);

    //gps.encode(incomingChar);
    gps.encode(Serial1.read());
  }

  if (gps.location.isUpdated())
  {
    populateGPSData();
    displayGPSData(); 
  }
}

void testaHora(unsigned long tempoAtual)
{ 
  if (tempoAtual - auxiliar >= 1000)
  {
    char dateTimeBuffer[64];
    sprintf(dateTimeBuffer, "Data/Hora (UTC): %02d/%02d/%d %02d:%02d:%02d", 
          currentDay, currentMonth, currentYear,
         currentHour, currentMinute, currentSecond);
    Serial.println(dateTimeBuffer);
    auxiliar = tempoAtual;
  }
}

void populateGPSData()
{
  isDataValid = gps.location.isValid();

  //Posição precisa do barco
  currentLatitude = (float)gps.location.lat();
  currentLongitude = (float)gps.location.lng();

  //SOG: Medida de velocidade do Barco, uso para calcular tempo de chegada [NOS]
  currentSpeedKnots = (float)gps.speed.knots();

  //COG: Direção real do Barco, aponta sempre ao Norte verdadeiro
  //  0/360   Norte (N)
  //  90      Leste (E)
  //  180     Sul   (S)
  //  270     Oeste (W)
  currentCourseDeg = (float)gps.course.deg();

  // HDOP - qualidade do sinal, CS - quantidade de Satelites, essecial para saber a precissao do sinal
  currentHDOP = (float)gps.hdop.value();
  currentSatellites = (uint8_t)gps.satellites.value();
  
  // Data e Hora
  currentDay = (uint8_t)gps.date.day();
  currentMonth = (uint8_t)gps.date.month();
  currentYear = (uint16_t)gps.date.year();
  currentHour = (uint8_t)gps.time.hour();
  currentMinute = (uint8_t)gps.time.minute();
  currentSecond = (uint8_t)gps.time.second();
}

void displayGPSData()
{
  Serial.println("--- [GPS] Dados Atuais ---");
  Serial.print("Status: ");
  Serial.println(isDataValid ? "VALIDO" : "AGUARDANDO FIX");

  Serial.print("Pos: ");
  Serial.print(currentLatitude, 6);
  Serial.print(", ");
  Serial.println(currentLongitude, 6);

  Serial.print("SOG/COG: ");
  Serial.print(currentSpeedKnots);
  Serial.print(" Nós / ");
  Serial.print(currentCourseDeg);
  Serial.println(" Graus");

  Serial.print("HDOP/Sats: ");
  Serial.print(currentHDOP);
  Serial.print(" / ");
  Serial.println(currentSatellites);
  
  char dateTimeBuffer[64];
  sprintf(dateTimeBuffer, "Data/Hora (UTC): %02d/%02d/%d %02d:%02d:%02d", 
          currentDay, currentMonth, currentYear,
          currentHour, currentMinute, currentSecond);
  Serial.println(dateTimeBuffer);
  
  Serial.println("--------------------------");
}
