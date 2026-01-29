#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <U8g2lib.h>

/* --- 1. CHANGEMENT CRITIQUE ICI --- */
// On remplace _F_ (Full framebuffer) par _1_ (Page buffer) pour économiser la RAM de la UNO
// Sinon la UNO plante car elle n'a que 2Ko de RAM.
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

// Variables pour stocker les données
float h, p, r; // Heading, Pitch, Roll

void setup() {
  // 2. Initialisation Série (Vitesse 115200 pour RealTerm)
  Serial.begin(115200);
  
  // Initialisation I2C et Ecran
  u8g2.begin();
  
  Serial.println("Demarrage...");
  
  // Initialisation BNO055
  if (!bno.begin()) {
    Serial.println("Erreur BNO055");
    while (1);
  }
  
  bno.setExtCrystalUse(true);
}

void loop() {
  // --- LECTURE DES DONNEES ---
  sensors_event_t event;
  bno.getEvent(&event);
  
  h = event.orientation.x;
  p = event.orientation.y;
  r = event.orientation.z;

  // --- ENVOI VERS REALTERM (CSV) ---
  // Format : Cap, Tangage, Roulis
  Serial.print(h, 2); Serial.print(",");
  Serial.print(p, 2); Serial.print(",");
  Serial.println(r, 2);

  // --- AFFICHAGE OLED (Mode Page Buffer) ---
  // Cette boucle est OBLIGATOIRE avec le constructeur _1_
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Titre
    u8g2.drawStr(0, 10, "BNO055 - UNO");
    u8g2.drawHLine(0, 12, 128);

    // Affichage des valeurs ligne par ligne
    // Note: sprintf %f ne marche pas bien sur UNO par défaut, on utilise print
    
    u8g2.setCursor(0, 25);
    u8g2.print("Cap (X): "); u8g2.print(h, 1);
    
    u8g2.setCursor(0, 40);
    u8g2.print("Tang(Y): "); u8g2.print(p, 1);
    
    u8g2.setCursor(0, 55);
    u8g2.print("Roul(Z): "); u8g2.print(r, 1);

  } while ( u8g2.nextPage() );
  
  delay(50); // Petit délai
}