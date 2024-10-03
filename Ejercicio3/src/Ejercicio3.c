/*
 * Utilizando Systick e interrupciones externas escribir un código en C que cuente
 * indefinidamente de 0 a 9. Un pulsador conectado a Eint0 reiniciará la cuenta a 0 y se mantendrá
 * en ese valor mientras el pulsador se encuentre presionado. Un pulsador conectado a Eint1
 * permitirá detener o continuar la cuenta cada vez que sea presionado. Un pulsador conectado a
 * Eint2 permitirá modificar la velocidad de incremento del contador. En este sentido, cada vez
 * que se presione ese pulsador el contador pasará a incrementar su cuenta de cada 1 segundo a cada
 * 1 milisegundo y viceversa. Considerar que el microcontrolador se encuentra funcionando con un reloj
 *  (cclk) de 16 Mhz. El código debe estar debidamente comentado y los cálculos realizados claramente
 *  expresados.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint32_t numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D,
		0x07, 0x7F, 0x67};
volatile uint32_t cuenta=0;
volatile uint32_t contador=0;
volatile uint32_t velocidad=1;
volatile uint32_t i=0;








void config_GPIO(){

	LPC_PINCON->PINSEL0 &= ~(0x3FFF); //coloca un cero para seleccionar de p0.0 a p0.6 como GPIO

	LPC_GPIO2->FIODIR0 |= (0x7F); //seteamos en 1 para la salida del display

	LPC_GPIO2->FIOMASK0 |= (0x7F); //los pines no se ven afectados por lo que se escriba
}

void configSysTick(){

	SysTick->LOAD = velocidad*(16000000/1000)-1; //va a interrumpir cada 1mS para mostrar los números
	SysTick->VAL = 0;
	SysTick->CTRL = (1<<2)|(1<<1)|(1<<0); //habilita valor del core, interrupción y el systick

}

void SysTickHandler(){
	if(cuenta){
			contador++;
			if(contador > 9){    //cada 1mS se muestra un múnero y en total son 9 números
				contador = 0;
			}
		}
}

void configEINT(){
	//PINSEL

	//EINT0
	LPC_SC->EXTMODE &= ~(0b1<<0); //por nivel
	LPC_SC->EXTPOLAR |= (0b1<<0); //por alto

	//EINT1
	LPC_SC->EXTMODE |= (0b1<<1); //por flanco
	LPC_SC->EXTPOLAR &= ~(0b1<<1); //por bajo

	//EINT2
	LPC_SC->EXTMODE |= (0b1<<2); //por flanco
	LPC_SC->EXTPOLAR &= ~(0b1<<2); //por bajo

	NVIC_EnableIRQ(EINT0_IRQn); //habilita las interrupciones
	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);
}

void EINT0_IRQHandler(){
	contador = 0; //reinicia la cuenta
	LPC_SC->EXTINT |= (1<<0); //baja la bandera
}
void EINT1_IRQHandler(){
	cuenta =! cuenta; //cambia el estdo de la cuenta
	LPC_SC->EXTINT |= (1<<1); //baja la bandera
}
void EINT2_IRQHandler(){
	if(velocidad == 1000){
			velocidad = 1; //cambia a 1ms

		}else{
			velocidad = 1000; //sigue cada 1 segundo
		}
		configSysTick();
		LPC_SC->EXTINT |= (1<<2); //baja la bandera
}
int main(void) {

	config_GPIO();
	configSysTick();
	configEINT();

    while(1) {

    	LPC_GPIO2->FIOPIN = numDisplay[contador]; //se usa este porque ya tenemos una variable definida


    }
    return 0 ;
}
