/*
* Disjuntor Inteligente 
* Derick Abreu Montagna e Edmiel Loiola de Oliveira 
* 
* É um projeto proposto pela disciplina de sistemas embarcados para obter a nota da M3, de maneira, que é proposto a realização de um projeto com todos os conhecimentos 
* adquiridos durante a disciplina.
* 22/11/2020
*/

/// Biblioteca
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

/// Definindo constantes
// Timer
BlynkTimer timer;

// Do Blynk
#define BLYNK_PRINT Serial
int rele1Comando;
int rele2Comando;

// Portas
#define sensor1 A4
#define sensor2 A5
#define rele1 25
#define rele2 26

// Do algoritmo
int VCC = 5;

// Conta de luz
float potenciaAtivaW[2] = {};  // vetor com as potencias ativa
float energiaKWH[2] = {};  // QuiloWatt-hora gastos 
float energiaTotal = 0; 
float valorAtualConta = 0.0; // Valor final da conta de luz
float valorFinalConta = 0.0; // Valor final da conta de luz
float tarifaCelesc = 0.50599; // Tarifa vigente em SC em reais por quilowatt-hora.

/// Estas são as declarações relacionadas ao the Blynk App
char auth[] = "_qtOUSD-l2L7iMNt5x955DfegMSkXSEi";
char ssid[] = "SN_azevedoWifi";                         
char pass[] = "97228552";

/// Configurando portas virtuais Blynk App
BLYNK_CONNECTED() {
    Blynk.syncAll();
}

BLYNK_WRITE(V0) {
     rele1Comando = param.asInt();
     if (rele1Comando == 1) {
      digitalWrite(rele1, HIGH);
    }
    else {
      digitalWrite(rele1, LOW);
    }
     Blynk.syncVirtual(V0); 
}

BLYNK_WRITE(V1) {
     rele2Comando = param.asInt();
     if (rele2Comando == 1) {
      digitalWrite(rele2, HIGH);
    }
    else {
      digitalWrite(rele2, LOW);
    }
     Blynk.syncVirtual(V1); 
}

/// Funções
void leituraACS712(){
  double Voltage = 0;
  double Corrente = 0;
  if(rele1Comando == 1){
    for(int i = 0; i < 1000; i++) {
      Voltage = (Voltage + (0.0048828125 * analogRead(sensor1))); // (5 V / 1024 (Analog) = 0.0049) which converter Measured analog input voltage to 5 V Range
      delay(1);
    }
    Voltage = Voltage /1000;
    Corrente = (Voltage - 2.5)/ 0.100;
    potenciaAtivaW[0] = potenciaAtivaW[0] + VCC*Corrente; 
  }
  else{
    Corrente = 0;
    potenciaAtivaW[0] = potenciaAtivaW[0] + VCC*Corrente; 
  }
  if(rele2Comando == 1){
    for(int i = 0; i < 1000; i++) {
      Voltage = (Voltage + (0.0048828125 * analogRead(sensor2))); // (5 V / 1024 (Analog) = 0.0049) which converter Measured analog input voltage to 5 V Range
      delay(1);
  }
    Voltage = Voltage /1000;
    Corrente = (Voltage - 2.5)/ 0.100;
    potenciaAtivaW[1] = potenciaAtivaW[1] + VCC*Corrente; 
  }
  else{
    Corrente = 0;
    potenciaAtivaW[1] = potenciaAtivaW[1] + VCC*Corrente; 
  }
}

void calcEnergiaKWH(){
  energiaKWH[0] = energiaKWH[0] + (potenciaAtivaW[0]/1000)*1;
  energiaKWH[1] = energiaKWH[1] + (potenciaAtivaW[1]/1000)*1;
  potenciaAtivaW[0] = 0;
  potenciaAtivaW[1] = 0;
  calcContaAtual();
}

void calcContaAtual(){
  energiaTotal = energiaKWH[0] + energiaKWH[1];
  valorAtualConta = energiaTotal * tarifaCelesc;
  Blynk.virtualWrite(V2, valorAtualConta);
  Blynk.virtualWrite(V3, energiaTotal);
  Blynk.virtualWrite(V4, valorAtualConta);
  envioDaContaFinal();
}

void envioDaContaFinal(){
  valorFinalConta = valorAtualConta;
}

void enviandoContaFinal(){
  String Mensagem = String("A sua conta de luz teve um total de ") + valorFinalConta + "reais.";
  Blynk.email("dam@edu.univali.br", "Conta de luz", Mensagem);
  Blynk.virtualWrite(V5, valorFinalConta);
  valorAtualConta = 0;
  valorFinalConta = 0;
  energiaTotal = 0;
  energiaKWH[0] = 0;
  energiaKWH[1] = 0; 
  Blynk.virtualWrite(V2, valorAtualConta);
  Blynk.virtualWrite(V3, energiaTotal);
  Blynk.virtualWrite(V4, valorAtualConta);
}


/// Programa principal
void setup() {
  Serial.begin(115200); //Inicia a comunicação serial
  Blynk.begin(auth, ssid, pass);

  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  
  timer.setInterval(42L, calcEnergiaKWH);   // 42  milisegundos representa 1h
  timer.setInterval(30000L, enviandoContaFinal);   // 30 segundos representa 30 dias 
}

void loop() {
  Blynk.run(); // Para o App se comunicar a todo instante
  timer.run();
  leituraACS712();
}
