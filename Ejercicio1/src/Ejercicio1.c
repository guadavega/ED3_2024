/*
 *Configurar la interrupción externa EINT1 para que interrumpa por flanco de
bajada y la interrupción EINT2 para que interrumpa por flanco de subida. En la
interrupción por flanco de bajada configurar el systick para desbordar cada 25
mseg, mientras que en la interrupción por flanco de subida configurarlo para que
desborde cada 60 mseg. Considerar que EINT1 tiene mayor prioridad que EINT2.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>



void configPIN(){

	LPC_PINCON->PINSEL4 |= (1<<22); //config EINT1
	LPC_SC->EXTMODE |= (1<<1);      //por flanco
	LPC_SC->EXTPOLAR &= ~(1<<1);   //por bajo

	LPC_PINCON->PINSEL4 |= (1<<24); //config EINT2
	LPC_SC->EXTMODE |= (1<<2);     //por flanco
	LPC_SC->EXTPOLAR |= (1<<2);    //por subida


}

void configEINT(){
	NVIC_SetPriority(EINT1_IRQn,0); //alta prioridad
	NVIC_EnableIRQ(EINT1_IRQn);     //interrrupción habilitada

	NVIC_SetPriority(EINT2_IRQn,1);  //prioridad menor que ENT1
	NVIC_EnableIRQ(EINT2_IRQn);     //interrupción habilitada
}

void configSysTick(uint32_t tiempo){
	SysTick->LOAD = tiempo * (10000000/100)-1;    //interrumpe dependiento de la variable tiempo
	SysTick->VAL = 0;
	SysTick->CTRL = (1<<2)|(1<<1)|(1<<0); //habilita interrupcion por systick
}

void EINT1_IRQHandler(){
	LPC_SC->EXTINT |= (1<<1);
	configSysTick(25);                  //el systick interrumpe cada 25mS
}

void EINT2_IRQHandler(){
	LPC_SC->EXTINT |= (1<<2);
	configSysTick(60);                 //el systick interrumpe cada 60mS
}
int main(void) {

   configPIN();
   configEINT();


    while(1) {

    }
    return 0 ;
}
