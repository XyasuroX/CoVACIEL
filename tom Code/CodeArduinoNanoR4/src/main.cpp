#include <Arduino.h>

void setup() {
  // Port USB relié à ton PC (pour lire le résultat)
  Serial.begin(9600);  
  
  // Port UART relié au Raspberry Pi (via la HAT)
  Serial1.begin(9600); 

  delay(1000);
  Serial.println("\n=== ARDUINO PRET EN UART ===");
  Serial.println("J'ecoute le Raspberry Pi...");
}

void loop() {
  // Dès qu'une donnée arrive du Pi via la HAT...
  if (Serial1.available() > 0) {
    char c = Serial1.read();
    
    // ...on l'affiche instantanément sur l'écran du PC
    Serial.print(c); 
  }
}
