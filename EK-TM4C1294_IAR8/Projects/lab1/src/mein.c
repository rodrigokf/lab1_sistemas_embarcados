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

//#define FREQ_120M


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
  uint8_t c = 0;
  
  uint8_t duty[2]; //vetor para converter para caracteres
  uint8_t freq[6]; //vetor para converter para caracteres
  
#ifdef FREQ_120M
 
  tl *= 1316;
  th *= 1471;
#else
  
  tl *= 6329 ;  //tempo em ns 
  th *= 7142 ; //tempo em ns
  
#endif
  
  
  
  periodo = tl + th;
  duty_cycle = (uint32_t) (th*100)/periodo;
  frequencia = 100000000/periodo; //valor em xxx.x KHz de frequencia
  
  
  //etapa que converte valor decimal para caractres ASCII
  freq[0] = (frequencia/100000);     //milhão
  frequencia -= freq[0] * 1000;
  freq[1] = (frequencia/10000);     //milhar
  frequencia -= freq[1] * 1000;
  freq[2] = (frequencia/1000);     //centena
  frequencia -= freq[2] * 1000;
  freq[3] = (frequencia/100);      //dezena
  frequencia -= freq[3] * 100;
  freq[4] = (frequencia/10);       //unidade
  frequencia -= freq[4] * 10;
  freq[5] = frequencia;          //decimal
  
  for(c = 0; c < 6; c++)
    freq[c] += 0x30;
  
  duty[0] = (duty_cycle/10);      //dezena
  duty_cycle -= duty[0] * 10;
  duty[1] = duty_cycle + 0x30;           //unidade
  
  duty[0] += 0x30;
  
  UART_send_string("freq = ", 7);
  UART_send_string(freq, 5);
  UART_send_string(".", 1);
  UART_send_string(&freq[5], 1); 
  UART_send_string("kHz", 3);
  UART_send_string("\r\n", 2);

  UART_send_string("duty cycle = ", 13);
  UART_send_string(duty, 2);
  UART_send_string("%", 1);
  UART_send_string("\r\n", 2);
  
  sys_delay(3000);
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
  sys_delay(5);
  UART_send_string("inicio", 6);
  UART_send_string("\r\n", 2);

  uint32_t tlimite = 0; //tempo limite para detectar se sinal é nível DC
  uint8_t entrada = 1;  //variável que recebe a leitura do pino de sinal
 
  uint32_t th = 0, tl = 0; //th = tempo high, tl = tempo low
  uint8_t flag_out = 0; //flag de saída, quando detectou condição de saída do loop
  while(1)
  {    
    th = 0;
    tl = 0;
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
       }  
       
       while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) != 0x00) //enquanto sinal nao for para zero
       {
         th++;
       }
       while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) == 0x00) //enquanto sinal nao for para zero
       {
         tl++;
       }
    
       calculo_tempo(tl, th);
       entrada = 1;
  }
   
    
  } 
