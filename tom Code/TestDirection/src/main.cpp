//Auteur Tom LIMOUZIN 
//Date : 13/01/2026
//Fonction du code : Gere la direction
#include <Arduino.h>
#include <Servo.h>   // Charge la bibliothèque qui permet de piloter des servomoteurs

Servo directionServo;
int pinServo = NUMERO_PIN; // Pin sur le quelle le serveau moteur est brancher

void setup() {
  directionServo.attach(pinServo); // "Attache" le servomoteur à la broche "NUMERO_PIN " pour que l'Arduino puisse lui envoyer des signaux
}

void loop() {
 directionServo.write(90);  // 1. Met les roues droites (90° est souvent le milieu)
  delay(2000);               // Attend 2 secondes pour te laisser le temps d'observer
  
  directionServo.write(120); // 2. Braque les roues à droite (angle de 120°)
  delay(2000);               // Attend 2 secondes
  
  directionServo.write(60);  // 3. Braque les roues à gauche (angle de 60°)
  delay(2000);               // Attend 2 secondes
}

