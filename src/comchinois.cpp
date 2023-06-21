#include <Arduino.h>
#include "ComChinois.h"

int lectureChinois()
{
    int valeur = 99;

    if (Serial1.available() == 3)
    {
        if(Serial1.read()== 'V')
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
