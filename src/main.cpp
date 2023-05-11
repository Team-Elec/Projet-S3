#include <Arduino.h>
#include "setup.h"

//Initiation de valeurs à 0
unsigned long CurrentMillis = 0;
int ValeurVolt = 0;
int VoltageDemander = 99;

//Utilisation debounce
bool lastTime5VBool = false;
unsigned long lastTime5V = 0;

#define TempsDebounce 2000


void setup()
{
  appelVariables();
}

void loop()
{
  CurrentMillis = millis();

  ValeurVolt = analogRead(VOLTAGE_PIN);

  ValeurVolt = float(((ValeurVolt * 25) / 1024));

  
}