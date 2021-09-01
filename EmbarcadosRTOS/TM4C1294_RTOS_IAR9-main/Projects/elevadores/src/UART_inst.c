#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os2.h" // CMSIS-RTOS

// includes da biblioteca driverlib
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "system_TM4C1294.h"

#include "drivers/driverleds.h"
//------
//
// Constantes.
//
#define NUMBEROFMESSAGES    9

#define NONE                -1
#define NUMBEROFSTAGES      16
#define NUMBEROFELEVATORS   3
#define FOREVER             1
#define TRUE                1
#define FALSE               0
#define BAUD_RATE           115200

#define UP                  0x01
#define FREE                0x00
#define DOWN                0x02
#define DIRECTIONMASK       0x03

#define DOORDONE            0x00
#define DOOROPENING         0x01
#define DOOROPEN            0x02
#define DOORCLOSING         0x04
#define DOORCLOSED          0x08

#define CENTRAL             0
#define RIGHT               1
#define LEFT                2

#define OPEN                'A'
#define CLOSED              'F'
#define CURRENTSTAGE        'S'
#define INNERCALL           'I'
#define OUTERCALL           'E'

#define RESET               'r'
#define OPENDOOR            'a'
#define CLOSEDOOR           'f'
#define GOUP                's'
#define GODOWN              'd'
#define STOP                'p'
#define TURNBUTTONON        'L'
#define TURNBUTTONOFF       'D'

#define OFF                 0
#define ON                  1

#define DOORWAITPERIOD      5000 // 5 segundos

#define STAGEPRIORITY       0
//------
//
// Tipos personalizados.
//
typedef enum {
  S0000,        // Inicialização dos Elevadores
  S0005,        // Elevador Livre; Aguardo de Chamada
  S0010,        // Verifica??o se o Bot?o ? no mesmo Andar que os Elevadores
  S0020,        // Abertura da Porta Interna dos Elevadores
  S0030,        // Escolha do Elevador de Prefer?ncia
  S0040,        // Prepara movimento do Elevador (decide a Dire??o,
   S0041,       //   aguarda 7 segundos e 
   S0042,       //   manda fechar a Porta Interna e
   S0043,       //   verifica se a Porta Interna fechou)
  S0045,        // Atualiza Alvo
  S0050,        // Abertura da Porta Interna do Elevador
  S0055,        // Atualiza Alvo
  S0060,        // Movimento do Elevador e
   S0061,       //   verifica se atende Chamada
  S0065,        // Atualiza Alvo
  S0070,        // Paragem do Elevador
  S0075,        // Atualiza Alvo
  S0080,        // Abertura da Porta Interna
   S0081,       //  aguarda Porta Interna abrir
  S0085,        // Atualiza Alvo
  S0090,        // Limpeza do Bot?o de Chamada
  S0095         // Atualiza Alvo        
} state_t;

typedef struct stElevatorMsg {
  osMessageQueueId_t     q;
  uint8_t                number;                     // N?mero do Elevador
  int8_t                 direction;                  // Dire??o
  uint8_t                target;                     // Alvo
  uint8_t                currentStage;               // Andar atual do Elevador
  uint8_t                doorState;                  // Estado da Porta Interna do Elevador
  uint8_t                *extButtons;                // Bot?es Externos chamados
  uint8_t                *intButtons;                // Bot?es Internos chamados
  struct stElevatorMsg** elevators;                  // Os tr?s elevadores.
} elevatorMsg_t;

typedef struct {
  uint8_t         elevatorNumber;
  elevatorMsg_t** elevators;
  uint8_t         event;
  uint8_t         stage;
  uint8_t         direction;
} eventArg_t;

typedef struct {
  elevatorMsg_t* elevator;
  uint8_t        elevatorNumber;
  uint8_t        stage;
} commandArg_t;

//------
//
// Prot?tipo das fun??es.
//
extern void UARTStdioIntHandler(void);
void ControlTask(void*);
void ElevatorTask(void*);
void UART0_Handler(void);

// Eventos provenientes do Estado F?sico.
void PortaAberta(eventArg_t);
void PortaFechada(eventArg_t);
void ChamadaExterna(eventArg_t);
void ChamadaInterna(eventArg_t);
void AndarAtual(eventArg_t);

// Comandos de atua??o sobre o Estado F?sico.
void AbrirPorta(commandArg_t);
void FecharPorta(commandArg_t);
void Subir(commandArg_t);
void Descer(commandArg_t);
void Parar(commandArg_t);
void LigarBotao(commandArg_t);
void DesligarBotao(commandArg_t);

// Helpers.
uint8_t ElevatorNumberEventHelper(uint8_t ui8Char);
uint8_t StageCharEventHelper(uint8_t ui8Char);
uint8_t NumericStageEventHelper(const char *sentence);
char ElevatorCharCommandHelper(uint8_t ui8ElevatorNumber);
char StageCharCommandHelper(uint8_t ui8StageNumber);

// Inicializadores de Estado.
void InitButtons(uint8_t* buttons);
void InitElevatorMsgsAndQs(elevatorMsg_t** elevators, 
                           uint8_t* extButtons, 
                           uint8_t* intButtons0, 
                           uint8_t* intButtons1, 
                           uint8_t* intButtons2);

// Fun??es de l?gica de neg?cio.
uint8_t AbertaPortaElevador(uint8_t ui8CurrentStage, elevatorMsg_t* elevator);
uint8_t VerificarPreferencia(elevatorMsg_t** elevators);
uint8_t ObterAlvo(uint8_t* ui8Direction, elevatorMsg_t* elevator);
uint8_t SemAlvosIguaisA(uint8_t ui8Target, elevatorMsg_t** elevators, uint8_t ui8ElevatorNumber);
uint8_t AtendeChamada(elevatorMsg_t* elevator);
//------
//
// Vari?veis globais.
//
osMessageQueueId_t g_qs[NUMBEROFELEVATORS];
osMutexId_t        g_uartMutex;
osMutexId_t        g_buttonSelect;
volatile uint32_t  g_uartTxCount = 0;
volatile uint32_t  g_uartRxCount = 0;

//------
//
// Ponto de entrada do programa.
//

void UARTInit(void){
  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Initialize the UART for console I/O.
  UARTStdioConfig(0, BAUD_RATE, SystemCoreClock);

  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
} // UARTInit

//------
//
// Ponto de entrada do programa.
//
void main(void){

  UARTInit();
  LEDInit(LED2 | LED1);
  
  osKernelInitialize();

  // Iniciar thread de controle.
  osThreadNew(ControlTask, NULL, NULL);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
}

//------
//
// Tarefas.
//
void ControlTask(void *arg) {
  // Declarar vari?veis.
  uint32_t txCount = 0,
           rxCount = 0;
  
  // Declarar e inicializar Bot?es Externos.
  uint8_t extButtons[NUMBEROFSTAGES],
          intButtons0[NUMBEROFSTAGES],
          intButtons1[NUMBEROFSTAGES],
          intButtons2[NUMBEROFSTAGES];

  InitButtons(extButtons);
  InitButtons(intButtons0);
  InitButtons(intButtons1);
  InitButtons(intButtons2);
  
  // Declarar e inicializar mensagens de Elevadores.
  elevatorMsg_t e0, e1, e2;
  elevatorMsg_t *elevators[NUMBEROFELEVATORS] = { &e0, &e1, &e2 };

  InitElevatorMsgsAndQs(elevators, extButtons, 
                        intButtons0, intButtons1, intButtons2);
  
  // Inicializar mutex da UART.
  g_uartMutex = osMutexNew(NULL);
  g_buttonSelect = osMutexNew(NULL);
  
  // Inicializar Estado F?sico.
  UARTprintf("cr\r");
  UARTprintf("dr\r");
  UARTprintf("er\r");
  UARTFlushTx(false);

  // Iniciar threads dos elevadores.
  for (int i = 0; i < NUMBEROFELEVATORS; ++i) {
    osThreadNew(ElevatorTask, (void*)elevators[i], NULL);
  }
  
  while (FOREVER) {
    eventArg_t eventArg;
    char buffer[UART_TX_BUFFER_SIZE];
    uint8_t elevatorNumber, stage;
      
    if (g_uartTxCount != txCount) {
      LEDOn(LED1);
      osDelay(10);
      LEDOff(LED1);
      txCount = g_uartTxCount;
    }
    
    if (g_uartRxCount != rxCount) {
      LEDOn(LED2);
      osDelay(10);
      LEDOff(LED2);
      rxCount = g_uartRxCount;
    }
    
    // Ler UART.
    UARTgets(buffer, 6);

    if ((elevatorNumber = ElevatorNumberEventHelper(buffer[0])) != 0xFF) {
      // Conseguiu o N?mero do Elevador.
      // Inicializar arg.
      eventArg.elevators = elevators;  
      eventArg.elevatorNumber = elevatorNumber;
      eventArg.event = buffer[1];
      
      if (buffer[1] == INNERCALL && 
                 (stage = StageCharEventHelper(buffer[2])) != 0xFF) {
        // ? Chamada Interna.
        eventArg.stage = stage;
        
      } else if (buffer[1] == OUTERCALL) {
        // ? Chamada Externa.
        eventArg.direction = buffer[4] == 's' 
          ? UP
          : DOWN;
        eventArg.stage = NumericStageEventHelper(&buffer[2]);

      } else if ((stage = NumericStageEventHelper(&buffer[1])) != 0xFF) {
        // Conseguiu o Andar atual.
        eventArg.event = CURRENTSTAGE;
        eventArg.stage = stage;
      }

      if (eventArg.stage != 0xFF && eventArg.elevatorNumber != 0xFF) {
        switch (eventArg.event) {
        case OPEN:
          PortaAberta(eventArg);
          break;
        case CLOSED:
          PortaFechada(eventArg);
          break;
        case CURRENTSTAGE:
          AndarAtual(eventArg);
          break;
        case INNERCALL:
          ChamadaInterna(eventArg);
          break;
        case OUTERCALL:
          ChamadaExterna(eventArg);
          break;
        default:
          break; // Ignora caracter estranho. N?o faz nada.
        }
      }
    }
  }
}

void
ElevatorTask(void* arg)
{
  // Inicializar a tarefa.
  elevatorMsg_t *pMsg = (elevatorMsg_t*)arg;
  elevatorMsg_t msg   = *pMsg;
  
  // Atributos do Elevador.
  state_t state          = S0000;
  uint8_t target         = 0xFF;
  uint8_t direction      = FREE;
  uint32_t doorTickCount = ~0U;
  
  while (FOREVER) {
    if (osOK == osMessageQueueGet(msg.q, (elevatorMsg_t*)pMsg, NULL, 0)) {
      // Se veio mensagem, atualizar tanto as vari?veis locais quanto
      // a mensagem original.
      pMsg->direction = direction;
      pMsg->target    = target;
      msg = *pMsg;
    }
    
    switch (state) {
    case S0000:
      {
        // Verificar por Chamadas no mesmo Andar que o Andar atual.
        if (AbertaPortaElevador(msg.currentStage, pMsg)) {
          doorTickCount = ~0U;
          state = S0020;
        } else {
          // Se n?o houve Chamada no mesmo Andar que o Andar atual do
          // Elevador, ent?o transitar para o Estado de escolha do Elevador
          // de Prefer?ncia.
          state = S0030;
        }
        break;
      }
    case S0020:
      {
        if (msg.doorState == DOOROPEN) {
          if (osKernelGetTickCount() - doorTickCount >= DOORWAITPERIOD) {
            state = S0000;
          }
        } else {
          doorTickCount = osKernelGetTickCount();
        }
        break;
      }
    case S0030:
//      {
//        // Verificar qual dos Elevadores ? o de Prefer?ncia.
//        uint8_t number = VerificarPreferencia(msg.elevators);
//
//        if (number == msg.number) {
//          // O Elevador de Prefer?ncia ? o atual.
//          state = S0040;
//        }
//        break;
//      }
    case S0040:
      {
        uint8_t currentStage = msg.currentStage;
        
        if (AbertaPortaElevador(currentStage, pMsg)) {
          doorTickCount = ~0U;
          state = S0050;
        } else {
          if (direction == FREE) {
            // Decide a Dire??o e o Alvo.
            target = ObterAlvo(&direction, pMsg);
          } else {
            doorTickCount = osKernelGetTickCount();
            state = S0041;
          }
        }
        break;
      }
    case S0041:
      {
        uint8_t tempDirection;
        uint8_t tempTarget;
        tempTarget = ObterAlvo(&tempDirection, pMsg);
        if (tempTarget != 0xFF && 
            (tempTarget > target && direction == UP ||
             tempTarget < target && direction == DOWN)) {
            target = tempTarget;
        }
        
        if (osKernelGetTickCount() - doorTickCount >= DOORWAITPERIOD) {
          state = S0042;
        }
        break;
      }
    case S0042:
      {
        uint8_t tempDirection;
        uint8_t tempTarget;
        tempTarget = ObterAlvo(&tempDirection, pMsg);
        if (tempTarget != 0xFF && 
            (tempTarget > target && direction == UP ||
             tempTarget < target && direction == DOWN)) {
            target = tempTarget;
        }

        commandArg_t cmdArg;
        cmdArg.elevatorNumber = msg.number;
        cmdArg.elevator = pMsg;
        FecharPorta(cmdArg);

        state = S0043;

        break;
      }
    case S0043:
      {
        uint8_t tempDirection;
        uint8_t tempTarget;
        tempTarget = ObterAlvo(&tempDirection, pMsg);
        if (tempTarget != 0xFF && 
            (tempTarget > target && direction == UP ||
             tempTarget < target && direction == DOWN)) {
            target = tempTarget;
        }

        uint8_t currentStage = msg.currentStage;
        doorTickCount = ~0U;

        if (AbertaPortaElevador(currentStage, pMsg)) {
          state = S0050;
        } else if (msg.doorState == DOORCLOSED) {
          state = S0060;
        }

        break;
      }
    case S0050:
      {
        if (msg.doorState == DOOROPEN) {
          if (osKernelGetTickCount() - doorTickCount >= DOORWAITPERIOD) {
            state = S0040;
          }
        } else {
          doorTickCount = osKernelGetTickCount();
        }

        break;
      }
    case S0060:
      {
        uint8_t tempDirection;
        uint8_t tempTarget;
        tempTarget = ObterAlvo(&tempDirection, pMsg);
        if (tempTarget != 0xFF && 
            (tempTarget > target && direction == UP ||
             tempTarget < target && direction == DOWN)) {
            target = tempTarget;
        }
        
        commandArg_t cmdArg;
        cmdArg.elevatorNumber = msg.number;
        
        switch (direction) {
        case DOWN:
          Descer(cmdArg);
          break;
        case UP:
          Subir(cmdArg);
          break;
        default:
          break;
        }
        
        state = S0061;
        
        break;
      }
    case S0061:
      {
        uint8_t tempDirection;
        uint8_t tempTarget;
        tempTarget = ObterAlvo(&tempDirection, pMsg);
        if (tempTarget != 0xFF && 
            (tempTarget > target && direction == UP ||
             tempTarget < target && direction == DOWN)) {
            target = tempTarget;
        }
        
        if (AtendeChamada(pMsg)) {
          state = S0070;
        }
        break;
      }
    case S0070:
      {
        uint8_t tempDirection;
        uint8_t tempTarget;
        tempTarget = ObterAlvo(&tempDirection, pMsg);
        if (tempTarget != 0xFF && 
            (tempTarget > target && direction == UP ||
             tempTarget < target && direction == DOWN)) {
            target = tempTarget;
        }

        commandArg_t cmdArg;
        cmdArg.elevatorNumber = msg.number;
        Parar(cmdArg);
        
        state = S0080;

        break;
      }
    case S0080:
      {
        commandArg_t cmdArg;
        cmdArg.elevatorNumber = msg.number;
        cmdArg.elevator = pMsg;
        AbrirPorta(cmdArg);
        state = S0081;
        break;
      }
    case S0081:
      {
        if (msg.doorState == DOOROPEN) {
          state = S0090;
        }
        break;
      }
    case S0090:
      {
        commandArg_t cmdArg;
        cmdArg.elevatorNumber = msg.number;
        cmdArg.stage = msg.currentStage;

        DesligarBotao(cmdArg);
        
        if (msg.currentStage == target) {
          target = 0xFF;
          direction = FREE;
          state = S0000;
        } else {
          state = S0040;
        }
        
        break;
      }
    default:
      break;
    }
  }
}

//------
//
// Fun??es de l?gica de neg?cio.
//
uint8_t
AbertaPortaElevador(uint8_t ui8Stage, elevatorMsg_t* elevator) 
{
  uint8_t result = FALSE;
  
  osMutexAcquire(g_buttonSelect, osWaitForever);
  if (ui8Stage == elevator->currentStage &&
      elevator->doorState != DOORCLOSED &&
       (elevator->intButtons[ui8Stage] ||
        elevator->extButtons[ui8Stage])) {
    // Elevador est? no Andar em que ocorreu Chamada.
    // Abrir a Porta Interna, como emerg?ncia.
    commandArg_t arg;

    arg.elevatorNumber = elevator->number;
    arg.stage = ui8Stage;
    arg.elevator = elevator;
    AbrirPorta(arg);
    DesligarBotao(arg);

    elevator->intButtons[ui8Stage] = FREE;
    elevator->extButtons[ui8Stage] = FREE;
    
    result = TRUE;
  }
  osMutexRelease(g_buttonSelect);
  
  return result;
}

uint8_t
VerificarPreferencia(elevatorMsg_t** elevators)
{
  uint8_t result = 0xFF;
  
  for (int i = 0; i < NUMBEROFELEVATORS; ++i) {
    if (elevators[i]->direction == FREE) {
      result = i;
      break;
    }
  }

  return result;
}

uint8_t
ObterAlvo(uint8_t* ui8Direction, elevatorMsg_t *elevator){

  // O Alvo ? o Andar com Chamada o mais distante poss?vel
  // do Andar atual do Elevador.
  uint8_t result = 0xFF;
  uint8_t absDistance = 0; // Valor m?nimo poss?vel.
  uint8_t currentStage = elevator->currentStage;
  
  osMutexAcquire(g_buttonSelect, osWaitForever);

  for (uint8_t i = NUMBEROFSTAGES; i > 0; --i) {
    if (elevator->intButtons[i - 1] || elevator->extButtons[i - 1]) {
      // O bot?o foi pressionado no Andar i.
      if (abs(i - 1 - currentStage) > absDistance && 
          SemAlvosIguaisA(i - 1, elevator->elevators, elevator->number)) {
        // A dist?ncia at? a Chamada no Andar i ? a maior de todas.
        result = i - 1;
        absDistance = (uint8_t)abs(i - 1 - currentStage);
      }
    }
  }

  osMutexRelease(g_buttonSelect);
  
  if (result != 0xFF) {
    // Foi detectado um poss?vel Alvo.
    *ui8Direction = currentStage < result
      ? UP
      : (currentStage > result
           ? DOWN
           : FREE);
  } else {
    // O Elevador continua Livre.
    *ui8Direction = FREE;
  }
  
  return result;
}

uint8_t SemAlvosIguaisA(uint8_t ui8Target, elevatorMsg_t** elevators, uint8_t ui8ElevatorNumber){

  uint8_t result = TRUE;
  
  for (int i = 0; i < NUMBEROFELEVATORS; ++i) {
    if (elevators[i]->target == ui8Target && i != ui8ElevatorNumber) {
      result = FALSE;
      break;
    }
  }
  
  return result;
}

uint8_t AtendeChamada(elevatorMsg_t* elevator){

  uint8_t result = FALSE;
  uint8_t currentStage = elevator->currentStage;
  
  osMutexAcquire(g_buttonSelect, osWaitForever);

  if (elevator->intButtons[currentStage] ||
      elevator->extButtons[currentStage] & elevator->direction ||
      elevator->target == currentStage) {
      // Atender ? Chamada no Andar do Elevador.
      result = TRUE;
      elevator->intButtons[currentStage] = FREE;
      if (currentStage == 0 || currentStage == 15) {
        elevator->extButtons[currentStage] = FREE;
      } else {
        elevator->extButtons[currentStage] &= 
          DIRECTIONMASK & ~elevator->direction;
      }
  }
  
  osMutexRelease(g_buttonSelect);
    
  return result;
}
        
//------
//
// Rotina de tratamento de interrup??o da UART.
//
void
UART0_Handler(void)
{
  //
  // Get and clear the current interrupt source(s)
  //
  uint32_t ui32Ints = MAP_UARTIntStatus(UART0_BASE, true);

  //
  // Are we being interrupted because the TX FIFO has space available?
  //
  if (ui32Ints & UART_INT_TX)
  {
    g_uartTxCount++;
  }

  //
  // Are we being interrupted due to a received character?
  //
  if (ui32Ints & (UART_INT_RX | UART_INT_RT))
  {
    g_uartRxCount++;
  }
  
  UARTStdioIntHandler();
}

//------
//
// Inicializadores de estados.
//
void
InitButtons(uint8_t *buttons)
{
  for (int i = 0; i < NUMBEROFSTAGES; ++i) {
    buttons[i] = FREE;
  }
}

void
InitElevatorMsgsAndQs(elevatorMsg_t **elevators, uint8_t *extButtons,
  uint8_t *intButtons0, uint8_t *intButtons1, uint8_t *intButtons2)
{
  for (int i = 0; i < NUMBEROFELEVATORS; ++i) {
    // Inicializar fila.
    g_qs[i] = osMessageQueueNew(NUMBEROFMESSAGES, sizeof(elevatorMsg_t), NULL);

    // Inicializar mensagem do Elevador.
    elevators[i]->number       = i;
    elevators[i]->elevators    = elevators;
    elevators[i]->q            = g_qs[i];
    elevators[i]->currentStage = 0;
    elevators[i]->doorState    = DOORCLOSED;
    elevators[i]->extButtons   = extButtons;
    elevators[i]->target       = 0xFF;
  }
  // Inicializando Bot?es Internos.
  elevators[0]->intButtons = intButtons0;
  elevators[1]->intButtons = intButtons1;
  elevators[2]->intButtons = intButtons2;
}

//------
//
// Eventos
//

void
PortaAberta(eventArg_t arg)
{
  elevatorMsg_t *elevator = arg.elevators[arg.elevatorNumber];
  elevator->doorState = DOOROPEN;
  osMessageQueuePut(elevator->q, (void*)elevator, 0, 0);
}

void
PortaFechada(eventArg_t arg)
{
  elevatorMsg_t *elevator = arg.elevators[arg.elevatorNumber];
  elevator->doorState = DOORCLOSED;
  osMessageQueuePut(elevator->q, (void*)elevator, 0, 0);
}

void
ChamadaExterna(eventArg_t arg)
{
  osMutexAcquire(g_buttonSelect, osWaitForever);
  for (uint8_t i = 0; i < NUMBEROFELEVATORS; ++i) {
    elevatorMsg_t *elevator = arg.elevators[i];
    elevator->extButtons[arg.stage] |= DIRECTIONMASK & arg.direction;
    osMessageQueuePut(elevator->q, (void*)elevator, 0, 0);
  }
  osMutexRelease(g_buttonSelect);
}

void 
ChamadaInterna(eventArg_t arg)
{
  elevatorMsg_t *elevator = arg.elevators[arg.elevatorNumber];
  
  osMutexAcquire(g_buttonSelect, osWaitForever);
  elevator->intButtons[arg.stage] |= DIRECTIONMASK;
  osMutexRelease(g_buttonSelect);

  osMessageQueuePut(elevator->q, (void*)elevator, 0, 0);
  
  // Ligar Bot?o Interno caso o Andar do Bot?o Interno n?o seja o Andar
  // atual do Elevador.
  if (arg.stage != elevator->currentStage) {
    commandArg_t cmdArg;
    cmdArg.elevatorNumber = arg.elevatorNumber;
    cmdArg.stage = arg.stage;
    LigarBotao(cmdArg);
  }
}

void 
AndarAtual(eventArg_t arg)
{
  elevatorMsg_t *elevator = arg.elevators[arg.elevatorNumber];
  elevator->currentStage = arg.stage;

  osMessageQueuePut(elevator->q, (void*)elevator, STAGEPRIORITY, 0);
}

// Helpers
//
uint8_t ElevatorNumberEventHelper(uint8_t ui8Char){

  uint8_t value;
  if ((value = ui8Char - 'c') >= NUMBEROFELEVATORS){
    return 0xFF; 
  }
  else{
    return value;
  } 
}

uint8_t  StageCharEventHelper(uint8_t ui8Char){

  uint8_t value;
  if ((value = ui8Char - 'a') >= NUMBEROFSTAGES){
      return 0xFF;
  }     
  else{
      return value;
  }  
}

uint8_t  NumericStageEventHelper(const char *sentence){

  if((*sentence >= '0' && *sentence <= '9')){
    return atoi(sentence);
  }
  else{
    return 0xFF;
  }
}

char  ElevatorCharCommandHelper(uint8_t ui8ElevatorNumber){

  if (ui8ElevatorNumber >= NUMBEROFELEVATORS){
    return 0xFF;
  }
  else{
    return (ui8ElevatorNumber + 'c');
  }
}

char  StageCharCommandHelper(uint8_t ui8StageNumber){

  if (ui8StageNumber >= NUMBEROFSTAGES){
    return 0xFF;
  }
  else{
    return (ui8StageNumber + 'a');
  }
}

// Comandos
void AbrirPorta(commandArg_t arg){

  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", OPENDOOR);
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
  arg.elevator->doorState = DOOROPENING;
}

void FecharPorta(commandArg_t arg) {

  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", CLOSEDOOR);
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
  arg.elevator->doorState = DOORCLOSING;
}

void  Subir(commandArg_t arg){

  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", GOUP);
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
}

void Descer(commandArg_t arg) {

  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", GODOWN);
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
}

void Parar(commandArg_t arg)
{
  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", STOP);
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
}

void LigarBotao(commandArg_t arg)
{
  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", TURNBUTTONON);
  UARTprintf("%c", StageCharCommandHelper(arg.stage));
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
}

void DesligarBotao(commandArg_t arg)
{
  osMutexAcquire(g_uartMutex, osWaitForever);
  UARTprintf("%c", ElevatorCharCommandHelper(arg.elevatorNumber));
  UARTprintf("%c", TURNBUTTONOFF);
  UARTprintf("%c", StageCharCommandHelper(arg.stage));
  UARTprintf("\r");
  UARTFlushTx(false);
  osMutexRelease(g_uartMutex);
}
