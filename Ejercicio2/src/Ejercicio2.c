/*
 * Utilizando interrupciones por GPIO realizar un código en C que permita, mediante 4 pines de
 * entrada GPIO, leer y guardar un número compuesto por 4 bits. Dicho número puede ser cambiado por
 * un usuario mediante 4 switches, los cuales cuentan con sus respectivas resistencias de pull up
 * externas. El almacenamiento debe realizarse en una variable del tipo array de forma tal que se
 * asegure tener disponible siempre los últimos 10 números elegidos por el usuario, garantizando
 * además que el número ingresado más antiguo, de este conjunto de 10, se encuentre en el elemento 9
 * y el número actual en el elemento 0 de dicho array. La interrupción por GPIO empezará teniendo la
 * máxima prioridad de interrupción posible y cada 200 números ingresados deberá disminuir en 1 su
 * prioridad hasta alcanzar la mínima posible. Llegado este momento, el programa deshabilitará todo
 * tipo de interrupciones producidas por las entradas GPIO.
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

volatile uint16_t prioridad_actual = 0;
volatile uint8_t numero_actual = 0;
volatile uint8_t numeros_guardados[10] = {0};
volatile uint32_t contador_numeros = 0;
volatile uint32_t i=0;


void configGPIO(){

	LPC_PINCON->PINSEL1 &= ~(0xFF); //los pines p0.0 a p0.3 como gpio
	LPC_GPIO0->FIODIR0 &= ~(0xF);   //los pines de p0.0 a p0.3 como entradas
	LPC_GPIOINT->IO0IntEnR |= (0xF); //interrupción por gpio por flanco de subida
}
void configNVIC(){
	 NVIC_SetPriority(EINT3_IRQn, prioridad_actual); //prioridad de interrupción
	 NVIC_EnableIRQ(EINT3_IRQn); //habilita interrupción
}

void EINT3_IRQHandler(){
	if((LPC_GPIOINT->IO0IntStatR & (0xF)) != 0) {
		numero_actual = (LPC_GPIO0->FIOPIN >> 0) & (0x0F);

		for (int i = 9; i > 0; i--) {             //el núm más antiguo se encuentra en el 9 lugar
		            numeros_guardados[i] = numeros_guardados[i-1];
		        }
		        numeros_guardados[0] = numero_actual;
	}
	contador_numeros++;

	if (contador_numeros % 200 == 0 && prioridad_actual < 31) {
            prioridad_actual++;
            NVIC_SetPriority(EINT3_IRQn, prioridad_actual);
        }

        if (prioridad_actual >= 31) {    //en la prioridad mín se deshabilita la interrupción
            NVIC_DisableIRQ(EINT3_IRQn);
        }
        LPC_GPIOINT->IO0IntClr |= (0x0F);

}
int main(void) {

    configGPIO();
    configNVIC();

    while(1) {

    }
    return 0 ;
}
