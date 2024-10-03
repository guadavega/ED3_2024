/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint32_t dutty_cycle = 0;
volatile uint32_t contador = 0;
volatile uint8_t valor_salida = 1;

void configGPIO(){
	//pines p0.0 a p0.3
	LPC_PINCON->PINSEL0 &= ~(0xFF); //P0.0 a P0.3 como GPIO
	LPC_GPIO0->FIODIR &= ~(0xFF); //como entrada

	//p1.1
	LPC_PINCON->PINSEL2 &= ~(3<<2); //como GPIO
	LPC_GPIO1->FIODIR |= (1<<1); //como salida

}

void configGPIOINT(){
	LPC_GPIOINT->IO0IntEnF |= (0x0F); //interrupción por flanco de bajada
	LPC_GPIOINT->IO0IntEnR |= (0x0F); //interrupción por flanco de subida
}

void configSyTick(){
	SysTick->LOAD = (6666-1); //(66uSx(100M/1000)/(1mS))----->los 66 salen de 1/15 como menor divisor
	SysTick->VAL = 0;
	SysTick->CTRL = (1<<2)|(1<<1)|(1<<0);
}

void SysTick_Handler(){
	contador++;
	if(contador <= dutty_cycle){
	  valor_salida = 1;
	}else if(contador > dutty_cycle && contador <= 15){
		valor_salida = 0;
	}else if(contador > 15){
		valor_salida = 1;
		contador = 0;
	}
}

void EINT3_Handler(){
	if(LPC_GPIOINT->IO0IntStatF & (0x0F) != 0){
		dutty_cycle = (LPC_GPIO0->FIOPIN0 & (0x0F));  //enmascaramos los 4 bits menos significativos
	}
}
int main(void) {


    while(1) {
       LPC_GPIO1->FIOPIN = valor_salida<<1 ;
    }
    return 0 ;
}
