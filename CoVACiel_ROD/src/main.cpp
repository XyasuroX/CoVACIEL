#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>  // Pour l'OLED SH1106

// Définition de l'OLED SH1106 (I2C hardware, résolution 128x64)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Définition de la BNO055 (ID par défaut 55, adresse I2C 0x28)
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);  // Si adresse différente, change 0x28 en 0x29

void setup() {
  // Initialisation I2C
  Wire.begin();

  // Initialisation OLED
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);  // Police basique

  // Initialisation BNO055
  if (!bno.begin()) {
    // Erreur si non détectée
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "BNO055 non detectee!");
    u8g2.drawStr(0, 40, "Verif connexions.");
    u8g2.sendBuffer();
    while (1);  // Boucle infinie en cas d'erreur
  }

  // Calibration basique (attends 1s pour stabilisation)
  delay(1000);
  bno.setExtCrystalUse(true);  // Utilise cristal externe pour précision

  // Message de démarrage sur OLED
  u8g2.clearBuffer();
  u8g2.drawStr(0, 20, "BNO055 OK");
  u8g2.drawStr(0, 40, "Test en cours...");
  u8g2.sendBuffer();
}

void loop() {
  // Lecture des données
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  // Affichage sur OLED
  u8g2.clearBuffer();
  
  // Accéléromètre
  u8g2.drawStr(0, 10, "Accel (m/s^2):");
  char buf[40];
  sprintf(buf, "X: %.2f Y: %.2f Z: %.2f", accel.x(), accel.y(), accel.z());
  u8g2.drawStr(0, 20, buf);
  
  // Gyroscope
  u8g2.drawStr(0, 30, "Gyro (rad/s):");
  sprintf(buf, "X: %.2f Y: %.2f Z: %.2f", gyro.x(), gyro.y(), gyro.z());
  u8g2.drawStr(0, 40, buf);
  
  // Orientation Euler
  u8g2.drawStr(0, 50, "Euler (deg):");
  sprintf(buf, "X: %.2f Y: %.2f Z: %.2f", euler.x(), euler.y(), euler.z());
  u8g2.drawStr(0, 60, buf);
  
  u8g2.sendBuffer();

  delay(1000);  // Mise à jour toutes les secondes
}


/*
// ======================
//  AFFICHAGE OLED
// ======================
void ecran(float valeur) {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x10_tr);

  char buf[32];
  snprintf(buf, sizeof(buf), "Accel X : %.2f", valeur);

  u8g2.drawStr(0, 20, buf);

  u8g2.sendBuffer();
}
*/