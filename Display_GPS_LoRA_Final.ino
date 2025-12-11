#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <MCUFRIEND_kbv.h>
#include <TinyGPS++.h>
#include <CayenneLPP.h>

MCUFRIEND_kbv tela;
TinyGPSPlus gps;
CayenneLPP dados(50);

// --- VARIÁVEIS DO DISPLAY ATUALIZADO ---
int porcentagemNivel = 0;
float velocidade = 0.0;
float intensidade = 0.0; // Vibração
float pressao_hpa = 1013.0;
float temperatura = 0.0;
float umidade = 0.0;
int direcaoMotor = 0; // 1=Frente, 2=Esq, 3=Dir
float rpm = 0.0;      // Recebido do sensor, mas nao exibido no display novo (usado no LoRa)

unsigned long ultimoUpdatePressao = 0;

// GPS e LoRa
bool infoValida = false;
float atualLatitude = 0.0;
float atualLongitude = 0.0;

// VARIAVEIS GRÁFICAS
int yTemp = 20; 
int yUmid = 20; 
int xAr = 2;    // Grafico Temp
int xUmid = 2;  // Grafico Umid

// TIMERS
unsigned long ultimoEnvioLoRa = 0;
const unsigned long intervaloEnvioLoRa = 15000;
unsigned long ultimoUpdateGrafico = 0;
unsigned long ultimoVel = 0;


void apagaGraf(int cordx, int cordy) {
  tela.fillRect(cordx + 1, cordy + 10, 238, 34, ILI9341_BLACK);
}

void graficoParametro(int &x, int cordx, int cordy, int valor, uint16_t cor, int &y) {
  valor = constrain(valor, 0, 100);
  valor = map(valor, 0, 100, 35, 15); // Mapeia para altura do grafico
  tela.drawLine(x - 1, cordy + y, x, cordy + valor, cor);
  y = valor;
  x++;
  if (x >= 239) {
    x = 2;
    apagaGraf(cordx, cordy);
  }
}

void atualizaGraficos() {
  // Temperatura (Verde)
  int tempExpandida = (temperatura - 20) * 5; // Ajuste visual
  if(tempExpandida < 0) tempExpandida = 0;
  graficoParametro(xAr, 0, 0, tempExpandida, ILI9341_GREEN, yTemp);
  
  tela.fillRect(100, 250, 40, 10, ILI9341_BLACK);
  tela.setCursor(100, 250);
  tela.setTextColor(ILI9341_WHITE);
  tela.print((int)temperatura);

  // Umidade (Ciano)
  graficoParametro(xUmid, 0, 45, (int)umidade, ILI9341_CYAN, yUmid);
  
  tela.fillRect(96, 270, 40, 10, ILI9341_BLACK);
  tela.setCursor(96, 270);
  tela.setTextColor(ILI9341_WHITE);
  tela.print((int)umidade);
}

void atualizaDirecaoTela(int dir) {
  tela.fillRect(150, 120, 60, 10, ILI9341_BLACK); // limpa área
  tela.setCursor(150, 120);
  tela.setTextColor(ILI9341_CYAN);
  tela.setTextSize(1);

  if (dir == 1) tela.print("Frente");
  else if (dir == 2) tela.print("Esquerda");
  else if (dir == 3) tela.print("Direita");
  else tela.print("Parado");
}

void atualizaPressaoTela(float pressao) {
  tela.fillRect(5, 120, 80, 10, ILI9341_BLACK);
  tela.setCursor(5, 120);
  tela.setTextColor(ILI9341_CYAN);
  tela.setTextSize(1);
  tela.print(pressao, 1);
  tela.print(" hPa");
}

void atualizaVibracaoTela(float val) {
  tela.fillRect(5, 175, 140, 10, ILI9341_BLACK);
  tela.setCursor(5, 175);
  tela.setTextColor(ILI9341_MAGENTA);
  tela.setTextSize(1);
  tela.print("Vibracao: ");
  tela.print(val, 3);
  tela.print(" g");
}

void atualizaVelocidadeTela(float vel) {
  tela.fillRect(150, 160, 60, 20, ILI9341_BLACK);
  tela.setCursor(150, 160);
  tela.setTextColor(ILI9341_PINK);
  tela.setTextSize(1);
  tela.print(vel, 1);
  tela.print(" kn");
}

void atualizaTanqueTela(int valor) {
  if (valor < 0) valor = 0;
  if (valor > 100) valor = 100;

  tela.fillRect(5, 216, 230, 12, ILI9341_BLACK);
  int largura = map(valor, 0, 100, 5, 235);
  tela.fillRect(5, 216, largura, 12, ILI9341_LIGHTGREY); 
  tela.drawRect(5, 216, 230, 12, ILI9341_WHITE); 

  tela.fillRect(125, 195, 40, 10, ILI9341_BLACK);
  tela.setCursor(125, 195);
  tela.setTextColor(ILI9341_WHITE);
  tela.setTextSize(1);
  tela.print(valor);
}

void atualizaGPSTela() {
  tela.fillRect(5, 160, 140, 10, ILI9341_BLACK);
  tela.setCursor(5, 160);
  tela.setTextColor(ILI9341_CYAN);
  if (infoValida) {
    tela.print(String(atualLatitude, 4) + " " + String(atualLongitude, 4));
  } else {
    tela.print("Sem Sinal GPS");
  }
}

// --- ALERTAS LORA (HEX → TEXTO) ---
String hexadecimalParaTexto(String textoHex) { 
  String resultado = ""; 
  textoHex.replace(" ", ""); 
  for (int i = 0; i < textoHex.length(); i += 2) { 
    String par = textoHex.substring(i, i + 2); 
    char caractere = (char)strtol(par.c_str(), NULL, 16); 
    resultado += caractere; 
  } 
  return resultado; 
}

void atualizaAlertaTela(String alerta){
  tela.fillRect(150, 180, 80, 20, ILI9341_BLACK); // Área do alerta
  tela.setCursor(150, 180);
  tela.setTextColor(ILI9341_RED);
  tela.setTextSize(1);
  tela.print(alerta);
}

void receberLoRaAlerta()
{
    if (!Serial2.available()) return;

    String linha = Serial2.readStringUntil('\n');
    linha.trim();

    Serial.print("[RAW LORA] ");
    Serial.println(linha);

    // Detectar se existe HEX no formato usual
    int pos = linha.indexOf(':');
    String hex;

    if (pos != -1) {
        // Ex: "+RECV=1,5:414C45525441"
        hex = linha.substring(pos + 1);
    } else {
        // Pode ser que venha só "414C45525441"
        hex = linha;
    }

    // Filtrar só caracteres HEX válidos
    String hexFiltrado = "";
    for (int i = 0; i < hex.length(); i++) {
        if (isxdigit(hex[i])) hexFiltrado += hex[i];
    }

    // Converter hex → texto
    String txt = hexadecimalParaTexto(hexFiltrado);

    Serial.print("[ALERTA DECODIFICADO] ");
    Serial.println(txt);

    if (txt.length() > 0) {
        atualizaAlertaTela(txt);
    }
}



void desenhaInterfaceBase() {
  tela.fillScreen(ILI9341_BLACK);

  tela.setCursor(5, 110); tela.setTextColor(ILI9341_WHITE); tela.print("Pressao Atmosferica:");
  tela.setCursor(5, 120); tela.setTextColor(ILI9341_CYAN); tela.print(String(pressao_hpa, 0) + " hPa");
  
  tela.setCursor(150, 110); tela.setTextColor(ILI9341_WHITE); tela.print("Direcao:");
  tela.setCursor(150, 120); tela.setTextColor(ILI9341_CYAN); tela.setTextSize(1); tela.print("---");

  tela.setCursor(5, 150); tela.setTextColor(ILI9341_WHITE);
  tela.print("Localizacao (GPS):");
  tela.setCursor(150, 150); tela.setTextColor(ILI9341_WHITE); tela.print("Velocidade:");
  tela.setCursor(5, 175); tela.setTextColor(ILI9341_WHITE); tela.print("Vibracao:");

  tela.setCursor(3, 3); tela.setTextColor(ILI9341_WHITE); tela.print("Temperatura Ar:");
  tela.setCursor(3, 48); tela.print("Umidade:");
  tela.setCursor(5, 195);
  tela.print("Nivel da gasolina:");

  tela.drawRect(0, 0, 240, 45, ILI9341_WHITE);
  tela.drawRect(0, 45, 240, 45, ILI9341_WHITE);
  tela.drawRect(0, 190, 240, 40, ILI9341_BLUE);
  tela.drawRect(5, 216, 230, 12, ILI9341_WHITE);
  
  tela.setCursor(5, 250); tela.print("Temp Digital:"); 
  tela.setCursor(5, 270); tela.print("Umid Digital:");
}

// ==========================================
//   SETUP (Conexão Robusta)
// ==========================================
void setup() {
  Serial.begin(9600);   
  Serial1.begin(9600); // GPS
  Serial2.begin(9600); // LoRa
  Serial3.begin(9600); // Sensor RX - Arduino Sensores

  uint16_t ID = tela.readID();
  tela.begin(ID);
  tela.setRotation(0); 
  desenhaInterfaceBase();

  Serial.println("--- INICIANDO ARDUINO DISPLAY ATUALIZADO ---");
  Serial.println("Aguardando 5s para LoRa...");
  delay(5000);

}

// 23,34,67,67,67,80

void recebeSensores() {
  if (Serial3.available()) {
    String rec = Serial3.readStringUntil('\n');
    rec.trim();
    if (rec.length() > 0) {
      int i1 = rec.indexOf(',');
      int i2 = rec.indexOf(',', i1+1);
      int i3 = rec.indexOf(',', i2+1);
      int i4 = rec.indexOf(',', i3+1);
      int i5 = rec.indexOf(',', i4+1);

      if (i1 > 0 && i5 > 0) {
        temperatura = rec.substring(0, i1).toFloat();
        umidade = rec.substring(i1+1, i2).toFloat();
        pressao_hpa = rec.substring(i2+1, i3).toFloat();
        intensidade = rec.substring(i3+1, i4).toFloat(); 
        rpm = rec.substring(i4+1, i5).toFloat();         
        porcentagemNivel = rec.substring(i5+1).toInt();
        
        if(rpm > 0) direcaoMotor = 1; 
        else direcaoMotor = 0;
      }
    }
  }
}


void loop() {
  if (Serial2.available()) {
    String linha = Serial2.readStringUntil('\n');
    linha.trim();
    Serial.print("[RX BRUTO LORA] ");
    Serial.println(linha);

    String dado = Serial2.readString();
    dado.trim();

    Serial.print("[LoRa RX]: ");
    Serial.println(dado);

    if (dado.length() >= 4 && isxdigit(dado[0]) && isxdigit(dado[1])) {
        receberLoRaAlerta();
    }

    
  }
  // 1. Recebe Sensores
  recebeSensores();

  // 2. Processa GPS
  while (Serial1.available()) gps.encode(Serial1.read());
  if (gps.location.isUpdated()) {
    infoValida = gps.location.isValid();
    atualLatitude = gps.location.lat();
    atualLongitude = gps.location.lng();
    atualizaGPSTela();
  }

  // 3. Atualiza Interface (Timers)
  unsigned long tempoAtual = millis();

  if (millis() - ultimoVel > 300) {
    ultimoVel = millis();
    atualizaVelocidadeTela(velocidade);
  }

  if (tempoAtual - ultimoUpdateGrafico > 500) {
    ultimoUpdateGrafico = tempoAtual;
    
    atualizaGraficos();         
    atualizaTanqueTela(porcentagemNivel); 
    atualizaPressaoTela(pressao_hpa);
    atualizaDirecaoTela(direcaoMotor); 
    atualizaVibracaoTela(intensidade); 
  }



}



void enviarLoRa() {
  Serial.println("----- Enviando via LoRa (CayenneLPP) -----");
  
  dados.reset(); 
  // Usa as variaveis ATUALIZADAS
  if (infoValida) {
     dados.addGPS(1, atualLatitude, atualLongitude, 0);
  } else {
     dados.addGPS(1, 0.0, 0.0, 0); // Envia 0 se sem sinal
  }
  
  dados.addTemperature(2, temperatura);
  dados.addRelativeHumidity(3, umidade);
  dados.addBarometricPressure(4, pressao_hpa);
  dados.addAnalogInput(5, velocidade);
  dados.addAnalogInput(6, intensidade); // Vibração
  dados.addAnalogInput(7, (float)porcentagemNivel);
  dados.addAnalogInput(8, (float)direcaoMotor); // Direção Motor (1,2,3)

  uint8_t *buffer = dados.getBuffer();
  String mensagem = "";
  for (int i = 0; i < dados.getSize(); i++) {
    if (buffer[i] < 16) mensagem += "0";
    mensagem += String(buffer[i], HEX);
  }
  mensagem.toUpperCase();
  
  String cmd = "AT+SENDB=1:" + mensagem;
  Serial.println("CMD: " + cmd); 
  Serial2.println(cmd);
}