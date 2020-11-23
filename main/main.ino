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
#define sensor1 32
#define sensor2 33
#define rele1 12
#define rele2 13

// Do algoritmo
// Sensor
int mVperAmp = 66; // use 185 para 5A ou 100 para 20A ou 66 para 30A - Sensibilidade do sensor
int Namostras = 150; // Numero de amostras a serem somadas para se obter a leitura final
float offsetCorrente = 2.5; // tensão de saída quando a corrente é zero -> vcc × 0.5 (2.5 v)
// Conta de luz
float potenciaAtivaW[2] = {};  // vetor com as potencias ativa
float energiaKWH[2] = {};  // QuiloWatt-hora gastos 
float energiaTotal = 0; 
float valorAtualConta = 0.0; // Valor final da conta de luz
float valorFinalConta = 0.0; // Valor final da conta de luz
float tarifaCelesc = 0.56; // Tarifa vigente em SC em reais por quilowatt-hora.

/// Estas são as declarações relacionadas ao the Blynk App
char auth[] = "_qtOUSD-l2L7iMNt5x955DfegMSkXSEi";
char ssid[] = "GVT-A6E1";                         
char pass[] = "1965002410";

/// Configurando portas virtuais Blynk App
BLYNK_CONNECTED() {
    Blynk.syncAll();
}

BLYNK_WRITE(V0) {
     rele1Comando = param.asInt();
     digitalWrite(rele1, rele1Comando);
}
BLYNK_WRITE(V1) {
     rele2Comando = param.asInt();
     digitalWrite(rele2, rele2Comando);
}

/// Funções
void leituraACS712(){
  float amostras1 = 0.0, valorLido1 = 0.0, valorMedio1 = 0.0, tensao1 = 0.0, corrente1 = 0.0;
  float amostras2 = 0.0, valorLido2 = 0.0, valorMedio2 = 0.0, tensao2 = 0.0, corrente2 = 0.0;
  
  for (int x = 0; x < Namostras; x++){
    valorLido1 = analogRead(sensor1);
    valorLido2 = analogRead(sensor2);
    amostras1 = amostras1 + valorLido1;
    amostras2 = amostras2 + valorLido2;
  }
  valorMedio1 = amostras1/Namostras;
  valorMedio2 = amostras2/Namostras;
  
  tensao1 = ((valorMedio1 * 5/1024.0));
  tensao2 = ((valorMedio2 * 5/1024.0));
  
  corrente1 = ((tensao1 - offsetCorrente)/ mVperAmp);
  corrente2 = ((tensao2 - offsetCorrente)/ mVperAmp);
  
  potenciaAtivaW[0] = potenciaAtivaW[0] + tensao1*corrente1; 
  potenciaAtivaW[1] = potenciaAtivaW[1] + tensao2*corrente2; 
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
  energiaKWH[0] = 0;
  energiaKWH[1] = 0; 
}

/// Programa principal
void setup() {
  Serial.begin(115200); //Inicia a comunicação serial
  Blynk.begin(auth, ssid, pass);

  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);

  timer.setInterval(3600000L, calcEnergiaKWH);
  timer.setInterval(2628000000L, enviandoContaFinal);
}

void loop() {
  Blynk.run(); // Para o App se comunicar a todo instante
  timer.run();
}
