/*
 * Configure el Systick Timer de modo que genere una forma de onda llamada PWM tal como la que
 * se muestra en la figura adjunta. Esta señal debe ser sacada por el pin P0.22 para que controle
 * la intensidad de brillo del led. El periodo de la señal debe ser de 10 mseg con un duty cycle
 * de 10%. Configure la interrupción externa EINT0 de modo que cada vez que se entre en una rutina
 * de interrupción externa el duty cycle incremente en un 10% (1 mseg). Esto se repite hasta llegar
 * al 100%, luego, si se entra nuevamente a la interrupción externa, el duty cycle volverá al 10%.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

//variables
volatile uint32_t dutty_cycle = 0;
volatile uint8_t  contador = 0;
volatile uint16_t valor_salida = 1;

void configGPIO(){

	LPC_PINCON->PINSEL1 &= ~(3<<12); //p0.22 como GPIO
	LPC_GPIO0->FIODIR |= (1<<22);   //como salida

}

void configEINT(){
	LPC_PINCON->PINSEL4 |= (1<<20); //pin p2.10 como EINT0
	LPC_SC->EXTMODE |= (1<<10); //por flanco
	LPC_SC->EXTPOLAR &= ~(1<<10); //por bajada
	NVIC_EnableIRQ(EINT0_IRQn); //habilita interrupción
	NVIC_SetPriority(EINT0_IRQn,2); //menor prioridad

}

void configSysTick(){
	SysTick->LOAD = (1000000/1000)-1; //cada 1mS
	SysTick->VAL = 0;
	SysTick->CTRL = (1<<2)|(1<<1)|(1<<0); //habilita interrupciones por systick

	NVIC_SetPriority(SysTick_IRQn,1); //mayor prioridad
}

void SysTickHandler(){
	contador++;

	if(contador <= dutty_cycle){
		valor_salida = 1;
	}else if (contador > dutty_cycle && contador < 10){
		valor_salida = 0;
	} else if (contador >= 10){
		contador = 0;
		valor_salida = 0;
	}
}
void EINT0_IRQHanlder(){
	dutty_cycle++;
		if(dutty_cycle>=10){  //aumenta un 10% cada vez que entra (1mS)
			dutty_cycle = 0;
		}
}

int main(void) {


    while(1) {

    }
    return 0 ;
}
