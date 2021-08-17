#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS

// osThreadId_t thread1_id, thread2_id;

//void thread1(void *arg){
//  uint8_t state = 0;
//  
//  while(1){
//    state ^= LED1;
//    LEDWrite(LED1, state);
//    osDelay(100);
//  } // while
//} // thread1
//
//void thread2(void *arg){
//  uint8_t state = 0;
//  uint32_t tick;
//  
//  while(1){
//    tick = osKernelGetTickCount();
//    
//    state ^= LED2;
//    LEDWrite(LED2, state);
//    
//    osDelayUntil(tick + 100);
//  } // while
//} // thread2

typedef struct {
  osThreadId_t thread_id; // id da thread
  uint8_t num_led;     // numero do led  
  uint32_t t_ativacao; // tiques do sistema
} led;

led set_pisca[] = {
    {.num_led = LED1, .t_ativacao = 200}, {.num_led = LED2, .t_ativacao = 300},
    {.num_led = LED3, .t_ativacao = 500}, {.num_led = LED4, .t_ativacao = 700},
};

void pisca(void *arg){
  uint8_t state = 0;
  uint32_t tick;
  led *aux = (led*)arg;
  
  while(1){
    tick = osKernelGetTickCount();
    state ^= aux->num_led;
    LEDWrite(aux->num_led, state);
    osDelayUntil(tick + aux->t_ativacao);
  } // while
} // pisca


void main(void){
  LEDInit(LED1 | LED2 | LED3 | LED4);
  osKernelInitialize();

  for (int count = 0; count < 4; count++)
    set_pisca[count].thread_id = osThreadNew(pisca, &set_pisca[count], NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
