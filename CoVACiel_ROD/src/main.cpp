#include <Arduino.h>


#define PIN_BATTERY A1
#define ADC_RESOLUTION 4095.0f
#define ADC_REF_VOLTAGE 5.0f


const float FACTEUR_DIVISEUR = 3.90f; // Facteur diviseur de la carte mezzanine


void setup() {
  Serial.begin(115200); // Connection série, plus rapide que 9600
  while (!Serial) delay(10); // attendre que la connexion série soit prête
  analogReadResolution(12); // Passe l'ADC en mode 12 bits, plus de précision que 10 bits


  Serial.println("=== Mesure U batterie ==="); // titre au démarrage
  Serial.print("Facteur diviseur mezzanine = ");
  Serial.println(FACTEUR_DIVISEUR);
}


void loop() {
  long sum = 0; // à 0 pour accumuler les lectures
  const int samples = 64; // 64 lectures
  for (int i = 0; i < samples; i++) { // boucle pour lire 64 fois analogread, et ajouter la valeur à sum
    sum += analogRead(PIN_BATTERY);
    delay(1); // délai de 1 ms
  }
  float adcAvg = sum / (float)samples; // divise la somme totale par le nombre de lectures, ce qui nous donne la moyenne


  float voltageInput = (adcAvg * ADC_REF_VOLTAGE) / ADC_RESOLUTION; // conversion de la valeur ADC moyenne en tension réelle sur la broche A1
  float batteryVoltage = voltageInput * FACTEUR_DIVISEUR; // Facteur de mezzanine pour remonter à la vraie tension de la batterie


  Serial.print("ADC avg = ");
  Serial.print(adcAvg, 1); // avec 1 décimale
  Serial.print(" | Vin ADC = ");
  Serial.print(voltageInput, 3); // avec 3 décimales
  Serial.print(" V | Ubat = ");
  Serial.print(batteryVoltage, 2); // avec 2 décimales
  Serial.println(" V");


  delay(800);
}
