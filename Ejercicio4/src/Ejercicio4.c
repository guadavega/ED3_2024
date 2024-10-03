/*
 * Algoritmo de antirrebote de un pulsador: Escribir un programa en C que ante la interrupción por
 * flanco de subida del pin P0.1, configurado como entrada digital con pull-down interno,
 * se incremente un contador de un dígito, se deshabilite esta interrupción y se permita la
 * interrupción por systick cada 20 milisegundos. En cada interrupción del systick se testeará
 * una vez el pin P0.1. Solo para el caso de haber testeado 3 estados altos seguidos se sacará
 * por los pines del puerto P2.0 al P2.7 el equivalente en ascii del valor del contador, se
 * desactivará las interrupción por systick y se habilitará nuevamente la interrupción por P0.1.
 * Por especificación de diseño se pide que los pines del puerto 2 que no sean utilizados deben
 * estar enmascarados por hardware. Considerar que el CPU se encuentra funcionando con el oscilador
 * interno RC (4Mhz).
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint32_t contador=0;
volatile uint8_t numASCII;
volatile uint32_t estado=0;

void configGPIO(){
	//P0.1
	LPC_PINCON->PINSEL0 &= ~(3<<2); //como GPIO
	LPC_GPIO0->FIODIR0 &= ~(1<<1);  //como entrada
	LPC_PINCON->PINMODE0 |= (2<<2); //pull-down habilitada

	//P2.0 a P2.7
	LPC_PINCON->PINSEL4 &= ~(0xFFFF); //como GPIO
	LPC_GPIO2->FIODIR0 |= (0xFF); //como salida
	LPC_GPIO2->FIOMASK &= ~(0xFF); //enmascarar los pines no usados del p2
}

void configEINT(){
	LPC_GPIOINT->IO0IntEnR |= (1<<1); //interrupción por flanco de subida
	NVIC_EnableIRQ(EINT3_IRQn);       //habilita la interrupción
}

void configSysTick(){
	SysTick->LOAD = (4000000/200)-1;  //interrumpe cada 20mS
	SysTick->VAL = 0;
	SysTick->CTRL = (1<<2)|(1<<1)|(1<<0);
}

void EINT3_IRQHandler(){

	if(LPC_GPIOINT->IO0IntStatR & (1<<1)){
		contador = contador + 1 % 10; //cuenta de 0 a 9
		numASCII = contador + '0';      //convierte el contador a ASCII

		LPC_GPIOINT->IO0IntClr = (1 << 1); //limpia la flag de interrupción
	    NVIC_DisableIRQ(EINT3_IRQn); //deshabilita interrupción
	    configSysTick(); //habilita interrupción por systick
	}

}

void SysTickHandler(){
	if(LPC_GPIOINT->IO0IntStatR & (1<<1)){
		estado++;
	}else{
		estado = 0;
	}
	if(estado>=3){
		LPC_GPIO2->FIOPIN = (numASCII << 0);
		SysTick->CTRL &= ~(1 << 0);   //deshabilita SysTick
		NVIC_EnableIRQ(EINT3_IRQn);   //rehabilita interrupción en P0.1
	    estado = 0;
	}
}
int main(void) {

	configGPIO();
	configEINT();


    while(1) {

    }
    return 0 ;
}
