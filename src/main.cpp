#include <Arduino.h>
#include "setup.h"
#include "variables.h"
#include "comchinois.h"

#define SendTime 1000

float ValThermistance1, ValThermistance2 = 0.0;
float Thermistance1, Thermistance2 = 0.0;
int LastSendTime = 0;
float T1, T2, C, Tension1, Tension2, Tension, Courant = 0;
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

  Courant = (C * 3.4) / 1023;
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
    Tension = Tension2 - Tension1;
  }
  else if (vitesse < 423)
  {
    digitalWrite(Inverse1, LOW);
    digitalWrite(Inverse2, HIGH);
    PWM = map(lecture, 423, 0, 0, 255);
    analogWrite(PWMInverse1, 0);
    analogWrite(PWMInverse2, PWM);
    Tension = Tension1 - Tension2;
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
    ValThermistance1 = ((analogRead(Therm1) * 5.0) / 1023.0);
    ValThermistance2 = ((analogRead(Therm2) * 5.0) / 1023.0);

    Thermistance1 = (4.9489 * pow((ValThermistance1), 3)) - (39.419 * pow((ValThermistance1), 2)) + (ValThermistance1 * 118.83) - 86.986;
    Thermistance2 = (4.9489 * pow((ValThermistance2), 3)) - (39.419 * pow((ValThermistance2), 2)) + (ValThermistance2 * 118.83) - 86.986;

    Serial.print("Potentimètre demander : ");
    Serial.print(PWM);
    Serial.print("\tTension : ");
    Serial.print(Tension);
    Serial.print("\t\tCourant : ");
    Serial.print(CourantMoyen);
    Serial.print("\t\tThermistance 1 : ");
    Serial.print(Thermistance1);
    Serial.print("\tThermistance 2 : ");
    Serial.println(Thermistance2);

    LastSendTime = CurrentMilis;
  }
  if (NbCourant > 1000)
  {
    NbCourant = 1;
    CourantVal = CourantMoyen;
  }
}
