#include <Arduino.h>
#include "ComChinois.h"

int lectureChinois()
{
    int valeur = 99;

    if (Serial1.available() == 3)
    {
        if (Serial1.read() == 'V')
        {
            int valeur1 = 0;
            int valeur2 = 0;

            valeur1 = Serial1.read();
            valeur2 = Serial1.read();

            valeur = (valeur1 * 10) + valeur2;
        }
    }

    return valeur;
}

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
