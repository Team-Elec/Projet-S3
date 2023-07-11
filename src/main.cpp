#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComBluetooth.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

// Valeurs importante a changer si jamais
// Vitesse envoie et vitesse de sécurite anti flicker
// En milisecondes
#define SendTime 1000
#define LumiereSecurite 2000
#define BatterieSecurite 2000

// Lieu des sortie Serial
bool Bluetooth = false;
bool SerialOrdi = false;
bool SerialOrdiVal = true;
bool BluetoothVal = false;

// Ajustement PID
float CoeAjustPSepic = 1;
float CoeAjustISepic = 2;
float CoeAjustPBatterie = 6;
float CoeAjustIBatterie = 4;

// Valeur des ajustement du potentiomètre
int MaximumAjust = 14;
int MinimumAjust = 10;

// Voltage SEPIC
int VoltageDemanderOFF = 0;

// Courant dans la batterie (en Volt)
float CourantDansLaBatterie = 2.5;

// Maximum de variation avant la fermeture (Valeur + chiffre et Valeur - chiffre)
float MaxVariation = 0.5;

// Valeur de la diode
float TensionDiode = 0.3;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;
unsigned long CurrentMicros = 0;

float VoltageDemanderLum, VoltageDemanderBattVide, VoltageDemanderBatt = 0;
float IPIDBatt, IPIDBattOn, IPIDOFF, IPIDLum = 0;
float ValeurAjustementSepicBatt, ValeurAjustementSepicBattOn, ValeurAjustementSepicLum, ValeurAjustementSepicOFF = 0;
float ModeSepic = 0;
float VoltageSepic, VoltageOFF, CourantBatterie, VoltageEntree = 0;
float PWMSEPIC = 0;
int16_t PWMSEPICINT = 0;
int thermo1, thermo2, thermo3, thermo4, thermo5 = 0;
float LastValTempsLum, LastValTempsBatt, LastValTempsBattOn, LastValTempsOFF = 0;
float ErreurLum, ErreurBatt, ErreurBattOn, ErreurOFF = 0;

// Debounce
unsigned lastsendtime, lastLumiere, lastBatterie = 0;

// Modes et protections
bool ModeBatterie = false;
bool ModeLumiere = false;
bool ProtectMode = false;
bool BatterieInitiale = false;
bool BatterieOuverte = false;

void setup()
{
  appelVariables();
  VitessePWM();

  ADS.begin();
  ADS.setGain(0);     // Pour avoir +- 6.144 V
  ADS.setDataRate(7); // Pour l'avoir vite en tas
  ADS.setMode(0);     // Mode continue
}

void loop()
{

  // Pour le temps dans le Code
  CurrentMillis = millis();
  CurrentMicros = micros();

  // Lecture des ports du ADS1115
  int16_t SORTIE_SEPIC = ADS.readADC(0);
  int16_t VALEUR_BATTERIE = ADS.readADC(1);
  int16_t COURANT = ADS.readADC(2);
  int16_t ENTREE = ADS.readADC(3);

  // Transfert vers les vrai valeurs
  VoltageDemanderLum = ((analogRead(Ajust) * (MaximumAjust - MinimumAjust)) / 1023) + MinimumAjust;
  VoltageDemanderBattVide = VALEUR_BATTERIE;
  VoltageSepic = (SORTIE_SEPIC * 6.144) / 32768;
  CourantBatterie = (COURANT * 6.144) / 32768;
  VoltageEntree = (ENTREE * 6.144) / 32768;

  // Lecture pour le mode du SEPIC (Potentiomètre)
  ModeSepic = analogRead(Mode);

  // Protection pour pas qu'il partent dans le mauvais mode
  // Doit avoir ProtectMode de true pour être capable d'entrer
  // Mode : <200 = batterie, <850 = lumière, <800 et >250 Mode off
  if (ModeSepic > 200 && ModeSepic < 850)
  {
    ProtectMode = true;
  }
  if (ProtectMode)
  {
    // Mode batterie
    if (ModeSepic < 200)
    {
      ModeBatterie = true;
      ModeLumiere = false;
    }
    // Mode off
    if (ModeSepic > 250 && ModeSepic < 800)
    {
      ModeBatterie = false;
      ModeLumiere = false;
    }
    // Mode Lumière
    if (ModeSepic > 850)
    {
      ModeBatterie = false;
      ModeLumiere = true;
    }
  }

  // Ajustement des convertisseur
  //  PID de l'ajustement
  if (ModeLumiere)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 1);

    ErreurLum = VoltageDemanderLum - VoltageSepic;
    PWMSEPIC = ((VoltageDemanderLum + TensionDiode) / (VoltageEntree + VoltageDemanderLum + TensionDiode));

    IPIDLum += (CurrentMicros - LastValTempsLum) * ErreurLum;
    LastValTempsLum = CurrentMicros;

    ValeurAjustementSepicLum = (ErreurLum * CoeAjustPSepic) + (IPIDLum * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicLum;
    PWMSEPICINT = int16_t(PWMSEPIC);

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > 255)
    {
      PWMSEPICINT = 255;
    }
    analogWrite(SEPIC, PWMSEPICINT);
  }

  if (ModeBatterie)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 2);

    ErreurBatt = VoltageDemanderBatt - VoltageSepic;
    PWMSEPIC = ((VoltageDemanderBatt + TensionDiode) / (VoltageEntree + VoltageDemanderBatt + TensionDiode));

    IPIDBatt += (CurrentMicros - LastValTempsBatt) * ErreurBatt;
    LastValTempsBatt = CurrentMicros;

    ValeurAjustementSepicBatt = (ErreurBatt * CoeAjustPSepic) + (IPIDBatt * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicBatt;
    PWMSEPICINT = int16_t(PWMSEPIC);

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > 255)
    {
      PWMSEPICINT = 255;
    }
    analogWrite(SEPIC, PWMSEPICINT);
  }

  if (!ModeLumiere && !ModeBatterie)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 3);

    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 4);

    ErreurOFF = VoltageDemanderOFF - VoltageSepic;
    PWMSEPIC = ((VoltageDemanderOFF + TensionDiode) / (VoltageEntree + VoltageDemanderOFF + TensionDiode));

    IPIDOFF += (CurrentMicros - LastValTempsOFF) * ErreurOFF;
    LastValTempsOFF = CurrentMicros;

    ValeurAjustementSepicOFF = (ErreurOFF * CoeAjustPSepic) + (IPIDOFF * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicOFF;
    PWMSEPICINT = int16_t(PWMSEPIC);

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > 255)
    {
      PWMSEPICINT = 255;
    }

    analogWrite(SEPIC, PWMSEPICINT);
  }

  // POSSIBILITÉ DE PROBLÈME A VÉRIFIER
  //  Code pour la Lumiere Activation
  if (ModeLumiere)
  {
    if (((VoltageDemanderLum - MaxVariation) < VoltageSepic) && ((VoltageDemanderLum + MaxVariation) > VoltageSepic))
    {
      // Petit code pour debogguer YOUPIIII
      printage(Bluetooth, SerialOrdi, 6);

      // Ouverture du MOSFET après le bon nombre de temps
      if (CurrentMillis - lastLumiere > LumiereSecurite)
      {
        // Petit code pour debogguer YOUPIIII
        printage(Bluetooth, SerialOrdi, 7);

        digitalWrite(OFF, LOW);
        digitalWrite(Batterie, LOW);
        digitalWrite(Lumiere, HIGH);
      }
    }

    if (((VoltageDemanderLum - MaxVariation) > VoltageSepic) && ((VoltageDemanderLum + MaxVariation) < VoltageSepic))
    {
      // Petit code pour debogguer YOUPIIII
      printage(Bluetooth, SerialOrdi, 8);

      // Securité pour la lumière pas qu'elle ouvre ferme rapidement
      lastLumiere = CurrentMillis;
      // Fermeture du MOSFET
      digitalWrite(Lumiere, LOW);
    }
  }

  // Code pour la Batterie Activation
  if (ModeBatterie)
  {
    if (BatterieOuverte == false)
    {
      // Petit code pour debogguer YOUPIIII
      printage(Bluetooth, SerialOrdi, 9);

      VoltageDemanderBatt = VoltageDemanderBattVide;
      if ((VoltageDemanderBatt < VoltageSepic) && ((VoltageDemanderBatt + MaxVariation) > VoltageSepic))
      {
        // Petit code pour debogguer YOUPIIII
        printage(Bluetooth, SerialOrdi, 10);

        BatterieOuverte = true;
        digitalWrite(OFF, LOW);
        digitalWrite(Lumiere, LOW);
        digitalWrite(Batterie, HIGH);
      }
    }
    if (BatterieOuverte == true)
    {
      // Petit code pour debogguer YOUPIIII
      printage(Bluetooth, SerialOrdi, 11);
      ErreurBattOn = (CourantDansLaBatterie - CourantBatterie);
      IPIDBattOn += (CurrentMicros - LastValTempsBattOn) * ErreurBattOn;
      LastValTempsBattOn = CurrentMicros;

      ValeurAjustementSepicBattOn = (ErreurBattOn * CoeAjustPBatterie) + ((IPIDBattOn)*CoeAjustIBatterie);

      VoltageDemanderBatt = VoltageDemanderBattVide + ValeurAjustementSepicBattOn;
    }

    if (CourantBatterie < 4)
    {
      // Ouverture du MOSFET après le bon nombre de temps
      if (CurrentMillis - lastBatterie > BatterieSecurite)
      {
        BatterieOuverte = true;
      }
    }

    if (CourantBatterie >= 4)
    {
      BatterieOuverte = false;
      // Fermeture du MOSFET
      digitalWrite(Batterie, LOW);

      // Securité pour la lumière pas qu'elle ouvre ferme rapidement
      lastBatterie = CurrentMillis;
    }
  }

  // Code pour le circuit SEPIC à OFF
  if (!ModeLumiere && !ModeBatterie)
  {
    BatterieOuverte = false;

    digitalWrite(Lumiere, LOW);
    digitalWrite(Batterie, LOW);
    digitalWrite(OFF, HIGH);
  }

  // Envoi du signal
  if (CurrentMillis - lastsendtime > SendTime)
  {
    if (SerialOrdiVal)
    {
      Serial.print("Mode : ");
      if (ModeLumiere)
      {
        Serial.print("Lumière");
      }
      if (ModeBatterie)
      {
        Serial.print("Batterie");
      }
      if (!ModeLumiere && !ModeBatterie)
      {
        Serial.print("Fuckall");
      }
      Serial.print(" \tValeur entrée : ");
      Serial.print(VoltageEntree);

      Serial.print(" \tTension demander Lumière : ");
      Serial.print(VoltageDemanderLum);

      Serial.print(" \tTension Sepic : ");
      Serial.print(VoltageSepic);

      Serial.print(" \tPWM SEPIC : ");
      Serial.print(PWMSEPICINT);

      Serial.print("\n");
    }
    if (BluetoothVal)
    {
      Serial1.print("Mode : ");
      if (ModeLumiere)
      {
        Serial.print("Lumière");
      }
      if (ModeBatterie)
      {
        Serial1.print("Batterie");
      }
      if (!ModeLumiere && !ModeBatterie)
      {
        Serial1.print("Fuckall");
      }
      Serial1.print(" \tValeur entrée : ");
      Serial1.print(VoltageEntree);

      Serial1.print(" \tTension demander Lumière : ");
      Serial1.print(VoltageDemanderLum);

      Serial1.print(" \tTension Sepic : ");
      Serial1.print(VoltageSepic);

      Serial1.print(" \tPWM SEPIC : ");
      Serial1.print(PWMSEPICINT);

      Serial1.print("\n");
    }
    lastsendtime = CurrentMillis;
  }

  if (SORTIE_SEPIC < 0)
  {
    SORTIE_SEPIC = 0;
  }
  if (COURANT < 0)
  {
    COURANT = 0;
  }
  if (VALEUR_BATTERIE < 0)
  {
    VALEUR_BATTERIE = 0;
  }
}
