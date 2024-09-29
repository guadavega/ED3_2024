/*
 * Generar con timer0 una señal de freq. variable.
 * Usando el capture “medir” el periodo usando otro timer.
 * Prender leds tipo vúmetro según la frecuencia.
 * Con un pulsador cambiar la frecuencia de pasos de 100khz. Actualizar el vúmetro.
 *
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#endif

#define START_FREQUENCY 100000 //100KHz

volatile uint32_t value = 0;   //lo que mide el Timer1
volatile uint32_t current_frequency = START_FREQUENCY;
uint32_t step_frequency = 100000; //frecuencia de paso de 100khz
volatile uint32_t current_capture_value = 0;
volatile uint32_t previous_capture_value = 0;
volatile uint16_t time_period = 0;
volatile uint16_t measure = 0;
volatile uint16_t pins_on = 1;

void config_LEDS(){
	//LEDS
	LPC_PINCON->PINSEL0 &= ~(0xFFFFF); //p0.0 a p0.9 como GPIO
	LPC_GPIO0->FIODIR |= (0x3FF); //p0.0 a p0.9 como salida
	LPC_GPIO0->FIOCLR = (0x3FF); //arranca con los leds apagados

}

void config_Pulsador(){
	//PULSADOR
	LPC_PINCON->PINSEL4 |= (1 << 20);  // p2.10 como EINT0
	LPC_GPIO2->FIODIR &= ~(1 << 10);    // p2.10 como entrada
	LPC_GPIO2->FIOPIN |= (1 << 10);     // activa pull-up en P2.10
    LPC_GPIOINT->IO2IntEnR |= (1 << 10); // habilita interrupción por flanco ascendente en p2.10
    NVIC_EnableIRQ(EINT3_IRQn);         //habilita la interrupción

}
void config_TMR0(){
	 TIM_TIMERCFG_Type timCfg;
	 timCfg.PrescaleOption = TIM_PRESCALE_USVAL; //en microsegundos
	 timCfg.PrescaleValue = 1; //actualiza el TC cada 1uS

	  //MAT0.0
	  TIM_MATCHCFG_Type matchCfg;
	  matchCfg.MatchChannel = 0;
	  matchCfg.IntOnMatch = ENABLE;
	  matchCfg.StopOnMatch = DISABLE;
	  matchCfg.ResetOnMatch = ENABLE;
	  matchCfg.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	  matchCfg.MatchValue = 1/(START_FREQUENCY*2)*1000000; // 1/200KHz * 1MHz para que cuente 5 veces 1uS

	  TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timCfg);
	  TIM_ConfigMatch(LPC_TIM0, &matchCfg);
	  TIM_Cmd(LPC_TIM0, ENABLE);

}

void config_TMR1(){
	//tiene que estar con capture para medir la señal del timer0

	 TIM_CAPTURECFG_Type captureConfig;

	 //configura la captura en el canal 0
	 captureConfig.CaptureChannel = 0;
	 captureConfig.RisingEdge = ENABLE;     //captura flanco de subida
	 captureConfig.FallingEdge = DISABLE;
	 captureConfig.IntOnCaption = ENABLE;

	 TIM_ConfigCapture(LPC_TIM1, &captureConfig);
	 TIM_Cmd(LPC_TIM1, ENABLE);  // Inicia el Timer1
	 NVIC_EnableIRQ(TIMER1_IRQn);  // Habilita interrupción de Timer1
}

void EINT3_IRQHandler(void) {
    if (LPC_GPIOINT->IO2IntStatR & (1 << 10)){  //la interrupción fue en P2.10?
    	current_frequency += step_frequency;
    	if(current_frequency > 1000000){
    		current_frequency = START_FREQUENCY;
    	}
    	TIM_UpdateMatchValue(LPC_TIM0,0, 1/(current_frequency*2)*1000000 );
        LPC_GPIOINT->IO2IntClr |= (1 << 10); //limpia la bandera de interrupción
    }
}

void TIMER1_IRQHandler(void) {
	if(TIM_GetIntCaptureStatus(LPC_TIM1, TIM_CR0_INT) == SET){
		current_capture_value = TIM_GetCaptureValue(LPC_TIM1, TIM_COUNTER_INCAP0);
		time_period = current_capture_value - previous_capture_value;
		previous_capture_value = current_capture_value;
		measure = 1/time_period;
	}
}


int main(void) {
	config_LEDS();
	config_Pulsador();
	config_TMR0();
	config_TMR1();

    while(1) {
    	pins_on = measure/100000; //divide para obtener la cantidad de pines prendidos entre 1-10
    	LPC_GPIO0->FIOPIN = (0b1<<pins_on) - 1; //prende los leds dependiendo del valor de pins_on
    }
    return 0 ;
}
