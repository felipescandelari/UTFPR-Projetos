## Projeto final

Este projeto é referente ao trabalho final da matéria de Sistemas Embarcados do curso de Engenharia Eletrônica da UTFPR.

# Contexto

No Projeto Final será utilizada programação concorrente com RTOS, interrupções e periféricos integrados de um microcontrolador para implementar um sistema de controle.

# Escolha do projeto final

O projeto final escolhido foi o de controle de um sistema de elevadores.

### Requisitos Funcionais

- [x] O projeto deverá possuir três elevadores;
- [x] Cada elevador deverá poder frequentar 16 andares, do térreo (0) ao último (15);
- [x] Os três elevadores deverão iniciar no andar térreo;
- [x] Os três elevadores deverão ter suas portas completamente abertas sempre que estiverem parados em um andar e não houver nenhuma requisição para seu uso no momento;
- [x] Cada elevador deverá receber o nome conforme seu respectivo bloco (E = esquerdo, C = centro, D = direito);
- [x] O elevador deverá fechar completamente suas portas antes de entrar em movimento;
- [x] O elevador não deverá atender uma requisição se já houver um elevador naquele andar da requisição;
- [x] O elevador deverá impedir o fechamento de suas portas caso haja uma requisição naquele andar, ou seja, se a porta estiver fechando (no momento da requisição) ela deve abrir novamente;
- [x] O elevador deverá permanecer com as portas fechadas completamente até que chegue no andar requisitado;
- [x] Os andares do edifício deverão ser identificados unicamente em ordem alfabética (a =  térreo, b = 1o andar, c = 2o andar ...);
- [x] O elevador deverá atender sequencialmente suas requisições de acordo com a ordem crescente (se estiver subindo) dos andares escolhidos internamente. Ou seja, se o elevador estiver no andar 4 e três pessoas apertem botões para irem até os andares 7, 5 e 6 respectivamente, por exemplo, a ordem de paradas deste elevador será: 5, 6 e, por último, 7.
- [x] O elevador deverá atender sequencialmente suas requisições de acordo com a ordem decrescente (se estiver descendo) dos andares escolhidos internamente. Ou seja, se o elevador estiver no andar 4 e três pessoas apertem botões para irem até os andares 2, 1 e 3 respectivamente, por exemplo, a ordem de paradas deste elevador será: 3, 2 e 1.
- [x] Dentro do elevador, o sistema deverá iluminar o botão (número do andar) no painel interno escolhido pelo usuário;
- [x] Dentro do elevador, após o elevador ter chego em seu andar de destino, o sistema deverá apagar o botão no painel interno aceso anteriormente;
- [x] O elevador deverá ficar parado no andar que visitou por último, até que outro usuário o chame;
- [x] O sistema deverá priorizar os usuários que desejam descer, a não ser que o elevador já esteja subindo.


### Restrições

- [x] Deverá ser escrito um programa concorrente em linguagem C (ou C + Assembly) utilizando o RTOS RTX5 para o kit EK-TM4C1294XL;
- [x] O ambiente de desenvolvimento do sistema deverá ser o IAR EWARM versão 9.10.2;
- [x] O programa concorrente deverá ser implementado com pelo menos três tarefas;
- [x] A comunicação serial (UART) deverá ser implementada por interrupção, tanto na recepção quanto na transmissão de caracteres;
- [x] A parte mecânica de cada sistema deverá ser simulada utilizando um dos simuladores desenvolvidos no projeto SimSE2;
- [x] A interligação física entre o simulador (software rodando no PC) e o kit de desenvolvimento se dará via porta serial (COM virtual sobre USB);
- [x] O simulador deverá reagir a comandos enviados pela porta serial do PC e informar o status do sistema de elevadores também pela porta serial do PC.
