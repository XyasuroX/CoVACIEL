#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08

// --- Configuration selon vos précisions ---
const int PIN_MOTEUR = 9;  
const int PIN_SERVO = 10; 
const int PIN_ENCODEUR = 2; // Label FOURCHE sur le schéma

// --- Valeurs d'étalonnage ---
const int MOTEUR_ARRET = 90;
const int MOTEUR_AVANT_TEST = 98;
const int MOTEUR_ARRIERE_TEST = 82; // Valeur type pour reculer
const int SERVO_DROIT = 90;

Servo moteurESC;
Servo directionServo;

volatile int8_t commandeAngle = 0;   // Reçu du Pi (-30 à 30 par ex)
volatile int8_t commandeVitesse = 0; // 0=Stop, 1=Avant, -1=Arrière

// Gestion de la marche arrière (Double Tap)
unsigned long tempsDernierNeutre = 0;
int phaseDoubleTap = 0; 

void receiveEvent(int howMany) {
    if (howMany >= 2) {
        commandeAngle = Wire.read();
        commandeVitesse = Wire.read();
    }
}

void setup() {
    Wire.begin(I2C_SLAVE_ADDR);
    Wire.onReceive(receiveEvent);

    moteurESC.attach(PIN_MOTEUR);
    directionServo.attach(PIN_SERVO);

    // Initialisation sécurisée au neutre
    moteurESC.write(MOTEUR_ARRET);
    directionServo.write(SERVO_DROIT);

    pinMode(PIN_ENCODEUR, INPUT_PULLUP);
}

void appliquerMoteur(int direction) {
    static int directionPrecedente = 0;

    if (direction == 1) { // MARCHE AVANT
        moteurESC.write(MOTEUR_AVANT_TEST);
        phaseDoubleTap = 0;
    } 
    else if (direction == -1) { // MARCHE ARRIÈRE (Logique Double Tap)
        if (phaseDoubleTap == 0) {
            moteurESC.write(MOTEUR_ARRIERE_TEST); // Premier coup (frein)
            delay(100);
            moteurESC.write(MOTEUR_ARRET);        // Retour neutre
            delay(100);
            phaseDoubleTap = 1;
        }
        moteurESC.write(MOTEUR_ARRIERE_TEST);     // Deuxième coup (recule)
    } 
    else { // ARRÊT
        moteurESC.write(MOTEUR_ARRET);
        if (directionPrecedente != 0) phaseDoubleTap = 0;
    }
    directionPrecedente = direction;
}

void loop() {
    // 1. Direction : Neutre (90) + correction
    int angleFinal = SERVO_DROIT + commandeAngle;
    directionServo.write(constrain(angleFinal, 60, 120));

    // 2. Moteur
    appliquerMoteur(commandeVitesse);
    
    delay(20); 
}