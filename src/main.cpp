#include <Arduino.h>
#include "setup.h"
#include "variables.h"
#include "comchinois.h"

#define SendTime 10000

int LastSendTime = 0;
float T1 = 0;
float T2 = 0;
float C = 0;
float Tension1 = 0;
float Courant = 0;
float Tension2 = 0;
int NbCourant = 0;
float CourantVal = 0;
float CourantMoyen = 0;
int PWM = 0;
float Gain = 40;

void setup()
{
  appelVariables();
  VitessePWM();
}

void loop()
{
  
  int CurrentMilis = millis();

  T1 = analogRead(ValSortieMoteur1);

  T2 = analogRead(ValSortieMoteur2);

  C = analogRead(ValCourant);

  Courant = (C * 3.1) / 1023;
  NbCourant += 1;
  CourantVal = CourantVal + Courant;
  CourantMoyen = CourantVal / NbCourant;

  Tension1 = ((T1 * Gain) / 1023);

  Tension2 = ((T2 * Gain) / 1023);

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
    PWM = -PWM;
  }
  else
  {
    digitalWrite(Inverse1, LOW);
    digitalWrite(Inverse2, LOW);
    analogWrite(PWMInverse1, 0);
    analogWrite(PWMInverse2, 0);
  }

  if (CurrentMilis - LastSendTime > SendTime)
  {
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
    Serial.print(CourantMoyen);
    Serial.print(" / ");
    Serial.println(C);
    LastSendTime = CurrentMilis;
  }
  if (NbCourant > 1000)
  {
   NbCourant = 1;
   CourantVal = CourantMoyen;
  }
}
