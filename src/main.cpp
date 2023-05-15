#include <Arduino.h>
#include "ADS1X15.h"
#include "setup.h"
#include "ComChinois.h"

// Variables pour le ADS
ADS1115 ADS(0x48);

// Initiation de valeurs à 0
unsigned long CurrentMillis = 0;
int VoltageDemander = 0;
int Voltage = 0;
int valPWM = 0;

// Debounce
unsigned lastsendtime = 0;
#define SendTime 1000

void setup()
{
  appelVariables();

  ADS.begin();
  ADS.setGain(0);    //Pour avoir +- 6.144 V 
  ADS.setDataRate(7);   //Pour l'avoir vite en tas
  ADS.setMode(0);     //Mode continue
  ADS.readADC(0);     
}

void loop()
{
  CurrentMillis = millis();

//Lecture de la pin A0 du ADS
  int16_t val_0 = ADS.readADC(0);
Serial.print("Valeur = ");
Serial.print(val_0);
Serial.print("\n");

  // Lecture du petit chinois pour la sortie du moteur
  VoltageDemander = lectureChinois();

  if (VoltageDemander <= 35)
  {
    Voltage = VoltageDemander;
  }
}