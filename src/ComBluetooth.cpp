#include <Arduino.h>
#include "ComBluetooth.h"

void printage(bool Bluetooth, bool SerialOrdi, int val)
{
    // Petit Code de débogguage
    if (SerialOrdi)
    {
        Serial.print("Je suis rendu ici ");
        Serial.println(val);
    }
    if (Bluetooth)
    {
        Serial1.print("Je suis rendu ici ");
        Serial1.println(val);
    }
}
