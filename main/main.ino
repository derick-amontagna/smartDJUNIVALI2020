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
#define BLYNK_PRINT Serial

/// Estas são as declarações relacionadas ao the Blynk App
char auth[] = "_qtOUSD-l2L7iMNt5x955DfegMSkXSEi";
char ssid[] = "GVT-A6E1";                         
char pass[] = "1965002410";

/// Configurando portas virtuais Blynk App
/// Funções
/// Programa principal
void setup() {
  Serial.begin(115200); //Inicia a comunicação serial
  Blynk.begin(auth, ssid, pass);

}

void loop() {
  Blynk.run(); // Para o App se comunicar a todo instante
}
