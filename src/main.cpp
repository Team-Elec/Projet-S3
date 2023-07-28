#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComBluetooth.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

// Valeurs importante a changer si jamais
// Vitesse envoie et vitesse de sécurite anti flicker
// En milisecondes
#define SendTime 500

// Lieu des sortie Serial
bool Bluetooth = false;
bool SerialOrdi = false;
bool SerialOrdiVal = true;
bool BluetoothVal = false;
bool ThermistanceValeurs = false;

// Ajustement PID
float CoeAjustPSepic = 1;
float CoeAjustISepic = 2.2;
float CoeAjustPBatterie = 0.5;
float CoeAjustIBatterie = 1.1;

// Valeur des ajustement du potentiomètre
float MaximumAjust = 14;
float MinimumAjust = 10;

// Courant dans la batterie (en Volt)
float CourantDansLaBatterie = 2.5;

// Tension Max batterie
float MaxBatterie = 14.7;

// Maximum de variation avant la fermeture (Valeur + chiffre et Valeur - chiffre)
float MaxVariation = 2;
float RendementSepic = 0.75;

// Valeur de la diode
float TensionDiode = 0.3;

// Valeur de MaxPWM
int MaxPWM = 200;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;
unsigned long CurrentMicros = 0;

float VoltageDemanderLum, VoltageDemanderBattVide, VoltageDemanderBatt = 0;
float IPIDBatt, IPIDBattOn, IPIDLum = 0;
float ValeurAjustementSepicBatt, ValeurAjustementSepicBattOn, ValeurAjustementSepicLum = 0;
int ModeSepic = 0;
float VoltageSepic, CourantBatterie, VoltageEntree = 0;
float PWMSEPIC = 0;
int16_t PWMSEPICINT = 0;
float LastValTempsLum, LastValTempsBatt, LastValTempsBattOn = 0;
float ErreurLum, ErreurBatt, ErreurBattOn, ErreurOFF = 0;
int16_t SORTIE_SEPIC, VALEUR_BATTERIE, COURANT, ENTREE = 0;
float ValThermistance1, ValThermistance2, ValThermistance3, ValThermistance4, ValThermistance5 = 0;
float Thermistance1, Thermistance2, Thermistance3, Thermistance4, Thermistance5 = 0;

// Debounce
unsigned lastsendtime, lastLumiere, lastBatterie = 0;

// Modes et protections
bool ModeBatterie = false;
bool ModeOff = true;
bool ModeLumiere = false;
bool ProtectMode = true;
bool BatterieDebut = false;
bool BatterieFin = false;
bool BatterieFini = false;

float ValResDiv = 143000.0;

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
  if (Bluetooth || BluetoothVal)
  {
    digitalWrite(BlueTooth, HIGH);
  }
  else
  {
    digitalWrite(BlueTooth, LOW);
  }

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
  CourantBatterie = (((COURANT * 6.144) / 32768) / 5);
  VoltageDemanderBattVide = (((VALEUR_BATTERIE * 6.144) / 32768) * diviseur);
  VoltageSepic = (((SORTIE_SEPIC * 6.144) / 32768) * diviseur);
  VoltageEntree = (((ENTREE * 6.144) / 32768) * diviseur);

  // Lecture pour le mode du SEPIC (Potentiomètre)
  ModeSepic = analogRead(Mode);

  // Protection pour pas qu'il partent dans le mauvais mode
  // Doit avoir ProtectMode de true pour être capable d'entrer
  // Mode : <50 = batterie, <950 = lumière, <950 et >50 Mode off
  if (ModeSepic > 200 && ModeSepic < 850)
  {
    ProtectMode = true;
  }

  if (ProtectMode)
  {
    // Mode batterie
    if (ModeSepic < 50)
    {
      ModeBatterie = true;
      ModeLumiere = false;
      ModeOff = false;
      BatterieDebut = true;
    }
    // Mode off
    if (ModeSepic > 50 && ModeSepic < 950)
    {
      ModeBatterie = false;
      ModeLumiere = false;
      ModeOff = true;
      BatterieDebut = false;
      digitalWrite(Lumiere, LOW);
      digitalWrite(Batterie, LOW);
    }
    // Mode Lumière
    if (ModeSepic > 950)
    {
      ModeBatterie = false;
      ModeLumiere = true;
      ModeOff = false;
      BatterieDebut = false;
    }
  }

  if (ModeOff)
  {
    ModeBatterie = false;
    ModeLumiere = false;
    PWMSEPICINT = 0;
    PWMSEPIC = 0;
    IPIDLum = 0;
    IPIDBatt = 0;
    IPIDBattOn = 0;
    ValeurAjustementSepicBatt = 0;
    ValeurAjustementSepicBattOn = 0;
    ValeurAjustementSepicLum = 0;
    VoltageDemanderLum = 0;
    VoltageDemanderBatt = 0;
    digitalWrite(Batterie, LOW);
    digitalWrite(Lumiere, LOW);
    analogWrite(SEPIC, 0);
  }

  // Ajustement des convertisseur
  //  PID de l'ajustement
  if (ModeLumiere)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 1);

    ErreurLum = VoltageDemanderLum - VoltageSepic;
    PWMSEPIC = (((VoltageDemanderLum + TensionDiode) / (VoltageEntree + VoltageDemanderLum + TensionDiode)) * 255) * RendementSepic;

    IPIDLum += ((CurrentMicros - LastValTempsLum) / 1000000) * ErreurLum;
    LastValTempsLum = CurrentMicros;

    ValeurAjustementSepicLum = (ErreurLum * CoeAjustPSepic) + (IPIDLum * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicLum;
    PWMSEPICINT = PWMSEPIC;

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > MaxPWM)
    {
      PWMSEPICINT = MaxPWM;
    }
    digitalWrite(Batterie, LOW);
    digitalWrite(Lumiere, HIGH);
    analogWrite(SEPIC, PWMSEPICINT);
  }

  // Code pour la Batterie Activation
  if (ModeBatterie)
  {
    if (BatterieFini == false)
    {
      if (BatterieDebut == true)
      {
        digitalWrite(Lumiere, LOW);
        digitalWrite(Batterie, HIGH);

        // Petit code pour debogguer YOUPIIII
        printage(Bluetooth, SerialOrdi, 9);

        ErreurBattOn = (CourantDansLaBatterie - CourantBatterie);
        IPIDBattOn += ((CurrentMicros - LastValTempsBattOn) / 1000000) * ErreurBattOn;
        LastValTempsBattOn = CurrentMicros;

        ValeurAjustementSepicBattOn = (ErreurBattOn * CoeAjustPBatterie) + ((IPIDBattOn)*CoeAjustIBatterie);

        VoltageDemanderBatt = VoltageDemanderBattVide + ValeurAjustementSepicBattOn;

        if (VoltageDemanderBattVide > MaxBatterie)
        {
          BatterieDebut = false;
          BatterieFin = true;
        }
      }
      if (BatterieFin == true)
      {
        // Petit code pour debogguer YOUPIIII
        printage(Bluetooth, SerialOrdi, 11);
        VoltageDemanderBatt = MaxBatterie;

        // arret à 50 milis ampère
        if (CourantBatterie < 0.25)
        {
          BatterieFini = true;
          BatterieDebut = true;
          BatterieFin = false;
          digitalWrite(Batterie, LOW);
        }
      }
    }
  }

  if (ModeBatterie)
  {
    // Petit code pour debogguer YOUPIIII
    printage(Bluetooth, SerialOrdi, 2);

    ErreurBatt = VoltageDemanderBatt - VoltageSepic;
    PWMSEPIC = (((VoltageDemanderBatt + TensionDiode) / (VoltageEntree + VoltageDemanderBatt + TensionDiode)) * 255);

    IPIDBatt += ((CurrentMicros - LastValTempsBatt) / 1000000) * ErreurBatt;
    LastValTempsBatt = CurrentMicros;

    ValeurAjustementSepicBatt = (ErreurBatt * CoeAjustPSepic) + (IPIDBatt * CoeAjustISepic);
    PWMSEPIC += ValeurAjustementSepicBatt;
    PWMSEPICINT = PWMSEPIC;

    if (PWMSEPICINT < 0)
    {
      PWMSEPICINT = 0;
    }
    if (PWMSEPICINT > MaxPWM)
    {
      PWMSEPICINT = MaxPWM;
    }
    analogWrite(SEPIC, PWMSEPICINT);
  }

  // Envoi du signal
  if (CurrentMillis - lastsendtime > SendTime)
  {
    if (ThermistanceValeurs)
    {
      ValThermistance1 = ((analogRead(Therm1) * 5) / 1023);
      ValThermistance2 = ((analogRead(Therm2) * 5) / 1023);
      ValThermistance3 = ((analogRead(Therm3) * 5) / 1023);
      ValThermistance4 = ((analogRead(Therm4) * 5) / 1023);
      ValThermistance5 = ((analogRead(Therm5) * 5) / 1023);

      Thermistance1 = (ValThermistance1 * ValThermistance1 * ValThermistance1 * 4.9489) + (ValThermistance1 * ValThermistance1 * 39.419) + (ValThermistance1 * 118.83) - 86.986;
      Thermistance2 = (ValThermistance2 * ValThermistance2 * ValThermistance2 * 4.9489) + (ValThermistance2 * ValThermistance2 * 39.419) + (ValThermistance2 * 118.83) - 86.986;
      Thermistance3 = (ValThermistance3 * ValThermistance3 * ValThermistance3 * 4.9489) + (ValThermistance3 * ValThermistance3 * 39.419) + (ValThermistance3 * 118.83) - 86.986;
      Thermistance4 = (ValThermistance4 * ValThermistance4 * ValThermistance4 * 4.9489) + (ValThermistance4 * ValThermistance4 * 39.419) + (ValThermistance4 * 118.83) - 86.986;
      Thermistance5 = (ValThermistance5 * ValThermistance5 * ValThermistance5 * 4.9489) + (ValThermistance5 * ValThermistance5 * 39.419) + (ValThermistance5 * 118.83) - 86.986;
    }
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
      if (ModeOff)
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

      Serial.print(" \tTension demander Batterie : ");
      Serial.print(VoltageDemanderBatt);

      Serial.print(" \tCourrant Batterie : ");
      Serial.print(CourantBatterie);

      Serial.print(" \tTension Batterie : ");
      Serial.print(VoltageDemanderBattVide);
      if (ThermistanceValeurs)
      {
        Serial.print("\n5V / SEPIC / Zener / Batterie / Protect ");
        Serial.print(Thermistance1);
        Serial.print("/");
        Serial.print(Thermistance2);
        Serial.print("/");
        Serial.print(Thermistance3);
        Serial.print("/");
        Serial.print(Thermistance4);
        Serial.print("/");
        Serial.print(Thermistance5);
      }

      Serial.print("\n");
    }
    if (BluetoothVal)
    {
      Serial1.print("Mode : ");
      if (ModeLumiere)
      {
        Serial1.print("Lumière");
      }
      if (ModeBatterie)
      {
        Serial1.print("Batterie");
      }
      if (ModeOff)
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

      Serial1.print(" \tTension demander Batterie : ");
      Serial1.print(VoltageDemanderBatt);

      Serial1.print(" \tCourrant Batterie : ");
      Serial1.print(CourantBatterie);

      Serial1.print(" \tTension Batterie : ");
      Serial1.print(VoltageDemanderBattVide);
      if (ThermistanceValeurs)
      {
        Serial1.print("\n5V / SEPIC / Zener / Batterie / Protect ");
        Serial1.print(Thermistance1);
        Serial1.print("/");
        Serial1.print(Thermistance2);
        Serial1.print("/");
        Serial1.print(Thermistance3);
        Serial1.print("/");
        Serial1.print(Thermistance4);
        Serial1.print("/");
        Serial1.print(Thermistance5);
      }

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
