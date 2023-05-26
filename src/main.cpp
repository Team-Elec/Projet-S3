#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComChinois.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

float VoltageTest = 0;
float CourantTest = 0;

// Valeurs importante a changer si jamais
#define SendTime 1000
int VoltageMax = 30;
float ResistanceCharge = 15.4;
bool Bluetooth = false;
bool SerialOrdi = false;

// Initiation de valeurs à 0
// Pas vraiment important tant que ça
unsigned long CurrentMillis = 0;
int VoltageDemander = 0;
float Voltage = 0;
float Courant = 0;
int valPWM = 0;
// Debounce
unsigned lastsendtime = 0;


void setup()
{
  appelVariables();

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
  int16_t val_0 = ADS.readADC(0);
  int16_t val_1 = ADS.readADC(1);

  // Calcul du voltage et du courant

  /*
  METTRE LE CALCUL DE MAXIMUM POUR QUE LE 5V SORTI DISENT LA VRAI SORTIE
  METTRE LE CALCUL DE MAXIMUM POUR QUE LE 5V SORTI DISENT LA VRAI SORTIE
  METTRE LE CALCUL DE MAXIMUM POUR QUE LE 5V SORTI DISENT LA VRAI SORTIE
  */

  VoltageTest = (analogRead(BIT_TEST) * 5) / 1023;
  CourantTest = (VoltageTest) / ResistanceCharge;

  Voltage = ((val_0 - val_1) * 6.144) / 32768;
  Courant = Voltage / ResistanceCharge;

  // Envoi du signal
  if (CurrentMillis - lastsendtime > SendTime)
  {
    if (SerialOrdi)
    {
      Serial.print("Courant : ");
      Serial.print(Courant);
      Serial.print("\t\tVoltage : ");
      Serial.println(Voltage);
    }
    if (Bluetooth)
    {
      Serial1.print("Courant : ");
      Serial1.print(Courant);
      Serial1.print("\t\tVoltage : ");
      Serial1.println(Voltage);
    }

    lastsendtime = CurrentMillis;
  }

  // Lecture du petit chinois pour la sortie du moteur
  // VoltageDemander = lectureChinois();

  // if (VoltageDemander <= 35)
  //{
  //   Voltage = VoltageDemander;
  // }
}