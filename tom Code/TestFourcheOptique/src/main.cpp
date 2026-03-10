#include <Arduino.h>

// Broche D2 pour le signal du capteur
const uint8_t SENSOR_PIN = 2; 
const uint8_t LED_STATUS = LED_BUILTIN;

volatile uint32_t compteur = 0;
volatile bool passageDetecte = false;

// Fonction d'interruption (ISR) - Sans IRAM_ATTR pour la Nano R4
void alertPassage() {
    compteur++;
    passageDetecte = true;
}

void setup() {
    Serial.begin(115200);
    // Sur Nano R4, on attend que le port série soit prêt
    while (!Serial); 

    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(LED_STATUS, OUTPUT);

    // Attache l'interruption sur la broche D2
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), alertPassage, FALLING);

    Serial.println("========================================");
    Serial.println("TEST CAPTEUR MEX100 - ARDUINO NANO R4");
    Serial.println("Statut : Pret et en attente...");
    Serial.println("========================================");
}

void loop() {
    if (passageDetecte) {
        digitalWrite(LED_STATUS, HIGH); 
        
        Serial.print("Passage detecte ! Total : ");
        Serial.println(compteur);
        
        // Un petit délai pour éviter les doubles comptages dus aux vibrations
        delay(100); 
        
        passageDetecte = false;
        digitalWrite(LED_STATUS, LOW);
    }
}