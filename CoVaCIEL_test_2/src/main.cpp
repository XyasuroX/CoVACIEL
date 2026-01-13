#include <Arduino.h>

void setup() {
  Serial.begin(9600);
}

void loop() {
  int val = analogRead(A0);
  float voltage = val * (5.0 / 1023.0);
  float distance = 29.988 * pow(voltage, -1.173);   // ou 27.0 / voltage pour approx

  Serial.print("Valeur brute: ");
  Serial.print(val);
  Serial.print(" | Tension: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Distance approx: ");
  Serial.print(distance, 1);
  Serial.println(" cm");

  delay(800);   // pause pour lire tranquillement
}