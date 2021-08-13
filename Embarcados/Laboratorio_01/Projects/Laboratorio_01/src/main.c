// Felipe Scandelari
// 1760262

// Laboratorio_01
// Sistemas Embarcados - UTFPR

#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#define f_24MHZ 24000000
#define f_120MHZ 120000000
uint8_t LED_D4 = 0;

void main()
{
  uint32_t clock = SysCtlClockFreqSet(SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480 | SYSCTL_XTAL_25MHZ, f_24MHZ); //24MHz
  // uint32_t clock = SysCtlClockFreqSet(SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480 | SYSCTL_XTAL_25MHZ, f_120MHZ); //120MHz

  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilita GPIO F (LED D4 = PF0)
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    ; // Aguarda final da habilitação

  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0); // LEDs D4 como saída
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);       // LEDs D4 apagados
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

  while (1)
  {
    // Laço de atraso feito com o for a seguir
    // for (int i = 0; i < 120000000; i++)      
    // {
    // }
    
    SysCtlDelay(8000000);
    
    // Logica SysCtlDelay
    // Para 24Mhz -> 1/24MHz = 41,67 ns
    // 41,67 * 3 instruçoes = 125 ns/loop
    // 125ns * 8.000.000 = 1 s (Conforme foi pedido no exercicio)
   
    // Para 120MHZ -> 1/120MHz = 8,33 ns
    // 8,33 * 3 instruçoes = 25ns/loop
    // 25 ns * 8.000.000 = 0,2 s
    
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, LED_D4);
    if (LED_D4 == 0)
    {
      LED_D4 = 1;
    }
    else
    {
      LED_D4 = 0;
    }

    // O LED ficou ativo permanentemente ao ser utilizado o laço de atraso
    // com o for, por conta do efeito da otimização com relação ao loop inutil
    // Com a função SysCtlDelay não teve altaração, já que a função
    // tem 3 instruçoes em assembly, não havendo como otimizar
  }
}