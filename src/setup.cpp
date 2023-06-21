#include <Arduino.h>
#include "setup.h"
#include "variables.h"

void appelVariables()
{
    Serial.begin(BAUD);
    Serial1.begin(BAUD);

    pinMode(AjustVitesse, INPUT);

    pinMode(Therm1, INPUT);
    pinMode(Therm3, INPUT);

    pinMode(ValSortieMoteur1, INPUT);
    pinMode(ValSortieMoteur2, INPUT);

    pinMode(BlueTooth, OUTPUT);
    pinMode(Inverse1, OUTPUT);
    pinMode(Inverse2, OUTPUT);

    pinMode(PWMInverse1, OUTPUT);
    pinMode(PWMInverse2, OUTPUT);
}

void VitessePWM()
{
    // PIN D11 D12
    TCCR1B = TCCR1B & B11111000 | B00000001;
    // PIN D9 D10
    TCCR2B = TCCR2B & B11111000 | B00000001;
}