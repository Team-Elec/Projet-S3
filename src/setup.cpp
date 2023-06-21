#include <Arduino.h>
#include "setup.h"
#include "variables.h"

void appelVariables()
{
    Serial.begin(BAUD);
    Serial1.begin(BAUD);

    pinMode(Therm1, INPUT);
    pinMode(Therm2, INPUT);
    pinMode(Therm3, INPUT);
    pinMode(Therm4, INPUT);
    pinMode(Therm5, INPUT);

    pinMode(Mode, INPUT);
    pinMode(Ajust, INPUT);

    pinMode(Batterie, OUTPUT);
    pinMode(Lumiere, OUTPUT);
    pinMode(BlueTooth, OUTPUT);

    pinMode(SEPIC, OUTPUT);
    pinMode(BUCK, OUTPUT);
}

void VitessePWM()
{
    // PIN D11 D12
    TCCR1B = TCCR1B & B11111000 | B00000001;
    // PIN D9 D10
    TCCR2B = TCCR2B & B11111000 | B00000001;
}