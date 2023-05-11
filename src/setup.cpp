#include <Arduino.h>
#include "setup.h"
#include "variables.h"

void appelVariables()
{
    Serial.begin(BAUD);
    Serial1.begin(BAUD);

    pinMode(VOLTAGE_PIN, INPUT);

}