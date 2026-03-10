/**
 * PROJET : NIVEAU À BULLE (Contrôles Inversés + Silencieux)
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>

// ================= REGLAGES =================
#define BUZZER_PIN  A3
#define SENSITIVITY 2.5 

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

float startRoll = 0, startPitch = 0;

void bip(int freq, int duree) {
  tone(BUZZER_PIN, freq, duree);
}

float getAngleSigned(float current, float start) {
  float delta = current - start;
  if (delta < -180) delta += 360;
  if (delta > 180) delta -= 360;
  return delta;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  u8g2.begin();

  if (!bno.begin()) {
    while (1); // Bloque sans bruit si erreur
  }
  bno.setExtCrystalUse(true);

  // --- CALIBRAGE ---
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "Calibration...");
    u8g2.drawStr(10, 45, "Poser a plat !");
  } while (u8g2.nextPage());
  
  delay(1000); 

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  startRoll    = euler.y();
  startPitch   = euler.z();
  
  // Juste un petit bip court pour dire "C'est prêt"
  bip(2000, 100); 
}

void loop() {
  // 1. Lecture
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  
  float roll  = getAngleSigned(euler.y(), startRoll);  
  float pitch = getAngleSigned(euler.z(), startPitch); 

  // 2. Calcul Position (CONTROLES INVERSÉS ICI)
  int centerX = 64;
  int centerY = 32;

  // J'ai inversé les signes + et - par rapport au code précédent
  // Si ça va toujours pas dans le sens que tu veux, inverse encore le + et le -
  int ballX = centerX + (roll * SENSITIVITY); 
  int ballY = centerY - (pitch * SENSITIVITY); 

  // Limites écran
  if (ballX < 4) ballX = 4;
  if (ballX > 124) ballX = 124;
  if (ballY < 4) ballY = 4;
  if (ballY > 60) ballY = 60;

  // Detection centre (juste pour l'affichage visuel "PARFAIT")
  bool isLevel = (abs(ballX - centerX) < 3 && abs(ballY - centerY) < 3);

  // 3. AFFICHAGE
  u8g2.firstPage();
  do {
    // Cible
    u8g2.drawCircle(centerX, centerY, 10, U8G2_DRAW_ALL); 
    u8g2.drawLine(centerX-15, centerY, centerX+15, centerY);
    u8g2.drawLine(centerX, centerY-15, centerX, centerY+15);

    // Bille
    u8g2.drawDisc(ballX, ballY, 4, U8G2_DRAW_ALL);

    // Infos texte
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.setCursor(0, 64); u8g2.print("X:"); u8g2.print(roll, 0);
    u8g2.setCursor(100, 64); u8g2.print("Y:"); u8g2.print(pitch, 0);

    if (isLevel) {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(40, 10, "PARFAIT !");
    }

  } while ( u8g2.nextPage() );

  // PLUS DE SON ICI (Mode Silencieux)
}