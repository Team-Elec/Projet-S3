#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComBluetooth.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

// Valeurs importante a changer si jamais
// Vitesse envoie et vitesse de sécurite anti flicker
// En milisecondes
#define SendTime 200
#define LumiereSecurite 2000
#define BatterieSecurite 2000

// Lieu des sortie Serial
bool Bluetooth = false;
bool SerialOrdi = false;
bool SerialOrdiVal = true;
bool BluetoothVal = false;

// Ajustement PID
float CoeAjustPSepic = 1;
float CoeAjustISepic = 2.2;
float CoeAjustPBatterie = 6;
float CoeAjustIBatterie = 4;

// Valeur des ajustement du potentiomètre
float MaximumAjust = 14;
float MinimumAjust = 10;

// Courant dans la batterie (en Volt)
float CourantDansLaBatterie = 2.5;

// Tension Max batterie
float MaxBattterie = 14.7;

// Maximum de variation avant la fermeture (Valeur + chiffre et Valeur - chiffre)
float MaxVariation = 2;
float RendementSepic = 0.75;

// Valeur de la diode
float TensionDiode = 0.3;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;
unsigned long CurrentMicros = 0;

float VoltageDemanderLum, VoltageDemanderBattVide, VoltageDemanderBatt = 0;
float IPIDBatt, IPIDBattOn, IPIDLum = 0;
float ValeurAjustementSepicBatt, ValeurAjustementSepicBattOn, ValeurAjustementSepicLum = 0;
float ModeSepic = 0;
float VoltageSepic, CourantBatterie, VoltageEntree = 0;
float PWMSEPIC = 0;
int16_t PWMSEPICINT = 0;
int thermo1, thermo2, thermo3, thermo4, thermo5 = 0;
float LastValTempsLum, LastValTempsBatt, LastValTempsBattOn = 0;
float ErreurLum, ErreurBatt, ErreurBattOn, ErreurOFF = 0;
int16_t SORTIE_SEPIC, VALEUR_BATTERIE, COURANT, ENTREE = 0;

// Debounce
unsigned lastsendtime, lastLumiere, lastBatterie = 0;

// Modes et protections
bool ModeBatterie = false;
bool ModeLumiere = false;
bool ProtectMode = true;
bool BatterieDebut = true;
bool BatterieFin = false;
bool BatterieFini = true;

float ValResDiv = 130000.0;

double diviseur = ((1000000 + ValResDiv) / ValResDiv);

int maxVal = ((38 * (1 / diviseur)) * 32750) / 6.144;

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
  if (ADS.readADC(0) < maxVal)
  {
    SORTIE_SEPIC = ADS.readADC(0);
  }

  if (ADS.readADC(1) < maxVal)
  {
    VALEUR_BATTERIE = ADS.readADC(1);
  }

  if (ADS.readADC(2) < maxVal)
  {
    COURANT = ADS.readADC(2);
  }

  if (ADS.readADC(3) < maxVal)
  {
    ENTREE = ADS.readADC(3);
  }

  // Transfert vers les vrai valeurs
  VoltageDemanderLum = ((analogRead(Ajust) * (MaximumAjust - MinimumAjust)) / 1023) + MinimumAjust;
  VoltageDemanderBattVide = (VALEUR_BATTERIE)*diviseur;
  VoltageSepic = ((SORTIE_SEPIC * 6.144) / 32768) * diviseur;
  CourantBatterie = (COURANT * 6.144) / 32768;
  VoltageEntree = ((ENTREE * 6.144) / 32768) * diviseur;

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
      BatterieFini = true;
    }
    // Mode Lumière
    if (ModeSepic > 850)
    {
      ModeBatterie = false;
      ModeLumiere = true;
    }
  }

  // Remise à zéro
  if (VoltageEntree < 5.0)
  {
    PWMSEPIC = 0;
    PWMSEPICINT = 0;
    IPIDLum = 0;
    IPIDBatt = 0;
    ValeurAjustementSepicBatt = 0;
    ValeurAjustementSepicBattOn = 0;
    ValeurAjustementSepicLum = 0;
  }

  // Ajustement des convertisseur
  //  PID de l'ajustement
  if (ModeLumiere)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 1);

    ErreurLum = VoltageDemanderLum - VoltageSepic;
    PWMSEPIC = (((VoltageDemanderLum + TensionDiode) / (VoltageEntree + VoltageDemanderLum + TensionDiode)) * 255);

    IPIDLum += ((CurrentMicros - LastValTempsLum) / 1000000) * ErreurLum;
    LastValTempsLum = CurrentMicros;

    ValeurAjustementSepicLum = (ErreurLum * CoeAjustPSepic) + (IPIDLum * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicLum;
    PWMSEPICINT = PWMSEPIC;

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > 179)
    {
      PWMSEPICINT = 179;
    }
    analogWrite(SEPIC, PWMSEPICINT);
  }

  if (ModeBatterie)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 2);

    ErreurBatt = VoltageDemanderBatt - VoltageSepic;
    PWMSEPIC = (((VoltageDemanderBatt + TensionDiode) / (VoltageEntree + VoltageDemanderBatt + TensionDiode)) * 255) * RendementSepic;

    IPIDBatt += ((CurrentMicros - LastValTempsBatt) / 1000000) * ErreurBatt;
    LastValTempsBatt = CurrentMicros;

    ValeurAjustementSepicBatt = (ErreurBatt * CoeAjustPSepic) + (IPIDBatt * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicBatt;
    PWMSEPICINT = PWMSEPIC;

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > 179)
    {
      PWMSEPICINT = 179;
    }
    analogWrite(SEPIC, PWMSEPICINT);
  }

  if (!ModeBatterie && !ModeLumiere)
  {
    PWMSEPICINT = 0;
    analogWrite(SEPIC, PWMSEPICINT);
  }

  // POSSIBILITÉ DE PROBLÈME A VÉRIFIER
  //  Code pour la Lumiere Activation
  if (ModeLumiere)
  {

    digitalWrite(Batterie, LOW);
    digitalWrite(Lumiere, HIGH);
  }

  // Code pour la Batterie Activation
  if (ModeBatterie)
  {
    if (!BatterieFini)
    {
      if (BatterieDebut == true)
      {
        // Petit code pour debogguer YOUPIIII
        printage(Bluetooth, SerialOrdi, 9);

        ErreurBattOn = (CourantDansLaBatterie - CourantBatterie);
        IPIDBattOn += (CurrentMicros - LastValTempsBattOn) * ErreurBattOn;
        LastValTempsBattOn = CurrentMicros;

        ValeurAjustementSepicBattOn = (ErreurBattOn * CoeAjustPBatterie) + ((IPIDBattOn)*CoeAjustIBatterie);

        VoltageDemanderBatt = VoltageDemanderBattVide + ValeurAjustementSepicBattOn;

        if (VoltageDemanderBattVide > MaxBattterie)
        {
          BatterieDebut = false;
          BatterieFin = true;
        }
      }
      if (BatterieFin == true)
      {
        // Petit code pour debogguer YOUPIIII
        printage(Bluetooth, SerialOrdi, 11);
        VoltageDemanderBatt = MaxBattterie;

        if (CourantBatterie < 0.25)
        {
          BatterieFini = true;
          BatterieDebut = true;
          BatterieFin = false;
        }
      }
    }
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
