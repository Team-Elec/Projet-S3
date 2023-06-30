#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComChinois.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

// Valeurs importante a changer si jamais
// Vitesse envoie et vitesse de sécurite anti flicker
#define SendTime 1000
#define LumiereSecurite 2000
#define BatterieSecurite 2000
// Lieu des sortie Serial
bool Bluetooth = false;
bool SerialOrdi = false;
// Ajustement PID
float CoeAjustPSepic = 1.9;
float CoeAjustISepic = 1;
float CoeAjustPBuck = 6;
float CoeAjustIBuck = 4;
float CoeAjustPBatterie = 6;
float CoeAjustIBatterie = 4;
// Valeur des ajustement du potentiomètre
int MaximumAjust = 14;
int MinimumAjust = 10;
// Voltage de BUCK
float VoltageDemanderBuck = 8.3;
int VoltageDemanderOFF = 10;
// Courant dans la batterie (en Volt)
float CourantDansLaBatterie = 2.5;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;

float VoltageDemanderLum, VoltageDemanderBattVide, VoltageDemanderBatt = 0;
float ValMoyLum, ValMoyBatt, MoyennePIDBatt, ValMoyBattOn, MoyennePIDBattOn, ValMoyBuck, MoyennePIDBuck, ValMoyOFF, MoyennePIDOFF, MoyennePIDLum = 0;
int NbLum, NbBatt, NbBattOn, NbBuck, NbOFF = 0;
int ValeurAjustementSepicBatt, ValeurAjustementSepicBattOn, ValeurAjustementSepicLum, ValeurAjustementBuck, ValeurAjustementSepicOFF, ModeSepic = 0;
float VoltageSepic, VoltageBuck, VoltageOFF, CourantBatterie = 0;
int PWMSEPIC, PWMBUCK = 0;
int thermo1, thermo2, thermo3, thermo4, thermo5 = 0;

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
  // Pour le temps
  CurrentMillis = millis();

  // Lecture du ADS1115
  int16_t SORTIE_SEPIC = ADS.readADC(0);
  int16_t VALEUR_BATTERIE = ADS.readADC(1);
  int16_t COURANT = ADS.readADC(2);
  int16_t VAL_BUCK = ADS.readADC(3);

  // Lecture pour le mode du SEPIC
  ModeSepic = analogRead(Mode);

  // Protection pour pas qu'il partent dans le mauvais mode
  if (ModeSepic > 200 && ModeSepic < 850)
  {
    ProtectMode = true;
  }
  // Doit avoir ProtectMode de true pour être capable d'entrer
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

  if (SORTIE_SEPIC < 0)
  {
    SORTIE_SEPIC = 0;
  }

  // Lecture de plusieurs valeurs
  VoltageDemanderLum = ((analogRead(Ajust) * (MaximumAjust - MinimumAjust)) / 1023) + MinimumAjust;
  VoltageDemanderBattVide = VALEUR_BATTERIE;
  VoltageSepic = (SORTIE_SEPIC * 6.144) / 32768;
  CourantBatterie = (COURANT * 6.144) / 32768;
  VoltageBuck = (VAL_BUCK * 6.144) / 32768;

  // PID de l'ajustement
  if (ModeLumiere)
  {
    NbLum = +1;
    ValMoyLum = ValMoyLum + VoltageSepic;
    MoyennePIDLum = (ValMoyLum) / NbLum;

    ValeurAjustementSepicLum = ((VoltageDemanderLum - VoltageSepic) * CoeAjustPSepic) + ((VoltageDemanderLum - MoyennePIDLum) * CoeAjustISepic);

    PWMSEPIC = PWMSEPIC + int(ValeurAjustementSepicLum);
    PWMSEPIC = int(PWMSEPIC);
    if (PWMSEPIC < 0)
    {
      PWMSEPIC = 0;
    }
    if (PWMSEPIC > 255)
    {
      PWMSEPIC = 255;
    }
    analogWrite(SEPIC, PWMSEPIC);
  }

  // Code pour le Sepic en Batterie
  if (ModeBatterie)
  {
    NbBatt = +1;
    ValMoyBatt = ValMoyBatt + VoltageSepic;
    MoyennePIDBatt = (ValMoyBatt) / NbBatt;

    ValeurAjustementSepicBatt = ((VoltageDemanderBatt - VoltageSepic) * CoeAjustPSepic) + ((VoltageDemanderBatt - MoyennePIDBatt) * CoeAjustISepic);

    PWMSEPIC = PWMSEPIC + int(ValeurAjustementSepicBatt);

    if (PWMSEPIC < 0)
    {
      PWMSEPIC = 0;
    }
    if (PWMSEPIC > 255)
    {
      PWMSEPIC = 255;
    }
    analogWrite(SEPIC, PWMSEPIC);
  }

  // OFF
  if (ModeLumiere == false && ModeBatterie == false)
  {
    BatterieOuverte = false;
    digitalWrite(Lumiere, LOW);
    digitalWrite(Batterie, LOW);
    digitalWrite(OFF, HIGH);
    NbOFF = +1;
    ValMoyOFF = ValMoyOFF + VoltageSepic;
    MoyennePIDLum = (ValMoyLum) / NbLum;

    ValeurAjustementSepicOFF = ((VoltageDemanderOFF - VoltageSepic) * CoeAjustPSepic) + ((VoltageDemanderOFF - MoyennePIDLum) * CoeAjustISepic);

    PWMSEPIC = PWMSEPIC + int(ValeurAjustementSepicOFF);
    PWMSEPIC = int(PWMSEPIC);
    if (PWMSEPIC < 0)
    {
      PWMSEPIC = 0;
    }
    if (PWMSEPIC > 255)
    {
      PWMSEPIC = 255;
    }
    analogWrite(SEPIC, PWMSEPIC);
  }

  // Code de Buck
  NbBuck = +1;
  ValMoyBuck = ValMoyBuck + VoltageBuck;
  MoyennePIDBuck = (ValMoyBuck) / NbBuck;

  ValeurAjustementBuck = ((VoltageDemanderBuck - VoltageBuck) * CoeAjustPBuck) + ((VoltageDemanderBuck - MoyennePIDBuck) * CoeAjustIBuck);

  PWMBUCK = PWMBUCK + int(ValeurAjustementBuck);

  if (PWMBUCK < 0)
  {
    PWMBUCK = 0;
  }
  if (PWMBUCK > 255)
  {
    PWMBUCK = 255;
  }
  analogWrite(BUCK, PWMBUCK);

  // Code pour la Lumiere Activation
  if (ModeLumiere)
  {
    if (((VoltageDemanderLum - 0.25) < VoltageSepic) && ((VoltageDemanderLum + 0.25) > VoltageSepic))
    {
      // Ouverture du MOSFET après le bon nombre de temps
      if (CurrentMillis - lastLumiere > LumiereSecurite)
      {
        digitalWrite(OFF, LOW);
        digitalWrite(Batterie, LOW);
        digitalWrite(Lumiere, HIGH);
      }
    }

    if (((VoltageDemanderLum - 0.25) > VoltageSepic) && ((VoltageDemanderLum + 0.25) < VoltageSepic))
    {
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
      VoltageDemanderBatt = VoltageDemanderBattVide;
    }
    if (BatterieOuverte == true)
    {
      NbBattOn = +1;
      ValMoyBattOn = ValMoyBattOn + CourantBatterie;
      MoyennePIDBattOn = (ValMoyBattOn) / NbBattOn;

      ValeurAjustementSepicBattOn = ((CourantDansLaBatterie - CourantBatterie) * CoeAjustPBatterie) + ((CourantDansLaBatterie - MoyennePIDBattOn) * CoeAjustIBatterie);

      VoltageDemanderBatt = VoltageDemanderBattVide + ValeurAjustementSepicBattOn;
    }

    if (CourantBatterie < 4)
    {
      // Ouverture du MOSFET après le bon nombre de temps
      if (CurrentMillis - lastBatterie > BatterieSecurite)
      {
        BatterieOuverte = true;
        digitalWrite(OFF, LOW);
        digitalWrite(Lumiere, LOW);
        digitalWrite(Batterie, HIGH);
      }
    }

    if (CourantBatterie >= 4 || CourantBatterie <= 0.2)
    {
      BatterieOuverte = false;
      // Securité pour la lumière pas qu'elle ouvre ferme rapidement
      lastBatterie = CurrentMillis;
      // Fermeture du MOSFET
      digitalWrite(Batterie, LOW);
    }
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
}