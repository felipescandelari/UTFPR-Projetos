# SISTEMAS EMBARCADOS

O projeto a seguir é um exercício prático realizado para a matéria de Sistemas Embarcados, pertencente ao curso de Engenharia Eletrônica da UTFPR.
Para a realização destes experimentos usaremos a seguinte placa de desenvolvimento: https://www.ti.com/tool/EK-TM4C1294XL


## Professor

Hugo Vieira Neto


## LABORATÓRIO_01

Conforme foi pedido no exercício, a temporização foi feita por software (laços de atraso), isto é, sem o uso de qualquer mecanismo de interrupção por hardware.
O código a seguir trata da realização deste requisito.

```
 for (int i = 0; i < 120000000; i++)      
     {
     }
```

Também, conforme requisitado, o nível de otimização do compilador C adotado no princípio foi baixo (low) e a frequência de clock (PLL) da CPU: 24MHz.

Depois, testou-se o comportamento do sistema para as seguintes alterações:
1. Diferentes níveis de otimização do compilador C
2. Frequência de clock (PLL) de 120 MHz

Para o cálculo do SysCtlDelay, na situação com frequência de clock igual 24 MHz, tem-se:
Período de 41,67 ns, pois T = 1/24MHz = 41,67 ns.
Para três instruções, tem-se 41,67 * 3 = 125 ns/loop
125ns * X = 1 s (Conforme foi pedido no exercicio)
Isolando a equação para X encontra-se X = 8000000.
Assim, utilizou-se SysCtlDelay(8000000).

   
Para a situação em que a frequência de clock requisitada foi de 120MHZ, temos:
T = 1/120MHz = 8,33 ns
8,33 ns * 3 instruçoes = 25ns/loop
25 ns * 8.000.000 = 0,2 s
    
Dessa forma, notou-se que quando a frequência de clock utilizada foi de 120 MHz o LED piscou cinco vezes mais rápido do que na situação anterior em que a frequência de clock era 24 MHz.

Com relação a alteração no comportamento do sistema causada por mudança no nível de otimização do compilador C, notou-se:

1. O LED permaneceu ativo permanentemente no caso em que utilizou-se o laço de atraso com o for, vista como inútil devido a otimização de nível alta do compilador C.
2. Com a função SysCtlDelay não pode-se notar nenhuma altaração, já que a função
tem 3 instruçoes em assembly, não havendo como otimizar.