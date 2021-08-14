#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "driverlib/systick.h"
#include "driverleds.h" // Projects/drivers

// MEF com apenas 2 estados e 1 evento temporal que alterna entre eles
// Seleção por estado

volatile int Evento = 0;

void SysTick_Handler(void){
  Evento = 1;
} // SysTick_Handler

void main(void){
  static int Estado = 000; // MEF -> estado inicial
  LEDInit(LED1); LEDInit(LED2); LEDInit(LED3);
  SysTickPeriodSet(12000000); // f = 1Hz para clock = 24MHz
  SysTickIntEnable();
  SysTickEnable();

  while(1){
    switch(Estado){
    case 000:
      LEDOn(LED1);
      if(Evento){
        LEDOff(LED1);
        Evento = 0;
        Estado = 001;
      } // if
      break;
    case 001:
      LEDOn(LED2);
      if(Evento){
        LEDOff(LED2);
        Evento = 0;
        Estado = 011;
      } // if
      break;
     case 011:
      LEDOn(LED3);
      if(Evento){
        LEDOff(LED3);
        Evento = 0;
        Estado = 111;
      } // if
      break;
     case 111:
      LEDOff(LED1);
      LEDOff(LED2);
      LEDOff(LED3);
      if(Evento){
        Evento = 0;
        Estado = 101;
      } // if
     case 101:
      LEDOn(LED3);
      if(Evento){
        LEDOff(LED3);
        Evento = 0;
        Estado = 100;
      } // if
      break;
     case 100:
      LEDOn(LED2);
      if(Evento){
        LEDOff(LED2);
        Evento = 0;
        Estado = 000;
      } // if
      break;
    } // switch
  } // while
} // main