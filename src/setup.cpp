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