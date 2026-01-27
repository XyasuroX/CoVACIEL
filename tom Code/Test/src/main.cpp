//Auteur Tom LIMOUZIN 
//Date : 13/01/2026
//Fonction du code : Gere la direction
#include <Arduino.h>
#include <Servo.h>   // Charge la bibliothèque qui permet de piloter des servomoteurs

Servo directionServo;
int pinServo = PIN_A0;  // Pin sur le quelle le serveau moteur est brancher

Servo escMoteur;
int pinESC = PIN_D9; // On connecte l'ESC à la broche D9


void setup() {
  directionServo.attach(pinServo);
  Serial.begin(9600);
  escMoteur.attach(pinESC);
  
  // Envoi du neutre et attente longue pour que l'ESC s'initialise
  escMoteur.write(90); 
  delay(5000); // Laissez 5 secondes à l'ESC pour faire ses "bips" de démarrage
}

void loop() {
  // Sur un ESC de voiture :
  // 90 = Arrêt
  // 180 = Pleine vitesse avant
  // 0 = Pleine vitesse arrière (ou frein)
  
// --- PHASE MARCHE AVANT ---
  escMoteur.write(100);   // Avance
  delay(2000);
  
  // --- PHASE FREINAGE ---
  escMoteur.write(90);    // Retour au neutre
  delay(1000);            // Pause pour stabiliser
  
  // --- PHASE MARCHE ARRIÈRE (Séquence Double-Tap) ---
  escMoteur.write(75);    // Premier coup : Freinage
  delay(100);
  escMoteur.write(90);    // Relâchement (Neutre)
  delay(100);
  escMoteur.write(75);    // Deuxième coup : Recul effectif
  delay(2000);
  
  // --- PHASE RETOUR À L'AVANT ---
  // C'est ici que ça bloquait : il faut impérativement repasser par 90
  escMoteur.write(90);    
  delay(1000);            // On attend que l'ESC valide l'arrêt avant de repartir
}


