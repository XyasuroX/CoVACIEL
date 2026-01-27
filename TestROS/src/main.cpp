#include <Arduino.h>
#include <Servo.h>

// --- Configuration des broches ---
const int PIN_MOTEUR = 9;    // Vers l'ESC (moteur de propulsion)
const int PIN_SERVO = 10;   // Vers le servomoteur de direction

// --- Paramètres de réglage ---
const int VITESSE_ARRET = 90;    // Point mort (souvent 90 ou 1500ms)
const int VITESSE_AVANT = 98;   // Marche avant lente (ajuste selon tes tests)
const int DIRECTION_DROIT = 90;  // Roues bien droites

// --- Variables de contrôle ---
Servo moteurPropulsion;
Servo servoDirection;
unsigned long tempsDernierMessage = 0;
const long DELAI_SECURITE = 500; // Arrêt si pas de signal pendant 500ms

void setup() {
  // Initialisation du port série (USB) pour communiquer avec le Pi
  Serial.begin(9600);
  
  // Attachement des servos
  moteurPropulsion.attach(PIN_MOTEUR);
  servoDirection.attach(PIN_SERVO);
  
  // Initialisation à l'arrêt pour la sécurité
  moteurPropulsion.write(VITESSE_ARRET);
  servoDirection.write(DIRECTION_DROIT);
  
  Serial.println("Arduino prêt - En attente de ROS...");
}

void loop() {
  
  

  // 2. Sécurité Failsafe (Essentiel !)
  // Si le LiDAR ou le Raspberry Pi s'arrête de parler, on stoppe tout
  if (millis() - tempsDernierMessage > DELAI_SECURITE) {
    moteurPropulsion.write(VITESSE_ARRET);
  }
}
