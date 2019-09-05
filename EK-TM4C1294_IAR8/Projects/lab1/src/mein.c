#include <stdint.h>
#include <stdbool.h>
// includes da biblioteca driverlib
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#define SECOND 3000000 //vai mudar o valor

void main(void){
   
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilita GPIO F 
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // Aguarda final da habilitação
  
  GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_1); // pino 1 como entrada
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  

  uint32_t tlimite = 0; //para 1 segundo c = 3000000
  uint8_t entrada = 0;
 
  uint32_t th = 0, tl = 0;
  uint8_t flag_out = 0;
  while(1)
  {      
    GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);           //le gpio
       if(entrada != 0) //se entrada for 1
       {
         tlimite = 0;   //reinicia contador limite de tempo
         while(flag_out == 0)
         {
           GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
           if(entrada == 0) //procura borda de descida
           {
             while(1)
             {
                GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1);
                if(entrada == 1)
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
             
             if(tlimite > SECOND)
             {
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
         if(tlimite > SECOND)
         {
           //chama uart
           //f = 0
           //duty = 0
           tlimite = 0;
         }
       }
  }
   
    
 } 
