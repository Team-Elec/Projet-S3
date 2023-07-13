#include <Arduino.h>
#include "setup.h"
#include "variables.h"
#include "comchinois.h"

void setup()
{
  appelVariables();
  VitessePWM();
}

void loop()
{

  float T1 = analogRead(ValSortieMoteur1);
  float T2 = analogRead(ValSortieMoteur2);
  float C = analogRead(ValCourant);

  float Courant = map(C, 0, 1023, 0, 3);
  float Tension1 = map(T1, 0, 1023, 0, 5);
  float Tension2 = map(T2, 0, 1023, 0, 5);

  int PWM = 0;

  int vitesse = analogRead(A6);

  if (vitesse > 524)
  {
    digitalWrite(Inverse2, LOW);
    digitalWrite(Inverse1, HIGH);
    vitesse = vitesse - 524;
    PWM = 2.55 * vitesse * 0.2;
    analogWrite(PWMInverse2, 0);
    analogWrite(PWMInverse1, PWM);
  }

  /*else if (vitesse < 500)
  {
    digitalWrite(Inverse1, LOW);
    digitalWrite(Inverse2, HIGH);
    PWM = 255-(2.55 * vitesse * 0.2);
    analogWrite(PWMInverse1, 0);
    analogWrite(PWMInverse2, PWM);
  }*/

  else
  {
    digitalWrite(Inverse1, LOW);
    digitalWrite(Inverse2, LOW);
    analogWrite(PWMInverse1, 0);
    analogWrite(PWMInverse2, 0);
  }

  // Inverse 1 va avec PWM1
  Serial.print("Potentimètre demander : ");
  Serial.print(vitesse);
  Serial.print("\tTension 1 : ");
  Serial.print(Tension1);
  Serial.print("\tTension 2 : ");
  Serial.print(Tension2);
  Serial.print("\tCourant : ");
  Serial.print(Courant);
}
