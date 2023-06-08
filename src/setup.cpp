#include <Arduino.h>
#include "setup.h"
#include "variables.h"
#include "ADS1X15.h"

void appelVariables()
{
    Serial.begin(BAUD);
    Serial1.begin(BAUD);

    pinMode(VOLTAGE_PIN_OUT_1, OUTPUT);
    pinMode(VOLTAGE_PIN_OUT_2, OUTPUT);
    pinMode(BIT_TEST, INPUT);

}

void VitessePWM(){
    //PIN D11 D12
    TCCR1B = TCCR1B & B11111000 | B00000001;
    //PIN D9 D10
    TCCR2B = TCCR2B & B11111000 | B00000001;
}