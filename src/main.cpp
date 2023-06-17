#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComChinois.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

// Valeurs importante a changer si jamais
#define SendTime 1000
#define LumiereSecurite 2000
#define BatterieSecurite 2000
bool Bluetooth = false;
bool SerialOrdi = false;
int MaximumAjust = 14;
int MinimumAjust = 10;
int VoltageDemanderBuck = 8;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;

float VoltageDemanderLum = 0;
float VoltageDemanderBatt = 0;
int ModeSepic = 0;

float VoltageSepic = 0;
float VoltageBuck = 0;
float CourantBatterie = 0;

int PWMSEPIC = 10;
int PWMBUCK = 0;

int thermo1 = 0;
int thermo2 = 0;
int thermo3 = 0;
int thermo4 = 0;
int thermo5 = 0;

// Debounce
unsigned lastsendtime = 0;
unsigned lastLumiere = 0;
unsigned lastBatterie = 0;

// Modes et protections
bool ModeBatterie = false;
bool ModeLumiere = false;
bool ProtectMode = false;
bool BatterieInitiale = false; 

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
  if (ModeSepic > 400 && ModeSepic < 600)
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
    if (ModeSepic > 400 && ModeSepic < 600)
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

  VoltageDemanderLum = ((analogRead(Ajust) * (MaximumAjust - MinimumAjust)) / 1023) + MinimumAjust;
  VoltageDemanderBatt = VALEUR_BATTERIE;
  VoltageSepic = (SORTIE_SEPIC * 6.144) / 32768;
  CourantBatterie = (COURANT * 6.144) / 32768;
  VoltageBuck = (VAL_BUCK * 6.144) / 32768;

  // Mettre ajustement du SEPIC ici
  if(VoltageDemanderLum < VoltageSepic){
    PWMSEPIC = PWMSEPIC + 1;
  }
  if(VoltageDemanderLum > VoltageSepic){
    PWMSEPIC = PWMSEPIC - 1;
  }

   if(VoltageDemanderBuck < VoltageBuck){
    PWMBUCK = PWMBUCK + 1;
  }
  if(VoltageDemanderBuck > VoltageBuck){
    PWMBUCK = PWMBUCK - 1;
  }
  // IF VALEUR SEPIC VALEUR DEMANDER +- PWM POUR SE RENDRE
  //Faire pour Buck et pour Sepic
  // Mettre ajustement du SEPIC ici

  // Code pour batterie
  if (ModeLumiere)
  {
    if (((VoltageDemanderLum - 0.25) < VoltageSepic) && ((VoltageDemanderLum + 0.25) > VoltageSepic))
    {
      // Ouverture du MOSFET après le bon nombre de temps
      if (CurrentMillis - lastLumiere > LumiereSecurite)
      {
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

  if (ModeBatterie)
  {
    if (((VoltageDemanderBatt - 0.25) < VoltageSepic) && ((VoltageDemanderBatt + 0.25) > VoltageSepic))
    {
      // Ouverture du MOSFET après le bon nombre de temps
      if (CurrentMillis - lastLumiere > LumiereSecurite)
      {
        digitalWrite(Batterie, HIGH);
      }
    }

    if (((VoltageDemanderBatt - 0.25) > VoltageSepic) && ((VoltageDemanderBatt + 0.25) < VoltageSepic))
    {
      // Securité pour la lumière pas qu'elle ouvre ferme rapidement
      lastLumiere = CurrentMillis;
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