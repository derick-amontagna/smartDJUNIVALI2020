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
// Do Blynk
#define BLYNK_PRINT Serial

// Do algoritmo
int mVperAmp = 66; // use 185 para 5A ou 100 para 20A ou 66 para 30A - Sensibilidade do sensor
int Namostras = 150; // Numero de amostras a serem somadas para se obter a leitura final
float offsetCorrente = 2.5; // tensão de saída quando a corrente é zero -> vcc × 0.5 (2.5 v)
float potenciaAtivaW = 0.0; // potencia ativa que irá somar todos os valores
float energiaKWH = 0.0;  // QuiloWatt-hora gastos 
float valorAtualConta = 0.0; // Valor final da conta de luz
float valorFinalConta = 0.0; // Valor final da conta de luz
float tarifaCelesc = 0.56; // Tarifa vigente em SC em reais por quilowatt-hora.

/// Estas são as declarações relacionadas ao the Blynk App
char auth[] = "_qtOUSD-l2L7iMNt5x955DfegMSkXSEi";
char ssid[] = "GVT-A6E1";                         
char pass[] = "1965002410";

/// Configurando portas virtuais Blynk App
/// Funções
void leituraACS712(){
  float amostras =0.0, valorLido = 0.0, valorMedio = 0.0, tensao = 0.0, corrente = 0.0;
  
  for (int x = 0; x < Namostras; x++){
    valorLido = analogRead(A0);
    amostras = amostras + valorLido;
  }
  valorMedio = amostras/Namostras;
  tensao = ((valorMedio * 5/1024.0));
  corrente = ((tensao - offsetCorrente)/ mVperAmp);
  potenciaAtivaW = potenciaAtivaW + tensao*corrente;
}

void calcContaDeLuz(){
  energiaKWH = (potenciaAtivaW/1000)*1;
  valorAtualConta = energiaKWH * tarifaCelesc;
  envioDaContaFinal();
}

void envioDaContaFinal(){
  valorFinalConta = valorFinalConta + valorAtualConta;
}

/// Programa principal
void setup() {
  Serial.begin(115200); //Inicia a comunicação serial
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run(); // Para o App se comunicar a todo instante
}
