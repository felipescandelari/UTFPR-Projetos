# SISTEMAS EMBARCADOS

A atividade a seguir é referente a um exercício realizado para a matéria de Sistemas Embarcados, pertencente ao curso de Engenharia Eletrônica da UTFPR.
Para a realização destes experimentos usaremos a seguinte placa de desenvolvimento: https://www.ti.com/tool/EK-TM4C1294XL


### PROFESSOR

Hugo Vieira Neto


### LABORATÓRIO_05

### Exercício 6

A intenção desta atividade é implementar a comunicação entre uma ISR de GPIO, provocada pelo acionamento de botão do kit, e uma tarefa que aciona os LEDs do kit conforme uma contagem binária realizada pela ISR.

Com base no projeto “prodcons” da área de trabalho “EK-TM4C1294_RTOS_IAR9-main”, foi implementado um programa concorrente em que a entidade produtora de dados é uma ISR e a entidade consumidora de dados é uma tarefa.


• Comportamento do software:

- [x] O pressionamento do botão SW1 do kit deverá causar uma requisição de interrupção.
- [x] A cada atendimento de interrupção pela ISR correspondente, uma variável global deverá ter o seu valor incrementado e colocado no buffer de comunicação com a tarefa.
- [x] A tarefa deverá retirar o valor do buffer de comunicação e apresentar o valor binário dos seus 4 bits menos significativos nos 4 LEDs do kit.


• Efeito desejado:
- [x] a cada pressionamento do botão SW1 deverá haver um incremento na contagem de 4 bits apresentada nos LEDs D1 a D4 do kit.


• Foram utilizadas as funcionalidades da biblioteca driverlib (TivaWare) para configurar a interrupção de GPIO no port em que os botões do kit estão conectados.


## Link Útil

https://www.keil.com/pack/doc/CMSIS/RTOS2/html/index.html