/**
 * PROJET : IMU ULTIME - Mouvement + Boussole + BUZZER + Mesure Tension Batterie
 * Version : Finale (Fix BNO055 + Design Tableau + Calibration Tension)
 * * Matériel : Arduino Nano R4, BNO055, OLED SH1106, Carte Mezzanine
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>
#include <Servo.h>

// ================================================================
// 1. REGLAGES & CONSTANTES
// ================================================================
#define BUZZER_PIN D6       // Pin du Buzzer
#define PIN_BATTERY A1      // Pin de mesure tension
#define PIN_ESC     9       // Pin du controleur moteur

// Seuls d'alarme et physique
#define SHOCK_LIMIT 8.0     // Seuil d'accélération pour le bip (m/s²)
#define DEADZONE 0.15       // Zone morte pour éviter le bruit du capteur
#define FRICTION 0.98       // Frottement simulé pour que la vitesse revienne à 0
#define STOP_SPEED 0.05     // Vitesse en dessous de laquelle on force 0

// Calibration Tension
#define ADC_RESOLUTION 4095.0f
#define ADC_REF_VOLTAGE 4.98f
// Correction appliquée pour ta batterie (7.62V réel vs 6.22V mesuré)
const float FACTEUR_DIVISEUR = 4.78f; 

// Initialisation ECRAN (SH1106)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Initialisation IMU (BNO055)
// Note: Adresse 0x28 par défaut. Si ça ne marche pas, essayer 0x29
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28); 

Servo escMoteur;

// ================================================================
// 2. VARIABLES GLOBALES
// ================================================================
// Physique
float accelX, accelY, accelZ;
float speedX = 0, speedY = 0, speedZ = 0;
float totalAccel = 0, totalSpeed = 0;

// Calibration & Orientation
float offsetX = 0, offsetY = 0, offsetZ = 0;
float startHeading = 0, startRoll = 0, startPitch = 0;
float relHeading = 0, relRoll = 0, relPitch = 0;

// Temps
unsigned long lastTime = 0;

// Moteur
unsigned long motorTimer = 0;
int motorStep = 0;

// Batterie
float batteryVoltage = 0.0f;

// ================================================================
// 3. FONCTIONS UTILITAIRES
// ================================================================

// Fait bipper le buzzer
void bip(int frequence, int duree) {
  tone(BUZZER_PIN, frequence, duree);
}

// Normalise un angle entre 0 et 360
float getAngle0to360(float current, float start) {
  float delta = current - start;
  while (delta < 0) delta += 360;
  while (delta >= 360) delta -= 360;
  return delta;
}

// Normalise un angle entre -180 et 180
float getAngleSigned(float current, float start) {
  float delta = current - start;
  if (delta < -180) delta += 360;
  if (delta > 180) delta -= 360;
  return delta;
}

// ================================================================
// 4. SETUP (Démarrage)
// ================================================================
void setup() {
  // --- A. SECURITE DEMARRAGE ---
  // Pause CRITIQUE pour laisser le BNO s'allumer avant de lui parler
  delay(1000); 

  Wire.begin();
  // On force une vitesse I2C standard pour éviter les erreurs
  Wire.setClock(100000); 

  Serial.begin(115200);

  // --- B. INIT PERIPHERIQUES SIMPLES ---
  escMoteur.attach(PIN_ESC);
  escMoteur.write(90); // Armement ESC (Neutre)
  
  pinMode(BUZZER_PIN, OUTPUT);
  bip(1000, 50); // Petit bip de vie

  u8g2.begin();
  analogReadResolution(12); // Mode 12 bits pour le Nano R4

  // --- C. INIT BNO055 (ROBUSTE) ---
  bool bnoDetected = false;
  // On tente 3 fois de le lancer au cas où il rate le premier coup
  for(int i=0; i<3; i++) {
     if (bno.begin()) {
        bnoDetected = true;
        break;
     }
     delay(200);
  }

  // Si échec total
  if (!bnoDetected) {
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(10, 30, "ERREUR BNO055");
      u8g2.drawStr(10, 45, "Verifier cables");
    } while (u8g2.nextPage());
    // On bloque tout et on alarme
    while (1) { bip(200, 500); delay(500); }
  }

  // Pause indispensable après le begin() car le BNO change de mode
  delay(500); 

  // IMPORTANT : On désactive le quartz externe pour éviter les plantages aléatoires
  bno.setExtCrystalUse(false); 

  // --- D. CALIBRATION (Tare) ---
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "Calibration...");
    u8g2.drawStr(10, 45, "NE PAS BOUGER !");
  } while (u8g2.nextPage());

  delay(1000); // Temps pour poser le robot

  // Tare de l'accéléromètre (moyenne de 100 mesures)
  float sumX = 0, sumY = 0, sumZ = 0;
  int numSamples = 100;
  for(int i = 0; i < numSamples; i++) {
    imu::Vector<3> vec = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    sumX += vec.x();
    sumY += vec.y();
    sumZ += vec.z();
    delay(10); // Petite pause pour le bus I2C
  }

  offsetX = sumX / (float)numSamples;
  offsetY = sumY / (float)numSamples;
  offsetZ = sumZ / (float)numSamples;

  // Enregistrement de l'orientation initiale
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  startHeading = euler.x();
  startRoll = euler.y();
  startPitch = euler.z();

  // Double bip de succès
  bip(3000, 80); delay(80); bip(3000, 80);

  lastTime = millis();
  motorTimer = millis();
}

// ================================================================
// 5. LOOP (Boucle Principale)
// ================================================================
void loop() {
  unsigned long now = millis();
  double dt = (now - lastTime) / 1000.0; // Temps écoulé en secondes
  lastTime = now;

  // --- 1. MESURE TENSION ---
  long sum = 0;
  int samples = 32;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(PIN_BATTERY);
    delay(1);
  }
  float adcAvg = sum / (float)samples;
  float voltageInput = (adcAvg * ADC_REF_VOLTAGE) / ADC_RESOLUTION;
  batteryVoltage = voltageInput * FACTEUR_DIVISEUR;

  // --- 2. LECTURE CAPTEURS & CALCULS ---
  // Lecture Accélération Linéaire (sans gravité)
  imu::Vector<3> linAcc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  // Application de la tare (offset)
  accelX = linAcc.x() - offsetX;
  accelY = linAcc.y() - offsetY;
  accelZ = linAcc.z() - offsetZ;

  // Calcul Vitesse (Intégration : V = a * t) avec friction
  auto updateSpeed = [&](float &v, float a) {
    if (abs(a) > DEADZONE) {
      v += a * dt; // On ajoute l'accélération
    } else {
      v *= FRICTION; // On freine doucement si pas de mouvement
      if (abs(v) < STOP_SPEED) v = 0; // Stop net si très lent
    }
  };

  updateSpeed(speedX, accelX);
  updateSpeed(speedY, accelY);
  updateSpeed(speedZ, accelZ);

  // Calcul des totaux (Pythagore en 3D)
  totalAccel = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  totalSpeed = sqrt(sq(speedX) + sq(speedY) + sq(speedZ));

  // Lecture Orientation
  imu::Vector<3> orient = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  relHeading = getAngle0to360(orient.x(), startHeading);

  // --- 3. ALARME CHOC ---
  if (totalAccel > SHOCK_LIMIT) { 
      bip(4000, 50); 
  }

  // --- 4. AFFICHAGE OLED (Design Tableau) ---
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);

    // -- EN-TÊTE --
    u8g2.drawHLine(0, 10, 128); // Ligne sous le titre
    
    // Cap
    u8g2.drawStr(0, 8, "C:");
    u8g2.setCursor(15, 8); 
    u8g2.print(relHeading, 0); 
    u8g2.print("\260");

    // Batterie
    u8g2.drawStr(80, 8, "Bat:");
    u8g2.setCursor(104, 8); 
    u8g2.print(batteryVoltage, 1); 
    u8g2.print("V");

    // -- CORPS (COLONNES) --
    u8g2.drawStr(15, 20, "ACC");
    u8g2.drawStr(75, 20, "VIT");
    
    u8g2.drawVLine(64, 10, 43); // Ligne verticale milieu

    // Lignes X Y Z
    int yX = 31, yY = 41, yZ = 51;
    u8g2.drawStr(0, yX, "X"); u8g2.drawStr(0, yY, "Y"); u8g2.drawStr(0, yZ, "Z");

    // Données Accel
    u8g2.setCursor(12, yX); u8g2.print(accelX, 1);
    u8g2.setCursor(12, yY); u8g2.print(accelY, 1);
    u8g2.setCursor(12, yZ); u8g2.print(accelZ, 1);

    // Données Vitesse
    u8g2.setCursor(72, yX); u8g2.print(speedX, 1);
    u8g2.setCursor(72, yY); u8g2.print(speedY, 1);
    u8g2.setCursor(72, yZ); u8g2.print(speedZ, 1);

    // -- PIED DE PAGE (TOTAUX) --
    u8g2.drawHLine(0, 54, 128); 
    
    // Total Accel
    u8g2.setCursor(0, 64); 
    u8g2.print(totalAccel, 1); 
    u8g2.drawStr(24, 64, "m/s2");

    // Total Vitesse
    u8g2.setCursor(68, 64); 
    u8g2.print(totalSpeed, 1); 
    u8g2.drawStr(92, 64, "m/s");

  } while (u8g2.nextPage());

  // --- 5. GESTION AUTOMATIQUE MOTEUR ---
  unsigned long elapsed = now - motorTimer;
  switch (motorStep) {
    case 0: // AVANT
      escMoteur.write(98);
      if (elapsed >= 2000) { motorStep = 1; motorTimer = now; }
      break;
    case 1: // FREIN
      escMoteur.write(90);
      if (elapsed >= 1000) { motorStep = 2; motorTimer = now; }
      break;
    case 2: // ARRIERE (Coup 1)
      escMoteur.write(80);
      if (elapsed >= 100) { motorStep = 3; motorTimer = now; }
      break;
    case 3: // NEUTRE
      escMoteur.write(90);
      if (elapsed >= 100) { motorStep = 4; motorTimer = now; }
      break;
    case 4: // ARRIERE (Coup 2)
      escMoteur.write(80);
      if (elapsed >= 2000) { motorStep = 5; motorTimer = now; }
      break;
    case 5: // RETOUR NEUTRE
      escMoteur.write(90);
      if (elapsed >= 1000) { motorStep = 0; motorTimer = now; }
      break;
  }
}