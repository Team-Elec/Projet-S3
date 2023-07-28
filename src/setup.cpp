#include <Arduino.h>
#include "setup.h"
#include "variables.h"

void appelVariables()
{
    Serial.begin(BAUD);
    Serial1.begin(BAUD);

    pinMode(AjustVitesse, INPUT);

    pinMode(Therm1, INPUT);
    pinMode(Therm2, INPUT);

    pinMode(ValSortieMoteur1, INPUT);
    pinMode(ValSortieMoteur2, INPUT);

    pinMode(BlueTooth, OUTPUT);
    pinMode(Inverse1, OUTPUT);
    pinMode(Inverse2, OUTPUT);

    pinMode(PWMInverse1, OUTPUT);
    pinMode(PWMInverse2, OUTPUT);

    pinMode(ValCourant, INPUT);
}

void VitessePWM()
{
    // PIN D4 D13 à 7812Hz
    TCCR0B = TCCR0B & B11111000 || B00000010;
    // PIN D6 D7 D8 à 3921 Hz
    TCCR4B = TCCR4B & B11111000 || B00000010;
}