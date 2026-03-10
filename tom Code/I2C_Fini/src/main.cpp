#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>


Servo directionServo;
int pinServo = A0;
Servo escMoteur;
int pinESC = 9;


volatile bool messagePret = false;
volatile char bufferI2C[32];
volatile int indexI2C = 0;


void receiveEvent(int howMany) {
  indexI2C = 0;
  while (Wire.available() && indexI2C < 31) {
    bufferI2C[indexI2C] = (char)Wire.read();
    indexI2C++;
  }
  bufferI2C[indexI2C] = '\0';
  messagePret = true;        
}


void setup() {
  Serial.begin(9600);
 
  // 1. Initialisation I2C Immédiate (Adresse 0x08)
  Wire.begin(0x08);
  Wire.onReceive(receiveEvent);


  Serial.println("\n=== INITIALISATION MOTEURS ===");
  directionServo.attach(pinServo);
  escMoteur.attach(pinESC);


  // Séquence d'armement ESC
  Serial.println("Armement ESC (Neutre 90)... 5s");
  directionServo.write(90);
  escMoteur.write(90);      
  delay(5000);


  Serial.println("=== PRET A RECEVOIR VOS TRAMES ===");
}


void loop() {
  if (messagePret) {
    String trameBrute = String((char*)bufferI2C);
    messagePret = false;
   
    trameBrute.trim();
    Serial.println("\n[I2C] Recu : " + trameBrute);


    // --- DECODAGE DE LA TRAME <M:xxx,D:yyy> ---
    int vitesse = 90; // Valeur par défaut (arrêt)
    int angle = 90;   // Valeur par défaut (droit)


    // La fonction sscanf cherche le motif exact et extrait les %d (entiers)
    if (sscanf(trameBrute.c_str(), "<M:%d,D:%d>", &vitesse, &angle) == 2) {
     
      // Sécurités matérielles (pour ne pas casser la direction)
      vitesse = constrain(vitesse, 0, 180);
      angle = constrain(angle, 40, 140); // Limites mécaniques pour ne pas casser le servo de direction


      Serial.print("   -> APPLICATION : Moteur=");
      Serial.print(vitesse);
      Serial.print(" | Direction=");
      Serial.println(angle);


      // Application physique aux actionneurs
      escMoteur.write(vitesse);
      directionServo.write(angle);
     
    } else {
      Serial.println("   [ERREUR] Format de trame invalide ! Attendu : <M:100,D:90>");
    }
  }
}
