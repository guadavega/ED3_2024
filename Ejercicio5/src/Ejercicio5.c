/*
 * Utilizando SysTick, programar el microcontrolador para que cada vez que se produzca una interrupción
 * por flanco ascendente en el pin p2.4 se saque por el pin p2.5 la secuencia mostrada en la figura. En
 * caso de que se produzca una nueva interrupción por p2.4 mientras se esta realizando la secuencia
 * esta volverá a realizarse desde su inicio descartando la que se venia sacando por el pin. El programa
 * NO debe hacer uso de retardos por software y deben enmascararse los pines del puerto 2 que no vayan
 * a utilizarse. Suponer una frecuencia del cclk de 80MHz (NO se pide configuración del reloj).
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint8_t estado = 0;
volatile uint8_t reiniciar = 0;
volatile uint32_t tiempo[]=(800000,1600000,3200000);

void configGPIO(){
	//P2.4
	LPC_PINCON->PINSEL4 &= ~(3<<8);   //como GPIO
	LPC_GPIO2->FIODIR0 &= ~(1<<4);    //pin 4 como entrada
	//P2.5
	LPC_PINCON->PINSEL4 &= ~(3<<10);  //como GPIO
	LPC_GPIO2->FIODIR0 |= (1<<5);     //pin 5 como salida

	LPC_GPIO2->FIOMASK0 &= ~(1<<4);
	LPC_GPIO2->FIOMASK0 &= ~(1<<5);  //se enmascaran los pines no utilizados del P2
}

void configGPIOINT(){
	LPC_GPIOINT->IO2IntEnR |= (1<<4);  //interrupción por flanco de subida en p2.4
	NVIC_EnableIRQ(EINT3_IRQn); //habilita interrupción
	NVIC_SetPriority(EINT3_IRQn,1); //prioridad para la interrupción por p2.4
}

void configSysTick(uint32_t recarga){
	SysTick->LOAD = recarga-1;
    SysTick->VAL = 0;
	SysTick->CTRL = (1<<2)|(1<<1)|(1<<0);
}

void EINT3_IRQHandler(){
	if(LPC_GPIOINT->IO0IntStatR & (1<<1)){  //pregunta si interrumpe por p2.4
		reiniciar = 1;           //se debe reiniciar
		estado = 0;             //reiniciar la secuencia
		configSysTick(tiempo[0]); //la onda de salida es de 10mS
		LPC_GPIO2->FIOCLR |= (1<<5);      //inicia la secuencia
		LPC_GPIOINT->IO2IntClr |= (1<<4); //baja la bandera de interrupción
	}
}

void SysTickHandler(){
	if(reiniciar == 1){
		reiniciar = 0;
		estado = 0;
		configSysTick(tiempo[]);
		LPC_GPIO2->FIOCLR |= (1<<5);
	}
	switch (estado) {
	        case 0:
	            LPC_GPIO2->FIOSET = (1 << 5); //P2.5 con estado alto por 10 ms
	            configSysTick(tiempo[0]);    //se espera 10 ms
	            estado++;
	            break;
	        case 1:
	            LPC_GPIO2->FIOCLR = (1 << 5); //si baja P2.5. con estado bajo por 20 ms
	            configSysTick(tiempos[1]);    //se espera 20 ms
	            estado++;
	            break;
	        case 2:
	            LPC_GPIO2->FIOSET = (1 << 5); //sube P2.5 con estado alto por 40 ms
	            configSysTick(tiempo[2]);    //se espera 40 ms
	            estado++;
	            break;
	        case 3:
	            LPC_GPIO2->FIOCLR = (1 << 5); //baja P2.5 con estado bajo por 20 ms
	            configSysTick(tiempo[1]);    //se espera 20 ms
	            estado = 0;          //reinicia la secuencia
	            break;

	       }
}
int main(void) {

   configGPIO();
   configGPIOINT();

    while(1) {

    }
    return 0 ;
}
