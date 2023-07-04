#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComChinois.h"

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

// Ajustement PID
float CoeAjustPSepic = 0.015;
float CoeAjustISepic = 0.025;
float CoeAjustPBuck = 0.04;
float CoeAjustIBuck = 0.099;
float CoeAjustPBatterie = 6;
float CoeAjustIBatterie = 4;

// Valeur des ajustement du potentiomètre
int MaximumAjust = 14;
int MinimumAjust = 10;

// Voltage de BUCK
float VoltageDemanderBuck = 9.3;

// Voltage SEPIC
int VoltageDemanderOFF = 10;

// Courant dans la batterie (en Volt)
float CourantDansLaBatterie = 2.5;

// Maximum de variation avant la fermeture (Valeur + chiffre et Valeur - chiffre)
float MaxVariation = 0.5;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;
unsigned long CurrentMicros = 0;

float VoltageDemanderLum, VoltageDemanderBattVide, VoltageDemanderBatt = 0;
float IPIDBatt, IPIDBattOn, IPIDBuck, IPIDOFF, IPIDLum = 0;
float ValeurAjustementSepicBatt, ValeurAjustementSepicBattOn, ValeurAjustementSepicLum, ValeurAjustementBuck, ValeurAjustementSepicOFF = 0;
float ModeSepic = 0;
float VoltageSepic, VoltageBuck, VoltageOFF, CourantBatterie = 0;
float PWMSEPIC, PWMBUCK = 0;
int PWMSEPICINT, PWMBUCKINT = 0;
int thermo1, thermo2, thermo3, thermo4, thermo5 = 0;
float LastValTempsLum, LastValTempsBuck, LastValTempsBatt, LastValTempsBattOn, LastValTempsOFF = 0;
float ErreurLum, ErreurBuck, ErreurBatt, ErreurBattOn, ErreurOFF = 0;

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
  int16_t VAL_BUCK = ADS.readADC(3);

  // Transfert vers les vrai valeurs
  VoltageDemanderLum = ((analogRead(Ajust) * (MaximumAjust - MinimumAjust)) / 1023) + MinimumAjust;
  VoltageDemanderBattVide = VALEUR_BATTERIE;
  VoltageSepic = (SORTIE_SEPIC * 6.144) / 32768;
  CourantBatterie = (COURANT * 6.144) / 32768;
  VoltageBuck = (VAL_BUCK * 6.144) / 32768;

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

    IPIDLum += (CurrentMicros - LastValTempsLum) * ErreurLum;
    LastValTempsLum = CurrentMicros;

    ValeurAjustementSepicLum = (ErreurLum * CoeAjustPSepic) + (IPIDLum * CoeAjustISepic);
    PWMSEPIC = PWMSEPIC + ValeurAjustementSepicLum;
    PWMSEPICINT = int(PWMSEPIC);

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

    IPIDBatt += (CurrentMicros - LastValTempsBatt) * ErreurBatt;
    LastValTempsBatt = CurrentMicros;

    ValeurAjustementSepicBatt = (ErreurBatt * CoeAjustPSepic) + (IPIDBatt * CoeAjustISepic);
    PWMSEPIC = PWMSEPIC + int(ValeurAjustementSepicBatt);
    PWMSEPICINT = int(PWMSEPIC);

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
    IPIDOFF += (CurrentMicros - LastValTempsOFF) * ErreurOFF;
    LastValTempsOFF = CurrentMicros;

    ValeurAjustementSepicOFF = (ErreurOFF * CoeAjustPSepic) + (IPIDOFF * CoeAjustISepic);
    PWMSEPIC = PWMSEPIC + ValeurAjustementSepicOFF;
    PWMSEPICINT = int(PWMSEPIC);

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

  // Code de Buck

  // Petit code pour debogguer YOUPIIII
  printage(Bluetooth, SerialOrdi, 5);

  ErreurBuck = VoltageDemanderBuck - VoltageSepic;

  IPIDBuck += (CurrentMicros - LastValTempsBuck) * ErreurBuck;
  LastValTempsBuck = CurrentMicros;

  ValeurAjustementBuck = (ErreurBuck * CoeAjustPBuck) + (IPIDBuck * CoeAjustIBuck);

  PWMBUCK = PWMBUCK + ValeurAjustementBuck;
  PWMBUCKINT = int(PWMBUCK);
  if (PWMBUCKINT < 0)
  {
    PWMBUCKINT = 0;
  }
  if (PWMBUCKINT > 255)
  {
    PWMBUCKINT = 255;
  }

  analogWrite(BUCK, PWMBUCKINT);

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
    if (SerialOrdi)
    {
      /*Serial.print("Courant : ");
      Serial.print(Courant);
      Serial.print("\t\tVoltage : ");
      Serial.println(Voltage);*/
    }
    if (Bluetooth)
    {
      /*Serial1.print("Courant : ");
      Serial1.print(Courant);
      Serial1.print("\t\tVoltage : ");
      Serial1.println(Voltage);*/
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
  if (VAL_BUCK < 0)
  {
    VAL_BUCK = 0;
  }
}
