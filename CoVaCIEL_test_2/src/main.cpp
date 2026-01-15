/*****************************************************
 *  TEST DE DEUX CAPTEURS DE DISTANCE
 *  - Sharp GP2 (analogique sur A0)
 *  - SRF10 Ultrason (I2C)
 *
 *  On peut choisir quel capteur tester
 *  en modifiant UNE seule variable.
 *****************************************************/

#include <Arduino.h>
#include <Wire.h>   // Bibliothèque pour la communication I2C

/* ===================================================
   SELECTION DU CAPTEUR A TESTER
   =================================================== */
// Mettre MODE_SHARP pour tester le capteur Sharp
// Mettre MODE_SRF10 pour tester le capteur ultrason SRF10
#define MODE_SHARP  0
#define MODE_SRF10  1

int testMode = MODE_SRF10;   // ⬅️ CHANGER ICI POUR SWITCHER

/* ===================================================
   DEFINITIONS POUR LE CAPTEUR SRF10 (I2C)
   =================================================== */
#define SRF10_ADDRESS 0x70   // Adresse I2C par défaut du SRF10
#define COMMAND_REG   0x00   // Registre de commande
#define RANGE_HIGH    0x02   // Octet fort de la distance

/* ===================================================
   PROTOTYPES DES FONCTIONS
   =================================================== */
void sharp_gp2_test();
void srf10_test();
int  readSRF10();

/* ===================================================
   SETUP : exécuté UNE SEULE FOIS au démarrage
   =================================================== */
void setup() {
  Serial.begin(9600);    // Démarre la communication série

  // Initialisation spécifique selon le capteur choisi
  if (testMode == MODE_SRF10) {
    Wire.begin();        // Démarre le bus I2C
    Serial.println("=== TEST DU CAPTEUR ULTRASON SRF10 ===");
  } else {
    Serial.println("=== TEST DU CAPTEUR SHARP GP2 ===");
  }
}

/* ===================================================
   LOOP : tourne en boucle à l'infini
   =================================================== */
void loop() {

  // Selon le mode choisi, on appelle la bonne fonction
  if (testMode == MODE_SHARP) {
    sharp_gp2_test();
  }
  else if (testMode == MODE_SRF10) {
    srf10_test();
  }
}

/* ===================================================
   TEST DU CAPTEUR SHARP GP2 (ANALOGIQUE)
   =================================================== */
void sharp_gp2_test() {

  // 1) Lire la valeur envoyée par le capteur (0 à 1023)
  int val = analogRead(A0);

  // 2) Convertir la valeur brute en tension (0 à 5V)
  float voltage = val * (5.0 / 1023.0);

  // 3) Convertir la tension en distance (formule empirique)
  float distance = 29.988 * pow(voltage, -1.173);

  // 4) Afficher les résultats sur le moniteur série
  Serial.print("Valeur brute: ");
  Serial.print(val);
  Serial.print(" | Tension: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Distance approx: ");
  Serial.print(distance, 1);
  Serial.println(" cm");

  delay(800);   // Pause pour rendre l'affichage lisible
}

/* ===================================================
   TEST DU CAPTEUR ULTRASON SRF10 (I2C)
   =================================================== */
void srf10_test() {

  // Lire la distance mesurée par le SRF10
  int distance = readSRF10();

  // Vérifier si la lecture s'est bien passée
  if (distance >= 0) {
    Serial.print("Distance SRF10: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Erreur de lecture du SRF10");
  }

  delay(500);   // Pause entre deux mesures
}

/* ===================================================
   FONCTION DE LECTURE DU SRF10
   =================================================== */
int readSRF10() {

  // 1) Envoyer la commande pour lancer une mesure en cm
  Wire.beginTransmission(SRF10_ADDRESS);
  Wire.write(COMMAND_REG);   // On écrit dans le registre de commande
  Wire.write(0x51);          // 0x51 = mesure en centimètres
  Wire.endTransmission();

  // 2) Attendre que le capteur fasse la mesure (~65 ms)
  delay(70);

  // 3) Demander l'octet fort de la distance
  Wire.beginTransmission(SRF10_ADDRESS);
  Wire.write(RANGE_HIGH);
  Wire.endTransmission();

  // 4) Lire 2 octets (distance = High << 8 + Low)
  if (Wire.requestFrom(SRF10_ADDRESS, 2) != 2) {
    return -1;   // Erreur de communication
  }

  int highByte = Wire.read();
  int lowByte  = Wire.read();

  // 5) Reconstituer la distance finale
  return (highByte << 8) | lowByte;
}
