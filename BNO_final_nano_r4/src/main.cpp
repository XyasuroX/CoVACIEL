/**
 * PROJET : IMU ULTIME - Mouvement + Boussole + BUZZER
 * Version : Calibration précise (Correction du biais Z sans forcer à 0)
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>
#include <Servo.h> 

// ================================================================
// 1. REGLAGES
// ================================================================
#define BUZZER_PIN  D6      
#define SHOCK_LIMIT 8.0     
#define DEADZONE    0.15    // Remis à ta valeur d'origine (plus sensible)
#define FRICTION    0.98    
#define STOP_SPEED  0.05    

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// ================================================================
// 2. VARIABLES
// ================================================================
float accelX, accelY, accelZ;
float speedX = 0, speedY = 0, speedZ = 0;
float totalAccel = 0, totalSpeed = 0;
float offsetX = 0, offsetY = 0, offsetZ = 0; 

float startHeading = 0, startRoll = 0, startPitch = 0;
float relHeading = 0, relRoll = 0, relPitch = 0;

unsigned long lastTime = 0;

// Variables pour la gestion du temps Moteur
unsigned long motorTimer = 0;
int motorStep = 0;

// ================================================================
// 3. FONCTIONS UTILITAIRES
// ================================================================
void bip(int frequence, int duree) {
  tone(BUZZER_PIN, frequence, duree);
}

float getAngle0to360(float current, float start) {
  float delta = current - start;
  while (delta < 0) delta += 360;
  while (delta >= 360) delta -= 360;
  return delta;
}

float getAngleSigned(float current, float start) {
  float delta = current - start;
  if (delta < -180) delta += 360;
  if (delta > 180) delta -= 360;
  return delta;
}

Servo escMoteur;
int pinESC = 9; 

// ================================================================
// 4. SETUP (C'est ici que tout se joue)
// ================================================================
void setup() {
  Serial.begin(115200);
  escMoteur.attach(pinESC);
  escMoteur.write(90); 
  delay(100); 

  pinMode(BUZZER_PIN, OUTPUT);
  bip(2000, 100); delay(150);
  bip(2500, 100);

  u8g2.begin();

  if (!bno.begin()) {
    while (1) { bip(500, 1000); delay(1000); }
  }
  bno.setExtCrystalUse(true);

  // --- ECRAN D'ATTENTE ---
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "Stabilisation...");
    u8g2.drawStr(10, 45, "NE PAS BOUGER !");
  } while (u8g2.nextPage());

  // [IMPORTANT] : On laisse le BNO055 "comprendre" la gravité avant de calibrer
  // Si on calibre trop vite, on enregistre une erreur qui va créer la dérive.
  delay(1500); 

  // -- 1. TARE ACCEL (Calibration précise) --
  // On prend la valeur moyenne sur 200 échantillons (environ 2 secondes)
  // Cette moyenne SERA la valeur qu'on enlèvera "au fur et à mesure"
  float sumX = 0, sumY = 0, sumZ = 0;
  int numSamples = 200;

  for(int i=0; i<numSamples; i++) {
    imu::Vector<3> vec = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    sumX += vec.x();
    sumY += vec.y();
    sumZ += vec.z();
    delay(10); 
  }
  
  offsetX = sumX / (float)numSamples;
  offsetY = sumY / (float)numSamples;
  offsetZ = sumZ / (float)numSamples;

  // -- 2. TARE ANGLES --
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  startHeading = euler.x();
  startRoll    = euler.y();
  startPitch   = euler.z();
  
  // Bip de fin de calibration
  bip(3000, 100); delay(100); bip(3000, 100);
  
  lastTime = millis();
  motorTimer = millis(); 
}

// ================================================================
// 5. LOOP (Identique à ton original, sans le hack agressif)
// ================================================================
void loop() {
  unsigned long now = millis();
  double dt = (now - lastTime) / 1000.0;
  lastTime = now;

  // --- CALCUL PHYSIQUE ---
  imu::Vector<3> linAcc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  
  // ICI : On enlève la valeur calculée au début. 
  // Si offsetZ a bien capturé le 0.1, alors (linAcc.z - offsetZ) sera égal à 0.
  accelX = linAcc.x() - offsetX;
  accelY = linAcc.y() - offsetY;
  accelZ = linAcc.z() - offsetZ; 

  auto updateSpeed = [&](float &v, float a) {
    if (abs(a) > DEADZONE) v += a * dt;
    else { v *= FRICTION; if (abs(v) < STOP_SPEED) v = 0; }
  };
  
  updateSpeed(speedX, accelX);
  updateSpeed(speedY, accelY);
  updateSpeed(speedZ, accelZ);

  totalAccel = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  totalSpeed = sqrt(sq(speedX) + sq(speedY) + sq(speedZ));

  if (totalAccel > SHOCK_LIMIT) { bip(4000, 50); }

  imu::Vector<3> orient = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  relHeading = getAngle0to360(orient.x(), startHeading); 
  relRoll    = getAngleSigned(orient.y(), startRoll);    
  relPitch   = getAngleSigned(orient.z(), startPitch);   

  // --- OLED & SERIAL ---
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(2, 9, "Cap:"); 
    u8g2.setCursor(28, 9); u8g2.print(relHeading, 0); u8g2.print("\260"); 
    u8g2.drawStr(80, 9, "VIT");
    u8g2.drawHLine(0, 12, 128);        
    u8g2.drawVLine(64, 0, 64);           
    u8g2.drawHLine(0, 52, 128);        

    int lineX = 24, lineY = 36, lineZ = 48;
    int colLeft = 20, colRight = 85;

    u8g2.drawStr(2, lineX, "X"); 
    u8g2.setCursor(colLeft, lineX); u8g2.print(accelX, 1);
    u8g2.setCursor(colRight, lineX); u8g2.print(speedX, 1);
    u8g2.drawStr(2, lineY, "Y"); 
    u8g2.setCursor(colLeft, lineY); u8g2.print(accelY, 1);
    u8g2.setCursor(colRight, lineY); u8g2.print(speedY, 1);
    u8g2.drawStr(2, lineZ, "Z"); 
    u8g2.setCursor(colLeft, lineZ); u8g2.print(accelZ, 1);
    u8g2.setCursor(colRight, lineZ); u8g2.print(speedZ, 1);
    u8g2.drawStr(2, 63, "T"); 
    u8g2.setCursor(colLeft, 63); u8g2.print(totalAccel, 1);
    u8g2.setCursor(colRight, 63); u8g2.print(totalSpeed, 1);
  } while ( u8g2.nextPage() );

  // ================================================================
  // GESTION DU MOTEUR
  // ================================================================
  unsigned long elapsed = now - motorTimer;

  switch (motorStep) {
    case 0: // MARCHE AVANT
      escMoteur.write(98);
      if (elapsed >= 2000) { motorStep = 1; motorTimer = now; }
      break;

    case 1: // FREINAGE
      escMoteur.write(90);
      if (elapsed >= 1000) { motorStep = 2; motorTimer = now; }
      break;

    case 2: // DOUBLE-TAP MARCHE AR (Coup 1)
      escMoteur.write(80);
      if (elapsed >= 100) { motorStep = 3; motorTimer = now; }
      break;

    case 3: // DOUBLE-TAP MARCHE AR (Neutre)
      escMoteur.write(90);
      if (elapsed >= 100) { motorStep = 4; motorTimer = now; }
      break;

    case 4: // DOUBLE-TAP MARCHE AR (Coup 2)
      escMoteur.write(80);
      if (elapsed >= 2000) { motorStep = 5; motorTimer = now; }
      break;

    case 5: // RETOUR NEUTRE
      escMoteur.write(90);
      if (elapsed >= 1000) { motorStep = 0; motorTimer = now; } 
      break;
  }
}