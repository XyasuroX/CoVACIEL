# 🚗 CoVACIEL
> **Co**urse de **V**oitures **A**utonomes du BTS **CIEL**  
> Projet BTS CIEL - Promotion 2026

<p align="center">
  <img src="https://img.shields.io/badge/BTS-CIEL-blue" alt="BTS CIEL">
  <img src="https://img.shields.io/badge/Edition-2025-green" alt="Edition 2025">
  <img src="https://img.shields.io/badge/Chassis-Tamiya_TT--02-orange" alt="Chassis">
</p>

---

## 📋 Présentation

CoVACIEL est un projet ambitieux développé dans le cadre de notre formation **BTS Cybersécurité, Informatique et réseaux, ÉLectronique (CIEL)**. Il s'agit de concevoir et programmer une voiture autonome capable de se déplacer sur circuit sans intervention humaine.

---

## 🏁 À propos de CoVACIEL

### Histoire du projet

CoVACIEL est une course de voitures autonomes destinée aux étudiants de BTS CIEL. Le projet vise à transformer une voiture de modélisme en véhicule autonome grâce à l'ajout d'électronique et d'informatique embarquées. CoVACIEL est ouvert à toutes les formations de BTS CIEL option A et B et constitue un support de projet technique pour l'épreuve E6.2 du BTS CIEL.

### Concept

À partir d'une voiture de modélisme (châssis Tamiya TT-02), les équipes doivent intégrer des systèmes électroniques et informatiques pour rendre leur véhicule totalement autonome. Les voitures s'affronteront ensuite lors de courses dans une ambiance conviviale et bienveillante.

---

## 🎯 Objectifs du projet

Notre mission se décompose en deux phases distinctes :

### Phase 1 : Qualification
- **Manche A** : Réaliser 1 tours complets sur une piste **sans obstacles**
- **Manche B** : Réaliser 1 tours complets sur une piste **avec obstacles fixes**
- La voiture doit être capable de détecter et d'éviter les obstacles de manière autonome
- Aucune intervention humaine n'est autorisée pendant le parcours

### Phase 2 : Course finale
- Course de **3 tours** contre deux autres voitures
- Le **premier à terminer les 3 tours gagne**
- Navigation autonome en présence d'autres véhicules
- Gestion des dépassements et stratégie de course

### Contraintes techniques
- Piste bordée de murs verts (droite) et rouges (gauche) de 200 mm de hauteur
- Largeur minimale de piste : 800 mm
- Sol : lino gris
- Le tracé exact n'est pas connu à l'avance
- Top départ sans fil (module XBEE)

---

## 👥 Équipe du projet

| Nom | Rôle | Responsabilités |
|-----|------|-----------------|
| **Mathis Provent** | Responsable détection d'obstacle | Gestion des capteurs LiDAR et ultrason, traitement des données environnementales |
| **Tom Limouzin** | Responsable Motricité | Contrôle moteur, variateur, direction et servomoteur |
| **Adel Bouhafs** | Responsable Transmission et Télémétrie | Communication RF, module XBEE, transmission de données |
| **Thibault Podechard** | Responsable conduite autonome | Algorithmes de navigation, IA de pilotage, stratégie de course |
| **Mathis Rodriguez** | Responsable Monitoring local | Interface homme-machine, affichage, boutons, buzzer |

---

## 🛠️ Technologies utilisées

### Base mécanique
- **Châssis** : Tamiya TT-02 (échelle 1/10ème)
- **Moteur** : Moteur d'origine Tamiya
- **Alimentation** : Batterie NiMH 7,2V (5000 mAh max)
- **Carrosserie** : TOYOTA Yaris (modèle réduit)

### Nano-ordinateur
- **Raspberry Pi 4 Model B**
  - GPIO pour interfaçage
  - PWM_Dir (direction)
  - PWM_Prop (propulsion)
  - PWM_Buzzer
  - I2C_5V
  - UART

### Capteurs et détection
- **LiDAR** : SLAMTECH - A2 (USB)
  - Détection d'environnement avant
  - ~Cartographie en temps réel
- **Télémètre Ultrason et Cpateur Infrarouge** : SRF10 (I2C_5V) / Sharp GP2Y0A21YK0F
  - Détection d'obstacles arrière
- **Centrale Inertielle** : BOSCH - bno055 (I2C_3V3)
  - Mesure de l'angle azimuth
  - Stabilisation

### Actionneurs
- **Servomoteur** : HITECH - HS5485 (PWM_Dir)
  - Contrôle de la direction
- **Variateur & Moteur** : TAMIYA - ESC (PWM_Prop)
  - Contrôle de la propulsion et de la vitesse

### Communication
- **Émetteur/Récepteur RF** : XBEE
  - Réception du top départ
  - Communication sans fil

### Interface utilisateur
- **Afficheur** : OLED - TF051 SH1106
  - Monitoring temps réel
- **Buzzer** : SONITRON - smart13 (PWM_Buzzer)
  - Signaux sonores
- **Bouton-Poussoir** : GPIO
  - Contrôles manuels

### Cartes électroniques personnalisées
- **Carte HAT** : Interface principale connectée à la Raspberry Pi
- **Carte Interface** : Connexion actionneurs et télémètre ultrason
- **Carte Mezzanine** : Connexion centrale inertielle et IHM

### Stockage d'énergie
- **Batterie principale** : LiPo - 3000mAh - 7,2V
  - Alimentation moteur et électronique de puissance

---

## 📁 Structure du projet

```
CoVACIEL/
├── README.md
├── src/
│   ├── navigation/      # Algorithmes de navigation autonome
│   ├── sensors/         # Gestion des capteurs (LiDAR, ultrason, infrarouge)
│   ├── control/         # Contrôle moteur et direction
│   ├── communication/   # Module XBEE et communication RF
│   └── monitoring/      # Interface utilisateur et affichage
├── hardware/
│   ├── schematics/      # Schémas électroniques
│   └── pcb/             # Designs des cartes HAT, Interface, Mezzanine
├── tests/               # Tests unitaires et d'intégration
└── docs/                # Documentation technique

```

---

## 🚀 Installation et démarrage

### Prérequis
- Raspberry Pi 4
- Utilisation de ROS2 Foxy : https://docs.ros.org/en/foxy/index.html ou Humble : https://docs.ros.org/en/humble/Installation.html

### Cloner le repository
```bash
git clone https://github.com/XyasuroX/CoVACIEL
cd CoVACIEL
```

### Installation des dépendances
```bash
pip install -r requirements.txt
```

### Lancement du système
```bash
python3 src/main.py
```

---

## 📊 Architecture système

Le système CoVACIEL suit une architecture modulaire basée sur le standard **SysML** :

- **Subsystem Capteurs** : Acquisition des données environnementales
- **Subsystem Traitement** : Raspberry Pi - traitement et décision
- **Subsystem Actionneurs** : Exécution des commandes de pilotage
- **Subsystem Stockage Énergie** : Gestion de l'alimentation
- **Subsystem Interface Homme-Machine** : Monitoring et contrôle

---

## 📈 Roadmap

- [x] Montage du châssis Tamiya TT-02
- [x] Conception des cartes électroniques
- [x] Intégration du Raspberry Pi
- [ ] Développement des algorithmes de navigation
- [ ] Tests sur piste sans obstacles
- [ ] Implémentation de la détection d'obstacles
- [ ] Tests sur piste avec obstacles
- [ ] Optimisation de la vitesse de course

---

## 📝 Règlement technique

### Dimensions maximales
- Longueur : conforme au châssis TT-02
- Largeur : conforme au châssis TT-02
- Hauteur : compatible avec carrosserie à 80% minimum

### Visibilité
La voiture doit présenter à l'arrière un rectangle plein :
- Largeur : 150 mm minimum
- Hauteur : 110 mm minimum

### Communication
- Top départ via XBEE (message broadcast : `$GO;`)
- Message d'arrêt : `STOP`
- Canal : C

---

## 🔗 Liens utiles

- [Site officiel CoVACIEL](https://www.covaciel.fr/)
- [Règlement complet](https://www.covaciel.fr/reglement/)
- [Documentation Tamiya TT-02](https://www.tamiya.com/)

---

## 📄 Licence

Ce projet est développé dans un cadre éducatif pour le BTS CIEL.

---

## 🙏 Remerciements

- Nos enseignants et encadrants du BTS CIEL
- L'organisateur de CoVACIEL : Antoine Azan
- Tous les lycées participants à cette première édition

---

<p align="center">
  <i>Projet réalisé dans le cadre du BTS CIEL - Promotion 2026</i><br>
</p>