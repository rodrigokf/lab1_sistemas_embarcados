#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#define SECOND 3000000 //vai mudar o valor

void main(void){
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilita GPIO F (LED D3 = PF4, LED D4 = PF0)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // Aguarda final da habilitação
  
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1); // pino 1 como entrada
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4); // LEDs D3 e D4 como saída
  GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, 0); // LEDs D3 e D4 apagados
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

  uint32_t tlimite = 0;
  uint8_t entrada = 0;
 
  uint32_t th = 0, tl = 0;
  uint8_t flag_out = 0;
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
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 1); // Acende LED D3 para mostrar PWM = 1
               //chama uart
               //f = 0
               //duty = 1
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
           GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 1); // Acende LED D4 para mostrar PWM = 0
           //chama uart
           //f = 0
           //duty = 0
           tlimite = 0;
         }
       }
  }
   
    
  } 
