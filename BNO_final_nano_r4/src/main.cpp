/**
 * PROJET : IMU ULTIME - Mouvement + Boussole + BUZZER
 * Version portée pour Arduino Nano R4
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>

// ================================================================
// 1. REGLAGES
// ================================================================
// --- BUZZER ---
// Note pour Nano R4 : La pin A3 fonctionne très bien en sortie numérique/tone.
#define BUZZER_PIN  D6      
#define SHOCK_LIMIT 8.0    // Si l'accélération dépasse 8 m/s², ça sonne

// --- PHYSIQUE ---
#define DEADZONE    0.15   
#define FRICTION    0.98   
#define STOP_SPEED  0.05   

// --- HARDWARE ---
// La Nano R4 gérera l'I2C matériel automatiquement sur les pins standards (SDA/SCL)
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// ================================================================
// 2. VARIABLES
// ================================================================
float accelX, accelY, accelZ;
float speedX = 0, speedY = 0, speedZ = 0;
float totalAccel = 0, totalSpeed = 0;
float offsetX = 0, offsetY = 0, offsetZ = 0; 

// Orientation
float startHeading = 0, startRoll = 0, startPitch = 0;
float relHeading = 0, relRoll = 0, relPitch = 0;

unsigned long lastTime = 0;

// ================================================================
// 3. FONCTIONS UTILITAIRES
// ================================================================

// Fonction pour faire Bip
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

// ================================================================
// 4. SETUP
// ================================================================
void setup() {
  Serial.begin(115200);
  
  // Sur la R4, il peut être utile d'attendre que le port série s'ouvre 
  // (décommente la ligne ci-dessous si tu veux voir les tout premiers messages)
  // while (!Serial) delay(10); 

  // Config du Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Petit bip de démarrage
  bip(2000, 100); delay(150);
  bip(2500, 100);

  u8g2.begin();
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "Calibration...");
    u8g2.drawStr(10, 45, "NE PAS BOUGER !");
  } while (u8g2.nextPage());

  if (!bno.begin()) {
    // Si erreur capteur : Bip grave et continu
    while (1) {
      bip(500, 1000); 
      delay(1000);
    }
  }
  bno.setExtCrystalUse(true);

  // -- 1. TARE ACCEL --
  Serial.println("Tare Accel...");
  float sumX = 0, sumY = 0, sumZ = 0;
  for(int i=0; i<100; i++) {
    imu::Vector<3> vec = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    sumX += vec.x(); sumY += vec.y(); sumZ += vec.z();
    delay(10); 
  }
  offsetX = sumX/100.0; offsetY = sumY/100.0; offsetZ = sumZ/100.0;

  // -- 2. TARE ANGLES --
  Serial.println("Tare Angles...");
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  startHeading = euler.x();
  startRoll    = euler.y();
  startPitch   = euler.z();
  
  // Bip de fin de calibration
  bip(3000, 200);
  lastTime = millis();
}

// ================================================================
// 5. LOOP
// ================================================================
void loop() {
  unsigned long now = millis();
  double dt = (now - lastTime) / 1000.0;
  lastTime = now;

  // --- CALCUL PHYSIQUE ---
  imu::Vector<3> linAcc = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  accelX = linAcc.x() - offsetX;
  accelY = linAcc.y() - offsetY;
  accelZ = linAcc.z() - offsetZ; 

  // Fonction lambda locale pour la vitesse
  auto updateSpeed = [&](float &v, float a) {
    if (abs(a) > DEADZONE) v += a * dt;
    else { v *= FRICTION; if (abs(v) < STOP_SPEED) v = 0; }
  };
  
  updateSpeed(speedX, accelX);
  updateSpeed(speedY, accelY);
  updateSpeed(speedZ, accelZ);

  totalAccel = sqrt(sq(accelX) + sq(accelY) + sq(accelZ));
  totalSpeed = sqrt(sq(speedX) + sq(speedY) + sq(speedZ));

  // --- ALARME DE CHOC ---
  if (totalAccel > SHOCK_LIMIT) {
    bip(4000, 50); 
  }

  // --- CALCUL ANGLES ---
  imu::Vector<3> orient = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  relHeading = getAngle0to360(orient.x(), startHeading); 
  relRoll    = getAngleSigned(orient.y(), startRoll);    
  relPitch   = getAngleSigned(orient.z(), startPitch);   

  // --- SERIAL ---
  Serial.print("Cap:"); Serial.print(relHeading, 0); 
  Serial.print("\tAccT:"); Serial.println(totalAccel, 2);

  // --- OLED ---
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);

    // En-tête
    u8g2.drawStr(2, 9, "Cap:"); 
    u8g2.setCursor(28, 9); u8g2.print(relHeading, 0); u8g2.print("\260"); 
    u8g2.drawStr(80, 9, "VIT");

    u8g2.drawHLine(0, 12, 128);       
    u8g2.drawVLine(64, 0, 64);        
    u8g2.drawHLine(0, 52, 128);       

    // Corps
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

    // Total
    u8g2.drawStr(2, 63, "T"); 
    u8g2.setCursor(colLeft, 63); u8g2.print(totalAccel, 1);
    u8g2.setCursor(colRight, 63); u8g2.print(totalSpeed, 1);

  } while ( u8g2.nextPage() );
  
  delay(20);
}