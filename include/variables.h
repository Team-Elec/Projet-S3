#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>

#define BAUD 115200

//Ajustement de la vitesse avec le potentiomètre
#define AjustVitesse A6

// PIN DE THERMO
#define Therm1 A2
#define Therm3 A4

//Valeur de sortie du moteur dépendament des modes
#define ValSortieMoteur1 A0
#define ValSortieMoteur2 A8

#define BlueTooth 25

//Pour inversion et controle de moteur toujours mettre inverse 1 avec PWM 1, si non court-circuit
#define Inverse1 29
#define Inverse2 33
#define PWMInverse1 4
#define PWMInverse2 6


#endif
