// Lab 03 - corrigido - reentrega
#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "driverlib/systick.h"
#include "driverleds.h" // Projects/drivers

// MEF com apenas 2 estados e 1 evento temporal que alterna entre eles
// Seleção por evento

int estado = 0;
volatile uint8_t Evento = 0;

void SysTick_Handler(void){
  Evento = 1;
} // SysTick_Handler

void main(void){

  LEDInit(LED1); LEDInit(LED2); LEDInit(LED3);
  SysTickPeriodSet(12000000); // f = 1Hz para clock = 24MHz
  SysTickIntEnable();
  SysTickEnable();
  
  while(1){
    if(Evento == 1){
      Evento = 0;
      switch(estado){
        case 0:
          LEDOff(LED1); LEDOff(LED2); LEDOff(LED3);
          break;
        case 1:
          LEDOff(LED1); LEDOff(LED2); LEDOn(LED3);
          break;
        case 2:
          LEDOff(LED1); LEDOn(LED2); LEDOn(LED3);
          break;
        case 3:
          LEDOff(LED1); LEDOn(LED2); LEDOff(LED3);
          break;
        case 4:
          LEDOn(LED1); LEDOn(LED2); LEDOff(LED3);
          break;
        case 5:
          LEDOn(LED1); LEDOn(LED2); LEDOn(LED3);
          break;
        case 6:
          LEDOn(LED1); LEDOff(LED2); LEDOn(LED3);
          break;
        case 7:
          LEDOn(LED1); LEDOff(LED2); LEDOff(LED3);
          break;
      } //switch
      estado ++;
      if(estado == 8){
        estado = 0;
      } // if
    } // if
  } // while
} // main