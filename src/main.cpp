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

  float T1 = 0;
  T1 = analogRead(ValSortieMoteur1);
  float T2 = 0;
  T2 = analogRead(ValSortieMoteur2);
  float C = 0;
  C = analogRead(ValCourant);

  float Courant = 0;
  Courant = (C*3)/1023;
  float Tension1 = 0;
  Tension1 = (T1*3)/1023;
  float Tension2 = 0;
  Tension2 = (T2*3)/1023;

  int PWM = 0;

  int lecture = analogRead(AjustVitesse);
  int vitesse = lecture;

  if (vitesse > 600)
  {
    digitalWrite(Inverse2, LOW);
    digitalWrite(Inverse1, HIGH);
    PWM = map(lecture, 600, 1023, 0, 255);
    analogWrite(PWMInverse2, 0);
    analogWrite(PWMInverse1, PWM);
  }
  else if (vitesse < 423)
  {
    digitalWrite(Inverse1, LOW);
    digitalWrite(Inverse2, HIGH);
    PWM = map(lecture, 423, 0, 0, 255);
    analogWrite(PWMInverse1, 0);
    analogWrite(PWMInverse2, PWM);
  }
  else
  {
    digitalWrite(Inverse1, LOW);
    digitalWrite(Inverse2, LOW);
    analogWrite(PWMInverse1, 0);
    analogWrite(PWMInverse2, 0);
  }

  Serial.print("Potentimètre demander : ");
  Serial.print(PWM);
  Serial.print("\tTension 1 : ");
  Serial.print(Tension1);
  Serial.print(" / ");
  Serial.print(T1);
  Serial.print("\tTension 2 : ");
  Serial.print(Tension2);
  Serial.print(" / ");
  Serial.print(T2);
  Serial.print("\tCourant : ");
  Serial.print(Courant);
  Serial.print(" / ");
  Serial.println(C);
}
