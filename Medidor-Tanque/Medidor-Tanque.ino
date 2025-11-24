const int altMax = 30;
const int distMin = 2;  

const int pinoTrig = 9;
const int pinoEcho = 10;

unsigned long tempoAnterior = 0;    
const long intervalo = 5000;         

long duracao;
int distanciaAr;    
int nivelLiquido;   
int porcentagem;    

void setup() {
  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);
  Serial.begin(9600);
  Serial.println("--- Monitoramento de Nivel de Tanque ---");
}

void loop() {
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervalo) {
    tempoAnterior = tempoAtual;

    digitalWrite(pinoTrig, LOW);
    delayMicroseconds(2);
    digitalWrite(pinoTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinoTrig, LOW);

    duracao = pulseIn(pinoEcho, HIGH);
    
    distanciaAr = duracao * 0.034 / 2;

    if (distanciaAr > altMax) distanciaAr = altMax;
    if (distanciaAr < distMin) distanciaAr = distMin;

    nivelLiquido = altMax - distanciaAr;
    porcentagem = map(nivelLiquido, 0, (altMax - distMin), 0, 100);
    porcentagem = constrain(porcentagem, 0, 100);

    Serial.print("Nivel: ");
    Serial.print(nivelLiquido);
    Serial.print("cm | ");
    Serial.print(porcentagem);
    Serial.println("%");
    
  }
}
