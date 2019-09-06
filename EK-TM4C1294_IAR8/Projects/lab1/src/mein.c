//RODRIGO KNELSEN FRIESEN
//YURI ANDREIKO

//EQUIPE: S11_G09

#define PART_TM4C1294NCPDT

#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "system_TM4C1294.h"
#define SECOND 3000000 //vai mudar o valor

//delay em ms
void sys_delay(uint32_t temp)
{
  temp *= 3000;
  while(temp--)
  {}
}

void UART_send_string(uint8_t *string, uint16_t qtd)
{
  while(qtd--)
    UARTCharPut(UART0_BASE, *string++);
}

void calculo_tempo(uint32_t tl, uint32_t th)
{
  uint32_t periodo;
  uint32_t duty_cycle;
  uint32_t frequencia;
  
  uint8_t duty[2]; //vetor para converter para caracteres
  uint8_t freq[4]; //vetor para converter para caracteres
  
  //para 24MHz
  tl *= 769;  //tempo em ns 
  th *= 1282; //tempo em ns
  
  periodo = tl + th;
  duty_cycle = (uint32_t) (th/periodo)*100;
  frequencia = 10000000/periodo; //valor em xxx.x KHz de frequencia
  
  
  //etapa que converte valor decimal para caractres ASCII
  freq[0] = (frequencia/1000) + 0x30;     //centena
  frequencia -= freq[0] * 1000;
  freq[1] = (frequencia/100) + 0x30;      //dezena
  frequencia -= freq[1] * 100;
  freq[2] = (frequencia/10) + 0x30;       //unidade
  frequencia -= freq[2] * 10;
  freq[3] = frequencia + 0x30;          //decimal
  
  duty[0] = (duty_cycle/10) + 0x30;      //dezena
  duty_cycle -= duty[0] * 10;
  duty[1] = duty_cycle + 0x30;           //unidade
  
  UART_send_string("freq = ", 7);
  UART_send_string(freq, 3);
  UART_send_string(".", 1);
  UART_send_string(&freq[3], 1); 
  UART_send_string("kHz", 3);
  UART_send_string("\r\n", 2);

  UART_send_string("duty cycle = ", 13);
  UART_send_string(duty, 2);
  UART_send_string("%", 1);
  UART_send_string("\r\n", 2);
}

void main(void){
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilita GPIO F (LED D3 = PF4, LED D4 = PF0)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // Aguarda final da habilitação
  
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1); // pino 1 como entrada
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // LEDs D3 e D4 como saída
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, 0); // LEDs D3 e D4 apagados
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);
  
  //
  // HABILITA UART0.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  sys_delay(10);

  //
  // Set GPIO A0 and A1 as UART pins.
  //
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  sys_delay(10);

  //
  // Configure the UART for 115,200, 8-N-1 operation.
  //
  UARTConfigSetExpClk(UART0_BASE, SystemCoreClock , 115200,
                          (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                           UART_CONFIG_PAR_NONE));
  sys_delay(10);
  UART_send_string("inicio", 6);
  UART_send_string("\r\n", 2);

  uint32_t tlimite = 0; //tempo limite para detectar se sinal é nível DC
  uint8_t entrada = 0;  //variável que recebe a leitura do pino de sinal
 
  uint32_t th = 0, tl = 0; //th = tempo high, tl = tempo low
  uint8_t flag_out = 0; //flag de saída, quando detectou condição de saída do loop
  while(1)
  {    
       while(entrada != 0)                                                                      //CODIGO QUE ESPERA 1 CICLO
       {                                                                                        //CODIGO QUE ESPERA 1 CICLO
         entrada = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);           //le gpio                //CODIGO QUE ESPERA 1 CICLO
         if(entrada == 0)                                                                       //CODIGO QUE ESPERA 1 CICLO
         {                                                                                      //CODIGO QUE ESPERA 1 CICLO
          while(entrada == 0)                                                                   //CODIGO QUE ESPERA 1 CICLO
          {                                                                                     //CODIGO QUE ESPERA 1 CICLO
              entrada = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);           //le gpio           //CODIGO QUE ESPERA 1 CICLO
          }                                                                                     //CODIGO QUE ESPERA 1 CICLO
          break;                                                                                //CODIGO QUE ESPERA 1 CICLO
         }                                                                                      //CODIGO QUE ESPERA 1 CICLO
       }                                                                                        //CODIGO QUE ESPERA 1 CICLO
    
       flag_out = 0;
       entrada = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);           //le gpio
       if(entrada != 0) //se entrada for 1 (detector de borda de subida)
       {
         tlimite = 0;   //reinicia contador limite de tempo
         while(flag_out == 0)
         {
           entrada = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
           if(entrada == 0) //procura borda de descida
           {
             while(1)
             {
                entrada = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
                if(entrada == 0x02) //procura borda de subida
                {
                  //calculo final
                  //chama uart
                  
                  calculo_tempo(tl, th);
                  
                  tl = 0;
                  th = 0;
                  tlimite = 0;
                  flag_out = 1;
                  break;
                }
                else
                  tl++;
             }
           }
           else
           {
             tlimite++;
             th++;
             
             if(tlimite > 1000000)
             {
               //chama uart
               //f = 0
               //duty = 1
               
               UART_send_string("freq = 0Hz", 10);
               UART_send_string("\r\n", 2);
               UART_send_string("duty cycle = 100%", 17);
               UART_send_string("\r\n", 2);
               tl = 0;
               th = 0;
               tlimite = 0;
               break;
             }
           }
         }
         
       }
       else
       {
         tlimite++;
         if(tlimite > 1000000)
         {
           //chama uart
           //f = 0
           //duty = 0
           UART_send_string("freq = 0Hz", 10);
           UART_send_string("\r\n", 2);
           UART_send_string("duty cycle = 0%", 15);
           UART_send_string("\r\n", 2);
           tlimite = 0;
         }
       }
  }
   
    
  } 
