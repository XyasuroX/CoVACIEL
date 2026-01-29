#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>

// Écran OLED SH1106 en mode Page Buffer (pour économiser la RAM de la UNO)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// BNO055
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// Variables globales
float accelX, accelY, accelZ; // Accélération linéaire (m/s^2)
float speedX = 0, speedY = 0; // Vitesse estimée (m/s)

// Gestion du temps pour le calcul de vitesse
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  
  u8g2.begin();
  
  if (!bno.begin()) {
    Serial.println("Erreur BNO055");
    while (1);
  }
  
  bno.setExtCrystalUse(true);
  lastTime = millis();
}

void loop() {
  // 1. GESTION DU TEMPS (dt)
  unsigned long now = millis();
  double dt = (now - lastTime) / 1000.0; // Temps écoulé en secondes
  lastTime = now;

  // 2. LECTURE DES CAPTEURS
  // On utilise VECTOR_LINEARACCEL pour ignorer la gravité terrestre
  imu::Vector<3> linearAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  
  accelX = linearAccel.x();
  accelY = linearAccel.y();
  accelZ = linearAccel.z();

  // 3. CALCUL DE LA VITESSE (INTEGRATION)
  // Zone morte (Deadzone) : si l'accélération est très faible (bruit), on considère que c'est 0
  // Cela évite que la vitesse n'augmente toute seule quand le capteur est posé.
  float deadzone = 0.15; 

  if (abs(accelX) > deadzone) {
    speedX += accelX * dt;
  } else {
    // Optionnel : décommenter la ligne suivante pour remettre la vitesse à 0 si pas de mouvement
    // speedX *= 0.95; // Effet de frottement virtuel pour arrêter la dérive
  }

  if (abs(accelY) > deadzone) {
    speedY += accelY * dt;
  } else {
    // speedY *= 0.95; 
  }

  // 4. AFFICHAGE REALTERM (CSV)
  // Format : AccelX, AccelY, VitesseX, VitesseY
  Serial.print(accelX, 2); Serial.print(",");
  Serial.print(accelY, 2); Serial.print(",");
  Serial.print(speedX, 2); Serial.print(",");
  Serial.println(speedY, 2);

  // 5. AFFICHAGE OLED
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // En-tête
    u8g2.drawStr(0, 10, "IMU - Mouvement");
    u8g2.drawHLine(0, 12, 128);

    // Colonne Gauche : Accélération
    u8g2.drawStr(0, 25, "Acc (m/s2)");
    u8g2.setCursor(0, 35); u8g2.print("X: "); u8g2.print(accelX, 1);
    u8g2.setCursor(0, 45); u8g2.print("Y: "); u8g2.print(accelY, 1);
    
    // Colonne Droite : Vitesse
    u8g2.drawStr(70, 25, "Vit (m/s)");
    u8g2.setCursor(70, 35); u8g2.print(speedX, 1);
    u8g2.setCursor(70, 45); u8g2.print(speedY, 1);

    // Note sur la dérive
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(0, 60, "Reset:Btn Reset");

  } while ( u8g2.nextPage() );
  
  delay(20); // Mise à jour rapide (50Hz environ)
}