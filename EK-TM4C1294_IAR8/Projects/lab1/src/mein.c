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

//delay em ms (aproximado)
void sys_delay(uint32_t temp)
{
  
#ifdef FREQ_120M
  temp *= 15000;
#else
  temp *= 3000;
#endif

  while(temp--)
  {}
}

//funçao que envia string para a serial
void UART_send_string(uint8_t *string, uint16_t qtd)
{
  while(qtd--)
    UARTCharPut(UART0_BASE, *string++);
}

void calculo_tempo(uint32_t tl_copy, uint32_t th_copy)
{
  uint64_t periodo;
  uint64_t duty_cycle;
  uint64_t frequencia;
  uint64_t tl;
  uint64_t th;
  uint8_t c = 0;
  
  uint8_t duty[3]; //vetor para converter para caracteres
  uint8_t freq[6]; //vetor para converter para caracteres
  
#ifdef FREQ_120M
 
  //parametros de tempo para clock de 120MHz
  tl = (uint64_t)12503 * tl_copy;        //tempo em ns *100
  th = (uint64_t)13337 * th_copy;        //tempo em ns *100
#else
  
  //parametros de tempo para clock de 24MHz
  tl = (uint64_t)62508 * tl_copy;       //tempo em ns *100
  th = (uint64_t)66676 * th_copy;       //tempo em ns *100
  
#endif
  
  periodo = tl + th;
  duty_cycle = (uint64_t)(th*100000)/periodo;
  frequencia = (uint64_t) 10000000000/periodo; //valor em xxxx.xx KHz de frequencia
  
  
  //etapa que converte valor decimal para caractres ASCII
  freq[0] = (frequencia/100000);     //milhão
  frequencia -= freq[0] * 100000;
  freq[1] = (frequencia/10000);     //milhar
  frequencia -= freq[1] * 10000;
  freq[2] = (frequencia/1000);     //centena
  frequencia -= freq[2] * 1000;
  freq[3] = (frequencia/100);      //dezena
  frequencia -= freq[3] * 100;
  freq[4] = (frequencia/10);       //unidade
  frequencia -= freq[4] * 10;
  freq[5] = frequencia;          //decimal
  
  for(c = 0; c < 6; c++)          //converte para ascii
    freq[c] += 0x30;
  
  //se o período estiver na ordem 
  if(periodo > 9999999)
  {
    duty[0] = (duty_cycle/10000);      //dezena
    duty_cycle -= duty[0] * 10000;
    duty[1] = (duty_cycle/1000);       //unidade
    duty_cycle -= duty[1] * 1000;
    duty[2] = duty_cycle/100;
  }
  else
  {
    duty[0] = (duty_cycle/10000);      //dezena
    duty_cycle -= duty[0] * 10000;
    duty[1] = (duty_cycle/1000);       //unidade
    duty_cycle -= duty[1] * 1000;
    duty[2] = duty_cycle/100;
  }
  
  duty[0] += 0x30;                 //converte para ascii
  duty[1] += 0x30;
  duty[2] += 0x30;
  
  //escreve na serial a frequencia
  UART_send_string("freq = ", 7);
  UART_send_string(freq, 4);
  UART_send_string(".", 1);
  UART_send_string(&freq[4], 2); 
  UART_send_string("kHz", 3);
  UART_send_string("\r\n", 2);

  //escreve na serial o duty cycle
  UART_send_string("duty cycle = ", 13);
  UART_send_string(duty, 2);
  UART_send_string(".", 1);
  UART_send_string(&duty[2], 1);
  UART_send_string("%", 1);
  UART_send_string("\r\n", 2);
  
  //espera cerca de 2 segundos para fazer nova medição
  sys_delay(2000);
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

  uint8_t entrada = 1;  //variável que recebe a leitura do pino de sinal
 
  uint32_t th = 0, tl = 0; //th = tempo high, tl = tempo low

  while(1)
  { 
    //inicializa contadores de tempo
    th = 0;
    tl = 0;
    
    //os dois próximos loops servem para garantir a espera de 1 ciclo do sinal para que a contagem inicie
    while(entrada != 0)                                                                     
    {                                                                                        
       entrada = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);            
    }
    while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) == 0)                                                                  
    {                                                                                     

    } 
    
    //realiza a contagem do tempo em alto enquanto o sinal estiver ativo
    while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) != 0x00) //enquanto sinal nao for para zero
    {
      th++;
    }
    asm("nop"); //apenas para esperar um ciclo de máquina para garantir que a leitura do pino não virá errada
    while(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) == 0x00) //enquanto sinal nao for para zero
    {
      tl++;
    }
    
    calculo_tempo(tl, th);
    entrada = 1; //garante que fará pelo menos uma leitura do pino no próximo loop
  }
   
    
} 
