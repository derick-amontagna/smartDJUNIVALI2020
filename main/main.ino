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
#include "ACS712.h"

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
#define rele1 26
#define rele2 25

// Do algoritmo
// Sensor
int mVperAmp = 100; // use 185 para 5A ou 100 para 20A ou 66 para 30A - Sensibilidade do sensor
int Namostras = 150; // Numero de amostras a serem somadas para se obter a leitura final
float VCC = 4.82; // tensão de saída quando a corrente é zero -> vcc × 0.5 (2.5 v)
float offsetCorrente = VCC*0.5; // tensão de saída quando a corrente é zero 
ACS712 sensor11(ACS712_20A, sensor1);
ACS712 sensor22(ACS712_20A, sensor2);

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
  if(rele2Comando == 1){
  float corrente1 = 0, corrente2 = 0;
  corrente1 = sensor11.getCurrentDC();
  Serial.println("Valor da corrente1  = ");
  Serial.println(corrente1);
  potenciaAtivaW[0] = potenciaAtivaW[0] + VCC*corrente1; 
  }
  if(rele1Comando == 1){
  corrente2 = sensor22.getCurrentDC();
  Serial.println("Valor da corrente2  = ");
  Serial.println(corrente2);
  potenciaAtivaW[1] = potenciaAtivaW[1] + VCC*corrente2; 
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
  valorFinalConta = valorFinalConta + valorAtualConta;
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

  Serial.println("Calibrating... Ensure that no current flows through the sensor at this moment");
  int zero1 = sensor11.calibrate();
  Serial.println("Done!");
  Serial.println("Zero point for this sensor = " + zero1);

   Serial.println("Calibrating... Ensure that no current flows through the sensor at this moment");
  int zero2 = sensor22.calibrate();
  Serial.println("Done!");
  Serial.println("Zero point for this sensor = " + zero2);
  
  timer.setInterval(60000L, calcEnergiaKWH);   // 1 mins em milisegundos 
  timer.setInterval(180000L, enviandoContaFinal);   // 3 mins em milisegundos
}

void loop() {
  Blynk.run(); // Para o App se comunicar a todo instante
  timer.run();
  leituraACS712();
}
